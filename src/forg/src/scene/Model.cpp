#include "forg_pch.h"

#include "debug/dbg.h"
#include "scene/Model.h"

#include <string>
#include <utility>

namespace forg::scene {
namespace {

std::string BaseDirectory(const char* filename)
{
    if (filename == nullptr)
        return {};

    std::string baseDir(filename);
    std::string::size_type lastSlash = baseDir.find_last_of('/');
    if (lastSlash == std::string::npos)
        lastSlash = baseDir.find_last_of('\\');

    if (lastSlash == std::string::npos)
        return {};

    baseDir.erase(lastSlash + 1);
    return baseDir;
}

} // namespace

Model::Model() : m_transform(Matrix4::Identity) {}

bool Model::Load(const char* filename, IRenderDevice* device, uint options)
{
    if (filename == nullptr || device == nullptr)
        return false;

    geometry::Mesh::ExtendedMaterialVec materials;
    geometry::Mesh::MeshPtr mesh =
        geometry::Mesh::FromFile(filename, options, device, materials);

    if (!mesh || mesh->GetNumVertices() == 0)
        return false;

    const std::string baseDir = BaseDirectory(filename);
    std::vector<core::RefPtr<ITexture>> textures;
    textures.reserve(materials.size());

    for (uint i = 0; i < materials.size(); i++)
    {
        const char* textureFilename = materials[i].TextureFilename.c_str();
        if (textureFilename[0] == 0)
        {
            textures.emplace_back();
            continue;
        }

        const std::string fullPath = baseDir + textureFilename;
        core::RefPtr<ITexture> texture(
            ITexture::FromFile(device, fullPath.c_str()));
        textures.push_back(std::move(texture));

        if (!textures.back())
            DBG_MSG(__T("Failed to load texture <%s>!\n"), fullPath.c_str());
    }

    m_mesh = std::move(mesh);
    m_materials = std::move(materials);
    m_textures = std::move(textures);
    m_transform = Matrix4::Identity;

    DBG_MSG("Mesh %s loaded. Vertices: %d, Faces: %d\n", filename,
            m_mesh->GetNumVertices(), m_mesh->GetNumFaces());

    return true;
}

void Model::SetMesh(geometry::Mesh::MeshPtr mesh)
{
    m_mesh = std::move(mesh);
    m_materials.clear();
    m_textures.clear();
    m_transform = Matrix4::Identity;
}

void Model::Clear()
{
    m_mesh.reset();
    m_materials.clear();
    m_textures.clear();
    m_transform = Matrix4::Identity;
}

bool Model::IsLoaded() const { return static_cast<bool>(m_mesh); }

geometry::Mesh* Model::GetMesh() const { return m_mesh.get(); }

geometry::Mesh::MeshPtr& Model::Mesh() { return m_mesh; }

const Matrix4& Model::GetTransform() const { return m_transform; }

Matrix4& Model::Transform() { return m_transform; }

void Model::SetTransform(const Matrix4& transform) { m_transform = transform; }

void Model::Render(IRenderDevice* device)
{
    if (!m_mesh || device == nullptr)
        return;

    device->SetTransform(TransformType_World, m_transform);

    if (!m_materials.empty())
    {
        for (uint i = 0; i < m_materials.size(); i++)
        {
            device->SetMaterial(&m_materials[i].Material3D);
            device->SetTexture(0, i < m_textures.size() ? m_textures[i].get()
                                                        : nullptr);
            m_mesh->DrawSubset(i);
        }
        return;
    }

    device->SetTexture(0, nullptr);
    m_mesh->DrawSubset(0);
}

} // namespace forg::scene
