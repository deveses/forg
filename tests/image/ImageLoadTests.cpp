#include <catch2/catch_test_macros.hpp>

#include "forg/image/Image.h"
#include "forg/image/ppm/ppm.h"

#include <filesystem>
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

TEST_CASE("Image loads and saves binary PPM files", "[image][ppm]")
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "forg-image-test.ppm";
    const std::filesystem::path copyPath =
        std::filesystem::temp_directory_path() / "forg-image-test-copy.ppm";
    std::filesystem::remove(path);
    std::filesystem::remove(copyPath);

    const forg::Color4b pixels[] = {
        forg::Color4b(255, 0, 0, 255),
        forg::Color4b(0, 255, 0, 255),
    };

    REQUIRE(forg::SavePpm(path.string(), pixels, 2, 1,
                          2 * sizeof(forg::Color4b)));

    forg::Image image;
    REQUIRE(image.Load(path.string().c_str()));
    REQUIRE(image.GetWidth() == 2);
    REQUIRE(image.GetHeight() == 1);

    const forg::Color4b* loaded =
        reinterpret_cast<const forg::Color4b*>(image.GetData(0));
    REQUIRE(loaded[0].r == 255);
    REQUIRE(loaded[0].g == 0);
    REQUIRE(loaded[0].b == 0);
    REQUIRE(loaded[0].a == 255);
    REQUIRE(loaded[1].r == 0);
    REQUIRE(loaded[1].g == 255);
    REQUIRE(loaded[1].b == 0);
    REQUIRE(loaded[1].a == 255);

    REQUIRE(image.Save(copyPath.string()));

    forg::Image copy;
    REQUIRE(copy.Load(copyPath.string().c_str()));
    REQUIRE(copy.GetWidth() == 2);
    REQUIRE(copy.GetHeight() == 1);

    std::filesystem::remove(path);
    std::filesystem::remove(copyPath);
}
