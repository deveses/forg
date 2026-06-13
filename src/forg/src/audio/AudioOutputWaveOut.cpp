#include "forg_pch.h"
#include "audio/AudioOutputWaveOut.h"
#include "debug/dbg.h"

#include <windows.h>

namespace forg { namespace audio {

class AudioOutputWaveOut : public IAudioOutput
{
    HWAVEOUT m_device;    
    WAVEHDR m_header[2];

    unsigned int m_buffer_size;
    SAudioBuffer m_buffers[2];
    unsigned int m_next_buffer;

public:
    AudioOutputWaveOut();
    ~AudioOutputWaveOut();

public:
    bool Init();
    void Close();

    void Release();

    bool CanWrite();
    void Write(char* data, unsigned int size);
    
    void Unprepare();

    void Update();

    static void CALLBACK CallbackProc(
        HWAVEOUT hwo,
        UINT uMsg,
        DWORD_PTR dwInstance,
        DWORD_PTR dwParam1,
        DWORD_PTR dwParam2
    );

private:
};

IAudioOutput* CreateAudioOutputWaveOut()
{
    return new AudioOutputWaveOut();
}

///////////////////////////////////////////////////////////////////////////////

AudioOutputWaveOut::AudioOutputWaveOut()
    : m_device(NULL)
{
    m_header[0].lpData = 0; 
    m_header[0].dwBufferLength = 0; 
    m_header[0].dwFlags = 0L; 
    m_header[0].dwLoops = 0L; 

    m_header[1] = m_header[0];

    m_buffers[0].ptr = 0;
    m_buffers[1].ptr = 0;
}

AudioOutputWaveOut::~AudioOutputWaveOut()
{
    Close();
}


bool AudioOutputWaveOut::Init()
{
    WAVEFORMATEX fmt;

    fmt.cbSize = 0;
    fmt.nSamplesPerSec = 44100;
    fmt.nChannels = 2;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = fmt.nChannels*(fmt.wBitsPerSample/8);
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec*fmt.nBlockAlign;
    fmt.wFormatTag = WAVE_FORMAT_PCM;

    m_buffer_size = fmt.nAvgBytesPerSec;
    m_buffers[0].ptr = new char[m_buffer_size];
    m_buffers[0].size = 0;
    m_buffers[1].ptr = new char[m_buffer_size];
    m_buffers[1].size = 0;

    m_next_buffer = 0;

    if (MMSYSERR_NOERROR == waveOutOpen(&m_device, WAVE_MAPPER, &fmt, (DWORD_PTR)CallbackProc, (DWORD_PTR)this, CALLBACK_FUNCTION))
    {
            m_header[0].lpData = m_buffers[0].ptr; 
            m_header[0].dwBufferLength = m_buffer_size; 
            m_header[0].dwFlags = 0L; 
            m_header[0].dwLoops = 0L; 

            if (MMSYSERR_NOERROR != waveOutPrepareHeader(m_device, &m_header[0], sizeof(WAVEHDR)))
            {
                DBG_MSG("waveOutPrepareHeader() failed.\n");
            }

            m_header[1].lpData = m_buffers[1].ptr; 
            m_header[1].dwBufferLength = m_buffer_size; 
            m_header[1].dwFlags = 0L; 
            m_header[1].dwLoops = 0L; 

            if (MMSYSERR_NOERROR != waveOutPrepareHeader(m_device, &m_header[1], sizeof(WAVEHDR))) 
            {
                DBG_MSG("waveOutPrepareHeader() failed.\n");
            }

        return true;
    } else
    {
        DBG_MSG("waveOutOpen() failed.\n");
    }

    return false;
}

void AudioOutputWaveOut::Close()
{
    if (m_device != NULL)
    {
        if (m_header[0].dwFlags != 0 && m_header[0].dwFlags != WHDR_DONE)
        {
            waveOutUnprepareHeader(m_device, &m_header[0], sizeof(WAVEHDR)); 
        }

        if (m_header[1].dwFlags != 0 && m_header[1].dwFlags != WHDR_DONE)
        {
            waveOutUnprepareHeader(m_device, &m_header[1], sizeof(WAVEHDR)); 
        }

        if (MMSYSERR_NOERROR != waveOutClose(m_device))
        {
            DBG_MSG("waveOutClose() failed.\n");
        }
    }

    delete [] m_buffers[0].ptr;
    delete [] m_buffers[1].ptr;
}

void AudioOutputWaveOut::Release()
{
    delete this;
}

bool AudioOutputWaveOut::CanWrite()
{
    if ((m_header[m_next_buffer].dwFlags != WHDR_PREPARED) && ((m_header[m_next_buffer].dwFlags&WHDR_DONE) != WHDR_DONE))
    {
        return false;
    }

    return true;
}

void AudioOutputWaveOut::Write(char* data, unsigned int size)
{
    if (CanWrite())
    {
        unsigned int len = min(size, m_buffer_size);
        memcpy(m_buffers[m_next_buffer].ptr, data, len);

        m_header[m_next_buffer].dwBufferLength = len;
        m_header[m_next_buffer].dwFlags &= ~WHDR_DONE;

        MMRESULT res = waveOutWrite(m_device, &m_header[m_next_buffer], sizeof(WAVEHDR));
        if (MMSYSERR_NOERROR != res)
        {
            DBG_MSG("waveOutWrite() failed (%d).\n", res);
        }

        m_next_buffer = (m_next_buffer+1)%2;
    }
}

void AudioOutputWaveOut::Unprepare()
{
    /*
    waveOutPrepareHeader(m_device, &m_header[0], sizeof(WAVEHDR)); 
    m_header[0].lpData = 0; 
    m_header[0].dwBufferLength = 0; 
    m_header[0].dwFlags = 0L; 
    */
}

void AudioOutputWaveOut::CallbackProc(HWAVEOUT hwo,
        UINT uMsg,
        DWORD_PTR dwInstance,
        DWORD_PTR dwParam1,
        DWORD_PTR dwParam2)
{
    if (uMsg == MM_WOM_DONE)
    {
        //((AudioOutputWaveOut*)dwInstance)->Unprepare();
        //((AudioOutputWaveOut*)dwInstance)->Update();
    }
}

}}