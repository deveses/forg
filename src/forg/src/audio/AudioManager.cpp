#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioMixer.h"
#include "forg/audio/AudioMixerProcessor.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/audio/SoundInstanceProcessorChain.h"
#include "forg_pch.h"

#include <memory>
#include <utility>
#include <vector>

namespace forg::audio {

struct AudioManager::Impl
{
    AudioMixer mixer;
    bool initialized = false;
    std::shared_ptr<SoundInstanceProcessorChain> processorChain;
    std::shared_ptr<AudioMixerProcessor> mixerProcessor;
    std::vector<std::unique_ptr<ProcessedSoundInstance>> instances;
    SoundInstanceId nextInstanceId = 1;

    SoundInstanceId NextInstanceId()
    {
        const SoundInstanceId id = nextInstanceId++;
        if (nextInstanceId == INVALID_SOUND_INSTANCE_ID)
            nextInstanceId = 1;
        return id;
    }

    ProcessedSoundInstance* FindInstance(SoundInstanceId id)
    {
        for (const std::unique_ptr<ProcessedSoundInstance>& instance :
             instances)
        {
            if (instance->Id() == id)
                return instance.get();
        }

        return nullptr;
    }

    const ProcessedSoundInstance* FindInstance(SoundInstanceId id) const
    {
        for (const std::unique_ptr<ProcessedSoundInstance>& instance :
             instances)
        {
            if (instance->Id() == id)
                return instance.get();
        }

        return nullptr;
    }
};

AudioManager::AudioManager() : m_impl(std::make_unique<Impl>())
{
    m_impl->processorChain = std::make_shared<SoundInstanceProcessorChain>();
    m_impl->mixerProcessor =
        std::make_shared<AudioMixerProcessor>(m_impl->mixer);
    m_impl->processorChain->AddProcessor(m_impl->mixerProcessor);
}

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
    StopAll();
    m_impl->mixer.Shutdown();
    m_impl->initialized = false;
}

void AudioManager::Update()
{
    if (!m_impl->initialized)
        return;

    m_impl->mixer.Update();

    for (auto it = m_impl->instances.begin(); it != m_impl->instances.end();)
    {
        (*it)->Update();
        if (!(*it)->IsActive())
            it = m_impl->instances.erase(it);
        else
            ++it;
    }
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

    std::unique_ptr<ProcessedSoundInstance> instance =
        std::make_unique<ProcessedSoundInstance>(m_impl->processorChain);
    instance->Parameters().volumeMultiplier = gain;
    instance->Parameters().pan = pan;

    if (!instance->Create(m_impl->NextInstanceId()) ||
        !m_impl->mixerProcessor->Configure(*instance, std::move(source),
                                           looping) ||
        !instance->Play())
    {
        return INVALID_SOUND_INSTANCE_ID;
    }

    const SoundInstanceId id = instance->Id();
    m_impl->instances.push_back(std::move(instance));
    return id;
}

void AudioManager::Stop(SoundInstanceId id)
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return;

    for (auto it = m_impl->instances.begin(); it != m_impl->instances.end();
         ++it)
    {
        if ((*it)->Id() != id)
            continue;

        (*it)->Stop();
        m_impl->instances.erase(it);
        return;
    }
}

void AudioManager::StopAll()
{
    for (const std::unique_ptr<ProcessedSoundInstance>& instance :
         m_impl->instances)
        instance->Stop();

    m_impl->instances.clear();
}

bool AudioManager::IsPlaying(SoundInstanceId id) const
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return false;

    const ProcessedSoundInstance* instance = m_impl->FindInstance(id);
    return instance != nullptr &&
           instance->State() == SoundInstanceState::Playing;
}

void AudioManager::SetGainPan(SoundInstanceId id, float gain, float pan)
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return;

    ProcessedSoundInstance* instance = m_impl->FindInstance(id);
    if (instance == nullptr)
        return;

    instance->Parameters().pan = pan;
    instance->SetVolumeMultiplier(gain);
}

} // namespace forg::audio
