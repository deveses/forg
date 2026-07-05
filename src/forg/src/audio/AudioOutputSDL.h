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
#include "forg/audio/AudioDefs.h"

namespace forg::audio {

// Creates the SDL2 audio output (Linux default): a push/queue model where
// CanWrite() reports queue headroom and Write() enqueues PCM via
// SDL_QueueAudio; no callback is involved.
IAudioOutput* CreateAudioOutputSDL();

} // namespace forg::audio
