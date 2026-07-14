#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <memory>
#include <vector>

#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioGenerator.h"
#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioMixer.h"
#include "forg/audio/IAudioSource.h"
#include "forg/audio/SoundInstanceProcessorChain.h"

#include <string>

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

class RecordingSoundProcessor : public forg::audio::SoundInstanceProcessor
{
  public:
    explicit RecordingSoundProcessor(std::vector<std::string>& log)
        : events(log)
    {
    }

    forg::audio::SoundProcessingResult
    OnCreate(forg::audio::ProcessedSoundInstance&,
             forg::audio::SoundProcessorContext&) override
    {
        events.push_back("create");
        return forg::audio::SoundProcessingResult::Continue;
    }

    forg::audio::SoundProcessingResult
    OnPlay(forg::audio::ProcessedSoundInstance&,
           forg::audio::SoundProcessorContext&) override
    {
        events.push_back("play");
        return forg::audio::SoundProcessingResult::Continue;
    }

    forg::audio::SoundProcessingResult
    OnUpdate(forg::audio::ProcessedSoundInstance&,
             forg::audio::SoundProcessorContext&) override
    {
        events.push_back("update");
        return forg::audio::SoundProcessingResult::Continue;
    }

    forg::audio::SoundProcessingResult
    OnStop(forg::audio::ProcessedSoundInstance&,
           forg::audio::SoundProcessorContext&) override
    {
        events.push_back("stop");
        return forg::audio::SoundProcessingResult::Continue;
    }

    forg::audio::SoundProcessingResult
    OnVolumeChanged(forg::audio::ProcessedSoundInstance&,
                    forg::audio::SoundProcessorContext&) override
    {
        events.push_back("volume");
        return forg::audio::SoundProcessingResult::Continue;
    }

    void OnDestroy(forg::audio::ProcessedSoundInstance&,
                   forg::audio::SoundProcessorContext&) override
    {
        events.push_back("destroy");
    }

  private:
    std::vector<std::string>& events;
};

} // namespace

TEST_CASE("AudioManager plays sources as separate sound instances",
          "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    std::shared_ptr<forg::audio::AudioGenerator> tone0 =
        std::make_shared<forg::audio::AudioGenerator>();
    std::shared_ptr<forg::audio::AudioGenerator> tone1 =
        std::make_shared<forg::audio::AudioGenerator>();

    const forg::audio::SoundInstanceId id0 = manager.Play(tone0, true);
    const forg::audio::SoundInstanceId id1 = manager.Play(tone1, true);

    REQUIRE(id0 != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(id1 != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(id0 != id1);
    REQUIRE(manager.IsPlaying(id0));
    REQUIRE(manager.IsPlaying(id1));

    REQUIRE(manager.Stop(id0));
    REQUIRE(manager.IsPlaying(id0));
    manager.Update();
    REQUIRE_FALSE(manager.IsPlaying(id0));
    REQUIRE(manager.IsPlaying(id1));

    REQUIRE(manager.StopAll());
    REQUIRE(manager.IsPlaying(id1));
    manager.Update();
    REQUIRE_FALSE(manager.IsPlaying(id1));
}

TEST_CASE("AudioManager rejects invalid play requests", "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    REQUIRE(manager.Play(tone) == forg::audio::INVALID_SOUND_INSTANCE_ID);

    REQUIRE(manager.InitWithOutput(&output));
    REQUIRE(manager.Play(nullptr) == forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE_FALSE(manager.IsPlaying(forg::audio::INVALID_SOUND_INSTANCE_ID));
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
    std::vector<forg::audio::SoundInstanceId> ids;

    for (unsigned int i = 0; i < forg::audio::AudioMixer::MAX_STREAMS; i++)
    {
        const forg::audio::SoundInstanceId id = manager.Play(tone, true);
        REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);
        ids.push_back(id);
    }

    REQUIRE(manager.Play(tone, true) == forg::audio::INVALID_SOUND_INSTANCE_ID);

    REQUIRE(manager.Stop(ids[3]));
    REQUIRE(manager.Play(tone, true) == forg::audio::INVALID_SOUND_INSTANCE_ID);
    manager.Update();
    const forg::audio::SoundInstanceId replacement = manager.Play(tone, true);
    REQUIRE(replacement != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(replacement != ids[3]);
}

TEST_CASE("AudioManager reclaims finished sound instances on Update",
          "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    std::shared_ptr<FiniteAudioSource> oneShot =
        std::make_shared<FiniteAudioSource>();
    const forg::audio::SoundInstanceId id = manager.Play(oneShot);
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(manager.IsPlaying(id));

    manager.Update();
    REQUIRE_FALSE(manager.IsPlaying(id));

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    const forg::audio::SoundInstanceId replacement = manager.Play(tone, true);
    REQUIRE(replacement != forg::audio::INVALID_SOUND_INSTANCE_ID);
    REQUIRE(replacement != id);
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

    const forg::audio::SoundInstanceId id = manager.Play(oneShot);
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);

    oneShot.reset();
    REQUIRE_FALSE(weak.expired());

    REQUIRE(manager.Stop(id));
    REQUIRE_FALSE(weak.expired());
    manager.Update();
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

TEST_CASE("AudioManager skips mixer voices reserved by another owner",
          "[audio][manager]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    const int reserved = manager.Mixer().AcquireVoice();
    REQUIRE(reserved == 0);

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    const forg::audio::SoundInstanceId id = manager.Play(tone, true);
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);

    REQUIRE(manager.Stop(id));
    manager.Update();
    manager.Mixer().ReleaseVoice(reserved);
}

TEST_CASE("AudioManager drives its processor chain through sound instances",
          "[audio][manager]")
{
    NullAudioOutput output;
    output.canWrite = false;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    std::vector<std::string> events;
    std::shared_ptr<forg::audio::SoundInstanceProcessorChain> chain =
        manager.ProcessorChain();
    REQUIRE(chain->Count() == 1);
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingSoundProcessor>(events)));

    std::shared_ptr<forg::audio::AudioGenerator> tone =
        std::make_shared<forg::audio::AudioGenerator>();
    const forg::audio::SoundInstanceId id = manager.Play(tone, true);
    REQUIRE(id != forg::audio::INVALID_SOUND_INSTANCE_ID);

    manager.SetGainPan(id, 0.5f, -0.25f);
    manager.Update();
    REQUIRE(manager.Stop(id));
    manager.Update();

    REQUIRE(events == std::vector<std::string>{"create", "play", "volume",
                                               "update", "stop", "destroy"});
}
