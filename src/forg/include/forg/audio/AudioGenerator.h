// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include <string_view>

#include "forg/audio/IAudioSource.h"
#include "forg/base.h"

namespace forg::audio {

enum class AudioWaveform
{
    Sine,
    Square,
};

FORG_API std::string_view AudioWaveformName(AudioWaveform waveform);
FORG_API AudioWaveform AudioWaveformFromName(std::string_view name);

// Endless mono waveform generator at the mixer sample rate.
class FORG_API AudioGenerator : public IAudioSource
{
    AudioWaveform m_waveform = AudioWaveform::Sine;
    float m_frequency = 440.0f;
    float m_amplitude = 0.5f;
    double m_phase = 0.0;

  public:
    void SetWaveform(AudioWaveform waveform);
    AudioWaveform Waveform() const;

    void SetFrequency(float frequencyHz);
    float Frequency() const;

    // Amplitude in [0, 1].
    void SetAmplitude(float amplitude);
    float Amplitude() const;

    unsigned int Read(short* samples, unsigned int frames) override;
    int Channels() const override;
    bool IsFinished() const override;
    void Reset() override;
};

} // namespace forg::audio
