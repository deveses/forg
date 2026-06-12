cmake_minimum_required(VERSION 3.1...3.26)

set(CMAKE_OSX_ARCHITECTURES arm64) # arch64: arm64, x86_64

project(
  Forg
  VERSION 1.0
  LANGUAGES CXX)

option(FORG_USE_OPENCL "Use OpenCL renderer" OFF)
option(FORG_USE_FREETYPE "Use Freetype library" OFF)
option(FORG_USE_ZLIB "Use ZLib library" OFF)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

message(NOTICE "CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message(NOTICE "CMAKE_HOST_SYSTEM: " ${CMAKE_HOST_SYSTEM})
message(NOTICE "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})
message(NOTICE "CMAKE_OSX_ARCHITECTURES: " ${CMAKE_OSX_ARCHITECTURES})

message(NOTICE "option FORG_USE_OPENCL: " ${FORG_USE_OPENCL})
message(NOTICE "option FORG_USE_FREETYPE: " ${FORG_USE_FREETYPE})
message(NOTICE "option FORG_USE_ZLIB: " ${FORG_USE_ZLIB})

set(FORG_PLATFORM_WINDOWS 0)
set(FORG_PLATFORM_OSX 0)
set(FORG_PLATFORM_IOS 0)
set(FORG_PLATFORM_ANDROID 0)

if(WIN32)
    set(FORG_PLATFORM_WINDOWS 1)
    add_compile_definitions(FORG_PLATFORM_WINDOWS)
    message("define FORG_PLATFORM_WINDOWS")
elseif(APPLE)
    if (IOS)
        set(FORG_PLATFORM_IOS 1)
        add_compile_definitions(FORG_PLATFORM_IOS)
        message("define FORG_PLATFORM_IOS")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(FORG_PLATFORM_OSX 1)
        add_compile_definitions(FORG_PLATFORM_OSX)
        message("define FORG_PLATFORM_OSX")
    endif()
else()
    message(FATAL_ERROR, "Unsupported platform!")
endif()

add_subdirectory(extern)
add_subdirectory(src)
