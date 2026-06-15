include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(forg_tests
    math/MathTests.cpp
    audio/WaveFileTests.cpp
    core/BitArrayTests.cpp
    script/XMLParserTests.cpp
    script/YAMLParserTests.cpp
    rendering/RenderingValueTests.cpp
    rendering/CameraTests.cpp
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

target_compile_features(forg_tests PRIVATE cxx_std_17)
target_compile_definitions(forg_tests
    PRIVATE
    FORG_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(forg_tests)
