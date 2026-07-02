add_library(swrenderer SHARED)
forg_configure_target(swrenderer)
target_sources(swrenderer
    PRIVATE
    SWRenderer.cpp
    $<$<BOOL:${APPLE}>:SWRenderDevice_osx.mm>
    $<$<BOOL:${WIN32}>:SWRenderDevice_win32.cpp>
    $<$<BOOL:${WIN32}>:main.cpp>
    $<$<BOOL:${FORG_PLATFORM_LINUX}>:SWRenderDevice_sdl.cpp>
)
target_link_libraries(swrenderer
    PRIVATE
    forg
    $<$<BOOL:${APPLE}>:$<LINK_LIBRARY:FRAMEWORK,Cocoa>>
    $<$<BOOL:${APPLE}>:$<LINK_LIBRARY:FRAMEWORK,QuartzCore>>
    $<$<BOOL:${WIN32}>:gdi32>
)
if(FORG_PLATFORM_LINUX)
    target_link_libraries(swrenderer PRIVATE ${FORG_SDL2_LIBRARIES})
    if(FORG_SDL2_INCLUDE_DIRS)
        target_include_directories(swrenderer PRIVATE ${FORG_SDL2_INCLUDE_DIRS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(swrenderer PRIVATE SWRENDERER_EXPORTS)
endif()
