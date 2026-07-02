#include "forg/audio/AudioEngine.h"
#include "forg_pch.h"

namespace forg::audio {

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() { Shutdown(); }

bool AudioEngine::Init() { return m_manager.Init(); }

void AudioEngine::Shutdown() { m_manager.Shutdown(); }

void AudioEngine::Update() { m_manager.Update(); }

bool AudioEngine::IsInitialized() const { return m_manager.IsInitialized(); }

AudioManager& AudioEngine::Manager() { return m_manager; }

const AudioManager& AudioEngine::Manager() const { return m_manager; }

} // namespace forg::audio
