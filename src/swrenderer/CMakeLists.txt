add_library(swrenderer SHARED)
forg_configure_target(swrenderer)
target_sources(swrenderer
    PRIVATE
    SWRenderer.cpp
    $<$<BOOL:${APPLE}>:SWRenderDevice_osx.mm>
    $<$<BOOL:${WIN32}>:SWRenderDevice.cpp>
    $<$<BOOL:${WIN32}>:main.cpp>
)
target_link_libraries(swrenderer
    PRIVATE
    forg
    $<$<BOOL:${APPLE}>:$<LINK_LIBRARY:FRAMEWORK,Cocoa>>
    $<$<BOOL:${APPLE}>:$<LINK_LIBRARY:FRAMEWORK,QuartzCore>>
    $<$<BOOL:${WIN32}>:gdi32>
)
if(WIN32)
    target_compile_definitions(swrenderer PRIVATE SWRENDERER_EXPORTS)
endif()
