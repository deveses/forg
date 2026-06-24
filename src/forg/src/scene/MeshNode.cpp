#include "forg_pch.h"

#include "scene/MeshNode.h"

#include "forg/io/ISerializer.h"

namespace forg::scene {

Model& MeshNode::GetModel() { return m_model; }

const Model& MeshNode::GetModel() const { return m_model; }

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
