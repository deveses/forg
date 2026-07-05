// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/audio/AudioDefs.h"

namespace forg::audio {

// Creates the SDL2 audio output (Linux default): a push/queue model where
// CanWrite() reports queue headroom and Write() enqueues PCM via
// SDL_QueueAudio; no callback is involved.
IAudioOutput* CreateAudioOutputSDL();

} // namespace forg::audio
