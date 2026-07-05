#pragma once
#include "scene/CameraNode.h"
#include "scene/MeshNode.h"
#include "scene/SoundEmitterNode.h"
#include "scene/SoundNode.h"

#include <memory>
#include <vector>

namespace forg::audio {
class AudioManager;
}

namespace forg::io {
class ISerializer;
}

namespace forg::fs {
class Filesystem;
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
    SoundNode& CreateSoundNode();
    SoundEmitterNode& CreateSoundEmitterNode();
    u32 NodeCount() const;
    SceneNode* Node(u32 index);
    const SceneNode* Node(u32 index) const;
    bool DestroyNode(SceneNode& node);
    void ClearNodes();

    bool Save(forg::io::ISerializer& serializer) const;
    bool Load(forg::io::ISerializer& serializer);
    bool LoadResources(IRenderDevice* device);
    bool LoadResources(const fs::Filesystem& filesystem, IRenderDevice* device);
    CameraNode* ActiveCameraNode();
    const CameraNode* ActiveCameraNode() const;

    void Update(double deltaSeconds);
    // Syncs SoundNodes with the audio manager, using the active camera as
    // the listener.
    void UpdateAudio(audio::AudioManager& manager);
    void Render(IRenderDevice* device);
};

} // namespace forg::scene
