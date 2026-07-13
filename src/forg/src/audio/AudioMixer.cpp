#include "forg/audio/AudioMixer.h"
#include "forg/audio/AudioDefs.h"
#include "forg/audio/IAudioSource.h"
#include "audio/AudioOutput.h"
#include "forg_pch.h"
// #include "forg/cpu/vector.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

namespace forg::audio {

namespace {

bool IsSupportedStreamFormat(const SAudioFormat& format)
{
    return format.freq == 44100 && format.bps == 2 && format.chan > 0 &&
           format.chan <= MAX_NUM_CHANNELS;
}

} // namespace

struct AudioMixer::Impl
{
    struct SStreamSource
    {
        std::shared_ptr<IAudioSource> source;
        float gain = 1.0f;
        float pan = 0.0f;
        bool looping = false;
    };

    IAudioOutput* output = nullptr;
    SAudioStream streams[MAX_STREAMS];
    SStreamSource sources[MAX_STREAMS];
    bool acquiredVoices[MAX_STREAMS] = {};
    unsigned int num_streams = 0;
    SAudioFormat format = {};

    unsigned int MixStreams(char* _out_buffer, unsigned int _out_size);
    unsigned int MixStreamsFloat(float* _out_samples, unsigned int _count);
};

AudioMixer::AudioMixer() : m_impl(std::make_unique<Impl>()) {}

AudioMixer::~AudioMixer() { Shutdown(); }

bool AudioMixer::Init() { return InitWithOutput(CreateDefaultAudioOutput()); }

bool AudioMixer::InitWithOutput(IAudioOutput* output)
{
    Shutdown();

    m_impl->format.freq = 44100;
    m_impl->format.bps = 2;
    m_impl->format.chan = 2;

    m_impl->num_streams = MAX_STREAMS;
    for (unsigned int i = 0; i < m_impl->num_streams; i++)
    {
        m_impl->streams[i].buffer.ptr = 0;
        m_impl->streams[i].buffer.size = 0;
        m_impl->streams[i].buffer.format = m_impl->format;
        m_impl->streams[i].format = m_impl->format;
        m_impl->streams[i].offset = 0;
        m_impl->streams[i].bytes_left = 0;
        m_impl->streams[i].state = SAudioStream::STATE_OFF;

        m_impl->sources[i].source.reset();
        m_impl->sources[i].gain = 1.0f;
        m_impl->sources[i].pan = 0.0f;
        m_impl->sources[i].looping = false;
        m_impl->acquiredVoices[i] = false;
    }

    m_impl->output = output;
    if (m_impl->output == nullptr)
        return false;

    if (!m_impl->output->Init())
    {
        m_impl->output->Release();
        m_impl->output = nullptr;
        return false;
    }

    return true;
}

void AudioMixer::Shutdown()
{
    if (m_impl->output)
    {
        m_impl->output->Release();
        m_impl->output = 0;
    }
}

bool AudioMixer::IsInitialized() const noexcept
{
    return m_impl->output != nullptr;
}

void AudioMixer::Update()
{
    char buffer[44100 * 2 * 2];

    if (m_impl->output == 0)
        return;

    if (m_impl->output->CanWrite())
    {
        unsigned int len = m_impl->MixStreams(buffer, sizeof(buffer));
        if (len > 0)
        {
            m_impl->output->Write(buffer, len);
        }
    }
}

void AudioMixer::SetStreamBuffer(unsigned int _stream, char* _buffer,
                                 unsigned int _size)
{
    if (_stream < m_impl->num_streams)
    {
        m_impl->sources[_stream].source.reset();
        m_impl->sources[_stream].gain = 1.0f;
        m_impl->sources[_stream].pan = 0.0f;
        m_impl->sources[_stream].looping = false;

        m_impl->streams[_stream].buffer.ptr = _buffer;
        m_impl->streams[_stream].buffer.size = _size;

        m_impl->streams[_stream].offset = 0;
        m_impl->streams[_stream].bytes_left = _size;
        m_impl->streams[_stream].state = _buffer != 0 && _size > 0
                                             ? SAudioStream::STATE_ON
                                             : SAudioStream::STATE_OFF;
    }
}

void AudioMixer::SetStreamSource(unsigned int _stream,
                                 std::shared_ptr<IAudioSource> source,
                                 bool looping)
{
    if (_stream < m_impl->num_streams)
    {
        m_impl->sources[_stream].source = std::move(source);
        m_impl->sources[_stream].looping = looping;

        m_impl->streams[_stream].buffer.ptr = 0;
        m_impl->streams[_stream].buffer.size = 0;
        m_impl->streams[_stream].offset = 0;
        m_impl->streams[_stream].bytes_left = 0;
        m_impl->streams[_stream].state =
            m_impl->sources[_stream].source != nullptr
                ? SAudioStream::STATE_ON
                : SAudioStream::STATE_OFF;
    }
}

void AudioMixer::SetStreamGainPan(unsigned int _stream, float gain, float pan)
{
    if (_stream < m_impl->num_streams)
    {
        m_impl->sources[_stream].gain = std::clamp(gain, 0.0f, 1.0f);
        m_impl->sources[_stream].pan = std::clamp(pan, -1.0f, 1.0f);
    }
}

bool AudioMixer::IsStreamActive(unsigned int _stream) const
{
    if (_stream >= m_impl->num_streams)
        return false;

    const SAudioStream& stream = m_impl->streams[_stream];
    if (stream.state != SAudioStream::STATE_ON)
        return false;

    if (m_impl->sources[_stream].source != nullptr)
        return true;

    return stream.bytes_left > 0;
}

int AudioMixer::AcquireVoice()
{
    if (!IsInitialized())
        return INVALID_VOICE;

    for (unsigned int i = 0; i < m_impl->num_streams; i++)
    {
        if (!m_impl->acquiredVoices[i] && !IsStreamActive(i))
        {
            m_impl->acquiredVoices[i] = true;
            return static_cast<int>(i);
        }
    }

    return INVALID_VOICE;
}

void AudioMixer::ReleaseVoice(int voiceId)
{
    if (voiceId < 0 || voiceId >= static_cast<int>(m_impl->num_streams))
        return;

    const unsigned int stream = static_cast<unsigned int>(voiceId);
    if (!m_impl->acquiredVoices[stream])
        return;

    SetStreamSource(stream, nullptr, false);
    m_impl->acquiredVoices[stream] = false;
}

bool AudioMixer::IsVoiceAcquired(int voiceId) const noexcept
{
    if (voiceId < 0 || voiceId >= static_cast<int>(m_impl->num_streams))
        return false;

    return m_impl->acquiredVoices[static_cast<unsigned int>(voiceId)];
}

void AudioMixer::SetStreamFormat(unsigned int _stream,
                                 const SAudioFormat& format)
{
    if (_stream < m_impl->num_streams)
    {
        if (!IsSupportedStreamFormat(format))
            return;

        m_impl->streams[_stream].format = format;
    }
}

// #define SHORT_TO_FLOAT 0.000030517578125
void MixSamples(float* _out, int _out_chan, short* _in, int _in_chan,
                uint32 _count, float _gain_l, float _gain_r)
{
    for (uint32 i = 0; i < _count; i++)
    {
        if (_in_chan == 1)
        {
            float x = (float)_in[0] / 32768;
            for (int j = 0; j < _out_chan; j++)
            {
                float gain = j == 0 ? _gain_l : _gain_r;
                _out[j] = _out[j] + x * gain;
            }
        }
        else
        {
            for (int j = 0; j < _out_chan && j < _in_chan; j++)
            {
                float x = (float)_in[j] / 32768;
                float gain = j == 0 ? _gain_l : _gain_r;
                //_out[j] = clamp((_out[j] + x)/2, -1.0f, 1.0f);
                _out[j] = _out[j] + x * gain;
            }
        }

        _in += _in_chan;
        _out += _out_chan;
    }
}

void ConvertSamples(float* _out, int _out_chan, short* _in, int _in_chan,
                    uint32 _count)
{
    for (uint32 i = 0; i < _count; i++)
    {
        for (int j = 0; j < _out_chan && j < _in_chan; j++)
        {
            _out[j] = (float)_in[j] / 32768;
        }

        _in += _in_chan;
        _out += _out_chan;
    }
}

void ConvertSamplesToIntegers(short* _out, float* _in, uint32 _count,
                              int _channels)
{
    for (uint32 i = 0; i < _count; i++)
    {
        for (int j = 0; j < _channels; j++)
        {
            const float sample = std::clamp(_in[j], -1.0f, 1.0f);
            _out[j] = sample >= 1.0f    ? 32767
                      : sample <= -1.0f ? -32768
                                        : (short)(sample * 32768.0f);
        }

        _in += _channels;
        _out += _channels;
    }
}

constexpr uint32 SAMPLES_BUFFER_SIZE = 1024;
constexpr std::size_t SAMPLES_BUFFER_CAPACITY =
    static_cast<std::size_t>(MAX_NUM_CHANNELS) * SAMPLES_BUFFER_SIZE;

unsigned int AudioMixer::Impl::MixStreams(char* _out_buffer,
                                          unsigned int _out_size)
{
    unsigned int mixed_size = 0;
    float samplesf[SAMPLES_BUFFER_CAPACITY];

    short* obuf = (short*)_out_buffer;
    uint32 sample_stride = (format.bps * format.chan);
    // output size in samples
    uint32 olength = _out_size / sample_stride;

    for (uint32 i = 0; i < olength / SAMPLES_BUFFER_SIZE; i++)
    {
        std::fill_n(samplesf, SAMPLES_BUFFER_CAPACITY, 0.0f);
        uint32 mix_samples = MixStreamsFloat(samplesf, SAMPLES_BUFFER_SIZE);

        if (mix_samples > 0)
        {
            ConvertSamplesToIntegers(obuf, samplesf, mix_samples, format.chan);

            obuf += mix_samples * format.chan;
            mixed_size += mix_samples * sample_stride;
        }
    }

    return mixed_size;
}

unsigned int AudioMixer::Impl::MixStreamsFloat(float* _out_samples,
                                               unsigned int _count)
{
    unsigned int out_length = 0;
    short pull_buffer[SAMPLES_BUFFER_CAPACITY];

    _count = min(_count, SAMPLES_BUFFER_SIZE);

    for (unsigned int s = 0; s < num_streams; s++)
    {
        if (streams[s].state == SAudioStream::STATE_OFF)
            continue;

        SStreamSource& stream_source = sources[s];
        const float pan = stream_source.pan;
        const float gain_l =
            stream_source.gain * (pan > 0.0f ? 1.0f - pan : 1.0f);
        const float gain_r =
            stream_source.gain * (pan < 0.0f ? 1.0f + pan : 1.0f);

        if (stream_source.source != nullptr)
        {
            std::shared_ptr<IAudioSource> source = stream_source.source;
            int in_chan = source->Channels();

            if (in_chan < 1 || in_chan > MAX_NUM_CHANNELS)
            {
                streams[s].state = SAudioStream::STATE_OFF;
                stream_source.source.reset();
                continue;
            }

            unsigned int got = 0;
            while (got < _count)
            {
                got += source->Read(pull_buffer + got * in_chan, _count - got);
                if (got >= _count)
                    break;

                if (!source->IsFinished())
                    break;

                if (!stream_source.looping)
                {
                    streams[s].state = SAudioStream::STATE_OFF;
                    stream_source.source.reset();
                    break;
                }

                source->Reset();
                if (source->IsFinished())
                {
                    // Empty source; stop to avoid pulling forever.
                    streams[s].state = SAudioStream::STATE_OFF;
                    stream_source.source.reset();
                    break;
                }
            }

            if (got > 0)
            {
                MixSamples(_out_samples, format.chan, pull_buffer, in_chan, got,
                           gain_l, gain_r);
                out_length = max(out_length, got);
            }
        }
        else if (streams[s].bytes_left > 0)
        {
            SAudioFormat stream_format = streams[s].format;

            int sample_size = (stream_format.chan * stream_format.bps);

            short* buff_in =
                (short*)(streams[s].buffer.ptr + streams[s].offset);
            unsigned int stream_length = streams[s].bytes_left / sample_size;
            stream_length = min(stream_length, _count);

            MixSamples(_out_samples, format.chan, buff_in, stream_format.chan,
                       stream_length, gain_l, gain_r);

            out_length = max(out_length, stream_length);

            stream_length = stream_length * sample_size;

            streams[s].bytes_left -= stream_length;
            streams[s].offset += stream_length;
        }
    }

    return out_length;
}

} // namespace forg::audio
