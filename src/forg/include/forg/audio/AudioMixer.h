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

#include "forg/audio/AudioDefs.h"
#include "forg/audio/IAudioSource.h"
#include "forg/base.h"

namespace forg::audio {

class FORG_API AudioMixer
{
  public:
    static constexpr unsigned int MAX_STREAMS = 10;

  private:
    struct SStreamSource
    {
        IAudioSource* source;
        float gain;
        float pan;
        bool looping;
    };

    IAudioOutput* m_output;
    SAudioStream m_streams[MAX_STREAMS];
    SStreamSource m_sources[MAX_STREAMS];
    unsigned int m_num_streams;
    SAudioFormat m_format;

  public:
    AudioMixer();
    ~AudioMixer();

  public:
    bool Init();
    // Takes ownership of output and releases it through
    // IAudioOutput::Release().
    bool InitWithOutput(IAudioOutput* output);
    void Shutdown();
    void Update();

    void SetStreamBuffer(unsigned int _stream, char* _buffer,
                         unsigned int size);
    void SetStreamFormat(unsigned int _stream, const SAudioFormat& format);

    // Attaches a pull source to the stream; the mixer does not take
    // ownership. Passing nullptr detaches the source and stops the stream.
    void SetStreamSource(unsigned int _stream, IAudioSource* source,
                         bool looping);
    // gain in [0, 1], pan in [-1, 1] (-1 = left, 1 = right).
    void SetStreamGainPan(unsigned int _stream, float gain, float pan);
    bool IsStreamActive(unsigned int _stream) const;

  private:
    unsigned int MixStreams(char* _out_buffer, unsigned int _out_size);
    unsigned int MixStreamsFloat(float* _out_samples, unsigned int _count);
};

} // namespace forg::audio

#endif
