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

#include "base.h"
#include "audio/AudioDefs.h"

namespace forg { namespace audio {

class FORG_API AudioMixer
{
    IAudioOutput* m_output;
    SAudioStream m_streams[10];
    unsigned int m_num_streams;
    SAudioFormat m_format;

public:
    AudioMixer();
    ~AudioMixer();

public:
    bool Init();
    void Shutdown();
    void Update();

    void SetStreamBuffer(unsigned int _stream, char* _buffer, unsigned int size);
    void SetStreamFormat(unsigned int _stream, SAudioFormat& format);

private:
    unsigned int MixStreams(char* _out_buffer, unsigned int _out_size);
    unsigned int MixStreamsFloat(float* _out_samples, unsigned int _count);
};

}}

#endif