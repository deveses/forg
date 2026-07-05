// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/audio/AudioManager.h"
#include "forg/base.h"

namespace forg::audio {

class FORG_API AudioEngine
{
    AudioManager m_manager;

  public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool Init();
    void Shutdown();
    void Update();

    bool IsInitialized() const;
    AudioManager& Manager();
    const AudioManager& Manager() const;
};

} // namespace forg::audio
