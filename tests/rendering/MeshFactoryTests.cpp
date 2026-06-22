#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <type_traits>

#include "forg/rendering/Mesh.h"
#include "forg/rendering/reference/SWRenderDevice.h"

using forg::geometry::Mesh;
using forg::rendering::reference::SWRenderDevice;

namespace {

void RequireEquivalentMesh(const Mesh* legacy, const Mesh* modern)
{
    REQUIRE(legacy != nullptr);
    REQUIRE(modern != nullptr);
    REQUIRE(legacy->GetNumVertices() == modern->GetNumVertices());
    REQUIRE(legacy->GetNumFaces() == modern->GetNumFaces());
    REQUIRE(legacy->GetNumBytesPerVertex() == modern->GetNumBytesPerVertex());
    REQUIRE(legacy->GetOptions() == modern->GetOptions());
    REQUIRE(legacy->GetVertexBuffer() != nullptr);
    REQUIRE(modern->GetVertexBuffer() != nullptr);
    REQUIRE(legacy->GetIndexBuffer() != nullptr);
    REQUIRE(modern->GetIndexBuffer() != nullptr);
}

} // namespace

TEST_CASE("Mesh exposes a standard unique-owner factory type",
          "[rendering][mesh][ownership]")
{
    STATIC_REQUIRE(std::is_same_v<Mesh::UniqueMeshPtr, std::unique_ptr<Mesh>>);
}

TEST_CASE("Modern mesh primitive factories match legacy wrappers",
          "[rendering][mesh][ownership]")
{
    SWRenderDevice device(nullptr);

    auto legacy_box = Mesh::Box(&device, 2.0f, 3.0f, 4.0f);
    auto modern_box = Mesh::MakeBox(&device, 2.0f, 3.0f, 4.0f);
    RequireEquivalentMesh(legacy_box.get(), modern_box.get());

    auto legacy_sphere = Mesh::Sphere(&device, 1.5f, 8, 8);
    auto modern_sphere = Mesh::MakeSphere(&device, 1.5f, 8, 8);
    RequireEquivalentMesh(legacy_sphere.get(), modern_sphere.get());

    auto legacy_cylinder = Mesh::Cylinder(&device, 1.0f, 0.5f, 3.0f, 4, 8);
    auto modern_cylinder = Mesh::MakeCylinder(&device, 1.0f, 0.5f, 3.0f, 4, 8);
    RequireEquivalentMesh(legacy_cylinder.get(), modern_cylinder.get());

    auto legacy_pyramid = Mesh::Pyramid(&device, 5, 1.0f, 2.0f);
    auto modern_pyramid = Mesh::MakePyramid(&device, 5, 1.0f, 2.0f);
    RequireEquivalentMesh(legacy_pyramid.get(), modern_pyramid.get());

    auto legacy_grid = Mesh::Grid(&device, 10.0f, 6.0f, 0xff00ffff, 3);
    auto modern_grid = Mesh::MakeGrid(&device, 10.0f, 6.0f, 0xff00ffff, 3);
    RequireEquivalentMesh(legacy_grid.get(), modern_grid.get());

    const float heights[] = {0.0f, 0.25f, 0.5f, 0.75f};
    const forg::Vector3 span(2.0f, 1.0f, 2.0f);
    auto legacy_landscape = Mesh::Landscape(&device, span, heights, 2, 2);
    auto modern_landscape = Mesh::MakeLandscape(&device, span, heights, 2, 2);
    RequireEquivalentMesh(legacy_landscape.get(), modern_landscape.get());
}

TEST_CASE("Modern mesh file loader matches legacy FromFile wrapper",
          "[rendering][mesh][ownership]")
{
    SWRenderDevice device(nullptr);
    const std::string filename =
        std::string(FORG_TEST_DATA_DIR) + "/gltf/triangle.gltf";

    Mesh::ExtendedMaterialVec legacy_materials;
    auto legacy =
        Mesh::FromFile(filename.c_str(), 0, &device, legacy_materials);

    Mesh::ExtendedMaterialVec modern_materials;
    auto modern =
        Mesh::LoadFromFile(filename.c_str(), 0, &device, modern_materials);

    RequireEquivalentMesh(legacy.get(), modern.get());
    REQUIRE(legacy_materials.size() == modern_materials.size());
}
