cmake_minimum_required(VERSION 3.1...3.26)

project(
  Forg
  VERSION 1.0
  LANGUAGES CXX)

option(FORG_USE_OPENCL "Use OpenCL renderer" OFF)
option(FORG_USE_FREETYPE "Use Freetype library" OFF)
option(FORG_USE_ZLIB "Use ZLib library" OFF)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

set(FORG_PLATFORM_WINDOWS 0)

if(WIN32)
    set(FORG_PLATFORM_WINDOWS 1)
    add_compile_definitions(FORG_PLATFORM_WINDOWS)
elseif(APPLE)
    if (DARWIN)
        set(FORG_PLATFORM_OSX 1)
        add_compile_definitions(FORG_PLATFORM_OSX)
    endif()
endif()

add_subdirectory(extern)
add_subdirectory(src)
