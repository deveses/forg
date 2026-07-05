#include "forg/audio/AudioManager.h"
#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioMixer.h"
#include "forg_pch.h"

#include <memory>
#include <utility>

namespace forg::audio {

struct AudioManager::Impl
{
    AudioMixer mixer;
    std::shared_ptr<IAudioSource> voices[AudioMixer::MAX_STREAMS];
    bool initialized = false;
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
    StopAll();
    m_impl->mixer.Shutdown();
    m_impl->initialized = false;
}

void AudioManager::Update()
{
    if (!m_impl->initialized)
        return;

    m_impl->mixer.Update();

    // Reclaim voices whose sources have finished playing.
    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_impl->voices[i] != nullptr && !m_impl->mixer.IsStreamActive(i))
        {
            m_impl->mixer.SetStreamSource(i, nullptr, false);
            m_impl->voices[i].reset();
        }
    }
}

bool AudioManager::IsInitialized() const { return m_impl->initialized; }

AudioMixer& AudioManager::Mixer() { return m_impl->mixer; }

const AudioMixer& AudioManager::Mixer() const { return m_impl->mixer; }

int AudioManager::Play(std::shared_ptr<IAudioSource> source, bool looping,
                       float gain, float pan)
{
    if (!m_impl->initialized || source == nullptr)
        return INVALID_VOICE;

    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_impl->voices[i] == nullptr && !m_impl->mixer.IsStreamActive(i))
        {
            m_impl->voices[i] = std::move(source);
            m_impl->mixer.SetStreamSource(i, m_impl->voices[i], looping);
            m_impl->mixer.SetStreamGainPan(i, gain, pan);
            return static_cast<int>(i);
        }
    }

    return INVALID_VOICE;
}

void AudioManager::Stop(int voice)
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return;

    if (m_impl->voices[voice] != nullptr)
    {
        m_impl->mixer.SetStreamSource(static_cast<unsigned int>(voice), nullptr,
                                      false);
        m_impl->voices[voice].reset();
    }
}

void AudioManager::StopAll()
{
    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_impl->voices[i] != nullptr)
        {
            m_impl->mixer.SetStreamSource(i, nullptr, false);
            m_impl->voices[i].reset();
        }
    }
}

bool AudioManager::IsPlaying(int voice) const
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return false;

    return m_impl->voices[voice] != nullptr &&
           m_impl->mixer.IsStreamActive(static_cast<unsigned int>(voice));
}

void AudioManager::SetGainPan(int voice, float gain, float pan)
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return;

    if (m_impl->voices[voice] != nullptr)
        m_impl->mixer.SetStreamGainPan(static_cast<unsigned int>(voice), gain,
                                       pan);
}

} // namespace forg::audio
