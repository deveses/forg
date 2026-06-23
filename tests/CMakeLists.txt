find_package(Catch2 3.15 CONFIG QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG 6ee0826dcae55ed1e06b2c5701981221e979e1e6
        GIT_SHALLOW FALSE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

add_executable(forg_tests
    math/MathTests.cpp
    audio/WaveFileTests.cpp
    core/BitArrayTests.cpp
    core/RefCounterTests.cpp
    core/StringTests.cpp
    script/XMLParserTests.cpp
    script/YAMLParserTests.cpp
    rendering/RenderingValueTests.cpp
    rendering/MeshFactoryTests.cpp
    rendering/CameraTests.cpp
    rendering/PluginTests.cpp
    rendering/ReferenceRenderTests.cpp
    scene/ModelTests.cpp
    mesh/GltfLoaderTests.cpp
    net/HttpRequestTests.cpp
    net/CommandTests.cpp
    net/CommandQueueTests.cpp
    control/SceneControlTests.cpp
)

target_link_libraries(forg_tests
    PRIVATE
    forg
    Catch2::Catch2WithMain
)

forg_configure_target(forg_tests)
target_compile_definitions(forg_tests
    PRIVATE
    FORG_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

if(TARGET swrenderer)
    add_dependencies(forg_tests swrenderer)
    target_compile_definitions(forg_tests PRIVATE
        FORG_TEST_SWRENDERER_PATH="$<TARGET_FILE:swrenderer>")
endif()
if(TARGET metalrenderer)
    add_dependencies(forg_tests metalrenderer)
    target_compile_definitions(forg_tests PRIVATE
        FORG_TEST_METALRENDERER_PATH="$<TARGET_FILE:metalrenderer>")
endif()
if(TARGET glrenderer)
    add_dependencies(forg_tests glrenderer)
    target_compile_definitions(forg_tests PRIVATE
        FORG_TEST_GLRENDERER_PATH="$<TARGET_FILE:glrenderer>")
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(forg_tests)

add_test(NAME forg-package-consumer
    COMMAND ${CMAKE_COMMAND}
        -DFORG_BINARY_DIR=${PROJECT_BINARY_DIR}
        -DFORG_SOURCE_DIR=${PROJECT_SOURCE_DIR}/tests/package
        -DFORG_TEST_CONFIG=$<CONFIG>
        -DFORG_TEST_CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DFORG_TEST_CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
        -P ${PROJECT_SOURCE_DIR}/tests/package/RunPackageTest.cmake)

file(GLOB_RECURSE forg_public_headers CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/forg/include/forg/*.h"
    "${PROJECT_SOURCE_DIR}/src/forg/include/forg/*.hpp")
set(forg_public_header_sources)
foreach(header IN LISTS forg_public_headers)
    file(RELATIVE_PATH relative_header
        "${PROJECT_SOURCE_DIR}/src/forg/include" "${header}")
    string(MAKE_C_IDENTIFIER "${relative_header}" source_name)
    set(source "${CMAKE_CURRENT_BINARY_DIR}/public_headers/${source_name}.cpp")
    file(GENERATE OUTPUT "${source}" CONTENT "#include <${relative_header}>\n")
    list(APPEND forg_public_header_sources "${source}")
endforeach()

add_library(forg-public-headers OBJECT ${forg_public_header_sources})
target_link_libraries(forg-public-headers PRIVATE forg)
forg_configure_target(forg-public-headers)
