include(FetchContent)

# Vendored, header-only third-party dependencies.

# cgltf - single-header glTF 2.0 parser (MIT). CGLTF_IMPLEMENTATION is defined
# in exactly one translation unit (src/forg/include/forg/mesh/GLTFLoader.cpp).
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cgltf)

if(FORG_USE_FREETYPE)
    # FreeType is used by forg::Font. Keep optional image/font compression
    # integrations disabled so enabling text rendering does not pull extra
    # third-party dependencies into the build.
    find_package(Freetype QUIET)

    if(FREETYPE_FOUND)
        if(TARGET Freetype::Freetype)
            set_property(TARGET Freetype::Freetype PROPERTY IMPORTED_GLOBAL TRUE)
            set(FORG_FREETYPE_LINK_TARGET Freetype::Freetype PARENT_SCOPE)
        else()
            add_library(Freetype::Freetype UNKNOWN IMPORTED GLOBAL)
            set_target_properties(Freetype::Freetype PROPERTIES
                IMPORTED_LOCATION "${FREETYPE_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIRS}")
            set(FORG_FREETYPE_LINK_TARGET Freetype::Freetype PARENT_SCOPE)
        endif()
    else()
        set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)

        FetchContent_Declare(
            freetype
            GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
            GIT_TAG VER-2-13-3
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(freetype)
        set(FORG_FREETYPE_LINK_TARGET freetype PARENT_SCOPE)
        set(FORG_FREETYPE_INCLUDE_DIRS
            ${freetype_SOURCE_DIR}/include
            ${freetype_BINARY_DIR}/include
            PARENT_SCOPE)
    endif()
endif()

if(FORG_USE_ZLIB)
    find_package(ZLIB QUIET)

    if(ZLIB_FOUND)
        if(TARGET ZLIB::ZLIB)
            set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_GLOBAL TRUE)
        else()
            add_library(ZLIB::ZLIB UNKNOWN IMPORTED GLOBAL)
            set_target_properties(ZLIB::ZLIB PROPERTIES
                IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}")
        endif()
        set(FORG_ZLIB_LINK_TARGET ZLIB::ZLIB PARENT_SCOPE)
    else()
        set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            zlib
            GIT_REPOSITORY https://github.com/madler/zlib.git
            GIT_TAG v1.3.1
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(zlib)

        if(TARGET zlibstatic)
            set(FORG_ZLIB_LINK_TARGET zlibstatic PARENT_SCOPE)
        elseif(TARGET zlib)
            set(FORG_ZLIB_LINK_TARGET zlib PARENT_SCOPE)
        endif()
        set(FORG_ZLIB_INCLUDE_DIRS
            ${zlib_SOURCE_DIR}
            ${zlib_BINARY_DIR}
            PARENT_SCOPE)
    endif()
endif()
