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

###############################################################################
# fs
###############################################################################
set(fs_includes
    File.h
    Filesystem.h
)
list(TRANSFORM fs_includes PREPEND "include/forg/fs/")
set(fs_sources
    File.cpp
)
list(TRANSFORM fs_sources PREPEND "src/fs/")

###############################################################################
# os
###############################################################################
set(os_includes
    File.h
    ILibrary.h
)
list(TRANSFORM os_includes PREPEND "include/forg/os/")
set(os_sources
    osx/File.cpp
    win32/File.cpp
)
list(TRANSFORM os_sources PREPEND "src/os/")

###############################################################################
# script
###############################################################################
set(script_includes
    lexer.h
    script.h
    xml/XMLParser.h
    xml/XMLSerializer.h
)
list(TRANSFORM script_includes PREPEND "include/forg/script/")
set(script_sources
    lexer.cpp
    xml/XMLParser.cpp
    xml/XMLSerializer.cpp
)
list(TRANSFORM script_sources PREPEND "src/script/")

###############################################################################
list(APPEND all_includes ${audio_includes} ${core_includes} ${fs_includes} ${os_includes} ${script_includes})
list(APPEND all_sources ${audio_sources} ${core_sources} ${fs_sources} ${os_sources} ${script_sources})
