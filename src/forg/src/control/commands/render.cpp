#include <string>

#include "control/commands/Commands.h"

namespace forg::control {

using forg::net::Command;
using forg::net::TryGetString;

std::string DispatchRender(SceneControlContext& ctx, const Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v == "render.capture")
    {
        if (ctx.device == nullptr)
            return fail("nodevice");

        std::string path = "backbuffer.ppm";
        TryGetString(cmd, "path", path);
        if (path.empty())
            return fail("badparam");

        if (ctx.device->SaveBackBuffer(path) != FORG_OK)
            return fail("capturefailed");

        return ok();
    }

    return fail("unknown");
}

} // namespace forg::control
