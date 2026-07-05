#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "forg/audio/AudioGenerator.h"

namespace {

std::vector<short> ReadFrames(forg::audio::AudioGenerator& generator,
                              unsigned int frames)
{
    std::vector<short> samples(frames);
    REQUIRE(generator.Read(samples.data(), frames) == frames);
    return samples;
}

} // namespace

TEST_CASE("AudioGenerator produces a mono sine wave", "[audio][generator]")
{
    forg::audio::AudioGenerator generator;
    generator.SetWaveform(forg::audio::AudioWaveform::Sine);
    generator.SetFrequency(441.0f); // period of exactly 100 samples
    generator.SetAmplitude(1.0f);

    REQUIRE(generator.Channels() == 1);
    REQUIRE_FALSE(generator.IsFinished());

    const std::vector<short> samples = ReadFrames(generator, 100);

    REQUIRE(samples[0] == 0);
    REQUIRE(samples[25] == Catch::Approx(32767).margin(1));  // quarter period
    REQUIRE(samples[50] == Catch::Approx(0).margin(21));     // half period
    REQUIRE(samples[75] == Catch::Approx(-32767).margin(1)); // three quarters
}

TEST_CASE("AudioGenerator produces a square wave", "[audio][generator]")
{
    forg::audio::AudioGenerator generator;
    generator.SetWaveform(forg::audio::AudioWaveform::Square);
    generator.SetFrequency(441.0f);
    generator.SetAmplitude(0.5f);

    const std::vector<short> samples = ReadFrames(generator, 100);
    const short high = static_cast<short>(0.5f * 32767.0f);

    REQUIRE(samples[10] == high);
    REQUIRE(samples[49] == high);
    REQUIRE(samples[51] == -high);
    REQUIRE(samples[99] == -high);
}

TEST_CASE("AudioGenerator Reset reproduces the same samples",
          "[audio][generator]")
{
    forg::audio::AudioGenerator generator;
    generator.SetFrequency(440.0f);

    const std::vector<short> first = ReadFrames(generator, 64);
    generator.Reset();
    const std::vector<short> second = ReadFrames(generator, 64);

    REQUIRE(first == second);
    REQUIRE_FALSE(generator.IsFinished());
}

TEST_CASE("AudioGenerator keeps phase across reads", "[audio][generator]")
{
    forg::audio::AudioGenerator continuous;
    continuous.SetFrequency(441.0f);
    const std::vector<short> whole = ReadFrames(continuous, 100);

    forg::audio::AudioGenerator split;
    split.SetFrequency(441.0f);
    std::vector<short> parts = ReadFrames(split, 50);
    const std::vector<short> tail = ReadFrames(split, 50);
    parts.insert(parts.end(), tail.begin(), tail.end());

    REQUIRE(whole == parts);
}

TEST_CASE("AudioWaveform names round-trip", "[audio][generator]")
{
    using forg::audio::AudioWaveform;

    REQUIRE(forg::audio::AudioWaveformName(AudioWaveform::Sine) == "sine");
    REQUIRE(forg::audio::AudioWaveformName(AudioWaveform::Square) == "square");
    REQUIRE(forg::audio::AudioWaveformFromName("square") ==
            AudioWaveform::Square);
    REQUIRE(forg::audio::AudioWaveformFromName("sine") == AudioWaveform::Sine);
    REQUIRE(forg::audio::AudioWaveformFromName("unknown") ==
            AudioWaveform::Sine);
}
