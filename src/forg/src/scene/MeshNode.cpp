#include "forg_pch.h"

#include "scene/MeshNode.h"

#include "forg/io/ISerializer.h"

namespace forg::scene {

Model& MeshNode::GetModel() { return m_model; }

const Model& MeshNode::GetModel() const { return m_model; }

ModelMeshType MeshNode::MeshType() const { return m_model.MeshType(); }

void MeshNode::SetPrimitive(ModelMeshType type) { m_model.SetPrimitive(type); }

void MeshNode::SetBox(float width, float height, float depth)
{
    m_model.SetBox(width, height, depth);
}

void MeshNode::SetSphere(float radius, int slices, int stacks)
{
    m_model.SetSphere(radius, slices, stacks);
}

void MeshNode::SetCylinder(float radius1, float radius2, float length,
                           int slices, int stacks)
{
    m_model.SetCylinder(radius1, radius2, length, slices, stacks);
}

void MeshNode::SetPyramid(uint numAngles, float radius, float height)
{
    m_model.SetPyramid(numAngles, radius, height);
}

void MeshNode::SetGrid(float sizeX, float sizeY, int color, uint subgrid)
{
    m_model.SetGrid(sizeX, sizeY, color, subgrid);
}

const char* MeshNode::TypeName() const { return "MeshNode"; }

bool MeshNode::Save(io::ISerializer& serializer) const
{
    return SceneNode::Save(serializer) && m_model.Save(serializer);
}

bool MeshNode::Load(io::ISerializer& serializer)
{
    return SceneNode::Load(serializer) && m_model.Load(serializer);
}

void MeshNode::Render(IRenderDevice* device)
{
    m_model.Render(device);
    SceneNode::Render(device);
}

} // namespace forg::scene
