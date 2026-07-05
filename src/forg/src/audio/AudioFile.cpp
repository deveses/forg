#include "forg/audio/AudioFile.h"
#include "forg_pch.h"

#include <algorithm>
#include <cstring>

namespace forg::audio {

void AudioFile::Close()
{
    m_samples.clear();
    m_channels = 0;
    m_position = 0;
}

bool AudioFile::IsOpen() const { return m_channels > 0 && !m_samples.empty(); }

unsigned int AudioFile::FrameCount() const
{
    if (m_channels <= 0)
        return 0;

    return static_cast<unsigned int>(m_samples.size()) /
           static_cast<unsigned int>(m_channels);
}

unsigned int AudioFile::Read(short* samples, unsigned int frames)
{
    if (samples == nullptr || !IsOpen())
        return 0;

    const unsigned int frames_left = FrameCount() - m_position;
    const unsigned int count = std::min(frames, frames_left);

    if (count > 0)
    {
        const std::size_t offset =
            static_cast<std::size_t>(m_position) * m_channels;
        std::memcpy(samples, m_samples.data() + offset,
                    static_cast<std::size_t>(count) * m_channels *
                        sizeof(short));
        m_position += count;
    }

    return count;
}

int AudioFile::Channels() const { return m_channels; }

bool AudioFile::IsFinished() const { return m_position >= FrameCount(); }

void AudioFile::Reset() { m_position = 0; }

} // namespace forg::audio
