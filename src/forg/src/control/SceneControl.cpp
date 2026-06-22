#include <string>

#include "control/SceneControl.h"
#include "control/commands/Commands.h"

namespace forg::control {

// Routes a command to its category handler by verb prefix. The per-category
// handlers live in commands/{camera,mesh,scene}.cpp.
std::string DispatchCommand(SceneControlContext& ctx, const net::Command& cmd)
{
    const std::string& v = cmd.verb;

    if (v.rfind("camera.", 0) == 0)
        return DispatchCamera(ctx, cmd);
    if (v.rfind("mesh.", 0) == 0)
        return DispatchMesh(ctx, cmd);
    if (v.rfind("light.", 0) == 0 || v.rfind("clear.", 0) == 0 || v == "state")
        return DispatchScene(ctx, cmd);

    return fail("unknown");
}

} // namespace forg::control
