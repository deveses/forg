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
