/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

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
