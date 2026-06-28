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
endif()
