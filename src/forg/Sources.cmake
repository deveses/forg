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
    RefCounter.cpp
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
# math
###############################################################################
set(math_sources
    Math.cpp
    Matrix4.cpp
    Plane.cpp
    Quaternion.cpp
    Vector2.cpp
    Vector3.cpp
    Vector4.cpp
)
list(TRANSFORM math_sources PREPEND "include/forg/math/")

###############################################################################
# rendering
###############################################################################
set(rendering_sources
    Camera.cpp
    Color.cpp
    IRenderDevice.cpp
    IRenderer.cpp
    ISurface.cpp
    ITexture.cpp
    Mesh.cpp
    Ray.cpp
    Vertex.cpp
    VertexDeclaration.cpp
    VertexElement.cpp
    reference/SWBuffers.cpp
    reference/SWRenderDevice.cpp
)
list(TRANSFORM rendering_sources PREPEND "include/forg/rendering/")

###############################################################################
# mesh
###############################################################################
set(mesh_sources
    XLoader.cpp
    ply/plyfile.cpp
    xfile/xbinreader.cpp
    xfile/xbzipreader.cpp
    xfile/xdata.cpp
    xfile/xdatamgr.cpp
    xfile/xdefs.cpp
    xfile/xfile.cpp
    xfile/xlexer.cpp
    xfile/xreader.cpp
    xfile/xstdtemplates.cpp
    xfile/xtemplate.cpp
    xfile/xtemplatesmgr.cpp
    xfile/xtexreader.cpp
    xfile/xtmembers.cpp
)
list(TRANSFORM mesh_sources PREPEND "include/forg/mesh/")

###############################################################################
# image
###############################################################################
set(image_sources
    Image.cpp
    bmp/bmp.cpp
    dds/dds.cpp
)
list(TRANSFORM image_sources PREPEND "include/forg/image/")

###############################################################################
# root / debug
###############################################################################
set(root_sources
    PerformanceCounter.cpp
    src/debug/debug.cpp
)

###############################################################################
list(APPEND all_includes ${audio_includes} ${core_includes} ${fs_includes} ${os_includes} ${script_includes})
list(APPEND all_sources ${audio_sources} ${core_sources} ${fs_sources} ${os_sources} ${script_sources}
    ${math_sources} ${rendering_sources} ${mesh_sources} ${image_sources} ${root_sources})
