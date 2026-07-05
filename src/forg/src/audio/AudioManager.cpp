#include "forg/audio/AudioManager.h"
#include "forg_pch.h"

#include <utility>

namespace forg::audio {

AudioManager::AudioManager() : m_voices{}, m_initialized(false) {}

AudioManager::~AudioManager() { Shutdown(); }

bool AudioManager::Init()
{
    if (m_initialized)
        return true;

    m_initialized = m_mixer.Init();
    return m_initialized;
}

bool AudioManager::InitWithOutput(IAudioOutput* output)
{
    if (m_initialized)
    {
        // Ownership contract: the output is always released, even when it
        // is not adopted because the manager is already initialized.
        if (output != nullptr)
            output->Release();
        return true;
    }

    m_initialized = m_mixer.InitWithOutput(output);
    return m_initialized;
}

void AudioManager::Shutdown()
{
    StopAll();
    m_mixer.Shutdown();
    m_initialized = false;
}

void AudioManager::Update()
{
    if (!m_initialized)
        return;

    m_mixer.Update();

    // Reclaim voices whose sources have finished playing.
    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_voices[i] != nullptr && !m_mixer.IsStreamActive(i))
        {
            m_mixer.SetStreamSource(i, nullptr, false);
            m_voices[i].reset();
        }
    }
}

bool AudioManager::IsInitialized() const { return m_initialized; }

AudioMixer& AudioManager::Mixer() { return m_mixer; }

const AudioMixer& AudioManager::Mixer() const { return m_mixer; }

int AudioManager::Play(std::shared_ptr<IAudioSource> source, bool looping,
                       float gain, float pan)
{
    if (!m_initialized || source == nullptr)
        return INVALID_VOICE;

    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_voices[i] == nullptr && !m_mixer.IsStreamActive(i))
        {
            m_voices[i] = std::move(source);
            m_mixer.SetStreamSource(i, m_voices[i], looping);
            m_mixer.SetStreamGainPan(i, gain, pan);
            return static_cast<int>(i);
        }
    }

    return INVALID_VOICE;
}

void AudioManager::Stop(int voice)
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return;

    if (m_voices[voice] != nullptr)
    {
        m_mixer.SetStreamSource(static_cast<unsigned int>(voice), nullptr,
                                false);
        m_voices[voice].reset();
    }
}

void AudioManager::StopAll()
{
    for (unsigned int i = 0; i < AudioMixer::MAX_STREAMS; i++)
    {
        if (m_voices[i] != nullptr)
        {
            m_mixer.SetStreamSource(i, nullptr, false);
            m_voices[i].reset();
        }
    }
}

bool AudioManager::IsPlaying(int voice) const
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return false;

    return m_voices[voice] != nullptr &&
           m_mixer.IsStreamActive(static_cast<unsigned int>(voice));
}

void AudioManager::SetGainPan(int voice, float gain, float pan)
{
    if (voice < 0 || voice >= static_cast<int>(AudioMixer::MAX_STREAMS))
        return;

    if (m_voices[voice] != nullptr)
        m_mixer.SetStreamGainPan(static_cast<unsigned int>(voice), gain, pan);
}

} // namespace forg::audio
