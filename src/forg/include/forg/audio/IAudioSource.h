// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
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
