#include "forg_pch.h"

#include "debug/dbg.h"
#include "forg/io/ISerializer.h"
#include "scene/Model.h"

#include <string>
#include <string_view>
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

template <typename T>
bool serializeValue(io::ISerializer& serializer, std::string_view name,
                    std::string_view legacyName, T& value)
{
    if (serializer.IsWriting())
        return serializer.Value(name, value);

    return serializer.Value(name, value) ||
           (!legacyName.empty() && serializer.Value(legacyName, value));
}

ModelMeshParams DefaultMeshParams(ModelMeshType type)
{
    ModelMeshParams params;
    switch (type)
    {
    case ModelMeshType::Box:
        params.Box = {1.0f, 1.0f, 1.0f};
        break;
    case ModelMeshType::Sphere:
        params.Sphere = {1.0f, 16, 16};
        break;
    case ModelMeshType::Cylinder:
        params.Cylinder = {1.0f, 1.0f, 2.0f, 16, 1};
        break;
    case ModelMeshType::Pyramid:
        params.Pyramid = {4, 1.0f, 1.0f};
        break;
    case ModelMeshType::Grid:
        params.Grid = {1.0f, 1.0f, -1, 1};
        break;
    case ModelMeshType::None:
    case ModelMeshType::File:
        break;
    }
    return params;
}

bool serializeMeshParams(io::ISerializer& serializer, ModelMeshType type,
                         ModelMeshParams& params)
{
    if (!serializer.BeginObject("mesh_params"))
        return false;

    bool ok = true;
    switch (type)
    {
    case ModelMeshType::Box:
        ok = serializeValue(serializer, "width", "box_width",
                            params.Box.Width) &&
             serializeValue(serializer, "height", "box_height",
                            params.Box.Height) &&
             serializeValue(serializer, "depth", "box_depth", params.Box.Depth);
        break;
    case ModelMeshType::Sphere:
        ok = serializeValue(serializer, "radius", "sphere_radius",
                            params.Sphere.Radius) &&
             serializeValue(serializer, "slices", "sphere_slices",
                            params.Sphere.Slices) &&
             serializeValue(serializer, "stacks", "sphere_stacks",
                            params.Sphere.Stacks);
        break;
    case ModelMeshType::Cylinder:
        ok = serializeValue(serializer, "radius1", "cylinder_radius1",
                            params.Cylinder.Radius1) &&
             serializeValue(serializer, "radius2", "cylinder_radius2",
                            params.Cylinder.Radius2) &&
             serializeValue(serializer, "length", "cylinder_length",
                            params.Cylinder.Length) &&
             serializeValue(serializer, "slices", "cylinder_slices",
                            params.Cylinder.Slices) &&
             serializeValue(serializer, "stacks", "cylinder_stacks",
                            params.Cylinder.Stacks);
        break;
    case ModelMeshType::Pyramid:
        ok = serializeValue(serializer, "num_angles", "pyramid_num_angles",
                            params.Pyramid.NumAngles) &&
             serializeValue(serializer, "radius", "pyramid_radius",
                            params.Pyramid.Radius) &&
             serializeValue(serializer, "height", "pyramid_height",
                            params.Pyramid.Height);
        break;
    case ModelMeshType::Grid:
        ok = serializeValue(serializer, "size_x", "grid_size_x",
                            params.Grid.SizeX) &&
             serializeValue(serializer, "size_y", "grid_size_y",
                            params.Grid.SizeY) &&
             serializeValue(serializer, "color", "grid_color",
                            params.Grid.Color) &&
             serializeValue(serializer, "subgrid", "grid_subgrid",
                            params.Grid.Subgrid);
        break;
    case ModelMeshType::None:
    case ModelMeshType::File:
        break;
    }

    if (!serializer.EndObject())
        return false;

    if (!ok)
    {
        return false;
    }

    return true;
}

const char* MeshTypeName(ModelMeshType type)
{
    switch (type)
    {
    case ModelMeshType::None:
        return "none";
    case ModelMeshType::File:
        return "file";
    case ModelMeshType::Box:
        return "box";
    case ModelMeshType::Sphere:
        return "sphere";
    case ModelMeshType::Cylinder:
        return "cylinder";
    case ModelMeshType::Pyramid:
        return "pyramid";
    case ModelMeshType::Grid:
        return "grid";
    }
    return "none";
}

bool MeshTypeFromName(const core::string& name, ModelMeshType& type)
{
    const std::string text = name.c_str();
    if (text == "none")
        type = ModelMeshType::None;
    else if (text == "file")
        type = ModelMeshType::File;
    else if (text == "box")
        type = ModelMeshType::Box;
    else if (text == "sphere")
        type = ModelMeshType::Sphere;
    else if (text == "cylinder")
        type = ModelMeshType::Cylinder;
    else if (text == "pyramid")
        type = ModelMeshType::Pyramid;
    else if (text == "grid")
        type = ModelMeshType::Grid;
    else
        return false;

    return true;
}

geometry::Mesh::MeshPtr CreatePrimitiveMesh(ModelMeshType type,
                                            const ModelMeshParams& params,
                                            IRenderDevice* device)
{
    switch (type)
    {
    case ModelMeshType::Box:
        return geometry::Mesh::Box(device, params.Box.Width, params.Box.Height,
                                   params.Box.Depth);
    case ModelMeshType::Sphere:
        return geometry::Mesh::Sphere(device, params.Sphere.Radius,
                                      params.Sphere.Slices,
                                      params.Sphere.Stacks);
    case ModelMeshType::Cylinder:
        return geometry::Mesh::Cylinder(
            device, params.Cylinder.Radius1, params.Cylinder.Radius2,
            params.Cylinder.Length, params.Cylinder.Slices,
            params.Cylinder.Stacks);
    case ModelMeshType::Pyramid:
        return geometry::Mesh::Pyramid(device, params.Pyramid.NumAngles,
                                       params.Pyramid.Radius,
                                       params.Pyramid.Height);
    case ModelMeshType::Grid:
        return geometry::Mesh::Grid(device, params.Grid.SizeX,
                                    params.Grid.SizeY, params.Grid.Color,
                                    params.Grid.Subgrid);
    case ModelMeshType::None:
    case ModelMeshType::File:
        return nullptr;
    }

    return nullptr;
}

} // namespace

Model::Model()
    : m_transform(Matrix4::Identity), m_mesh_type(ModelMeshType::None),
      m_load_options(0)
{
}

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
    m_mesh_type = ModelMeshType::File;
    m_mesh_params = DefaultMeshParams(ModelMeshType::File);
    m_source_path = filenameText.c_str();
    m_load_options = options;

    DBG_MSG("Mesh %s loaded. Vertices: %d, Faces: %d\n", filenameText.c_str(),
            m_mesh->GetNumVertices(), m_mesh->GetNumFaces());

    return true;
}

bool Model::LoadResources(IRenderDevice* device)
{
    if (device == nullptr)
        return false;

    const Matrix4 transform = m_transform;
    bool loaded = false;

    if (m_mesh_type == ModelMeshType::File && m_source_path.length() != 0)
    {
        loaded = Load(m_source_path.c_str(), device, m_load_options);
    }
    else
    {
        geometry::Mesh::MeshPtr mesh =
            CreatePrimitiveMesh(m_mesh_type, m_mesh_params, device);
        if (mesh)
        {
            m_mesh = std::move(mesh);
            m_materials.clear();
            m_textures.clear();
            loaded = true;
        }
    }

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
    m_mesh_type = ModelMeshType::None;
    m_mesh_params = DefaultMeshParams(ModelMeshType::None);
    m_source_path.clear();
    m_load_options = 0;
}

void Model::Clear()
{
    m_mesh.reset();
    m_materials.clear();
    m_textures.clear();
    m_transform = Matrix4::Identity;
    m_mesh_type = ModelMeshType::None;
    m_mesh_params = DefaultMeshParams(ModelMeshType::None);
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

ModelMeshType Model::MeshType() const { return m_mesh_type; }

const ModelMeshParams& Model::MeshParams() const { return m_mesh_params; }

void Model::SetSource(std::string_view filename, uint options)
{
    const std::string filenameText(filename);
    m_mesh_type =
        filenameText.empty() ? ModelMeshType::None : ModelMeshType::File;
    m_mesh_params = DefaultMeshParams(m_mesh_type);
    m_source_path = filenameText.c_str();
    m_load_options = options;
}

void Model::SetPrimitive(ModelMeshType type)
{
    m_mesh_type = type == ModelMeshType::File ? ModelMeshType::None : type;
    m_mesh_params = DefaultMeshParams(m_mesh_type);
    m_source_path.clear();
    m_load_options = 0;
}

void Model::SetBox(float width, float height, float depth)
{
    SetPrimitive(ModelMeshType::Box);
    m_mesh_params.Box = {width, height, depth};
}

void Model::SetSphere(float radius, int slices, int stacks)
{
    SetPrimitive(ModelMeshType::Sphere);
    m_mesh_params.Sphere = {radius, slices, stacks};
}

void Model::SetCylinder(float radius1, float radius2, float length, int slices,
                        int stacks)
{
    SetPrimitive(ModelMeshType::Cylinder);
    m_mesh_params.Cylinder = {radius1, radius2, length, slices, stacks};
}

void Model::SetPyramid(uint numAngles, float radius, float height)
{
    SetPrimitive(ModelMeshType::Pyramid);
    m_mesh_params.Pyramid = {numAngles, radius, height};
}

void Model::SetGrid(float sizeX, float sizeY, int color, uint subgrid)
{
    SetPrimitive(ModelMeshType::Grid);
    m_mesh_params.Grid = {sizeX, sizeY, color, subgrid};
}

bool Model::Save(io::ISerializer& serializer) const
{
    if (!serializer.BeginObject("model"))
        return false;

    core::string meshType(MeshTypeName(m_mesh_type));
    core::string source = m_source_path;
    uint options = m_load_options;
    Matrix4 transform = m_transform;
    ModelMeshParams meshParams = m_mesh_params;

    if (!serializer.Value("mesh_type", meshType) ||
        !serializer.Value("source_path", source) ||
        !serializer.Value("load_options", options) ||
        !serializeMeshParams(serializer, m_mesh_type, meshParams) ||
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
    core::string meshTypeText;
    uint options = 0;
    Matrix4 transform = Matrix4::Identity;
    ModelMeshParams meshParams;

    const bool hasMeshType = serializer.Value("mesh_type", meshTypeText);
    if (!serializer.Value("source_path", source) ||
        !serializer.Value("load_options", options))
    {
        return false;
    }

    ModelMeshType meshType = ModelMeshType::None;
    if (hasMeshType)
    {
        if (!MeshTypeFromName(meshTypeText, meshType))
            return false;
    }
    else if (source.length() != 0)
    {
        meshType = ModelMeshType::File;
    }
    else
    {
        meshType = ModelMeshType::None;
    }

    if (meshType == ModelMeshType::File && source.length() == 0)
        return false;

    meshParams = DefaultMeshParams(meshType);
    const bool hasMeshParams =
        serializeMeshParams(serializer, meshType, meshParams);

    if (!serializeMatrix(serializer, transform))
    {
        return false;
    }

    if (!serializer.EndObject())
        return false;

    Clear();
    m_mesh_type = meshType;
    if (hasMeshParams)
        m_mesh_params = meshParams;
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
