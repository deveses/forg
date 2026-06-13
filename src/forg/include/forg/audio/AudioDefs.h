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

#ifndef _FORG_AUDIO_AUDIODEFS_H_
#define _FORG_AUDIO_AUDIODEFS_H_

#if _MSC_VER > 1000
#pragma once
#endif

namespace forg { namespace audio {

enum
{
    MAX_NUM_CHANNELS = 8,
};

struct SAudioFormat
{
    int freq;   // samples per second
    int bps;    // bytes per sample
    int chan;   // channels
};

struct SAudioBuffer
{
    char* ptr;
    unsigned int size;

    SAudioFormat format;
};

struct SAudioStream
{
    enum EAudioStreamState
    {
        STATE_OFF = 0,
        STATE_ON
    };

    SAudioBuffer buffer;
    SAudioFormat format;

    unsigned int offset;
    unsigned int bytes_left;

    unsigned int state;
};

class IAudioOutput
{
public:
    virtual bool CanWrite() = 0;
    virtual void Write(char* data, unsigned int size) = 0;

    virtual bool Init() = 0;
    virtual void Release() = 0;
};

}}

#endif