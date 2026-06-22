add_library(metalrenderer SHARED)
forg_configure_target(metalrenderer)
target_sources(metalrenderer
    PRIVATE
    MetalRenderer.cpp
    MetalRenderDevice.mm
    MetalBuffers.mm
)
target_link_libraries(metalrenderer
    PRIVATE
    forg
    $<LINK_LIBRARY:FRAMEWORK,Metal>
    $<LINK_LIBRARY:FRAMEWORK,QuartzCore>
    $<LINK_LIBRARY:FRAMEWORK,Cocoa>
)
