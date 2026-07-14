#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioMixer.h"
#include "forg/audio/AudioMixerProcessor.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/audio/SoundInstanceManager.h"
#include "forg/audio/SoundInstanceProcessorChain.h"
#include "forg_pch.h"

#include <memory>
#include <utility>

namespace forg::audio {

struct AudioManager::Impl
{
    AudioMixer mixer;
    bool initialized = false;
    std::shared_ptr<SoundInstanceProcessorChain> processorChain =
        std::make_shared<SoundInstanceProcessorChain>();
    std::shared_ptr<AudioMixerProcessor> mixerProcessor =
        std::make_shared<AudioMixerProcessor>(mixer);
    SoundInstanceManager instanceManager{AudioMixer::MAX_STREAMS,
                                         processorChain};

    Impl() { processorChain->AddProcessor(mixerProcessor); }
};

AudioManager::AudioManager() : m_impl(std::make_unique<Impl>()) {}

AudioManager::~AudioManager() { Shutdown(); }

bool AudioManager::Init()
{
    if (m_impl->initialized)
        return true;

    m_impl->initialized = m_impl->mixer.Init();
    return m_impl->initialized;
}

bool AudioManager::InitWithOutput(IAudioOutput* output)
{
    if (m_impl->initialized)
    {
        // Ownership contract: the output is always released, even when it
        // is not adopted because the manager is already initialized.
        if (output != nullptr)
            output->Release();
        return true;
    }

    m_impl->initialized = m_impl->mixer.InitWithOutput(output);
    return m_impl->initialized;
}

void AudioManager::Shutdown()
{
    m_impl->instanceManager.Clear();
    m_impl->mixer.Shutdown();
    m_impl->initialized = false;
}

void AudioManager::Update()
{
    if (!m_impl->initialized)
        return;

    m_impl->mixer.Update();

    m_impl->instanceManager.Update();
}

bool AudioManager::IsInitialized() const { return m_impl->initialized; }

AudioMixer& AudioManager::Mixer() { return m_impl->mixer; }

const AudioMixer& AudioManager::Mixer() const { return m_impl->mixer; }

std::shared_ptr<SoundInstanceProcessorChain>
AudioManager::ProcessorChain() const
{
    return m_impl->processorChain;
}

AudioMixerProcessor& AudioManager::MixerProcessor() noexcept
{
    return *m_impl->mixerProcessor;
}

const AudioMixerProcessor& AudioManager::MixerProcessor() const noexcept
{
    return *m_impl->mixerProcessor;
}

SoundInstanceId AudioManager::Play(std::shared_ptr<IAudioSource> source,
                                   bool looping, float gain, float pan)
{
    if (!m_impl->initialized || source == nullptr)
        return INVALID_SOUND_INSTANCE_ID;

    SoundInstanceDescription description;
    description.source = std::move(source);
    description.looping = looping;
    description.parameters.volumeMultiplier = gain;
    description.parameters.pan = pan;

    return m_impl->instanceManager.Play(std::move(description));
}

bool AudioManager::Stop(SoundInstanceId id)
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return false;

    return m_impl->instanceManager.Stop(id);
}

bool AudioManager::StopAll() { return m_impl->instanceManager.StopAll(); }

bool AudioManager::IsPlaying(SoundInstanceId id) const
{
    return m_impl->instanceManager.IsPlaying(id);
}

bool AudioManager::SetGainPan(SoundInstanceId id, float gain, float pan)
{
    return m_impl->instanceManager.SetGainPan(id, gain, pan);
}

} // namespace forg::audio
