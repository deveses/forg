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

#ifndef _FORG_AUDIO_IAUDIOSOURCE_H_
#define _FORG_AUDIO_IAUDIOSOURCE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/base.h"

namespace forg::audio {

// Pull-based producer of interleaved signed 16-bit PCM at 44100 Hz
// (the AudioMixer stream format).
class FORG_API IAudioSource
{
  public:
    virtual ~IAudioSource() = default;

    // Fills up to 'frames' frames (Channels() samples each).
    // Returns the number of frames written; fewer than 'frames'
    // only at the end of data.
    virtual unsigned int Read(short* samples, unsigned int frames) = 0;

    // Number of interleaved channels: 1 or 2.
    virtual int Channels() const = 0;

    // True once the source has no more data; endless sources
    // (generators) always return false.
    virtual bool IsFinished() const = 0;

    // Rewinds the source to its start.
    virtual void Reset() = 0;
};

} // namespace forg::audio

#endif
