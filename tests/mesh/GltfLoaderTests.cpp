#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>

#include "forg/mesh/GLTFLoader.h"

using Catch::Approx;
using forg::gltf::GltfLoader;

namespace {
std::string Asset(const char* name)
{
    return std::string(FORG_TEST_DATA_DIR) + "/gltf/" + name;
}

bool Flatten(const char* name, GltfLoader::CpuMesh& out)
{
    return GltfLoader::Flatten(Asset(name).c_str(), out);
}
} // namespace

TEST_CASE("GltfLoader flattens an embedded .gltf triangle", "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("triangle.gltf", m));

    REQUIRE(m.vertices.size() == 3);
    REQUIRE(m.indices.size() == 3);
    REQUIRE(m.subsets.size() == 1);
    REQUIRE(m.materials.size() == 1);
    REQUIRE_FALSE(m.use32bit);

    // Positions preserved (identity node transform).
    REQUIRE(m.vertices[1].Position.X == Approx(1.0f));
    REQUIRE(m.vertices[2].Position.Y == Approx(1.0f));

    // Provided normals (+Z) preserved.
    REQUIRE(m.vertices[0].Normal.Z == Approx(1.0f));

    // Texture coordinates preserved.
    REQUIRE(m.vertices[1].Tu == Approx(1.0f));
    REQUIRE(m.vertices[2].Tv == Approx(1.0f));

    // baseColorFactor -> diffuse colour.
    REQUIRE(m.materials[0].Material3D.Diffuse.r == Approx(0.25f));
    REQUIRE(m.materials[0].Material3D.Diffuse.g == Approx(0.5f));
    REQUIRE(m.materials[0].Material3D.Diffuse.b == Approx(0.75f));
    REQUIRE(m.materials[0].TextureFilename.size() == 0);

    // Single subset spanning the whole mesh.
    REQUIRE(m.subsets[0].AttribId == 0);
    REQUIRE(m.subsets[0].FaceStart == 0);
    REQUIRE(m.subsets[0].FaceCount == 1);
    REQUIRE(m.subsets[0].VertexStart == 0);
    REQUIRE(m.subsets[0].VertexCount == 3);
}

TEST_CASE("GltfLoader loads an external .bin buffer", "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("triangle_external.gltf", m));

    REQUIRE(m.vertices.size() == 3);
    REQUIRE(m.indices.size() == 3);
    REQUIRE(m.vertices[1].Position.X == Approx(1.0f));
    REQUIRE(m.materials[0].Material3D.Diffuse.b == Approx(0.75f));
}

TEST_CASE("GltfLoader bakes static node transforms into positions",
          "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("translated_triangle.gltf", m));

    // Node is translated by +10 on X; every vertex is shifted accordingly.
    REQUIRE(m.vertices[0].Position.X == Approx(10.0f));
    REQUIRE(m.vertices[1].Position.X == Approx(11.0f));
    REQUIRE(m.vertices[2].Position.X == Approx(10.0f));
    REQUIRE(m.vertices[2].Position.Y == Approx(1.0f));
}

TEST_CASE("GltfLoader computes normals when missing", "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("no_normals.gltf", m));

    REQUIRE(m.vertices.size() == 3);
    for (forg::uint i = 0; i < m.vertices.size(); ++i)
    {
        const forg::Vector3& n = m.vertices[i].Normal;
        // Triangle lies in the XY plane, so the computed normal is unit-length
        // along Z.
        REQUIRE(std::fabs(n.Z) == Approx(1.0f));
        REQUIRE(n.X == Approx(0.0f));
        REQUIRE(n.Y == Approx(0.0f));
    }
}

TEST_CASE("GltfLoader produces one subset per primitive/material",
          "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("two_materials.gltf", m));

    REQUIRE(m.vertices.size() == 6);
    REQUIRE(m.indices.size() == 6);
    REQUIRE(m.subsets.size() == 2);
    REQUIRE(m.materials.size() == 2);

    REQUIRE(m.subsets[0].AttribId == 0);
    REQUIRE(m.subsets[0].FaceStart == 0);
    REQUIRE(m.subsets[0].VertexStart == 0);

    REQUIRE(m.subsets[1].AttribId == 1);
    REQUIRE(m.subsets[1].FaceStart == 1);
    REQUIRE(m.subsets[1].VertexStart == 3);

    // Second primitive's indices are offset into the merged vertex buffer.
    REQUIRE(m.indices[3] == 3);

    REQUIRE(m.materials[0].Material3D.Diffuse.r == Approx(1.0f));
    REQUIRE(m.materials[1].Material3D.Diffuse.g == Approx(1.0f));
}

TEST_CASE("GltfLoader reads a textured .glb and its texture filename",
          "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("quad_textured.glb", m));

    REQUIRE(m.vertices.size() == 4);
    REQUIRE(m.indices.size() == 6);
    REQUIRE(m.materials.size() == 1);
    REQUIRE(m.materials[0].TextureFilename.size() > 0);
    REQUIRE(std::string(m.materials[0].TextureFilename.c_str()) == "wood.png");
}

TEST_CASE("GltfLoader flags 32-bit indices above 65535 vertices",
          "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE(Flatten("grid32.glb", m));

    REQUIRE(m.vertices.size() == 65540);
    REQUIRE(m.use32bit);
}

TEST_CASE("GltfLoader fails on a missing file", "[mesh][gltf]")
{
    GltfLoader::CpuMesh m;
    REQUIRE_FALSE(Flatten("does_not_exist.gltf", m));
}
