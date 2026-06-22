# Phase 3: Ownership and Containers

## Status

Done. `RefPtr`, atomic reference counting, parser ownership fixes, standard
mesh containers, mesh `std::unique_ptr` factories, reference software buffer
RAII, image/audio/UI/OpenCL ownership cleanup, and X-loader temporary-buffer
RAII have landed. The custom destructive-copy pointer header has been removed,
`MeshPtr` is a `std::unique_ptr<Mesh>` alias, and first-party maintained code no
longer uses `core::vector` or `shared_array`.

## Objective

Make ownership explicit and exception-safe internally.

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

3. **Mesh factories**
   - Add `Mesh::UniqueMeshPtr = std::unique_ptr<Mesh>`.
   - Add modern `MakeBox`, `MakeSphere`, `MakeCylinder`, `MakePyramid`,
     `MakeGrid`, `MakeLandscape`, and `LoadFromFile` entry points.
   - Implement one unique-owner core path; `Box`, `Sphere`, `Cylinder`,
     `Pyramid`, `Grid`, `Landscape`, and `FromFile` return the same standard
     owner type through `MeshPtr`.
   - Defer `MakeTorus` until the existing declared-only `Torus` factory gets an
     implementation or is removed from the 1.x surface.

4. **Legacy container retirement**
   - In-tree `core::vector` and `shared_array` uses have been removed from
     maintained code.
   - The custom destructive-copy pointer and its installed header have been
     removed.
   - `MeshPtr` now has move-only `std::unique_ptr` semantics.

## Public Interfaces

`MeshPtr` and `UniqueMeshPtr` are standard unique-owner mesh pointers. Existing
raw pointers and intrusive resource interfaces remain supported where they are
part of renderer/device ownership.

## Acceptance Gates

- No maintained first-party target uses the custom destructive-copy pointer,
  `core::vector`, or `shared_array`.
- ASan/UBSan and ownership failure-path tests report no leaks or invalid access.
- Named mesh factories produce equivalent geometry and materials through the
  standard unique-owner path.
- Public headers compile without the removed custom pointer compatibility
  header.
