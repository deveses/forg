# Plan: Native Metal rendering backend for FORG (macOS)

## Context

FORG's only hardware-accelerated backend, `glrenderer`, is Windows-only and built on **legacy
fixed-function OpenGL** (WGL context, `glBegin/glEnd`, matrix stack, fixed lighting). On macOS:

- **OpenGL is deprecated** (since 10.14) and capped at the 2.1 legacy profile. It still runs, but
  it is a dead end.
- **Metal** is Apple's native, supported, hardware-accelerated API.

The user wants a *forward-looking* macOS backend, so the decision is to **write a new
`MetalRenderDevice` that implements FORG's existing `IRenderDevice` abstraction** — *not* to port
`glrenderer`. The fixed-function GL code does not translate (Metal has no fixed-function pipeline);
the `IRenderDevice` interface is the seam the engine was designed to swap backends at. Other options
(Vulkan/MoltenVK, bgfx) were rejected: extra translation layers / another abstraction under FORG's
own, with no upside on Apple hardware.

**Outcome:** a `libmetalrenderer.dylib` plugin that the existing `macapp` loads exactly like
`libswrenderer.dylib`, rendering the demo cylinder lit, on the GPU, via Metal.

## Scope

Implement the subset of `IRenderDevice` the demo actually exercises (verified by exploration), with
clean, correct Metal — not a 1:1 emulation of every D3D-style render state. Demo needs:
`Clear`, `Begin/EndScene`, `Present`, `Reset`, `SetTransform` (World/View/Proj), `SetViewport`,
`SetRenderState` (Cull/Shade/Fill/Lighting/blend — honor what's cheap, stub the rest),
`SetLight`/`LightEnable`, `SetVertexDeclaration`, `SetStreamSource`, `SetIndices`,
`DrawIndexedPrimitive` (TriangleList, 16-bit), `SetTexture(0, null)`.

Out of scope for first pass (interface implemented as no-op/stub, clearly marked): textures &
`CreateTexture*`, `SetMaterial`, `DrawIndexedUserPrimitives`, non-triangle primitives, multi-stream.
These are easy follow-ups once the pipeline is proven.

## Plugin structure (mirror `src/swrenderer/`)

New directory `src/metalrenderer/`. Follow the existing factory/entry-point convention exactly
(`src/swrenderer/SWRenderer.{h,cpp}`):

- `extern "C" forg::IRenderer* forgCreateRenderer()` is the dlopen'd symbol (no `__declspec` needed
  on macOS — `DLLEXPORT` is empty there).
- Factory `MetalRenderer : IRenderer` → `CreateDevice(HWIN, RENDER_PARAMETERS*)` news the device and
  calls an `Initialize(width,height)` (same shape as `SWRenderer::CreateDevice`).

### Files to create

- `src/metalrenderer/MetalRenderer.cpp` — `IRenderer` factory + `forgCreateRenderer`. Plain C++ (no
  Metal here) so it matches `SWRenderer.cpp`.
- `src/metalrenderer/MetalRenderDevice.h` — `class MetalRenderDevice : public IRenderDevice;`
  declares all interface methods + private state. Keep ObjC types out of the header (use
  `void*`/opaque or pimpl) so it can be included from plain `.cpp`.
- `src/metalrenderer/MetalRenderDevice.mm` — the real implementation (Objective-C++). All Metal
  objects (`MTLDevice`, `CAMetalLayer`, command queue, pipeline state, depth state, depth texture)
  and the per-frame encode/draw logic live here.
- `src/metalrenderer/MetalBuffers.{h,mm}` — `MetalVertexBuffer`/`MetalIndexBuffer : I*Buffer`,
  each wrapping an `MTLBuffer` (storageModeShared). `Lock` → `[buf contents] + offset`; `Unlock` →
  no-op. `Mesh::DrawSubset` locks at creation to upload geometry, so this is required.
- `src/metalrenderer/CMakeLists.txt` — `add_library(metalrenderer SHARED)`, sources above, link
  `forg` + frameworks `Metal`, `QuartzCore`, `Cocoa` (MetalKit not needed — we use `CAMetalLayer`
  directly, no `MTKView`).

MSL shaders are embedded as a source string in `MetalRenderDevice.mm` and compiled with
`newLibraryWithSource:` at init — avoids adding a `.metal` build step.

### Files to modify

- `src/CMakeLists.txt` — add `add_subdirectory(metalrenderer)` inside the existing `if(APPLE)` block.
- `src/macapp/CMakeLists.txt` — add `metalrenderer` to `add_dependencies(macapp ...)` and a
  `copy_if_different $<TARGET_FILE:metalrenderer>` line next to the existing `swrenderer` copy.
- `src/macapp/config.xml` — switch `<renderer driver="libmetalrenderer.dylib"/>` to run Metal by
  default (swap back to `libswrenderer.dylib` to compare). `macapp/main.mm` needs **no changes** —
  it already passes the `NSView*` as `HWIN` and dlopens by driver name.

## Key technical decisions (the tricky bits)

1. **Presentation — plugin swaps in a `CAMetalLayer`.** In `Initialize`/`Reset`, take
   `NSView* view = (NSView*)GetHWIN()`, create a `CAMetalLayer` (`device`, `pixelFormat =
   BGRA8Unorm`, `framebufferOnly = YES`, `contentsScale`, `drawableSize`), and host it:
   `view.layer = metalLayer; view.wantsLayer = YES;`. Keeps `macapp` renderer-agnostic (same trick
   `swrenderer` uses with `layer.contents`).

2. **`Clear()` precedes `BeginScene()`** in the demo's frame. Design around it:
   - `Clear(flags,color,z,stencil)` — just **records** clear color/flags + depth value (no GPU work).
   - `BeginScene()` — `[layer nextDrawable]`, build `MTLRenderPassDescriptor` with
     `loadAction = Clear` using the recorded color and the depth attachment, create command buffer +
     render command encoder, set viewport + depth-stencil state + cull mode.
   - draws encode into that encoder.
   - `EndScene()` — `[encoder endEncoding]`.
   - `Present()` — `[cmdBuf presentDrawable:drawable]; [cmdBuf commit];`.

3. **Transforms — no CPU transpose.** Device keeps `m_world/m_view/m_proj` (set by `SetTransform`)
   and computes `MVP = World·View·Proj` using FORG's own `Matrix4::Multiply` (the same combine the
   reference `SWRenderDevice::SetTransform` does). Upload the 16 raw floats into a constant buffer.
   FORG is row-major with **row-vector** convention (`v·M`); MSL `float4x4` loaded column-major from
   those same bytes equals `M^T`, so `mvp * float4(pos,1)` in the vertex shader is exactly correct.
   Also upload `World` separately for lighting (world-space position/normal).

4. **Lighting — per-pixel point-light diffuse** in the fragment shader using the `Light` struct
   (`Position`, `Diffuse`, `Ambient`). Recommend proper Lambert: `d = max(0, dot(N, normalize(Lpos -
   Pworld)))`, `color = (Ambient + d*Diffuse) * baseColor`. (The reference renderer uses a quirky
   per-vertex `(dot+1)*0.5 * Diffuse*Ambient`; we can match it exactly if visual parity with
   `swrenderer` is required — note this as a toggle, but a correct Lambert is the forward-looking
   choice.) Demo world matrix is identity, so untransformed normals are fine; still multiply normal
   by `World` for correctness.

5. **Vertex layout & pipeline cache.** `SetVertexDeclaration` builds an `MTLVertexDescriptor` from
   the FORG `VertexElement[]` (map `DeclarationType_Float3→float3`, `Float2→float2`, etc.; attribute
   index by `DeclarationUsage`: Position=0, Normal=1, TexCoord=2). Build/cache one
   `MTLRenderPipelineState` keyed by (declaration, blend state). For the demo this is a single
   pipeline; the cache keeps it general.

6. **Depth buffer** recreated whenever drawable size changes (in `Reset()`, sized from
   `view.bounds`, mirroring `SWRenderDevice::Reset`): a `Depth32Float` texture + a
   `MTLDepthStencilState` (depthCompare=Less, write enabled). Demo clears Z and relies on the test.

7. **ObjC/Cocoa header-ordering gotcha** (already documented for the project): in `.mm` files,
   `#import <Metal/Metal.h>`, `<QuartzCore/CAMetalLayer.h>`, `<Cocoa/Cocoa.h>` **before** any FORG
   header — `base.h` defines `null`/`IN`/`OUT` macros that break system headers otherwise.

## Implementation phases (each independently verifiable)

1. **Skeleton + clear.** Plugin builds & loads; `CAMetalLayer` hosted; `Clear`+`Begin/End`+`Present`
   show a gray window via Metal. *Verify:* `macapp` opens a gray 800×600 window, no software buffer.
2. **Geometry + transform.** Vertex/index buffers, vertex descriptor, pipeline, depth; draw the
   cylinder **unlit** (flat color) with correct MVP + depth. *Verify:* tapered cylinder appears in
   correct camera pose, solid, depth-correct.
3. **Lighting.** Point-light diffuse in the shader. *Verify:* cylinder is shaded; visually matches
   the `swrenderer` reference scene (switch `config.xml` to compare).
4. **Polish/stubs.** Honor Cull/Fill/blend render states; stub `CreateTexture*`/`SetMaterial`/
   `DrawIndexedUserPrimitives` with clear no-ops + comments. *Verify:* full clean build, no warnings
   beyond expected, demo stable at the 60fps timer cap.

## Verification (end to end)

```sh
cmake --preset debug && cmake --build --preset debug
./build/debug/src/macapp/macapp        # config.xml driver = libmetalrenderer.dylib
```

- Window shows the **lit tapered cylinder** on a gray background, GPU-rendered via Metal, hitting the
  60fps timer cap (release: `cmake --preset release && cmake --build --preset release`).
- **Side-by-side correctness:** flip `config.xml` driver between `libmetalrenderer.dylib` and
  `libswrenderer.dylib` — same camera, same cylinder, comparable shading.
- **Confirm the GPU path:** run with the Metal HUD (`MTL_HUD_ENABLED=1`) or a quick Instruments
  Metal capture to confirm real draw calls / a `CAMetalLayer` drawable (not a CPU blit).

## Notes / risks

- No tests exist in FORG; verification is visual + build success (consistent with the project).
- This adds a genuinely new module; it touches only new files plus three small wiring edits
  (`src/CMakeLists.txt`, `macapp/CMakeLists.txt`, `config.xml`) — no changes to `forg` core or
  `macapp/main.mm`.
- `CMAKE_OSX_ARCHITECTURES` is hardcoded `arm64` (root `CMakeLists.txt`) — fine for Metal on Apple
  Silicon.
