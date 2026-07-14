#include <catch2/catch_test_macros.hpp>

#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioGenerator.h"
#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioMixer.h"
#include "forg/audio/AudioMixerProcessor.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/audio/SoundInstanceProcessorChain.h"

#include <memory>
#include <string>
#include <vector>

namespace {

using forg::audio::AudioManager;
using forg::audio::AudioMixer;
using forg::audio::AudioMixerProcessor;
using forg::audio::ProcessedSoundInstance;
using forg::audio::SoundInstanceDescription;
using forg::audio::SoundInstanceLifecyclePhase;
using forg::audio::SoundInstanceProcessor;
using forg::audio::SoundInstanceProcessorChain;
using forg::audio::SoundInstanceState;
using forg::audio::SoundProcessingResult;
using forg::audio::SoundProcessorContext;

class NullAudioOutput : public forg::audio::IAudioOutput
{
  public:
    bool CanWrite() override { return false; }
    void Write(char*, unsigned int) override {}
    bool Init() override { return true; }
    void Release() override {}
};

class RecordingProcessor : public SoundInstanceProcessor
{
  public:
    RecordingProcessor(std::string processorName, std::vector<std::string>& log)
        : name(std::move(processorName)), events(log)
    {
    }

    SoundProcessingResult createResult = SoundProcessingResult::Continue;
    SoundProcessingResult playResult = SoundProcessingResult::Continue;
    SoundProcessingResult updateResult = SoundProcessingResult::Continue;
    SoundProcessingResult stopResult = SoundProcessingResult::Continue;
    SoundProcessingResult pauseResult = SoundProcessingResult::Continue;
    SoundProcessingResult resumeResult = SoundProcessingResult::Continue;
    SoundProcessingResult volumeResult = SoundProcessingResult::Continue;

    SoundProcessingResult OnCreate(ProcessedSoundInstance&,
                                   SoundProcessorContext&) override
    {
        events.push_back(name + ":create");
        return createResult;
    }

    SoundProcessingResult OnPlay(ProcessedSoundInstance&,
                                 SoundProcessorContext&) override
    {
        events.push_back(name + ":play");
        return playResult;
    }

    SoundProcessingResult OnUpdate(ProcessedSoundInstance&,
                                   SoundProcessorContext&) override
    {
        events.push_back(name + ":update");
        return updateResult;
    }

    SoundProcessingResult OnStop(ProcessedSoundInstance&,
                                 SoundProcessorContext&) override
    {
        events.push_back(name + ":stop");
        return stopResult;
    }

    SoundProcessingResult OnPause(ProcessedSoundInstance&,
                                  SoundProcessorContext&) override
    {
        events.push_back(name + ":pause");
        return pauseResult;
    }

    SoundProcessingResult OnResume(ProcessedSoundInstance&,
                                   SoundProcessorContext&) override
    {
        events.push_back(name + ":resume");
        return resumeResult;
    }

    SoundProcessingResult OnVolumeChanged(ProcessedSoundInstance&,
                                          SoundProcessorContext&) override
    {
        events.push_back(name + ":volume");
        return volumeResult;
    }

    void OnDestroy(ProcessedSoundInstance&, SoundProcessorContext&) override
    {
        events.push_back(name + ":destroy");
    }

  private:
    std::string name;
    std::vector<std::string>& events;
};

class ContextProcessor : public SoundInstanceProcessor
{
  public:
    ContextProcessor(int contextValue, std::vector<int>& values)
        : value(contextValue), observed(values)
    {
    }

    SoundProcessingResult OnCreate(ProcessedSoundInstance&,
                                   SoundProcessorContext& context) override
    {
        context.Emplace<int>(value);
        return SoundProcessingResult::Continue;
    }

    SoundProcessingResult OnPlay(ProcessedSoundInstance&,
                                 SoundProcessorContext& context) override
    {
        int* stored = context.Get<int>();
        observed.push_back(stored != nullptr ? *stored : -1);
        return SoundProcessingResult::Continue;
    }

  private:
    int value;
    std::vector<int>& observed;
};

class OneShotWaitProcessor : public SoundInstanceProcessor
{
  public:
    explicit OneShotWaitProcessor(std::vector<std::string>& log) : events(log)
    {
    }

    SoundProcessingResult OnPlay(ProcessedSoundInstance&,
                                 SoundProcessorContext&) override
    {
        if (!waited)
        {
            waited = true;
            events.push_back("wait:play-wait");
            return SoundProcessingResult::Wait;
        }

        events.push_back("wait:play-continue");
        return SoundProcessingResult::Continue;
    }

    SoundProcessingResult OnUpdate(ProcessedSoundInstance&,
                                   SoundProcessorContext&) override
    {
        events.push_back("wait:update");
        return SoundProcessingResult::Continue;
    }

  private:
    bool waited = false;
    std::vector<std::string>& events;
};

std::shared_ptr<SoundInstanceProcessorChain> MakeChain()
{
    return std::make_shared<SoundInstanceProcessorChain>();
}

SoundInstanceDescription MakeMixerDescription(bool looping = false)
{
    SoundInstanceDescription description;
    description.source = std::make_shared<forg::audio::AudioGenerator>();
    description.looping = looping;
    return description;
}

} // namespace

TEST_CASE("ProcessedSoundInstance runs processors in lifecycle order",
          "[audio][soundinstance]")
{
    std::vector<std::string> events;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("a", events)));
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("b", events)));

    {
        ProcessedSoundInstance instance(chain);
        REQUIRE(instance.State() == SoundInstanceState::Pending);
        REQUIRE(instance.Id() == forg::audio::INVALID_SOUND_INSTANCE_ID);
        REQUIRE(events.empty());
        REQUIRE_FALSE(instance.Play());
        REQUIRE(instance.Create(42));
        REQUIRE(instance.Id() == 42);
        REQUIRE(instance.Play());
        instance.Update();
    }

    REQUIRE(events == std::vector<std::string>{"a:create", "b:create", "a:play",
                                               "b:play", "a:update", "b:update",
                                               "a:destroy", "b:destroy"});
}

TEST_CASE("SoundProcessorContext stores isolated typed processor data",
          "[audio][soundinstance]")
{
    std::vector<int> observed;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    REQUIRE(
        chain->AddProcessor(std::make_shared<ContextProcessor>(11, observed)));
    REQUIRE(
        chain->AddProcessor(std::make_shared<ContextProcessor>(22, observed)));

    ProcessedSoundInstance instance(chain);
    REQUIRE(instance.ProcessorContextCount() == 0);
    REQUIRE(instance.Create());
    REQUIRE(instance.ProcessorContextCount() == 2);
    REQUIRE(instance.ProcessorContext(0) != instance.ProcessorContext(1));

    REQUIRE(instance.Play());
    REQUIRE(observed == std::vector<int>{11, 22});
}

TEST_CASE("SoundInstanceProcessorChain enforces its static processor limit",
          "[audio][soundinstance]")
{
    std::vector<std::string> events;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    std::shared_ptr<RecordingProcessor> processor =
        std::make_shared<RecordingProcessor>("processor", events);

    for (std::size_t i = 0; i < SoundInstanceProcessorChain::MAX_PROCESSORS;
         ++i)
    {
        REQUIRE(chain->AddProcessor(processor));
    }

    REQUIRE_FALSE(chain->AddProcessor(processor));
    REQUIRE(chain->Count() == SoundInstanceProcessorChain::MAX_PROCESSORS);

    ProcessedSoundInstance instance(chain);
    REQUIRE(instance.Create());
    REQUIRE(instance.ProcessorContextCount() ==
            SoundInstanceProcessorChain::MAX_PROCESSORS);
    REQUIRE(instance.ProcessorContext(
                SoundInstanceProcessorChain::MAX_PROCESSORS - 1) != nullptr);
    REQUIRE(instance.ProcessorContext(
                SoundInstanceProcessorChain::MAX_PROCESSORS) == nullptr);
}

TEST_CASE("Bypassed processors skip lifecycle callbacks except destroy",
          "[audio][soundinstance]")
{
    std::vector<std::string> events;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    std::shared_ptr<RecordingProcessor> bypassing =
        std::make_shared<RecordingProcessor>("a", events);
    bypassing->playResult = SoundProcessingResult::Bypass;

    REQUIRE(chain->AddProcessor(bypassing));
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("b", events)));

    {
        ProcessedSoundInstance instance(chain);
        REQUIRE(instance.Create());
        REQUIRE(instance.Play());
        instance.Update();
        instance.Stop();
    }

    REQUIRE(events == std::vector<std::string>{"a:create", "b:create", "a:play",
                                               "b:play", "b:update", "b:stop",
                                               "a:destroy", "b:destroy"});
}

TEST_CASE("Pending play resumes from the waiting processor on update",
          "[audio][soundinstance]")
{
    std::vector<std::string> events;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("a", events)));
    REQUIRE(
        chain->AddProcessor(std::make_shared<OneShotWaitProcessor>(events)));
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("c", events)));

    ProcessedSoundInstance instance(chain);
    REQUIRE(instance.Create());
    REQUIRE_FALSE(instance.Play());
    REQUIRE(instance.HasPendingWork());
    REQUIRE(instance.PendingPhase() == SoundInstanceLifecyclePhase::Play);
    REQUIRE(instance.PendingProcessorIndex() == 1);

    instance.Update();

    REQUIRE_FALSE(instance.HasPendingWork());
    REQUIRE(instance.State() == SoundInstanceState::Playing);
    REQUIRE(events == std::vector<std::string>{
                          "a:create", "c:create", "a:play", "wait:play-wait",
                          "wait:play-continue", "c:play", "a:update",
                          "wait:update", "c:update"});
}

TEST_CASE("Terminal processor results transition and destroy the instance",
          "[audio][soundinstance]")
{
    SECTION("stop")
    {
        std::vector<std::string> events;
        std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
        std::shared_ptr<RecordingProcessor> processor =
            std::make_shared<RecordingProcessor>("a", events);
        processor->updateResult = SoundProcessingResult::Stop;
        REQUIRE(chain->AddProcessor(processor));

        ProcessedSoundInstance instance(chain);
        REQUIRE(instance.Create());
        REQUIRE(instance.Play());
        instance.Update();

        REQUIRE(instance.IsStopped());
        REQUIRE(events == std::vector<std::string>{"a:create", "a:play",
                                                   "a:update", "a:destroy"});
    }

    SECTION("fail")
    {
        std::vector<std::string> events;
        std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
        std::shared_ptr<RecordingProcessor> processor =
            std::make_shared<RecordingProcessor>("a", events);
        processor->updateResult = SoundProcessingResult::Fail;
        REQUIRE(chain->AddProcessor(processor));

        ProcessedSoundInstance instance(chain);
        REQUIRE(instance.Create());
        REQUIRE(instance.Play());
        instance.Update();

        REQUIRE(instance.IsFailed());
        REQUIRE(events == std::vector<std::string>{"a:create", "a:play",
                                                   "a:update", "a:destroy"});
    }

    SECTION("complete")
    {
        std::vector<std::string> events;
        std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
        std::shared_ptr<RecordingProcessor> processor =
            std::make_shared<RecordingProcessor>("a", events);
        processor->updateResult = SoundProcessingResult::Complete;
        REQUIRE(chain->AddProcessor(processor));

        ProcessedSoundInstance instance(chain);
        REQUIRE(instance.Create());
        REQUIRE(instance.Play());
        instance.Update();

        REQUIRE(instance.IsComplete());
        REQUIRE(events == std::vector<std::string>{"a:create", "a:play",
                                                   "a:update", "a:destroy"});
    }
}

TEST_CASE("Deferred commands are drained in update order",
          "[audio][soundinstance]")
{
    std::vector<std::string> events;
    std::shared_ptr<SoundInstanceProcessorChain> chain = MakeChain();
    REQUIRE(
        chain->AddProcessor(std::make_shared<RecordingProcessor>("a", events)));

    ProcessedSoundInstance instance(chain);
    REQUIRE(instance.Create());
    REQUIRE(instance.Play());

    REQUIRE(instance.RequestPause());
    REQUIRE(instance.RequestVolumeMultiplier(0.25f));
    REQUIRE(instance.RequestResume());
    instance.Update();

    REQUIRE(instance.State() == SoundInstanceState::Playing);
    REQUIRE(instance.Parameters().volumeMultiplier == 0.25f);
    REQUIRE(events == std::vector<std::string>{"a:create", "a:play", "a:pause",
                                               "a:volume", "a:resume",
                                               "a:update"});
}

TEST_CASE("Deferred command queue has a fixed reported capacity",
          "[audio][soundinstance]")
{
    ProcessedSoundInstance instance(nullptr);
    REQUIRE(instance.Create());
    REQUIRE(instance.Play());

    for (std::size_t i = 0; i < ProcessedSoundInstance::MAX_QUEUED_COMMANDS;
         ++i)
    {
        REQUIRE(instance.RequestVolumeMultiplier(static_cast<float>(i)));
    }

    REQUIRE_FALSE(instance.RequestStop());
    instance.Update();

    REQUIRE(
        instance.Parameters().volumeMultiplier ==
        static_cast<float>(ProcessedSoundInstance::MAX_QUEUED_COMMANDS - 1));
}

TEST_CASE("Sound instances expose default object routing identity",
          "[audio][soundinstance]")
{
    int sender = 1;
    int emitter = 2;
    ProcessedSoundInstance instance(nullptr);

    instance.SetSenderObject(&sender);
    instance.SetEmitterObject(&emitter);

    REQUIRE(instance.SenderObject() == &sender);
    REQUIRE(instance.EmitterObject() == &emitter);
}

TEST_CASE("AudioMixerProcessor acquires isolated voices during play",
          "[audio][soundinstance][mixerprocessor]")
{
    NullAudioOutput output;
    AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));
    AudioMixer& mixer = manager.Mixer();

    std::shared_ptr<SoundInstanceProcessorChain> chain =
        manager.ProcessorChain();
    AudioMixerProcessor& processor = manager.MixerProcessor();
    REQUIRE(chain->Count() == 1);
    REQUIRE(chain->Processor(0) == &processor);

    ProcessedSoundInstance first(chain, MakeMixerDescription());
    REQUIRE(processor.VoiceId(first) == AudioMixer::INVALID_VOICE);
    REQUIRE(first.Create());
    REQUIRE(processor.VoiceId(first) == AudioMixer::INVALID_VOICE);
    REQUIRE(first.Play());
    REQUIRE(processor.VoiceId(first) == 0);
    REQUIRE(mixer.IsVoiceAcquired(0));

    {
        ProcessedSoundInstance second(chain, MakeMixerDescription());
        REQUIRE(second.Create());
        REQUIRE(second.Play());
        REQUIRE(processor.VoiceId(second) == 1);
        REQUIRE(mixer.IsVoiceAcquired(1));
    }

    REQUIRE_FALSE(mixer.IsVoiceAcquired(1));
    first.Stop();
    REQUIRE(processor.VoiceId(first) == AudioMixer::INVALID_VOICE);
    REQUIRE_FALSE(mixer.IsVoiceAcquired(0));
}

TEST_CASE("AudioMixerProcessor requires a source during instance creation",
          "[audio][soundinstance][mixerprocessor]")
{
    NullAudioOutput output;
    AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    ProcessedSoundInstance instance(manager.ProcessorChain());

    REQUIRE_FALSE(instance.Create());
    REQUIRE(instance.IsFailed());
    REQUIRE(manager.MixerProcessor().VoiceId(instance) ==
            AudioMixer::INVALID_VOICE);
}

TEST_CASE("AudioMixerProcessor waits until a voice becomes available",
          "[audio][soundinstance][mixerprocessor]")
{
    NullAudioOutput output;
    AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));
    AudioMixer& mixer = manager.Mixer();

    std::vector<int> reserved;
    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
        reserved.push_back(mixer.AcquireVoice());

    std::shared_ptr<SoundInstanceProcessorChain> chain =
        manager.ProcessorChain();
    AudioMixerProcessor& processor = manager.MixerProcessor();

    ProcessedSoundInstance instance(chain, MakeMixerDescription());
    REQUIRE(instance.Create());
    REQUIRE_FALSE(instance.Play());
    REQUIRE(instance.HasPendingWork());
    REQUIRE(instance.PendingPhase() == SoundInstanceLifecyclePhase::Play);
    REQUIRE(processor.VoiceId(instance) == AudioMixer::INVALID_VOICE);

    mixer.ReleaseVoice(reserved[4]);
    instance.Update();

    REQUIRE(instance.State() == SoundInstanceState::Playing);
    REQUIRE(processor.VoiceId(instance) == reserved[4]);

    instance.Stop();
    for (int voice : reserved)
        mixer.ReleaseVoice(voice);
}

TEST_CASE("AudioMixerProcessor fails play with an uninitialized mixer",
          "[audio][soundinstance][mixerprocessor]")
{
    AudioManager manager;
    std::shared_ptr<SoundInstanceProcessorChain> chain =
        manager.ProcessorChain();
    AudioMixerProcessor& processor = manager.MixerProcessor();

    ProcessedSoundInstance instance(chain, MakeMixerDescription());
    REQUIRE(instance.Create());
    REQUIRE_FALSE(instance.Play());
    REQUIRE(instance.IsFailed());
    REQUIRE(processor.VoiceId(instance) == AudioMixer::INVALID_VOICE);
}
