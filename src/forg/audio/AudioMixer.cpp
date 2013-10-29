#include "forg_pch.h"
#include "audio/AudioMixer.h"
#include "audio/AudioOutputWaveOut.h"
#include "cpu/vector.h"

namespace forg { namespace audio {

AudioMixer::AudioMixer()
: m_output(0)
{
}

AudioMixer::~AudioMixer()
{
    Shutdown();
}

bool AudioMixer::Init()
{
    m_output = CreateAudioOutputWaveOut();
    m_output->Init();

    m_format.freq = 44100;
    m_format.bps = 2;
    m_format.chan = 2;

    m_num_streams = 10;
    for (unsigned int i=0; i<m_num_streams; i++)
    {
        m_streams[i].state = 0;
    }

    return true;
}

void AudioMixer::Shutdown()
{
    if (m_output)
    {
        m_output->Release();
    }
}

void AudioMixer::Update()
{
    char buffer[44100*2*2];
    
    if (m_output->CanWrite())
    {
        unsigned int len = MixStreams(buffer, sizeof(buffer));
        if (len > 0)
        {
            m_output->Write(buffer, len);
        }
    }    
}

void AudioMixer::SetStreamBuffer(unsigned int _stream, char* _buffer, unsigned int _size)
{
    if (_stream < m_num_streams)
    {
        m_streams[_stream].buffer.ptr = _buffer;
        m_streams[_stream].buffer.size = _size;
        
        m_streams[_stream].offset = 0;
        m_streams[_stream].bytes_left = _size;
        m_streams[_stream].state = SAudioStream::STATE_ON;
    }
}

void AudioMixer::SetStreamFormat(unsigned int _stream, SAudioFormat& format)
{
    if (_stream < m_num_streams)
    {
        m_streams[_stream].format = format;
    }
}

//#define SHORT_TO_FLOAT 0.000030517578125
void MixSamples(float* _out, int _out_chan, short* _in, int _in_chan, uint32 _count)
{
    for (int i=0; i<_count; i++)
    {
        for (int j=0; j<_out_chan && j<_in_chan; j++)
        {
            float x = (float)_in[j]/32768;
            //_out[j] = clamp((_out[j] + x)/2, -1.0f, 1.0f);
            _out[j] = _out[j] + x;
        }

        _in += _in_chan;
        _out += _out_chan;
    }
}

void ConvertSamples(float* _out, int _out_chan, short* _in, int _in_chan, uint32 _count)
{
    for (int i=0; i<_count; i++)
    {
        for (int j=0; j<_out_chan && j<_in_chan; j++)
        {
            _out[j] = (float)_in[j]/32768;
        }

        _in += _in_chan;
        _out += _out_chan;
    }
}

void ConvertSamplesToIntegers(short* _out, float* _in, uint32 _count, int _channels)
{
    for (int i=0; i<_count; i++)
    {
        for (int j=0; j<_channels; j++)
        {
            _out[j] = (short)(_in[j]*32768);
        }

        _in += _channels;
        _out += _channels;
    }
}

enum { SAMPLES_BUFFER_SIZE = 1024 };

unsigned int  AudioMixer::MixStreams(char* _out_buffer, unsigned int _out_size)
{
    unsigned int mixed_size = 0;
    float samplesf[MAX_NUM_CHANNELS*SAMPLES_BUFFER_SIZE];

    short* obuf = (short*)_out_buffer;
    uint32 sample_stride = (m_format.bps * m_format.chan);
    // output size in samples
    uint32 olength = _out_size / sample_stride; 

    for (int i=0; i<olength/SAMPLES_BUFFER_SIZE; i++)
    {
        memset(samplesf, 0, MAX_NUM_CHANNELS*SAMPLES_BUFFER_SIZE*4);
        uint32 mix_samples = MixStreamsFloat(samplesf, SAMPLES_BUFFER_SIZE);

        if (mix_samples > 0)
        {
            ConvertSamplesToIntegers(obuf, samplesf, mix_samples, m_format.chan);

            obuf += mix_samples * m_format.chan;
            mixed_size += mix_samples * sample_stride;
        }
    }

    return mixed_size;
}

unsigned int  AudioMixer::MixStreamsFloat(float* _out_samples, unsigned int _count)
{
    unsigned int out_length = 0;

    for (unsigned int s=0; s<m_num_streams; s++)
    {
        if (m_streams[s].state != SAudioStream::STATE_OFF && m_streams[s].bytes_left > 0)
        {
            SAudioFormat stream_format = m_streams[s].format;

            int sample_size = (stream_format.chan * stream_format.bps);

            short* buff_in = (short*)(m_streams[s].buffer.ptr+m_streams[s].offset);
            unsigned int stream_length = m_streams[s].bytes_left / sample_size;
            stream_length = min(stream_length, _count);

            MixSamples(_out_samples, m_format.chan, buff_in, stream_format.chan, stream_length);

            out_length = max(out_length, stream_length);

            stream_length = stream_length*sample_size;

            m_streams[s].bytes_left -= stream_length;
            m_streams[s].offset += stream_length;
        }
    }

    return out_length;
}

}}