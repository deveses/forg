// ControlCommands: maps parsed net::Command verbs onto the live scene state.
// This is the only place that knows how a verb (e.g. "camera.orbit") turns
// into a forg call. It runs on the main thread, drained from the CommandQueue.

#ifndef _FORG_MACAPP_CONTROLCOMMANDS_H_
#define _FORG_MACAPP_CONTROLCOMMANDS_H_

#include <string>

#include "forg.h"
#include "forg/net/Command.h"

/// Pointers to the mutable scene state a command may touch. Owned by the app.
struct SceneControlContext
{
    forg::Camera*                       camera;
    forg::geometry::Mesh::MeshPtr*      mesh;       // the app's m_mesh
    forg::Matrix4*                      meshTm;     // the app's m_mesh_tm
    forg::Light*                        light;      // the app's s_Light
    forg::Color*                        clearColor; // the app's m_clear_color
    forg::IRenderDevice*                device;     // needed to build/load meshes
};

/// Executes one command against the scene and returns a JSON response body.
std::string DispatchCommand(SceneControlContext& ctx, const forg::net::Command& cmd);

#endif //_FORG_MACAPP_CONTROLCOMMANDS_H_
