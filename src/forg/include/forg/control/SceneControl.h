// forg/control: maps parsed net::Command verbs onto live scene state.
// This is the layer that knows how a verb (e.g. "camera.orbit") turns into a
// forg call. It is transport-agnostic (no sockets) and deals only in forg
// rendering types, so any forg-based application can drive its scene with it.

#ifndef _FORG_CONTROL_SCENECONTROL_H_
#define _FORG_CONTROL_SCENECONTROL_H_

#include <string>

#include "base.h"
#include "Input.h"
#include "math/Matrix4.h"
#include "net/Command.h"
#include "rendering/Camera.h"
#include "rendering/Color.h"
#include "rendering/IRenderDevice.h"
#include "rendering/Light.h"
#include "scene/Model.h"

namespace forg::control {

using InputHandler = bool (*)(const forg::InputEvent& event, void* userData);

/// Pointers to the mutable scene state a command may touch. The application
/// owns the objects; this struct is just a non-owning view passed per call.
struct SceneControlContext
{
    forg::Camera* camera = nullptr;
    forg::scene::Model* model = nullptr;   // optional active renderable model
    forg::Light* light = nullptr;          // the app's light
    forg::Color* clearColor = nullptr;     // the app's clear color
    forg::IRenderDevice* device = nullptr; // needed to build/load meshes
    InputHandler inputHandler = nullptr;   // optional input event sink
    void* inputUserData = nullptr;
};

/// Executes one command against the scene and returns a JSON response body.
FORG_API std::string DispatchCommand(SceneControlContext& ctx,
                                     const net::Command& cmd);

} // namespace forg::control

#endif //_FORG_CONTROL_SCENECONTROL_H_
