#ifndef FORG_SCENE_CAMERANODE_H
#define FORG_SCENE_CAMERANODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "core/string.hpp"
#include "rendering/Camera.h"
#include "scene/SceneNode.h"

namespace forg::scene {

enum class CameraProjection
{
    Perspective,
    Orthogonal,
    Screen
};

class FORG_API CameraNode : public SceneNode
{
    Camera m_camera;
    CameraProjection m_projection = CameraProjection::Perspective;
    bool m_active = true;
    bool m_controllable = false;

  public:
    CameraNode();

    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;

    Camera& GetCamera();
    const Camera& GetCamera() const;

    void SetProjection(CameraProjection projection);
    CameraProjection Projection() const;

    void SetActive(bool active);
    bool Active() const;

    void SetControllable(bool controllable);
    bool Controllable() const;

    void Apply(IRenderDevice* device);
};

const char* CameraProjectionName(CameraProjection projection);
bool CameraProjectionFromName(const core::string& name,
                              CameraProjection& projection);

} // namespace forg::scene

#endif // FORG_SCENE_CAMERANODE_H
