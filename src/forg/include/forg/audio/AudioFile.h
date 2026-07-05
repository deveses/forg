// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include <string_view>
#include <vector>

#include "forg/audio/IAudioSource.h"
#include "forg/base.h"

namespace forg::audio {

// Base class for decoded audio files. Derived classes implement Open()
// to fill m_samples with interleaved 16-bit PCM at 44100 Hz; the pull
// interface is served from that buffer.
class FORG_API AudioFile : public IAudioSource
{
  protected:
    std::vector<short> m_samples;
    int m_channels = 0;
    unsigned int m_position = 0;

  public:
    virtual bool Open(std::string_view filename) = 0;
    void Close();

    bool IsOpen() const;
    unsigned int FrameCount() const;

    unsigned int Read(short* samples, unsigned int frames) override;
    int Channels() const override;
    bool IsFinished() const override;
    void Reset() override;
};

} // namespace forg::audio
