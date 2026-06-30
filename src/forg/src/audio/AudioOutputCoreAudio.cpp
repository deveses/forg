#include "audio/AudioOutputCoreAudio.h"
#include "debug/dbg.h"
#include "forg_pch.h"

#include <algorithm>
#include <atomic>
#include <cstring>

#include <AudioToolbox/AudioToolbox.h>

namespace forg::audio {

namespace {

constexpr unsigned int AUDIO_OUTPUT_BUFFER_COUNT = 2;
constexpr unsigned int AUDIO_OUTPUT_SAMPLE_RATE = 44100;
constexpr unsigned int AUDIO_OUTPUT_CHANNELS = 2;
constexpr unsigned int AUDIO_OUTPUT_BYTES_PER_SAMPLE = 2;
constexpr unsigned int AUDIO_OUTPUT_BYTES_PER_FRAME =
    AUDIO_OUTPUT_CHANNELS * AUDIO_OUTPUT_BYTES_PER_SAMPLE;
constexpr unsigned int AUDIO_OUTPUT_BUFFER_SIZE =
    AUDIO_OUTPUT_SAMPLE_RATE * AUDIO_OUTPUT_BYTES_PER_FRAME;

} // namespace

class AudioOutputCoreAudio final : public IAudioOutput
{
    AudioQueueRef m_queue;
    AudioQueueBufferRef m_buffers[AUDIO_OUTPUT_BUFFER_COUNT];
    std::atomic_bool m_buffer_available[AUDIO_OUTPUT_BUFFER_COUNT];
    unsigned int m_next_buffer;
    unsigned int m_buffer_size;
    bool m_started;

  public:
    AudioOutputCoreAudio();
    ~AudioOutputCoreAudio();

    bool Init();
    void Close();

    void Release();

    bool CanWrite();
    void Write(char* data, unsigned int size);

    static void OutputCallback(void* userData, AudioQueueRef queue,
                               AudioQueueBufferRef buffer);
};

IAudioOutput* CreateAudioOutputCoreAudio() { return new AudioOutputCoreAudio(); }

///////////////////////////////////////////////////////////////////////////////

AudioOutputCoreAudio::AudioOutputCoreAudio()
    : m_queue(nullptr), m_next_buffer(0), m_buffer_size(AUDIO_OUTPUT_BUFFER_SIZE),
      m_started(false)
{
    for (unsigned int i = 0; i < AUDIO_OUTPUT_BUFFER_COUNT; ++i)
    {
        m_buffers[i] = nullptr;
        m_buffer_available[i].store(true);
    }
}

AudioOutputCoreAudio::~AudioOutputCoreAudio() { Close(); }

bool AudioOutputCoreAudio::Init()
{
    AudioStreamBasicDescription fmt = {};
    fmt.mSampleRate = AUDIO_OUTPUT_SAMPLE_RATE;
    fmt.mFormatID = kAudioFormatLinearPCM;
    fmt.mFormatFlags =
        kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    fmt.mBytesPerPacket = AUDIO_OUTPUT_BYTES_PER_FRAME;
    fmt.mFramesPerPacket = 1;
    fmt.mBytesPerFrame = AUDIO_OUTPUT_BYTES_PER_FRAME;
    fmt.mChannelsPerFrame = AUDIO_OUTPUT_CHANNELS;
    fmt.mBitsPerChannel = AUDIO_OUTPUT_BYTES_PER_SAMPLE * 8;

    OSStatus status = AudioQueueNewOutput(&fmt, OutputCallback, this, nullptr,
                                          nullptr, 0, &m_queue);
    if (status != noErr)
    {
        DBG_MSG("AudioQueueNewOutput() failed (%d).\n", status);
        return false;
    }

    for (unsigned int i = 0; i < AUDIO_OUTPUT_BUFFER_COUNT; ++i)
    {
        status =
            AudioQueueAllocateBuffer(m_queue, m_buffer_size, &m_buffers[i]);
        if (status != noErr)
        {
            DBG_MSG("AudioQueueAllocateBuffer() failed (%d).\n", status);
            Close();
            return false;
        }
        m_buffer_available[i].store(true);
    }

    m_next_buffer = 0;
    m_started = false;
    return true;
}

void AudioOutputCoreAudio::Close()
{
    if (m_queue != nullptr)
    {
        if (m_started)
            AudioQueueStop(m_queue, true);

        for (unsigned int i = 0; i < AUDIO_OUTPUT_BUFFER_COUNT; ++i)
        {
            if (m_buffers[i] != nullptr)
            {
                AudioQueueFreeBuffer(m_queue, m_buffers[i]);
                m_buffers[i] = nullptr;
            }
            m_buffer_available[i].store(true);
        }

        AudioQueueDispose(m_queue, true);
        m_queue = nullptr;
    }

    m_next_buffer = 0;
    m_started = false;
}

void AudioOutputCoreAudio::Release() { delete this; }

bool AudioOutputCoreAudio::CanWrite()
{
    if (m_queue == nullptr || m_buffers[m_next_buffer] == nullptr)
        return false;

    return m_buffer_available[m_next_buffer].load();
}

void AudioOutputCoreAudio::Write(char* data, unsigned int size)
{
    if (!CanWrite())
        return;

    AudioQueueBufferRef buffer = m_buffers[m_next_buffer];
    const unsigned int len = std::min(size, m_buffer_size);
    std::memcpy(buffer->mAudioData, data, len);
    buffer->mAudioDataByteSize = len;

    m_buffer_available[m_next_buffer].store(false);
    OSStatus status = AudioQueueEnqueueBuffer(m_queue, buffer, 0, nullptr);
    if (status != noErr)
    {
        m_buffer_available[m_next_buffer].store(true);
        DBG_MSG("AudioQueueEnqueueBuffer() failed (%d).\n", status);
        return;
    }

    if (!m_started)
    {
        status = AudioQueueStart(m_queue, nullptr);
        if (status != noErr)
        {
            DBG_MSG("AudioQueueStart() failed (%d).\n", status);
            return;
        }
        m_started = true;
    }

    m_next_buffer = (m_next_buffer + 1) % AUDIO_OUTPUT_BUFFER_COUNT;
}

void AudioOutputCoreAudio::OutputCallback(void* userData, AudioQueueRef,
                                          AudioQueueBufferRef buffer)
{
    AudioOutputCoreAudio* output =
        static_cast<AudioOutputCoreAudio*>(userData);
    if (output == nullptr)
        return;

    for (unsigned int i = 0; i < AUDIO_OUTPUT_BUFFER_COUNT; ++i)
    {
        if (output->m_buffers[i] == buffer)
        {
            output->m_buffer_available[i].store(true);
            return;
        }
    }
}

} // namespace forg::audio
