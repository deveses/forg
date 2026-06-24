#include "forg_pch.h"

#include "debug/dbg.h"
#include "forg/io/ISerializer.h"
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

bool serializeMatrix(io::ISerializer& serializer, Matrix4& transform)
{
    static const char* names[] = {"m11", "m12", "m13", "m14", "m21", "m22",
                                  "m23", "m24", "m31", "m32", "m33", "m34",
                                  "m41", "m42", "m43", "m44"};
    float* values = &transform.M11;

    if (!serializer.BeginObject("transform"))
        return false;

    for (uint i = 0; i < 16; ++i)
    {
        if (!serializer.Value(names[i], values[i]))
            return false;
    }

    return serializer.EndObject();
}

} // namespace

Model::Model() : m_transform(Matrix4::Identity), m_load_options(0) {}

bool Model::Load(const char* filename, IRenderDevice* device, uint options)
{
    if (filename == nullptr || device == nullptr)
        return false;

    const std::string filenameText(filename);
    geometry::Mesh::ExtendedMaterialVec materials;
    geometry::Mesh::MeshPtr mesh = geometry::Mesh::FromFile(
        filenameText.c_str(), options, device, materials);

    if (!mesh || mesh->GetNumVertices() == 0)
        return false;

    const std::string baseDir = BaseDirectory(filenameText.c_str());
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
    m_source_path = filenameText.c_str();
    m_load_options = options;

    DBG_MSG("Mesh %s loaded. Vertices: %d, Faces: %d\n", filenameText.c_str(),
            m_mesh->GetNumVertices(), m_mesh->GetNumFaces());

    return true;
}

bool Model::LoadResources(IRenderDevice* device)
{
    if (device == nullptr || m_source_path.length() == 0)
        return false;

    const Matrix4 transform = m_transform;
    const bool loaded = Load(m_source_path.c_str(), device, m_load_options);
    if (loaded)
        m_transform = transform;

    return loaded;
}

void Model::SetMesh(geometry::Mesh::MeshPtr mesh)
{
    m_mesh = std::move(mesh);
    m_materials.clear();
    m_textures.clear();
    m_transform = Matrix4::Identity;
    m_source_path.clear();
    m_load_options = 0;
}

void Model::Clear()
{
    m_mesh.reset();
    m_materials.clear();
    m_textures.clear();
    m_transform = Matrix4::Identity;
    m_source_path.clear();
    m_load_options = 0;
}

bool Model::IsLoaded() const { return static_cast<bool>(m_mesh); }

geometry::Mesh* Model::GetMesh() const { return m_mesh.get(); }

geometry::Mesh::MeshPtr& Model::Mesh() { return m_mesh; }

const Matrix4& Model::GetTransform() const { return m_transform; }

Matrix4& Model::Transform() { return m_transform; }

void Model::SetTransform(const Matrix4& transform) { m_transform = transform; }

const core::string& Model::SourcePath() const { return m_source_path; }

uint Model::LoadOptions() const { return m_load_options; }

void Model::SetSource(const char* filename, uint options)
{
    m_source_path = filename == nullptr ? "" : filename;
    m_load_options = options;
}

bool Model::Save(io::ISerializer& serializer) const
{
    if (!serializer.BeginObject("model"))
        return false;

    core::string source = m_source_path;
    uint options = m_load_options;
    Matrix4 transform = m_transform;

    if (!serializer.Value("source_path", source) ||
        !serializer.Value("load_options", options) ||
        !serializeMatrix(serializer, transform))
    {
        return false;
    }

    return serializer.EndObject();
}

bool Model::Load(io::ISerializer& serializer)
{
    if (!serializer.BeginObject("model"))
        return false;

    core::string source;
    uint options = 0;
    Matrix4 transform = Matrix4::Identity;

    if (!serializer.Value("source_path", source) ||
        !serializer.Value("load_options", options) ||
        !serializeMatrix(serializer, transform))
    {
        return false;
    }

    if (!serializer.EndObject())
        return false;

    Clear();
    m_source_path = source;
    m_load_options = options;
    m_transform = transform;
    return true;
}

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
