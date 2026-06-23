#ifndef FORG_SCENE_SCENE_H
#define FORG_SCENE_SCENE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scene/MeshNode.h"

#include <memory>
#include <vector>

namespace forg::scene {

class FORG_API Scene : public TreeNode
{
    std::vector<std::unique_ptr<SceneNode>> m_nodes;

  public:
    SceneNode& CreateNode();
    MeshNode& CreateMeshNode();
    uint NodeCount() const;
    SceneNode* Node(uint index);
    const SceneNode* Node(uint index) const;
    bool DestroyNode(SceneNode& node);
    void ClearNodes();

    void Update(double deltaSeconds);
    void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_SCENE_H
