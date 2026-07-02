#include "forg/audio/AudioManager.h"
#include "forg_pch.h"

namespace forg::audio {

AudioManager::AudioManager() : m_initialized(false) {}

AudioManager::~AudioManager() { Shutdown(); }

bool AudioManager::Init()
{
    if (m_initialized)
        return true;

    m_initialized = m_mixer.Init();
    return m_initialized;
}

void AudioManager::Shutdown()
{
    m_mixer.Shutdown();
    m_initialized = false;
}

void AudioManager::Update()
{
    if (m_initialized)
        m_mixer.Update();
}

bool AudioManager::IsInitialized() const { return m_initialized; }

AudioMixer& AudioManager::Mixer() { return m_mixer; }

const AudioMixer& AudioManager::Mixer() const { return m_mixer; }

} // namespace forg::audio
