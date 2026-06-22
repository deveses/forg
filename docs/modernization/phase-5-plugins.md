# Phase 5: Renderer and Plugin Boundary

## Status

In progress. Software, Metal, and OpenGL plugins expose a versioned descriptor;
macOS and Windows loaders fall back to legacy `forgCreateRenderer`; software and
Metal lifecycle smoke tests pass on macOS. Destruction ABI, headless rendering,
and Windows/OpenGL runtime validation remain.

## Objective

Make plugin loading versioned, diagnosable, and safe across module boundaries
before any renderer virtual interface changes.

## Implementation Slices

1. **Descriptor lifecycle contract**
   - Define a version-2 descriptor containing structure size, API version,
     create callback, and destroy callback, using only fixed-width C-compatible
     fields and function pointers.
   - Keep loader support for the current version-1 descriptor and for plugins
     exposing only `forgCreateRenderer`.
   - Use the destroy callback for version 2 so Windows does not delete plugin
     objects through a potentially different CRT boundary.

2. **Loader behavior**
   - Centralize descriptor probing and validation instead of duplicating it in
     `macapp` and `winapp`.
   - Return distinct diagnostics for load failure, missing symbols, unsupported
     version, truncated descriptor, factory failure, and destroy failure.
   - Keep the dynamic-library handle alive until all plugin objects are destroyed.

3. **Backend coverage**
   - Implement the current descriptor in every canonical CMake backend: software,
     Metal, and OpenGL.
   - Document OpenCL and C++ AMP as non-canonical legacy plugins; do not claim
     compatibility until they receive build targets and tests.

4. **Rendering tests**
   - Add descriptor rejection and legacy-fallback unit tests.
   - Add load/create/destroy smoke tests for each backend on its supported host.
   - Add a deterministic headless reference-renderer triangle test with a stable
     pixel checksum or explicit sampled pixels.

## Public Interfaces

Version 2 adds a destroy callback without changing `IRenderer` or
`IRenderDevice` virtual layouts. Version 1 and legacy factory entry points remain
accepted throughout 1.x.

## Acceptance Gates

- All canonical plugins load, validate, create, and destroy on supported hosts.
- Version-1, version-2, malformed, future-version, and legacy-only cases are tested.
- No host performs cross-module `delete` for a version-2 plugin object.
- Headless reference output is stable in debug, release, and sanitizer builds.
