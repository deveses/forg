#include <string>

#include "control/commands/Commands.h"

namespace forg
{
namespace control
{

using forg::net::Command;
using forg::net::TryGetFloat;

std::string DispatchCamera(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v == "camera.orbit")
    {
        float dx = 0.0f, dy = 0.0f;
        TryGetFloat(cmd, "dx", dx);
        TryGetFloat(cmd, "dy", dy);
        ctx.camera->Orbit(dx, dy);
        return ok();
    }
    if (v == "camera.truck")
    {
        float dx = 0.0f, dy = 0.0f;
        TryGetFloat(cmd, "dx", dx);
        TryGetFloat(cmd, "dy", dy);
        ctx.camera->Truck(dx, dy);
        return ok();
    }
    if (v == "camera.dolly")
    {
        float camera = 0.0f, target = 0.0f;
        TryGetFloat(cmd, "camera", camera);
        TryGetFloat(cmd, "target", target);
        ctx.camera->Dolly(camera, target);
        return ok();
    }
    if (v == "camera.zoom")
    {
        float fov = 0.0f;
        if (!TryGetFloat(cmd, "fov", fov))
            return fail("badparam");
        ctx.camera->FieldOfView(fov);
        return ok();
    }
    if (v == "camera.roll")
    {
        float angle = 0.0f;
        TryGetFloat(cmd, "angle", angle);
        ctx.camera->Roll(angle);
        return ok();
    }
    if (v == "camera.pan")
    {
        float x = 0.0f, y = 0.0f;
        TryGetFloat(cmd, "x", x);
        TryGetFloat(cmd, "y", y);
        ctx.camera->Pan(x, y);
        return ok();
    }
    if (v == "camera.place")
    {
        float px, py, pz, tx, ty, tz;
        if (!(TryGetFloat(cmd, "px", px) && TryGetFloat(cmd, "py", py) &&
              TryGetFloat(cmd, "pz", pz) && TryGetFloat(cmd, "tx", tx) &&
              TryGetFloat(cmd, "ty", ty) && TryGetFloat(cmd, "tz", tz)))
        {
            return fail("badparam");
        }
        ctx.camera->set_Position(forg::math::Vector3(px, py, pz));
        ctx.camera->set_Target(forg::math::Vector3(tx, ty, tz));
        return ok();
    }

    return fail("unknown");
}

} // namespace control
} // namespace forg
