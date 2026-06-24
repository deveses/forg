# Scene Serialization Implementation Plan

## Summary
Save this plan to `docs/scene-serialization.md`, then add serializer-based `Scene`, `SceneNode`, and `MeshNode` serialization. `Scene` uses `io::ISerializer` only, with no filename or render-device dependency.

## Key Changes
- Extend `io::ISerializer` into a bidirectional structured interface:
  - reading/writing mode
  - begin/end object
  - begin/end array with count
  - typed scalar values for `int`, `uint`, `float`, and `core::string`
- Add memory-backed serializer tests first, so scene round-trips do not require filesystem IO.
- Add `Scene` APIs:
  - `bool Save(io::ISerializer& serializer) const;`
  - `bool Load(io::ISerializer& serializer);`
- Add `SceneNode` virtual serialization hooks:
  - `TypeName()`
  - `Save(io::ISerializer&) const`
  - `Load(io::ISerializer&)`
- Add `MeshNode` serialization for model metadata.
- Add `Model` serializable metadata:
  - source path
  - load options
  - transform matrix
- Keep render resource loading separate:
  - `Model::LoadResources(IRenderDevice*)`
  - optional `Scene::LoadResources(IRenderDevice*)`
  - existing `Model::Load(filename, device, options)` remains as convenience and records metadata.

## Behavior
- `Scene::Save` serializes only nodes owned by `Scene`.
- `Scene::Load` restores object graph, node order, parent links, node types, and model metadata.
- `Scene::Load` validates input before mutating the current scene.
- Mesh geometry and textures are not embedded in v1; only asset references and transforms are serialized.
- Deserialization never requires `IRenderDevice`.

## Test Plan
- Round-trip empty, flat, nested, and mixed scenes through an in-memory serializer.
- Verify node count, order, parent links, runtime types, model path/options, and matrix values.
- Verify malformed input fails and leaves the existing scene unchanged.
- Verify `Model::LoadResources(device)` reloads from restored metadata.
- Run existing scene and engine tests.

## Assumptions
- Plan file path: `docs/scene-serialization.md`.
- v1 supports built-in `SceneNode` and `MeshNode`; custom node factories can be added later.
- Serializer implementations own file or memory backing; scene classes never open files directly.
