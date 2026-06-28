// forg/control: maps parsed net::Command verbs onto live scene state.
// This is the layer that knows how a verb (e.g. "camera.orbit") turns into a
// forg call. It is transport-agnostic (no sockets) and deals only in forg
// rendering types, so any forg-based application can drive its scene with it.

#ifndef _FORG_CONTROL_SCENECONTROL_H_
#define _FORG_CONTROL_SCENECONTROL_H_

#include <string>

#include "base.h"
#include "math/Matrix4.h"
#include "net/Command.h"
#include "rendering/Camera.h"
#include "rendering/Color.h"
#include "rendering/IRenderDevice.h"
#include "rendering/Light.h"
#include "scene/Model.h"

namespace forg::control {

/// Pointers to the mutable scene state a command may touch. The application
/// owns the objects; this struct is just a non-owning view passed per call.
struct SceneControlContext
{
    forg::Camera* camera;
    forg::scene::Model* model;   // optional active renderable model
    forg::Light* light;          // the app's light
    forg::Color* clearColor;     // the app's clear color
    forg::IRenderDevice* device; // needed to build/load meshes
};

/// Executes one command against the scene and returns a JSON response body.
FORG_API std::string DispatchCommand(SceneControlContext& ctx,
                                     const net::Command& cmd);

} // namespace forg::control

#endif //_FORG_CONTROL_SCENECONTROL_H_
