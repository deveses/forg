#include <catch2/catch_test_macros.hpp>

#include "forg/audio/AudioDefs.h"
#include "forg/audio/IAudioSource.h"
#include "forg/audio/AudioMixer.h"

#include <cstring>
#include <memory>
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

class StubAudioSource : public forg::audio::IAudioSource
{
  public:
    std::vector<short> samples; // interleaved
    int channels = 1;
    unsigned int position = 0; // frames
    unsigned int resetCount = 0;

    unsigned int Read(short* out, unsigned int frames) override
    {
        const unsigned int total =
            static_cast<unsigned int>(samples.size()) / channels;
        const unsigned int count = std::min(frames, total - position);
        std::memcpy(out, samples.data() + position * channels,
                    count * channels * sizeof(short));
        position += count;
        return count;
    }

    int Channels() const override { return channels; }

    bool IsFinished() const override
    {
        return position >= samples.size() / static_cast<unsigned int>(channels);
    }

    void Reset() override
    {
        position = 0;
        ++resetCount;
    }
};

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

TEST_CASE("AudioMixer mixes a pull source into the output", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->channels = 2;
    source->samples = {1000, -1000, 2000, -2000};

    mixer.SetStreamSource(0, source, false);
    REQUIRE(mixer.IsStreamActive(0));
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(WrittenSamples(output) ==
            std::vector<short>({1000, -1000, 2000, -2000}));
    REQUIRE_FALSE(mixer.IsStreamActive(0));

    mixer.Update();
    REQUIRE(output.writeCount == 1);
}

TEST_CASE("AudioMixer duplicates a mono pull source to stereo",
          "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->samples = {500, -600};

    mixer.SetStreamSource(0, source, false);
    mixer.Update();

    REQUIRE(WrittenSamples(output) ==
            std::vector<short>({500, 500, -600, -600}));
}

TEST_CASE("AudioMixer loops a pull source", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->channels = 2;
    source->samples = {100, 200};

    mixer.SetStreamSource(0, source, true);
    mixer.Update();

    REQUIRE(output.writeCount == 1);
    REQUIRE(mixer.IsStreamActive(0));
    REQUIRE(source->resetCount > 0);

    const std::vector<short> samples = WrittenSamples(output);
    REQUIRE(samples.size() >= 4);
    REQUIRE(samples[0] == 100);
    REQUIRE(samples[1] == 200);
    REQUIRE(samples[2] == 100);
    REQUIRE(samples[3] == 200);

    mixer.SetStreamSource(0, nullptr, false);
    REQUIRE_FALSE(mixer.IsStreamActive(0));
}

TEST_CASE("AudioMixer stops an empty looping source", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    mixer.SetStreamSource(0, source, true);
    mixer.Update();

    REQUIRE(output.writeCount == 0);
    REQUIRE_FALSE(mixer.IsStreamActive(0));
}

TEST_CASE("AudioMixer applies stream gain", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->channels = 2;
    source->samples = {8000, -8000};

    mixer.SetStreamSource(0, source, false);
    mixer.SetStreamGainPan(0, 0.5f, 0.0f);
    mixer.Update();

    REQUIRE(WrittenSamples(output) == std::vector<short>({4000, -4000}));
}

TEST_CASE("AudioMixer pans a stream between channels", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->channels = 2;
    source->samples = {8000, 8000};

    SECTION("full right silences the left channel")
    {
        mixer.SetStreamSource(0, source, false);
        mixer.SetStreamGainPan(0, 1.0f, 1.0f);
        mixer.Update();

        REQUIRE(WrittenSamples(output) == std::vector<short>({0, 8000}));
    }

    SECTION("full left silences the right channel")
    {
        mixer.SetStreamSource(0, source, false);
        mixer.SetStreamGainPan(0, 1.0f, -1.0f);
        mixer.Update();

        REQUIRE(WrittenSamples(output) == std::vector<short>({8000, 0}));
    }
}

TEST_CASE("AudioMixer resets gain and pan when a buffer replaces a source",
          "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->channels = 2;
    source->samples = {8000, 8000};

    mixer.SetStreamSource(0, source, false);
    mixer.SetStreamGainPan(0, 0.5f, 1.0f);
    mixer.Update();
    REQUIRE(WrittenSamples(output) == std::vector<short>({0, 4000}));

    short input[] = {1000, -1000};
    mixer.SetStreamBuffer(0, reinterpret_cast<char*>(input), sizeof(input));
    mixer.Update();

    // Legacy buffer playback must not inherit the previous stream's
    // gain/pan.
    REQUIRE(WrittenSamples(output) == std::vector<short>({1000, -1000}));
}

TEST_CASE("AudioMixer reserves and reuses voice ids", "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;

    REQUIRE_FALSE(mixer.IsInitialized());
    REQUIRE(mixer.AcquireVoice() == forg::audio::AudioMixer::INVALID_VOICE);
    REQUIRE(mixer.InitWithOutput(&output));
    REQUIRE(mixer.IsInitialized());

    for (unsigned int i = 0; i < forg::audio::AudioMixer::MAX_STREAMS; i++)
    {
        const int voice = mixer.AcquireVoice();
        REQUIRE(voice == static_cast<int>(i));
        REQUIRE(mixer.IsVoiceAcquired(voice));
    }

    REQUIRE(mixer.AcquireVoice() == forg::audio::AudioMixer::INVALID_VOICE);

    mixer.ReleaseVoice(3);
    REQUIRE_FALSE(mixer.IsVoiceAcquired(3));
    REQUIRE(mixer.AcquireVoice() == 3);
}

TEST_CASE("AudioMixer does not acquire a directly active stream",
          "[audio][mixer]")
{
    CapturingAudioOutput output;
    forg::audio::AudioMixer mixer;
    REQUIRE(mixer.InitWithOutput(&output));

    std::shared_ptr<StubAudioSource> source =
        std::make_shared<StubAudioSource>();
    source->samples = {100};
    mixer.SetStreamSource(0, source, false);

    REQUIRE(mixer.AcquireVoice() == 1);
}
