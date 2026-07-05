#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "forg/audio/AudioFileWav.h"
#include "forg/audio/WaveFile.h"

namespace {

template <typename T> void WriteValue(std::ofstream& out, T value)
{
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

std::filesystem::path TempPath(const char* filename)
{
    return std::filesystem::temp_directory_path() / filename;
}

void WriteWaveFile(const std::filesystem::path& path, unsigned int sample_rate,
                   unsigned short channels, unsigned short bits_per_sample,
                   const std::vector<char>& data)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    REQUIRE(out);

    const unsigned int format_size = 16;
    const unsigned int data_size = static_cast<unsigned int>(data.size());
    const unsigned int riff_size = 4 + (8 + format_size) + (8 + data_size);
    const unsigned short block_align = channels * (bits_per_sample / 8);

    WriteValue(out, static_cast<unsigned int>(RiffID));
    WriteValue(out, riff_size);
    WriteValue(out, static_cast<unsigned int>(WaveID));

    WriteValue(out, static_cast<unsigned int>(FormatID));
    WriteValue(out, format_size);
    WriteValue(out, static_cast<unsigned short>(1)); // PCM
    WriteValue(out, channels);
    WriteValue(out, sample_rate);
    WriteValue(out, sample_rate * block_align); // byte rate
    WriteValue(out, block_align);
    WriteValue(out, bits_per_sample);

    WriteValue(out, static_cast<unsigned int>(DataID));
    WriteValue(out, data_size);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

std::vector<char> AsBytes(const std::vector<short>& samples)
{
    std::vector<char> bytes(samples.size() * sizeof(short));
    std::memcpy(bytes.data(), samples.data(), bytes.size());
    return bytes;
}

struct TempWave
{
    std::filesystem::path path;

    explicit TempWave(const char* filename) : path(TempPath(filename))
    {
        std::filesystem::remove(path);
    }

    ~TempWave() { std::filesystem::remove(path); }
};

} // namespace

TEST_CASE("AudioFileWav reads a 16-bit mono file", "[audio][wavsource]")
{
    TempWave wave("forg_audiofile_mono16.wav");
    const std::vector<short> samples = {1000, -1000, 32767, -32768};
    WriteWaveFile(wave.path, 44100, 1, 16, AsBytes(samples));

    forg::audio::AudioFileWav file;
    REQUIRE(file.Open(wave.path.string()));
    REQUIRE(file.IsOpen());
    REQUIRE(file.Channels() == 1);
    REQUIRE(file.FrameCount() == 4);
    REQUIRE_FALSE(file.IsFinished());

    std::vector<short> read(8, 0);
    REQUIRE(file.Read(read.data(), 8) == 4);
    read.resize(4);
    REQUIRE(read == samples);
    REQUIRE(file.IsFinished());
    REQUIRE(file.Read(read.data(), 4) == 0);

    file.Reset();
    REQUIRE_FALSE(file.IsFinished());
    REQUIRE(file.Read(read.data(), 4) == 4);
    REQUIRE(read == samples);
}

TEST_CASE("AudioFileWav reads interleaved stereo frames", "[audio][wavsource]")
{
    TempWave wave("forg_audiofile_stereo16.wav");
    const std::vector<short> samples = {100, -100, 200, -200, 300, -300};
    WriteWaveFile(wave.path, 44100, 2, 16, AsBytes(samples));

    forg::audio::AudioFileWav file;
    REQUIRE(file.Open(wave.path.string()));
    REQUIRE(file.Channels() == 2);
    REQUIRE(file.FrameCount() == 3);

    std::vector<short> read(4, 0);
    REQUIRE(file.Read(read.data(), 2) == 2);
    REQUIRE(read == std::vector<short>({100, -100, 200, -200}));
    REQUIRE(file.Read(read.data(), 2) == 1);
    REQUIRE(read[0] == 300);
    REQUIRE(read[1] == -300);
    REQUIRE(file.IsFinished());
}

TEST_CASE("AudioFileWav converts 8-bit samples to 16-bit", "[audio][wavsource]")
{
    TempWave wave("forg_audiofile_mono8.wav");
    const std::vector<char> data = {
        static_cast<char>(0x80), // midpoint -> 0
        static_cast<char>(0xff), // max -> 127 << 8
        static_cast<char>(0x00), // min -> -128 << 8
    };
    WriteWaveFile(wave.path, 44100, 1, 8, data);

    forg::audio::AudioFileWav file;
    REQUIRE(file.Open(wave.path.string()));
    REQUIRE(file.Channels() == 1);

    std::vector<short> read(3, 1);
    REQUIRE(file.Read(read.data(), 3) == 3);
    REQUIRE(read == std::vector<short>({0, 127 << 8, -(128 << 8)}));
}

TEST_CASE("AudioFileWav rejects unsupported files", "[audio][wavsource]")
{
    forg::audio::AudioFileWav file;

    SECTION("missing file")
    {
        TempWave wave("forg_audiofile_missing.wav");
        REQUIRE_FALSE(file.Open(wave.path.string()));
        REQUIRE_FALSE(file.IsOpen());
    }

    SECTION("wrong sample rate")
    {
        TempWave wave("forg_audiofile_8khz.wav");
        WriteWaveFile(wave.path, 8000, 1, 16, AsBytes({1, 2, 3}));
        REQUIRE_FALSE(file.Open(wave.path.string()));
        REQUIRE_FALSE(file.IsOpen());
    }

    SECTION("too many channels")
    {
        TempWave wave("forg_audiofile_4ch.wav");
        WriteWaveFile(wave.path, 44100, 4, 16, AsBytes({1, 2, 3, 4}));
        REQUIRE_FALSE(file.Open(wave.path.string()));
    }

    SECTION("empty data chunk")
    {
        TempWave wave("forg_audiofile_empty.wav");
        WriteWaveFile(wave.path, 44100, 1, 16, {});
        REQUIRE_FALSE(file.Open(wave.path.string()));
    }
}

TEST_CASE("AudioFileWav rejects data not aligned to whole frames",
          "[audio][wavsource]")
{
    forg::audio::AudioFileWav file;

    SECTION("stereo 16-bit data with a lone sample")
    {
        TempWave wave("forg_audiofile_partial_frame.wav");
        // One 16-bit sample = 2 bytes, but a stereo 16-bit frame needs 4.
        WriteWaveFile(wave.path, 44100, 2, 16, AsBytes({1000}));
        REQUIRE_FALSE(file.Open(wave.path.string()));
        REQUIRE_FALSE(file.IsOpen());
    }

    SECTION("mono 16-bit data with a trailing odd byte")
    {
        TempWave wave("forg_audiofile_odd_byte.wav");
        WriteWaveFile(wave.path, 44100, 1, 16, {0x01, 0x02, 0x03});
        REQUIRE_FALSE(file.Open(wave.path.string()));
    }
}
