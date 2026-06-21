
add_library(forg STATIC)
add_library(Forg::forg ALIAS forg)

include(Sources.cmake)

###############################################################################

target_sources(forg 
    PRIVATE ${all_includes} ${all_sources}
)

if(FORG_ENABLE_PCH)
    target_precompile_headers(forg PRIVATE src/forg_pch.h)
endif()
target_include_directories(forg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/forg>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/forg>
    PRIVATE
        src
        include/forg/mesh/ply
        include/forg/mesh/xfile
        include/forg/image/bmp
        include/forg/image/dds
)
# cgltf (header-only) is used privately by the glTF mesh loader.
target_include_directories(forg PRIVATE ${PROJECT_SOURCE_DIR}/extern/cgltf)
forg_configure_target(forg)

if(WIN32)
    target_compile_definitions(forg PUBLIC FORG_STATIC NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(forg PRIVATE winmm ws2_32)
endif()

set_target_properties(forg PROPERTIES EXPORT_NAME forg)
install(TARGETS forg
    EXPORT ForgTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
