add_library(glrenderer SHARED
    GLDeviceCaps.cpp
    GLFunc.cpp
    GLIndexBuffer.cpp
    GLRenderDevice.cpp
    GLRenderer.cpp
    GLTexture.cpp
    GLVertexBuffer.cpp
    main.cpp)

forg_configure_target(glrenderer)
target_include_directories(glrenderer PRIVATE ${PROJECT_SOURCE_DIR}/extern/opengl)
target_compile_definitions(glrenderer PRIVATE GLRENDERER_EXPORTS NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(glrenderer PRIVATE forg opengl32)
