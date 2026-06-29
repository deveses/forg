#ifndef FORG_SCENE_SCENE_H
#define FORG_SCENE_SCENE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scene/CameraNode.h"
#include "scene/MeshNode.h"

#include <memory>
#include <vector>

namespace forg::io {
class ISerializer;
}

namespace forg::ui {
class GuiNode;
}

namespace forg::scene {

class FORG_API Scene : public TreeNode
{
    std::vector<std::unique_ptr<SceneNode>> m_nodes;

  public:
    SceneNode& CreateNode();
    CameraNode& CreateCameraNode();
    MeshNode& CreateMeshNode();
    ui::GuiNode& CreateGuiNode();
    uint NodeCount() const;
    SceneNode* Node(uint index);
    const SceneNode* Node(uint index) const;
    bool DestroyNode(SceneNode& node);
    void ClearNodes();

    bool Save(forg::io::ISerializer& serializer) const;
    bool Load(forg::io::ISerializer& serializer);
    bool LoadResources(IRenderDevice* device);
    CameraNode* ActiveCameraNode();
    const CameraNode* ActiveCameraNode() const;

    void Update(double deltaSeconds);
    void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_SCENE_H
