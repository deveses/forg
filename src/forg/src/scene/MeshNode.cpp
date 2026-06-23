#include "forg_pch.h"

#include "scene/MeshNode.h"

namespace forg::scene {

Model& MeshNode::GetModel() { return m_model; }

const Model& MeshNode::GetModel() const { return m_model; }

void MeshNode::Render(IRenderDevice* device)
{
    m_model.Render(device);
    SceneNode::Render(device);
}

} // namespace forg::scene
