#pragma once
#include "base.h"

namespace forg {

class Camera;

struct CameraOrbitController
{
    float OrbitSpeed = 0.01f;
    float TruckSpeed = 0.01f;
    float ZoomSpeed = 0.30f;
    float MinTargetDistance = 0.5f;

    void OrbitPixels(Camera& camera, float dx, float dy) const;
    void TruckPixels(Camera& camera, float dx, float dy) const;
    void ZoomLines(Camera& camera, float delta) const;
};

} // namespace forg
