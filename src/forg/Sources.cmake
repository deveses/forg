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
set(core_includes)
set(core_sources
    BitArray.cpp
    RefCounter.cpp
)
list(TRANSFORM core_sources PREPEND "src/core/")

###############################################################################
# nn
###############################################################################
set(nn_includes
    Matrix.h
    Mnist.h
    Module.h
    Value.h
)
list(TRANSFORM nn_includes PREPEND "include/forg/nn/")
set(nn_sources
    Matrix.cpp
    Mnist.cpp
    Module.cpp
    Value.cpp
)
list(TRANSFORM nn_sources PREPEND "src/nn/")

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
    $<${FORG_PLATFORM_OSX}:osx/File.cpp>
    $<${FORG_PLATFORM_WINDOWS}:win32/File.cpp>
)
list(TRANSFORM os_sources PREPEND "src/os/")

###############################################################################
# io
###############################################################################
set(io_includes
    ISerializer.h
    MemorySerializer.h
)
list(TRANSFORM io_includes PREPEND "include/forg/io/")
set(io_sources
    MemorySerializer.cpp
)
list(TRANSFORM io_sources PREPEND "src/io/")

###############################################################################
# script
###############################################################################
set(script_includes
    generic/Document.h
    lexer.h
    ParserBase.h
    script.h
    xml/XMLParser.h
    xml/XMLSerializer.h
    yaml/YAMLParser.h
    yaml/YAMLSerializer.h
)
list(TRANSFORM script_includes PREPEND "include/forg/script/")
set(script_sources
    generic/Document.cpp
    lexer.cpp
    ParserBase.cpp
    xml/XMLParser.cpp
    xml/XMLSerializer.cpp
    yaml/YAMLParser.cpp
    yaml/YAMLSerializer.cpp
)
list(TRANSFORM script_sources PREPEND "src/script/")

###############################################################################
# net
###############################################################################
set(net_includes
    Command.h
    HttpRequest.h
    CommandQueue.h
    HttpControlServer.h
)
list(TRANSFORM net_includes PREPEND "include/forg/net/")
set(net_sources
    Command.cpp
    HttpRequest.cpp
    CommandQueue.cpp
    HttpControlServer.cpp
    $<${FORG_PLATFORM_WINDOWS}:HttpControlSocketServer_win32.cpp>
    $<$<NOT:$<BOOL:${FORG_PLATFORM_WINDOWS}>>:HttpControlSocketServer_posix.cpp>
)
list(TRANSFORM net_sources PREPEND "src/net/")

###############################################################################
# control
###############################################################################
set(control_includes
    SceneControl.h
)
list(TRANSFORM control_includes PREPEND "include/forg/control/")
set(control_sources
    SceneControl.cpp
    commands/camera.cpp
    commands/input.cpp
    commands/mesh.cpp
    commands/scene.cpp
)
list(TRANSFORM control_sources PREPEND "src/control/")

###############################################################################
# scene
###############################################################################
set(scene_includes
    CameraNode.h
    MeshNode.h
    Model.h
    Scene.h
    SceneNode.h
    TreeNode.h
)
list(TRANSFORM scene_includes PREPEND "include/forg/scene/")
set(scene_sources
    CameraNode.cpp
    MeshNode.cpp
    Model.cpp
    Scene.cpp
    SceneNode.cpp
    TreeNode.cpp
)
list(TRANSFORM scene_sources PREPEND "src/scene/")

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
list(TRANSFORM math_sources PREPEND "src/math/")

###############################################################################
# rendering
###############################################################################
set(rendering_sources
    Camera.cpp
    CameraOrbitController.cpp
    Color.cpp
    Font.cpp
    IRenderDevice.cpp
    IRenderer.cpp
    ISurface.cpp
    ITexture.cpp
    Mesh.cpp
    Ray.cpp
    Sprite.cpp
    Vertex.cpp
    VertexDeclaration.cpp
    VertexElement.cpp
    reference/SWBuffers.cpp
    reference/SWRenderDevice.cpp
)
list(TRANSFORM rendering_sources PREPEND "src/rendering/")

###############################################################################
# ui
###############################################################################
set(ui_includes
    gui.h
)
list(TRANSFORM ui_includes PREPEND "include/forg/ui/")
set(ui_sources
    gui.cpp
)
list(TRANSFORM ui_sources PREPEND "src/ui/")

###############################################################################
# mesh
###############################################################################
set(mesh_sources
    XLoader.cpp
    GLTFLoader.cpp
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
list(TRANSFORM mesh_sources PREPEND "src/mesh/")

###############################################################################
# image
###############################################################################
set(image_sources
    Image.cpp
    bmp/bmp.cpp
    dds/dds.cpp
)
list(TRANSFORM image_sources PREPEND "src/image/")

###############################################################################
# root / debug
###############################################################################
set(root_includes
    include/forg/Input.h
)
set(root_sources
    PerformanceCounter.cpp
    src/Engine.cpp
    src/debug/debug.cpp
)

###############################################################################
list(APPEND all_includes ${audio_includes} ${core_includes} ${nn_includes} ${fs_includes} ${os_includes} ${io_includes} ${script_includes}
    ${net_includes} ${control_includes} ${scene_includes} ${ui_includes})
list(APPEND all_includes ${root_includes})
list(APPEND all_sources ${audio_sources} ${core_sources} ${nn_sources} ${fs_sources} ${os_sources} ${io_sources} ${script_sources}
    ${net_sources} ${control_sources} ${scene_sources} ${math_sources} ${rendering_sources} ${mesh_sources} ${image_sources}
    ${ui_sources} ${root_sources})
