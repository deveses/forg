// Internal (non-installed) header shared by the forg/control command handlers.
// One Dispatch<Category> function per command file; the router in
// SceneControl.cpp forwards by verb prefix. Not part of the public API.

#ifndef _FORG_CONTROL_COMMANDS_H_
#define _FORG_CONTROL_COMMANDS_H_

#include <string>

#include "control/SceneControl.h"

namespace forg
{
namespace control
{

std::string DispatchCamera(SceneControlContext& ctx, const net::Command& cmd);
std::string DispatchMesh(SceneControlContext& ctx, const net::Command& cmd);
std::string DispatchScene(SceneControlContext& ctx, const net::Command& cmd);

// Shared response builders (inline: one definition across the command TUs).
inline std::string ok() { return "{\"ok\":true}"; }

inline std::string fail(const char* error)
{
    return std::string("{\"ok\":false,\"error\":\"") + error + "\"}";
}

} // namespace control
} // namespace forg

#endif //_FORG_CONTROL_COMMANDS_H_
