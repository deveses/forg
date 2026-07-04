#include "audio/AudioOutputSDL.h"
#include "debug/dbg.h"
#include "forg_pch.h"

#include <algorithm>

#include <SDL.h>

namespace forg::audio {

namespace {

constexpr unsigned int AUDIO_OUTPUT_SAMPLE_RATE = 44100;
constexpr unsigned int AUDIO_OUTPUT_CHANNELS = 2;
constexpr unsigned int AUDIO_OUTPUT_BYTES_PER_SAMPLE = 2;
constexpr unsigned int AUDIO_OUTPUT_BYTES_PER_FRAME =
    AUDIO_OUTPUT_CHANNELS * AUDIO_OUTPUT_BYTES_PER_SAMPLE;
constexpr unsigned int AUDIO_OUTPUT_BUFFER_SIZE =
    AUDIO_OUTPUT_SAMPLE_RATE * AUDIO_OUTPUT_BYTES_PER_FRAME;

} // namespace

class AudioOutputSDL final : public IAudioOutput
{
    SDL_AudioDeviceID m_device;
    unsigned int m_buffer_size;
    bool m_audio_initialized;
    bool m_playback_started;

  public:
    AudioOutputSDL();
    ~AudioOutputSDL();

    bool Init();
    void Close();

    void Release();

    bool CanWrite();
    void Write(char* data, unsigned int size);
};

IAudioOutput* CreateAudioOutputSDL() { return new AudioOutputSDL(); }

///////////////////////////////////////////////////////////////////////////////

AudioOutputSDL::AudioOutputSDL()
    : m_device(0), m_buffer_size(AUDIO_OUTPUT_BUFFER_SIZE),
      m_audio_initialized(false), m_playback_started(false)
{
}

AudioOutputSDL::~AudioOutputSDL() { Close(); }

bool AudioOutputSDL::Init()
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        DBG_MSG("SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n",
                SDL_GetError());
        return false;
    }

    m_audio_initialized = true;

    SDL_AudioSpec desired = {};
    desired.freq = AUDIO_OUTPUT_SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = AUDIO_OUTPUT_CHANNELS;
    desired.samples = 4096;
    desired.callback = nullptr;

    SDL_AudioSpec obtained = {};
    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_device == 0)
    {
        DBG_MSG("SDL_OpenAudioDevice() failed: %s\n", SDL_GetError());
        Close();
        return false;
    }

    DBG_MSG("AudioOutputSDL: Opened audio device with %d Hz, %d channels, %d "
            "bytes per sample\n",
            obtained.freq, obtained.channels,
            SDL_AUDIO_BITSIZE(obtained.format) / 8);

    m_playback_started = false;
    return true;
}

void AudioOutputSDL::Close()
{
    if (m_device != 0)
    {
        SDL_PauseAudioDevice(m_device, 1);
        SDL_ClearQueuedAudio(m_device);
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }

    if (m_audio_initialized)
    {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        m_audio_initialized = false;
    }

    m_playback_started = false;
}

void AudioOutputSDL::Release() { delete this; }

bool AudioOutputSDL::CanWrite()
{
    if (m_device == 0)
        return false;

    return SDL_GetQueuedAudioSize(m_device) <= m_buffer_size;
}

void AudioOutputSDL::Write(char* data, unsigned int size)
{
    if (!CanWrite())
        return;

    const unsigned int len = std::min(size, m_buffer_size);
    if (SDL_QueueAudio(m_device, data, len) != 0)
    {
        DBG_MSG("SDL_QueueAudio() failed: %s\n", SDL_GetError());
        return;
    }

    if (!m_playback_started)
    {
        SDL_PauseAudioDevice(m_device, 0);
        m_playback_started = true;
    }
}

} // namespace forg::audio
