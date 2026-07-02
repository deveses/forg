#include <catch2/catch_test_macros.hpp>

#include "forg/fs/Filesystem.h"

#include <filesystem>

TEST_CASE("Filesystem resolves mounted read paths", "[fs]")
{
    forg::fs::Filesystem filesystem;
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "forg-fs-root";

    REQUIRE(filesystem.Mount("data:", root));
    REQUIRE(filesystem.HasMount("data"));
    REQUIRE(filesystem.HasMount("data:"));

    std::filesystem::path resolved;
    REQUIRE(filesystem.ResolveReadPath("data:models/cube.gltf", resolved));
    REQUIRE(resolved == root / "models/cube.gltf");

    REQUIRE(filesystem.ResolveReadPath("data:/ui/dialog.yml", resolved));
    REQUIRE(resolved == root / "ui/dialog.yml");
}

TEST_CASE("Filesystem preserves native paths", "[fs]")
{
    forg::fs::Filesystem filesystem;

    std::filesystem::path resolved;
    REQUIRE(filesystem.ResolveReadPath("data/ui/dialog.yml", resolved));
    REQUIRE(resolved == std::filesystem::path("data/ui/dialog.yml"));

    REQUIRE(filesystem.ResolveReadPath("/tmp/scene.yml", resolved));
    REQUIRE(resolved == std::filesystem::path("/tmp/scene.yml"));
}

TEST_CASE("Filesystem rejects unknown mounts and traversal", "[fs]")
{
    forg::fs::Filesystem filesystem;
    REQUIRE(filesystem.Mount("data:", "data"));

    std::filesystem::path resolved;
    REQUIRE_FALSE(filesystem.ResolveReadPath("missing:file.txt", resolved));
    REQUIRE_FALSE(filesystem.ResolveReadPath("data:../secret.txt", resolved));
    REQUIRE_FALSE(
        filesystem.ResolveReadPath("data:models/../secret.txt", resolved));
}

TEST_CASE("Filesystem resolves children relative to mounted base paths", "[fs]")
{
    forg::fs::Filesystem filesystem;
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "forg-fs-root";
    REQUIRE(filesystem.Mount("data:", root));

    std::filesystem::path resolved;
    REQUIRE(filesystem.ResolveReadPathRelative("data:models/cube.gltf",
                                               "wood.png", resolved));
    REQUIRE(resolved == root / "models/wood.png");

    REQUIRE(filesystem.ResolveReadPathRelative(
        "data:models/cube.gltf", "data:textures/wood.png", resolved));
    REQUIRE(resolved == root / "textures/wood.png");

    REQUIRE_FALSE(filesystem.ResolveReadPathRelative("data:models/cube.gltf",
                                                     "../wood.png", resolved));
}
