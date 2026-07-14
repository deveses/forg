// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once

#include "forg/api.h"
#include "forg/audio/ProcessedSoundInstance.h"

#include <memory>

namespace forg::audio {

class AudioMixer;
class AudioMixerProcessor;
class IAudioOutput;
class IAudioSource;
class SoundInstanceProcessorChain;

class FORG_API AudioManager
{
    struct Impl;
    std::unique_ptr<Impl> m_impl;

  public:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool Init();
    // Takes ownership of output and releases it through
    // IAudioOutput::Release(), even when the manager is already
    // initialized and the output is not adopted.
    bool InitWithOutput(IAudioOutput* output);
    void Shutdown();
    // Pumps the mixer and updates all active processed sound instances.
    void Update();

    bool IsInitialized() const;
    AudioMixer& Mixer();
    const AudioMixer& Mixer() const;
    std::shared_ptr<SoundInstanceProcessorChain> ProcessorChain() const;
    AudioMixerProcessor& MixerProcessor() noexcept;
    const AudioMixerProcessor& MixerProcessor() const noexcept;

    // Creates and plays a processed sound instance, returning its id or
    // INVALID_SOUND_INSTANCE_ID on failure.
    SoundInstanceId Play(std::shared_ptr<IAudioSource> source,
                         bool looping = false, float gain = 1.0f,
                         float pan = 0.0f);
    bool Stop(SoundInstanceId id);
    bool StopAll();
    bool IsPlaying(SoundInstanceId id) const;
    void SetGainPan(SoundInstanceId id, float gain, float pan);
};

} // namespace forg::audio
