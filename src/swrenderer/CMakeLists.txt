add_library(swrenderer SHARED)
target_sources(swrenderer
    PRIVATE
    SWRenderer.cpp
    SWRenderDevice_osx.mm
)
target_link_libraries(swrenderer
    PRIVATE
    forg
    $<LINK_LIBRARY:FRAMEWORK,Cocoa>
    $<LINK_LIBRARY:FRAMEWORK,QuartzCore>
)
