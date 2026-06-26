#ifndef FORG_SCENE_MODEL_H
#define FORG_SCENE_MODEL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/RefPtr.h"
#include "math/Matrix4.h"
#include "rendering/ITexture.h"
#include "rendering/Mesh.h"

#include <string_view>
#include <vector>

namespace forg::io {
class ISerializer;
}

namespace forg::scene {

using math::Matrix4;

enum class ModelMeshType
{
    None,
    File,
    Box,
    Sphere,
    Cylinder,
    Pyramid,
    Grid
};

struct ModelBoxParams
{
    float Width;
    float Height;
    float Depth;
};

struct ModelSphereParams
{
    float Radius;
    int Slices;
    int Stacks;
};

struct ModelCylinderParams
{
    float Radius1;
    float Radius2;
    float Length;
    int Slices;
    int Stacks;
};

struct ModelPyramidParams
{
    uint NumAngles;
    float Radius;
    float Height;
};

struct ModelGridParams
{
    float SizeX;
    float SizeY;
    int Color;
    uint Subgrid;
};

struct ModelMeshParams
{
    union
    {
        ModelBoxParams Box;
        ModelSphereParams Sphere;
        ModelCylinderParams Cylinder;
        ModelPyramidParams Pyramid;
        ModelGridParams Grid;
    };

    ModelMeshParams() : Box{1.0f, 1.0f, 1.0f} {}
};

class FORG_API Model
{
    geometry::Mesh::MeshPtr m_mesh;
    Matrix4 m_transform;
    geometry::Mesh::ExtendedMaterialVec m_materials;
    std::vector<core::RefPtr<ITexture>> m_textures;
    ModelMeshType m_mesh_type;
    ModelMeshParams m_mesh_params;
    core::string m_source_path;
    uint m_load_options;

  public:
    Model();

    bool Load(const char* filename, IRenderDevice* device, uint options = 0);
    bool LoadResources(IRenderDevice* device);
    void SetMesh(geometry::Mesh::MeshPtr mesh);
    void Clear();

    bool IsLoaded() const;
    geometry::Mesh* GetMesh() const;
    geometry::Mesh::MeshPtr& Mesh();

    const Matrix4& GetTransform() const;
    Matrix4& Transform();
    void SetTransform(const Matrix4& transform);

    const core::string& SourcePath() const;
    uint LoadOptions() const;
    ModelMeshType MeshType() const;
    const ModelMeshParams& MeshParams() const;
    void SetSource(std::string_view filename, uint options = 0);
    void SetPrimitive(ModelMeshType type);
    void SetBox(float width, float height, float depth);
    void SetSphere(float radius, int slices, int stacks);
    void SetCylinder(float radius1, float radius2, float length, int slices,
                     int stacks);
    void SetPyramid(uint numAngles, float radius, float height);
    void SetGrid(float sizeX, float sizeY, int color, uint subgrid);

    bool Save(forg::io::ISerializer& serializer) const;
    bool Load(forg::io::ISerializer& serializer);

    void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_MODEL_H
