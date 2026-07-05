/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef _FORG_AUDIO_AUDIOMIXER_H_
#define _FORG_AUDIO_AUDIOMIXER_H_

#if _MSC_VER > 1000
#pragma once
#endif

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

#endif
