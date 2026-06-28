# Engine/Scene Refactor Plan

Goal: move platform-neutral rendering, scene, camera, and control glue out of
both app layers (`src/macapp` and `src/winapp`) and into `forg::Engine` /
`forg::scene::Scene`. Keep platform code responsible for windows, timers,
native input capture, message loops, UI overlays, and process/resource directory
setup.

## Implementation status

Implemented in this pass:

- `Engine::LoadScene(...)` loads YAML scenes and resources.
- Engine owns default light state and applies it during scene rendering.
- Engine renders the scene before invoking the render callback, so app callbacks
  can be overlay-only.
- Engine owns clear color for control commands.
- `Engine::DispatchCommand(...)` builds the control context internally.
- Engine owns the HTTP control server and command queue; apps only start the
  server from config.
- macapp and winapp both load `scene.yml`.
- Neither app auto-selects a model; mesh commands return `nomodel` until an app
  selects one.
- macapp and winapp share `CameraOrbitController` for orbit/truck/zoom input
  math.

## Non-goals

- Do not move Cocoa or Win32 window creation into `Engine`.
- Do not move native event registration, timers, message loops, or bundle /
  executable directory switching into `Engine`.
- Do not redesign serialization, renderer plugins, or the scene graph while
  doing the first pass.

## Step 1: Add Engine scene loading

Add an `Engine::LoadScene(std::string_view filename)` helper that wraps:

- opening a YAML serializer;
- `Scene().Load(...)`;
- `Scene().LoadResources(Device())`;
- useful `LastError()` messages.

Then replace the manual `scene.yml` load/resource block in `src/macapp/main.mm`.
Make `src/winapp` use the same `Engine::LoadScene(...)` path so both app
targets load `scene.yml`.

Verification:

- mac app still launches from `scene.yml`;
- winapp launches from `scene.yml`;
- failure paths report through `Engine::LastError()`;
- existing tests/build still pass.

## Step 2: Centralize default light state

Move the duplicated default point light setup out of app code.

Short-term API options:

```cpp
static Light Engine::DefaultLight();
void Engine::SetLight(uint index, const Light& light);
Light& Engine::Light(uint index);
```

Keep this in `Engine` initially because the current renderer API applies lights
through `IRenderDevice::SetLight` and `LightEnable`. A later, larger step can
introduce a real `LightNode` in `Scene`.

Verification:

- remove the large `s_Light` initialization from `main.mm`;
- remove the matching duplicate setup from `src/winapp/Viewport.cpp`;
- rendered scene lighting remains unchanged.

## Step 3: Remove app render callback where possible

The mac render callback currently sets light state and calls
`Scene().Render(device)`. Win32 does the same scene/light work before drawing its
debug UI overlay. Once Engine owns light application and scene rendering, both
apps should rely on `Engine::Render()` for the shared scene path.

The mac app should be able to delete:

- `-[AppDelegate render]`;
- `RenderEngineFrame(...)`;
- `m_engine.SetRenderCallback(...)` from mac startup/shutdown.

Win32 may still need a callback while it renders debug UI overlays after the
scene. If so, keep the callback there but reduce it to app/UI-only concerns.

Verification:

- mac app renders through the default `Engine::Render()` path;
- winapp scene rendering goes through the shared engine path;
- Win32 UI overlay still renders if the callback remains there.

## Step 4: Move clear color ownership fully into Engine

`Engine` already stores clear color internally, but the mac app keeps a duplicate
`m_clear_color` so control commands can mutate it. Win32 should also use the
same engine-owned clear color path when it participates in shared control or
scene commands.

Suggested API:

```cpp
const Color& Engine::ClearColor() const;
void Engine::SetClearColor(const Color& color);
```

If command dispatch moves into `Engine`, direct mutable access may not be needed.

Verification:

- remove `m_clear_color` from `AppDelegate`;
- ensure winapp does not add or keep separate clear-color state for shared
  commands;
- control commands can still read and update clear color;
- no per-frame `SetClearColor(m_clear_color)` call remains.

## Step 5: Route control commands through Engine

Move app-side command context assembly into an engine-level helper.

Suggested API:

```cpp
std::string Engine::DispatchCommand(const net::Command& cmd);
```

The HTTP server and `CommandQueue` are Engine-owned. They handle transport and
threading while command execution uses engine-owned camera, scene, active model
if one is selected, light, clear color, and render device state. This is usable
from both macapp and winapp.

Verification:

- app code only starts the server from config;
- any winapp command/control entrypoint uses the same engine dispatch API;
- control commands still operate on camera, mesh/model, light, and clear color;
- command execution remains on the main thread.

## Step 6: Extract camera interaction math

Keep native input capture in each app, but move the shared interpretation of
mouse deltas / scroll deltas into reusable code. macapp converts Cocoa event
deltas; winapp converts Win32 mouse messages.

Suggested small type:

```cpp
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
```

Verification:

- macapp and winapp use the same controller values;
- native handlers only convert events to deltas/buttons;
- orbit, truck, and zoom behavior remains unchanged.

## Suggested execution order

1. `Engine::LoadScene(...)`
2. Engine-owned light setup
3. Remove mac render callback
4. Engine-owned clear color access
5. `Engine::DispatchCommand(...)`
6. Camera interaction controller

This order keeps each change small and lets the mac app shrink steadily while
preserving the platform boundary.
