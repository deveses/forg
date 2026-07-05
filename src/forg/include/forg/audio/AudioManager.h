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

#ifndef _FORG_AUDIO_AUDIOMANAGER_H_
#define _FORG_AUDIO_AUDIOMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/audio/AudioMixer.h"
#include "forg/base.h"

namespace forg::audio {

class FORG_API AudioManager
{
    AudioMixer m_mixer;
    IAudioSource* m_voices[AudioMixer::MAX_STREAMS];
    bool m_initialized;

  public:
    static constexpr int INVALID_VOICE = -1;

    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool Init();
    // Takes ownership of output and releases it through
    // IAudioOutput::Release().
    bool InitWithOutput(IAudioOutput* output);
    void Shutdown();
    void Update();

    bool IsInitialized() const;
    AudioMixer& Mixer();
    const AudioMixer& Mixer() const;

    // Starts playing a source on a free voice and returns its handle,
    // or INVALID_VOICE when no voice is free. The source is not owned;
    // callers must Stop() the voice (or StopAll()) before destroying it.
    int Play(IAudioSource* source, bool looping = false, float gain = 1.0f,
             float pan = 0.0f);
    void Stop(int voice);
    void StopAll();
    bool IsPlaying(int voice) const;
    void SetGainPan(int voice, float gain, float pan);
};

} // namespace forg::audio

#endif
