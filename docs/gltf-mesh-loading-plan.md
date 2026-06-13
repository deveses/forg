# glTF Static Mesh Loading Plan

## Summary

Implement v1 glTF 2.0 loading for static models using vendored `cgltf`.

The first milestone supports `.glb` and `.gltf` static meshes with positions, normals, UVs, indices, materials, and base-color textures. Animation, skinning, morph targets, cameras, and lights are out of scope for v1.

## Key Changes

- Vendor `cgltf` as a single-header dependency under `extern/cgltf/cgltf.h`.
- Add a new FORG mesh loader, `GltfLoader`, beside the existing X/PLY loaders.
- Add `Mesh::FromGltf(...)` and update `Mesh::FromFile(...)` dispatch:
  - `.ply` -> existing PLY loader.
  - `.gltf` / `.glb` -> new glTF loader.
  - everything else -> existing X loader for backward compatibility.
- Build one flattened `geometry::Mesh` from the glTF default scene:
  - Use `PositionNormalTextured::Declaration`.
  - Apply static node transforms to vertex positions and normals.
  - Merge all triangle primitives into one vertex/index buffer.
  - Use 16-bit indices when possible, otherwise `MeshFlags::Use32Bit`.
  - Compute normals when missing.
  - Use `(0, 0)` UVs when missing.
- Map glTF materials into existing `ExtendedMaterial`:
  - `baseColorFactor` -> `Material3D.Diffuse`.
  - `baseColorTexture` -> `TextureFilename`, resolved relative to the model file.
  - One `AttributeRange` per glTF primitive/material subset.
- Update build source lists so the new loader is compiled by the library build.

## Public API / Interfaces

- Keep existing app usage unchanged:
  - `forg::geometry::Mesh::FromFile(path, options, device, materials)`
  - `forg::scene::Model::Load(path, device)`
- Add only private/internal mesh-loading API:
  - `Mesh::FromGltf(const char* filename, uint options, IRenderDevice* device, ExtendedMaterialVec& materials)`
  - `forg::gltf::GltfLoader::Load(...)`
- No changes to `IRenderer`, `IRenderDevice`, or the Win32 `Viewport` model-loading flow.

## Test Plan

- Build the project after adding the loader.
- Verify existing behavior still works:
  - Load a `.ply` model.
  - Load an existing `.x` model, if available.
- Add or test with simple glTF assets:
  - `.glb` triangle or cube without texture.
  - `.gltf` with external `.bin`.
  - `.gltf` or `.glb` with base-color texture.
  - Model with multiple primitives/materials.
  - Model with more than 65,535 vertices to exercise 32-bit indices.
  - Model missing normals to verify computed normals.
- In the Win32 app, press `O`, select a `.glb`/`.gltf`, and confirm it displays with expected geometry, material color, and texture.

## Assumptions

- Use `cgltf` rather than `tinygltf` or Assimp.
- v1 prioritizes reliable static display over full glTF feature coverage.
- glTF PBR is approximated through FORG's existing fixed-function material model.
- Unsupported glTF features should be ignored gracefully, not treated as load failures unless mesh geometry cannot be produced.
