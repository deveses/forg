#include "forg/audio/AudioGenerator.h"
#include "forg_pch.h"

#include <algorithm>
#include <cmath>

namespace forg::audio {

namespace {

constexpr double kSampleRate = 44100.0;
constexpr double kTwoPi = 6.283185307179586476925286766559;

} // namespace

std::string_view AudioWaveformName(AudioWaveform waveform)
{
    switch (waveform)
    {
    case AudioWaveform::Square:
        return "square";
    case AudioWaveform::Sine:
    default:
        return "sine";
    }
}

AudioWaveform AudioWaveformFromName(std::string_view name)
{
    if (name == "square")
        return AudioWaveform::Square;

    return AudioWaveform::Sine;
}

void AudioGenerator::SetWaveform(AudioWaveform waveform)
{
    m_waveform = waveform;
}

AudioWaveform AudioGenerator::Waveform() const { return m_waveform; }

void AudioGenerator::SetFrequency(float frequencyHz)
{
    if (frequencyHz > 0.0f)
        m_frequency = frequencyHz;
}

float AudioGenerator::Frequency() const { return m_frequency; }

void AudioGenerator::SetAmplitude(float amplitude)
{
    m_amplitude = std::clamp(amplitude, 0.0f, 1.0f);
}

float AudioGenerator::Amplitude() const { return m_amplitude; }

unsigned int AudioGenerator::Read(short* samples, unsigned int frames)
{
    if (samples == nullptr)
        return 0;

    const double phase_step = kTwoPi * m_frequency / kSampleRate;

    for (unsigned int i = 0; i < frames; i++)
    {
        float value = static_cast<float>(std::sin(m_phase));
        if (m_waveform == AudioWaveform::Square)
            value = value >= 0.0f ? 1.0f : -1.0f;

        samples[i] = static_cast<short>(value * m_amplitude * 32767.0f);

        m_phase += phase_step;
        if (m_phase >= kTwoPi)
            m_phase -= kTwoPi;
    }

    return frames;
}

int AudioGenerator::Channels() const { return 1; }

bool AudioGenerator::IsFinished() const { return false; }

void AudioGenerator::Reset() { m_phase = 0.0; }

} // namespace forg::audio
