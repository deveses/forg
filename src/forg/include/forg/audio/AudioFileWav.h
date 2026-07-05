// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/audio/AudioFile.h"
#include "forg/base.h"

namespace forg::audio {

// Loads PCM WAV files at the mixer rate (44100 Hz, 1-2 channels,
// 8 or 16 bits per sample); other formats are rejected by Open().
class FORG_API AudioFileWav : public AudioFile
{
  public:
    bool Open(std::string_view filename) override;
};

} // namespace forg::audio
