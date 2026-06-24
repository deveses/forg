#include "forg_pch.h"

#include "scene/SceneNode.h"

#include "forg/io/ISerializer.h"

namespace forg::scene {

const char* SceneNode::TypeName() const { return "SceneNode"; }

bool SceneNode::Save(io::ISerializer&) const { return true; }

bool SceneNode::Load(io::ISerializer&) { return true; }

void SceneNode::Update(double deltaSeconds)
{
    for (TreeNode* child : Children())
    {
        SceneNode* sceneNode = dynamic_cast<SceneNode*>(child);
        if (sceneNode != nullptr)
            sceneNode->Update(deltaSeconds);
    }
}

void SceneNode::Render(IRenderDevice* device)
{
    for (TreeNode* child : Children())
    {
        SceneNode* sceneNode = dynamic_cast<SceneNode*>(child);
        if (sceneNode != nullptr)
            sceneNode->Render(device);
    }
}

} // namespace forg::scene
