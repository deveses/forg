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

    REQUIRE(
        forg::SavePpm(path.string(), pixels, 2, 1, 2 * sizeof(forg::Color4b)));

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

TEST_CASE("Image saves and loads BMP files", "[image][bmp]")
{
    const std::filesystem::path ppmPath =
        std::filesystem::temp_directory_path() /
        "forg-image-test-bmp-source.ppm";
    const std::filesystem::path bmpPath =
        std::filesystem::temp_directory_path() / "forg-image-test.bmp";
    std::filesystem::remove(ppmPath);
    std::filesystem::remove(bmpPath);

    const forg::Color4b pixels[] = {
        forg::Color4b(255, 0, 0, 255),   forg::Color4b(0, 255, 0, 255),
        forg::Color4b(0, 0, 255, 255),   forg::Color4b(255, 255, 0, 255),
        forg::Color4b(0, 255, 255, 255), forg::Color4b(255, 0, 255, 255),
    };

    REQUIRE(forg::SavePpm(ppmPath.string(), pixels, 3, 2,
                          3 * sizeof(forg::Color4b)));

    forg::Image image;
    REQUIRE(image.Load(ppmPath.string()));
    REQUIRE(image.Save(bmpPath.string()));

    forg::Image copy;
    REQUIRE(copy.Load(bmpPath.string()));
    REQUIRE(copy.GetWidth() == 3);
    REQUIRE(copy.GetHeight() == 2);

    const forg::Color4b* loaded =
        reinterpret_cast<const forg::Color4b*>(copy.GetData(0));
    for (uint i = 0; i < 6; ++i)
    {
        REQUIRE(loaded[i].r == pixels[i].r);
        REQUIRE(loaded[i].g == pixels[i].g);
        REQUIRE(loaded[i].b == pixels[i].b);
        REQUIRE(loaded[i].a == 255);
    }

    std::filesystem::remove(ppmPath);
    std::filesystem::remove(bmpPath);
}
