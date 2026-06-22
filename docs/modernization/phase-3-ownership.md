# Phase 3: Ownership and Containers

## Status

Done. `RefPtr`, atomic reference counting, parser ownership fixes, standard
mesh containers, mesh `std::unique_ptr` factories, reference software buffer
RAII, image/audio/UI/OpenCL ownership cleanup, and X-loader temporary-buffer
RAII have landed. The custom `auto_ptr`, `core::vector`, and `shared_array`
headers remain installed for 1.x source compatibility; first-party canonical
builds no longer use `core::vector` or `shared_array`, and `auto_ptr` use is
limited to the legacy mesh compatibility typedef/API surface.

## Objective

Make ownership explicit and exception-safe internally while preserving the 1.x
source API.

## Implementation Slices

1. **Ownership inventory**
   - Document each factory/member as owning, borrowed, intrusive-reference, or
     platform-owned.
   - Prioritize image/DDS/BMP storage, software buffers, audio buffers, OpenGL
     resources, UI controls, lexer states, and X-file object graphs.

2. **Internal RAII migration**
   - Use `std::vector` for variable arrays, `std::array` for fixed buffers, and
     `std::unique_ptr` for exclusive heap objects.
   - Use `RefPtr` only for objects whose lifetime is governed by `AddRef/Release`;
     make adoption versus retention explicit before adding copy semantics.
   - Convert one subsystem per pull request and remove its manual cleanup only
     after failure-path tests exist.

3. **Compatibility factories**
   - Add `Mesh::UniqueMeshPtr = std::unique_ptr<Mesh>`.
   - Add modern `MakeBox`, `MakeSphere`, `MakeCylinder`, `MakePyramid`,
     `MakeGrid`, `MakeLandscape`, and `LoadFromFile` entry points.
   - Implement one unique-owner core path; existing `Box`, `Sphere`,
     `Cylinder`, `Pyramid`, `Grid`, `Landscape`, and `FromFile` APIs wrap the
     released pointer in legacy `MeshPtr`.
   - Defer `MakeTorus` until the existing declared-only `Torus` factory gets an
     implementation or is removed from the 1.x surface.

4. **Legacy container retirement**
   - In-tree `core::vector` and `shared_array` uses have been removed from the
     canonical build.
   - Custom `auto_ptr` remains only where required by legacy `MeshPtr` APIs and
     compatibility headers.
   - Compatibility headers stay installed through 1.x; remove them only in v2.

## Public Interfaces

Modern factories and `UniqueMeshPtr` are additive. Existing raw pointers,
`MeshPtr`, and intrusive resource interfaces remain supported during 1.x.

## Acceptance Gates

- No first-party canonical target uses `core::vector` or `shared_array`.
- ASan/UBSan and ownership failure-path tests report no leaks or invalid access.
- Legacy and modern mesh factories produce equivalent geometry and materials.
- Compatibility headers compile; legacy `MeshPtr` remains the only first-party
  `auto_ptr` bridge retained for 1.x source compatibility.
