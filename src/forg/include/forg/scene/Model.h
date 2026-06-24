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

#include <vector>

namespace forg::io {
class ISerializer;
}

namespace forg::scene {

using math::Matrix4;

class FORG_API Model
{
    geometry::Mesh::MeshPtr m_mesh;
    Matrix4 m_transform;
    geometry::Mesh::ExtendedMaterialVec m_materials;
    std::vector<core::RefPtr<ITexture>> m_textures;
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
    void SetSource(const char* filename, uint options = 0);

    bool Save(forg::io::ISerializer& serializer) const;
    bool Load(forg::io::ISerializer& serializer);

    void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_MODEL_H
