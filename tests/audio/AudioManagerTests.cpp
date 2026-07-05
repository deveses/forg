#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <memory>
#include <vector>

#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioGenerator.h"
#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioMixer.h"
#include "forg/audio/IAudioSource.h"

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

    std::shared_ptr<forg::audio::AudioGenerator> tone0 =
        std::make_shared<forg::audio::AudioGenerator>();
    std::shared_ptr<forg::audio::AudioGenerator> tone1 =
        std::make_shared<forg::audio::AudioGenerator>();

    const int voice0 = manager.Play(tone0, true);
    const int voice1 = manager.Play(tone1, true);

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

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    REQUIRE(manager.Play(tone) == forg::audio::AudioManager::INVALID_VOICE);

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

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    std::vector<int> voices;

    for (unsigned int i = 0; i < forg::audio::AudioMixer::MAX_STREAMS; i++)
    {
        const int voice = manager.Play(tone, true);
        REQUIRE(voice != forg::audio::AudioManager::INVALID_VOICE);
        voices.push_back(voice);
    }

    REQUIRE(manager.Play(tone, true) ==
            forg::audio::AudioManager::INVALID_VOICE);

    manager.Stop(voices[3]);
    REQUIRE(manager.Play(tone, true) == voices[3]);
}

TEST_CASE("AudioManager reclaims finished voices on Update", "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    std::shared_ptr<FiniteAudioSource> oneShot =
        std::make_shared<FiniteAudioSource>();
    const int voice = manager.Play(oneShot);
    REQUIRE(voice != forg::audio::AudioManager::INVALID_VOICE);
    REQUIRE(manager.IsPlaying(voice));

    manager.Update(); // drains the source, stream turns off, voice reclaimed
    REQUIRE_FALSE(manager.IsPlaying(voice));

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    REQUIRE(manager.Play(tone, true) == voice);
}

TEST_CASE("AudioManager keeps a playing shared source alive",
          "[audio][manager]")
{
    NullAudioOutput output;
    output.canWrite = false;

    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    std::shared_ptr<FiniteAudioSource> oneShot =
        std::make_shared<FiniteAudioSource>();
    std::weak_ptr<FiniteAudioSource> weak = oneShot;

    const int voice = manager.Play(oneShot);
    REQUIRE(voice != forg::audio::AudioManager::INVALID_VOICE);

    oneShot.reset();
    REQUIRE_FALSE(weak.expired());

    manager.Stop(voice);
    REQUIRE(weak.expired());
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
