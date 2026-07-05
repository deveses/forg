// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once

#include "forg/api.h"

#include <memory>

namespace forg::audio {

class AudioMixer;
class IAudioOutput;
class IAudioSource;

class FORG_API AudioManager
{
    struct Impl;
    std::unique_ptr<Impl> m_impl;

  public:
    static constexpr int INVALID_VOICE = -1;

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
    // Pumps the mixer and reclaims voices whose sources have finished,
    // making their slots available to Play() again.
    void Update();

    bool IsInitialized() const;
    AudioMixer& Mixer();
    const AudioMixer& Mixer() const;

    // Starts playing a source on a free voice and returns its handle,
    // or INVALID_VOICE when no voice is free (at most
    // AudioMixer::MAX_STREAMS voices play at once). The manager and mixer
    // share ownership while the voice is attached.
    int Play(std::shared_ptr<IAudioSource> source, bool looping = false,
             float gain = 1.0f, float pan = 0.0f);
    void Stop(int voice);
    void StopAll();
    // True while the voice is attached and its stream still produces
    // audio; finished one-shot voices turn false after Update().
    bool IsPlaying(int voice) const;
    void SetGainPan(int voice, float gain, float pan);
};

} // namespace forg::audio
