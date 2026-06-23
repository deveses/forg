cmake_minimum_required(VERSION 3.21...4.0)

project(
  Forg
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(FORG_USE_OPENCL "Use OpenCL renderer" OFF)
option(FORG_USE_FREETYPE "Use Freetype library" OFF)
option(FORG_USE_ZLIB "Use ZLib library" OFF)
option(FORG_WARNINGS_AS_ERRORS "Treat FORG compiler warnings as errors" OFF)
option(FORG_ENABLE_CLANG_TIDY "Run clang-tidy while compiling FORG targets" OFF)
option(FORG_ENABLE_PCH "Use precompiled headers for FORG targets" ON)

find_program(FORG_CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(FORG_CLANG_FORMAT_EXECUTABLE)
    add_custom_target(forg-format-check
        COMMAND ${CMAKE_COMMAND}
            -DCLANG_FORMAT_EXECUTABLE=${FORG_CLANG_FORMAT_EXECUTABLE}
            -DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFormat.cmake
        VERBATIM)
endif()

add_custom_target(forg-no-exceptions-check
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckNoExceptions.cmake
    VERBATIM)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

if(FORG_ENABLE_CLANG_TIDY)
    find_program(FORG_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

function(forg_configure_target target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE
        "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>"
        "$<${msvc_cxx}:/W4;/wd4100;/wd4101;/wd4189;/wd4244>"
        "$<${msvc_cxx}:/wd4245;/wd4267;/wd4456;/wd4702>"
    )
    target_compile_definitions(${target} PRIVATE
        "$<${msvc_cxx}:_CRT_SECURE_NO_WARNINGS>"
    )

    if(FORG_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE
            "$<${gcc_like_cxx}:-Werror>"
            "$<${msvc_cxx}:/WX>"
        )
    endif()

    if(FORG_ENABLE_CLANG_TIDY)
        set_property(TARGET ${target} PROPERTY CXX_CLANG_TIDY
            "${FORG_CLANG_TIDY_EXECUTABLE};--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
    endif()
endfunction()

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
    enable_language(RC)
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

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(DIRECTORY src/forg/include/forg
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(EXPORT ForgTargets
    FILE ForgTargets.cmake
    NAMESPACE Forg::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forg)

configure_package_config_file(
    cmake/ForgConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ForgConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forg)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ForgConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ForgConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ForgConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Forg)
