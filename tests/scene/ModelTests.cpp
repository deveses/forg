#include <catch2/catch_test_macros.hpp>

#include <string>

#include "forg/scene/Model.h"
#include "forg/rendering/reference/SWRenderDevice.h"

namespace {

void RequireIdentity(const forg::Matrix4& m)
{
    REQUIRE(m.M11 == 1.0f);
    REQUIRE(m.M22 == 1.0f);
    REQUIRE(m.M33 == 1.0f);
    REQUIRE(m.M44 == 1.0f);
    REQUIRE(m.M41 == 0.0f);
    REQUIRE(m.M42 == 0.0f);
    REQUIRE(m.M43 == 0.0f);
}

} // namespace

TEST_CASE("Model starts empty with identity transform", "[scene][model]")
{
    forg::scene::Model model;

    REQUIRE_FALSE(model.IsLoaded());
    REQUIRE(model.GetMesh() == nullptr);
    RequireIdentity(model.GetTransform());
}

TEST_CASE("Model owns procedural meshes", "[scene][model]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    forg::scene::Model model;

    model.SetMesh(
        forg::geometry::Mesh::Cylinder(&device, 1.0f, 0.5f, 3.0f, 4, 8));

    REQUIRE(model.IsLoaded());
    REQUIRE(model.GetMesh() != nullptr);
    REQUIRE(model.GetMesh()->GetNumVertices() > 0);
    REQUIRE(model.GetMesh()->GetNumFaces() > 0);
    RequireIdentity(model.GetTransform());
}

TEST_CASE("Model loads a glTF mesh", "[scene][model]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    forg::scene::Model model;
    const std::string filename =
        std::string(FORG_TEST_DATA_DIR) + "/gltf/triangle.gltf";

    REQUIRE(model.Load(filename.c_str(), &device));

    REQUIRE(model.IsLoaded());
    REQUIRE(model.GetMesh() != nullptr);
    REQUIRE(model.GetMesh()->GetNumVertices() == 3);
    REQUIRE(model.GetMesh()->GetNumFaces() == 1);
    REQUIRE(model.SourcePath() == filename.c_str());
    REQUIRE(model.LoadOptions() == 0);
    RequireIdentity(model.GetTransform());
}

TEST_CASE("Model reloads resources from serialized metadata", "[scene][model]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    forg::scene::Model model;
    const std::string filename =
        std::string(FORG_TEST_DATA_DIR) + "/gltf/triangle.gltf";

    forg::Matrix4 transform = forg::Matrix4::Identity;
    transform.M41 = 7.0f;
    transform.M42 = 8.0f;
    transform.M43 = 9.0f;
    model.SetSource(filename.c_str());
    model.SetTransform(transform);

    REQUIRE(model.LoadResources(&device));

    REQUIRE(model.IsLoaded());
    REQUIRE(model.GetMesh() != nullptr);
    REQUIRE(model.GetMesh()->GetNumVertices() == 3);
    REQUIRE(model.GetTransform().M41 == 7.0f);
    REQUIRE(model.GetTransform().M42 == 8.0f);
    REQUIRE(model.GetTransform().M43 == 9.0f);
}

TEST_CASE("Model failed load preserves the previous mesh", "[scene][model]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    forg::scene::Model model;
    model.SetMesh(forg::geometry::Mesh::Box(&device, 1.0f, 2.0f, 3.0f));
    const forg::uint vertices = model.GetMesh()->GetNumVertices();
    const forg::uint faces = model.GetMesh()->GetNumFaces();

    REQUIRE_FALSE(model.Load("missing-model.gltf", &device));

    REQUIRE(model.IsLoaded());
    REQUIRE(model.GetMesh() != nullptr);
    REQUIRE(model.GetMesh()->GetNumVertices() == vertices);
    REQUIRE(model.GetMesh()->GetNumFaces() == faces);
}
