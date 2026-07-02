#include "audio/AudioOutput.h"
#include "forg_pch.h"

#ifdef FORG_PLATFORM_WINDOWS
#include "audio/AudioOutputWaveOut.h"
#endif

#ifdef FORG_PLATFORM_OSX
#include "audio/AudioOutputCoreAudio.h"
#endif

namespace forg::audio {

IAudioOutput* CreateDefaultAudioOutput()
{
#ifdef FORG_PLATFORM_WINDOWS
    return CreateAudioOutputWaveOut();
#elif defined(FORG_PLATFORM_OSX)
    return CreateAudioOutputCoreAudio();
#else
    return nullptr;
#endif
}

} // namespace forg::audio
