#include <catch2/catch_test_macros.hpp>

#include "forg/image/Image.h"

#include <string>

namespace {

std::string RepoDataPath(const char* filename)
{
    return std::string(FORG_TEST_REPO_DATA_DIR) + "/" + filename;
}

} // namespace

TEST_CASE("Image loads the UI DDS texture", "[image][dds]")
{
    forg::Image image;

    REQUIRE(image.Load(RepoDataPath("ui/debug_texture2.dds").c_str()));
    REQUIRE(image.GetWidth() == 256);
    REQUIRE(image.GetHeight() == 256);
    REQUIRE(image.GetData(0) != nullptr);
}
