# Scene Serialization

`Scene`, `SceneNode`, `MeshNode`, and `Model` support serializer-based save/load
without direct filesystem or render-device dependencies. Callers provide an
`io::ISerializer`, so the backing store can be YAML, XML, memory, or a file
owned by the serializer implementation.

## API

```cpp
bool Scene::Save(io::ISerializer& serializer) const;
bool Scene::Load(io::ISerializer& serializer);
bool Scene::LoadResources(IRenderDevice* device);
```

`Scene::Load` restores the object graph, node order, parent links, node types,
and model metadata. It validates input before replacing the current scene.

Deserialization does not require an `IRenderDevice`. Render resources are created
later with `Scene::LoadResources(device)` or `Model::LoadResources(device)`.

## Serialized Model Data

Models serialize asset references and primitive metadata, not embedded geometry
or textures.

Serialized fields:

- mesh type: `none`, `file`, `box`, `sphere`, `cylinder`, `pyramid`, or `grid`
- source path and load options for file-backed meshes
- primitive parameters for generated meshes
- transform matrix

`Model::Load(filename, device, options)` remains a convenience path for loading
a file-backed mesh immediately. It also records the source path and options so
the model can be serialized later.

For primitives, use `SetBox`, `SetSphere`, `SetCylinder`, `SetPyramid`, or
`SetGrid` before `LoadResources(device)` so the mesh parameters remain
serializable.

## macapp

The macOS sample loads `scene.yml` from its resource directory after engine
initialization:

```cpp
forg::io::YAMLSerializer serializer;
serializer.OpenRead("scene.yml");
engine.Scene().Load(serializer);
engine.Scene().LoadResources(engine.Device());
```

`src/macapp/scene.yml` currently contains one cylinder `MeshNode`, matching the
old hardcoded demo scene. CMake copies it next to `config.yml` in the macapp
build output.

## Control Commands

The control server still operates on the application's current model pointer.
In `macapp`, that pointer is bound to the first loaded `MeshNode`.

Mesh commands update serializable metadata:

- `mesh.load` loads a file and records `source_path` plus `load_options`
- `mesh.box` records box parameters and creates resources
- `mesh.sphere` records sphere parameters and creates resources
- `mesh.cylinder` records cylinder parameters and creates resources
- `mesh.transform` updates the model transform

Scene-wide node selection and adding/removing nodes through control commands are
not implemented yet.

## Format Notes

YAML output uses a structured object/array shape produced by `YAMLSerializer`.
Example:

```yaml
scene:
  version: "1"
  nodes:
    count: "1"
    item_0:
      type: "MeshNode"
      parent: "-1"
      model:
        mesh_type: "cylinder"
        source_path: ""
        load_options: "0"
        mesh_params:
          radius1: "1.000000"
          radius2: "2.000000"
          length: "5.000000"
          slices: "10"
          stacks: "40"
        transform:
          m11: "1.000000"
          m22: "1.000000"
          m33: "1.000000"
          m44: "1.000000"
```

Version `1` supports built-in `SceneNode` and `MeshNode`. Custom node factories
can be added later.

## Tests

Coverage includes memory-backed scene round trips, YAML text/file round trips,
malformed input preserving the existing scene, model metadata restoration, and
primitive resource creation from serialized metadata.
