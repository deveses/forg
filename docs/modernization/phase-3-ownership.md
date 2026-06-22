# Phase 3: Ownership and Containers

## Status

In progress. `RefPtr`, atomic reference counting, parser ownership fixes,
standard mesh containers, mesh `std::unique_ptr` factories, reference software
buffer RAII, and X-loader temporary-buffer RAII have landed. Custom `auto_ptr`,
`core::vector`, raw arrays, and manual cleanup remain in several rendering,
image, audio, UI, OpenCL, and Windows paths.

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
   - Remove in-tree uses of `core::vector`, `shared_array`, and custom `auto_ptr`.
   - Mark compatibility templates deprecated only after the canonical build is
     warning-clean and no first-party call site uses them.
   - Keep compatibility headers installed through 1.x; remove them only in v2.

## Public Interfaces

Modern factories and `UniqueMeshPtr` are additive. Existing raw pointers,
`MeshPtr`, and intrusive resource interfaces remain supported during 1.x.

## Acceptance Gates

- No first-party canonical target uses custom containers or custom `auto_ptr`.
- ASan/UBSan and ownership failure-path tests report no leaks or invalid access.
- Legacy and modern mesh factories produce equivalent geometry and materials.
- Compatibility headers compile but are not used internally.
