#include "forg_pch.h"

#include "scene/SceneNode.h"

namespace forg::scene {

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
