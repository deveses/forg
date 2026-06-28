#include "forg_pch.h"

#include "forg/rendering/CameraOrbitController.h"

#include "forg/rendering/Camera.h"

namespace forg {

void CameraOrbitController::OrbitPixels(Camera& camera, float dx,
                                        float dy) const
{
    camera.Orbit(-dx * OrbitSpeed, dy * OrbitSpeed);
}

void CameraOrbitController::TruckPixels(Camera& camera, float dx,
                                        float dy) const
{
    camera.Truck(-dx * TruckSpeed, dy * TruckSpeed);
}

void CameraOrbitController::ZoomLines(Camera& camera, float delta) const
{
    float dolly = delta * ZoomSpeed;
    const float distance = (camera.get_Target() - camera.get_Position()).Length();
    if (dolly > distance - MinTargetDistance)
        dolly = distance - MinTargetDistance;

    camera.Dolly(dolly, 0.0f);
}

} // namespace forg
