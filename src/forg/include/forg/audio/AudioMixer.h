// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/api.h"

#include <memory>

namespace forg::audio {

class IAudioOutput;
class IAudioSource;
struct SAudioFormat;

class FORG_API AudioMixer
{
  public:
    static constexpr unsigned int MAX_STREAMS = 10;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

  public:
    AudioMixer();
    ~AudioMixer();

  public:
    bool Init();
    // Takes ownership of output and releases it through
    // IAudioOutput::Release().
    bool InitWithOutput(IAudioOutput* output);
    void Shutdown();
    // Mixes all active streams (buffer- and source-driven) into 16-bit
    // stereo PCM and pushes it to the output whenever it can accept data.
    // Pushes up to one second of audio per call, so gain/pan changes can
    // lag by up to that much.
    void Update();

    // Hands a raw PCM buffer to the stream (legacy push path). Detaches
    // any pull source and resets the stream's gain/pan to defaults.
    void SetStreamBuffer(unsigned int _stream, char* _buffer,
                         unsigned int size);
    void SetStreamFormat(unsigned int _stream, const SAudioFormat& format);

    // Attaches a pull source to the stream; shared ownership keeps it alive
    // while the stream is attached. Passing nullptr detaches the source and
    // stops the stream.
    void SetStreamSource(unsigned int _stream,
                         std::shared_ptr<IAudioSource> source, bool looping);
    // gain in [0, 1], pan in [-1, 1] (-1 = left, 1 = right).
    void SetStreamGainPan(unsigned int _stream, float gain, float pan);
    // True while the stream still produces audio; a drained buffer or a
    // finished non-looping source counts as inactive so the slot can be
    // reused.
    bool IsStreamActive(unsigned int _stream) const;
};

} // namespace forg::audio
