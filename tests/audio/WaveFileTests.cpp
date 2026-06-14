#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

#include "forg/audio/WaveFile.h"

namespace
{
template <typename T>
void WriteValue(std::ofstream& out, T value)
{
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void WriteBytes(std::ofstream& out, const char* data, std::size_t size)
{
    out.write(data, static_cast<std::streamsize>(size));
}

std::filesystem::path TempPath(const char* filename)
{
    return std::filesystem::temp_directory_path() / filename;
}

void WriteTestWaveFile(const std::filesystem::path& path)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    REQUIRE(out);

    constexpr unsigned int format_size = 16;
    constexpr unsigned int data_size = 4;
    constexpr unsigned int riff_size = 4 + (8 + format_size) + (8 + data_size);

    WriteValue(out, static_cast<unsigned int>(RiffID));
    WriteValue(out, riff_size);
    WriteValue(out, static_cast<unsigned int>(WaveID));

    WriteValue(out, static_cast<unsigned int>(FormatID));
    WriteValue(out, format_size);
    WriteValue(out, static_cast<unsigned short>(1));    // PCM
    WriteValue(out, static_cast<unsigned short>(1));    // mono
    WriteValue(out, static_cast<unsigned int>(8000));   // sample rate
    WriteValue(out, static_cast<unsigned int>(8000));   // byte rate
    WriteValue(out, static_cast<unsigned short>(1));    // block align
    WriteValue(out, static_cast<unsigned short>(8));    // bits per sample

    const std::array<char, data_size> samples = {
        static_cast<char>(0x01),
        static_cast<char>(0x02),
        static_cast<char>(0x7f),
        static_cast<char>(0xff),
    };

    WriteValue(out, static_cast<unsigned int>(DataID));
    WriteValue(out, data_size);
    WriteBytes(out, samples.data(), samples.size());
}
} // namespace

TEST_CASE("WaveFile reads format and data chunks", "[audio][wave]")
{
    const std::filesystem::path path = TempPath("forg_wavefile_valid.wav");
    std::filesystem::remove(path);
    WriteTestWaveFile(path);

    forg::audio::WaveFile wave;
    REQUIRE(wave.Open(path.string().c_str()));

    std::array<char, 16> format = {};
    REQUIRE(wave.GetFormat(format.data(), static_cast<unsigned int>(format.size())));
    REQUIRE(static_cast<unsigned char>(format[0]) == 0x01);
    REQUIRE(static_cast<unsigned char>(format[1]) == 0x00);
    REQUIRE(static_cast<unsigned char>(format[2]) == 0x01);
    REQUIRE(static_cast<unsigned char>(format[3]) == 0x00);

    forg::audio::SWaveChunk data_chunk = {};
    REQUIRE(wave.GetChunk(DataID, data_chunk));
    REQUIRE(data_chunk.header.nSize == 4);

    std::array<char, 8> samples = {};
    REQUIRE(wave.ReadChunkData(data_chunk, samples.data(),
                               static_cast<unsigned int>(samples.size())) == 4);
    REQUIRE(static_cast<unsigned char>(samples[0]) == 0x01);
    REQUIRE(static_cast<unsigned char>(samples[1]) == 0x02);
    REQUIRE(static_cast<unsigned char>(samples[2]) == 0x7f);
    REQUIRE(static_cast<unsigned char>(samples[3]) == 0xff);

    std::filesystem::remove(path);
}

TEST_CASE("WaveFile reports missing chunks and files", "[audio][wave]")
{
    const std::filesystem::path missing_path =
        TempPath("forg_wavefile_missing.wav");
    std::filesystem::remove(missing_path);

    forg::audio::WaveFile wave;
    REQUIRE_FALSE(wave.Open(missing_path.string().c_str()));

    forg::audio::SWaveChunk chunk = {};
    REQUIRE_FALSE(wave.GetChunk(DataID, chunk));
}

TEST_CASE("WaveFile Close clears loaded chunks", "[audio][wave]")
{
    const std::filesystem::path path = TempPath("forg_wavefile_close.wav");
    std::filesystem::remove(path);
    WriteTestWaveFile(path);

    forg::audio::WaveFile wave;
    REQUIRE(wave.Open(path.string().c_str()));

    forg::audio::SWaveChunk chunk = {};
    REQUIRE(wave.GetChunk(DataID, chunk));

    wave.Close();
    REQUIRE_FALSE(wave.GetChunk(DataID, chunk));

    std::filesystem::remove(path);
}
