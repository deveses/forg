#include "forg/audio/AudioFileWav.h"
#include "forg/audio/WaveFile.h"
#include "forg_pch.h"

#include <string>

namespace forg::audio {

namespace {

struct SWaveFormat
{
    unsigned short tag;
    unsigned short channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};

constexpr unsigned short WAVE_FORMAT_TAG_PCM = 1;
constexpr unsigned int MIXER_SAMPLE_RATE = 44100;

} // namespace

bool AudioFileWav::Open(std::string_view filename)
{
    Close();

    WaveFile wave;
    if (!wave.Open(std::string(filename).c_str()))
        return false;

    SWaveFormat format = {};
    if (!wave.GetFormat(reinterpret_cast<char*>(&format), sizeof(format)))
        return false;

    if (format.tag != WAVE_FORMAT_TAG_PCM ||
        format.sample_rate != MIXER_SAMPLE_RATE || format.channels < 1 ||
        format.channels > 2 ||
        (format.bits_per_sample != 8 && format.bits_per_sample != 16))
        return false;

    SWaveChunk data_chunk = {};
    if (!wave.GetChunk(DataID, data_chunk))
        return false;

    const unsigned int data_size = data_chunk.header.nSize;
    const unsigned int bytes_per_sample = format.bits_per_sample / 8;
    const unsigned int sample_count = data_size / bytes_per_sample;
    if (sample_count == 0)
        return false;

    std::vector<char> data(data_size);
    if (wave.ReadChunkData(data_chunk, data.data(), data_size) != data_size)
        return false;

    m_samples.resize(sample_count);
    if (format.bits_per_sample == 16)
    {
        const short* src = reinterpret_cast<const short*>(data.data());
        m_samples.assign(src, src + sample_count);
    }
    else
    {
        for (unsigned int i = 0; i < sample_count; i++)
        {
            const int value = static_cast<unsigned char>(data[i]) - 128;
            m_samples[i] = static_cast<short>(value << 8);
        }
    }

    m_channels = format.channels;
    m_position = 0;

    return true;
}

} // namespace forg::audio
