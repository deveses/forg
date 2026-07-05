#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <vector>

#include "forg/audio/AudioGenerator.h"
#include "forg/audio/AudioManager.h"

namespace {

class NullAudioOutput : public forg::audio::IAudioOutput
{
  public:
    bool canWrite = true;
    bool released = false;

    bool CanWrite() override { return canWrite; }
    void Write(char*, unsigned int) override {}
    bool Init() override { return true; }
    void Release() override { released = true; }
};

class FiniteAudioSource : public forg::audio::IAudioSource
{
  public:
    unsigned int frames = 4;
    unsigned int position = 0;

    unsigned int Read(short* out, unsigned int count) override
    {
        const unsigned int left = frames - position;
        if (count > left)
            count = left;
        std::memset(out, 0, count * sizeof(short));
        position += count;
        return count;
    }

    int Channels() const override { return 1; }
    bool IsFinished() const override { return position >= frames; }
    void Reset() override { position = 0; }
};

} // namespace

TEST_CASE("AudioManager plays sources on separate voices", "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::audio::AudioGenerator tone0;
    forg::audio::AudioGenerator tone1;

    const int voice0 = manager.Play(&tone0, true);
    const int voice1 = manager.Play(&tone1, true);

    REQUIRE(voice0 != forg::audio::AudioManager::INVALID_VOICE);
    REQUIRE(voice1 != forg::audio::AudioManager::INVALID_VOICE);
    REQUIRE(voice0 != voice1);
    REQUIRE(manager.IsPlaying(voice0));
    REQUIRE(manager.IsPlaying(voice1));

    manager.Stop(voice0);
    REQUIRE_FALSE(manager.IsPlaying(voice0));
    REQUIRE(manager.IsPlaying(voice1));

    manager.StopAll();
    REQUIRE_FALSE(manager.IsPlaying(voice1));
}

TEST_CASE("AudioManager rejects invalid play requests", "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;

    forg::audio::AudioGenerator tone;
    REQUIRE(manager.Play(&tone) == forg::audio::AudioManager::INVALID_VOICE);

    REQUIRE(manager.InitWithOutput(&output));
    REQUIRE(manager.Play(nullptr) == forg::audio::AudioManager::INVALID_VOICE);
    REQUIRE_FALSE(manager.IsPlaying(-1));
    REQUIRE_FALSE(manager.IsPlaying(1000));
}

TEST_CASE("AudioManager runs out of voices at the mixer stream limit",
          "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::audio::AudioGenerator tone;
    std::vector<int> voices;

    for (unsigned int i = 0; i < forg::audio::AudioMixer::MAX_STREAMS; i++)
    {
        const int voice = manager.Play(&tone, true);
        REQUIRE(voice != forg::audio::AudioManager::INVALID_VOICE);
        voices.push_back(voice);
    }

    REQUIRE(manager.Play(&tone, true) ==
            forg::audio::AudioManager::INVALID_VOICE);

    manager.Stop(voices[3]);
    REQUIRE(manager.Play(&tone, true) == voices[3]);
}

TEST_CASE("AudioManager reclaims finished voices on Update",
          "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    FiniteAudioSource oneShot;
    const int voice = manager.Play(&oneShot);
    REQUIRE(voice != forg::audio::AudioManager::INVALID_VOICE);
    REQUIRE(manager.IsPlaying(voice));

    manager.Update(); // drains the source, stream turns off, voice reclaimed
    REQUIRE_FALSE(manager.IsPlaying(voice));

    forg::audio::AudioGenerator tone;
    REQUIRE(manager.Play(&tone, true) == voice);
}

TEST_CASE("AudioManager releases an output offered while already initialized",
          "[audio][manager]")
{
    NullAudioOutput first;
    NullAudioOutput second;
    forg::audio::AudioManager manager;

    REQUIRE(manager.InitWithOutput(&first));
    REQUIRE(manager.InitWithOutput(&second));

    REQUIRE_FALSE(first.released);
    REQUIRE(second.released);

    manager.Shutdown();
    REQUIRE(first.released);
}
