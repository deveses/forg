#include <catch2/catch_test_macros.hpp>

#include "forg/audio/AudioMixer.h"

#include <cstring>
#include <vector>

namespace {

class CapturingAudioOutput : public forg::audio::IAudioOutput
{
  public:
    bool canWrite = true;
    bool initialized = false;
    bool released = false;
    unsigned int writeCount = 0;
    std::vector<char> written;

    bool CanWrite() override { return canWrite; }

    void Write(char* data, unsigned int size) override
    {
        ++writeCount;
        written.assign(data, data + size);
    }

    bool Init() override
    {
        initialized = true;
        return true;
    }

    void Release() override { released = true; }
};

std::vector<short> WrittenSamples(const CapturingAudioOutput& output)
{
    std::vector<short> samples(output.written.size() / sizeof(short));
    std::memcpy(samples.data(), output.written.data(), output.written.size());
    return samples;
}

} // namespace

TEST_CASE("AudioMixer writes a stereo stream to the output", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE(mixer.InitWithOutput(&output));
    REQUIRE(output.initialized);

    forg::audio::SAudioFormat stereo16 = {44100, 2, 2};
    short input[] = {1000, -1000, 2000, -2000, -3000, 3000};

    mixer.SetStreamFormat(0, stereo16);
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(input), sizeof(input));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(output.written.size() == sizeof(input));
    REQUIRE(WrittenSamples(output) ==
            std::vector<short>({1000, -1000, 2000, -2000, -3000, 3000}));

    mixer.Update();
    REQUIRE(output.writeCount == 1);
}

TEST_CASE("AudioMixer sums active stereo streams", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE(mixer.InitWithOutput(&output));

    forg::audio::SAudioFormat stereo16 = {44100, 2, 2};
    short stream0[] = {1000, 2000, -1000, -2000};
    short stream1[] = {300, 400, -300, -400};

    mixer.SetStreamFormat(0, stereo16);
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(stream0), sizeof(stream0));
    mixer.SetStreamFormat(1, stereo16);
    mixer.SetStreamBuffer(1, reinterpret_cast<char*>(stream1), sizeof(stream1));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) ==
            std::vector<short>({1300, 2400, -1300, -2400}));
}

TEST_CASE("AudioMixer waits until the output can accept data", "[audio][mixer]")
{
    CapturingAudioOutput output;
    output.canWrite = false;

    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    forg::audio::SAudioFormat stereo16 = {44100, 2, 2};
    short input[] = {100, 200};

    mixer.SetStreamFormat(0, stereo16);
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(input), sizeof(input));
    mixer.Update();

    REQUIRE(output.writeCount == 0);

    output.canWrite = true;
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) == std::vector<short>({100, 200}));
}

TEST_CASE("AudioMixer uses the default stream format", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE(mixer.InitWithOutput(&output));

    short input[] = {500, 600};

    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(input), sizeof(input));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) == std::vector<short>({500, 600}));
}

TEST_CASE("AudioMixer duplicates mono input to stereo output", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE(mixer.InitWithOutput(&output));

    forg::audio::SAudioFormat mono16 = {44100, 2, 1};
    short input[] = {1000, -2000};

    mixer.SetStreamFormat(0, mono16);
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(input), sizeof(input));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) ==
            std::vector<short>({1000, 1000, -2000, -2000}));
}

TEST_CASE("AudioMixer clamps mixed output to 16-bit range", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE(mixer.InitWithOutput(&output));

    forg::audio::SAudioFormat stereo16 = {44100, 2, 2};
    short stream0[] = {30000, -30000};
    short stream1[] = {30000, -30000};

    mixer.SetStreamFormat(0, stereo16);
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(stream0), sizeof(stream0));
    mixer.SetStreamFormat(1, stereo16);
    mixer.SetStreamBuffer(1, reinterpret_cast<char*>(stream1), sizeof(stream1));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) == std::vector<short>({32767, -32768}));
}
