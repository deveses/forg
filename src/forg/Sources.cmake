###############################################################################
# audio
###############################################################################
set(audio_includes
    AudioDefs.h
    AudioDSP.h
    AudioMixer.h
    WaveFile.h
)
list(TRANSFORM audio_includes PREPEND "include/forg/audio/")
set(audio_sources
    AudioDSP.cpp
    AudioMixer.cpp
    $<${FORG_PLATFORM_WINDOWS}:AudioOutputWaveOut.h>
    $<${FORG_PLATFORM_WINDOWS}:AudioOutputWaveOut.cpp>
    WaveFile.cpp
)
list(TRANSFORM audio_sources PREPEND "src/audio/")

###############################################################################
# core
###############################################################################
set(core_includes
    auto_ptr.hpp
)
list(TRANSFORM core_includes PREPEND "include/forg/core/")
set(core_sources
    BitArray.cpp
)
list(TRANSFORM core_sources PREPEND "src/core/")
