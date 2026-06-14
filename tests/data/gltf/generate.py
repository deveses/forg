#!/usr/bin/env python3
"""Generate the small glTF 2.0 fixtures used by GltfLoaderTests.

Pure standard-library (struct/json/base64) - no third-party deps. Run from any
directory; assets are written next to this script:

    python3 tests/data/gltf/generate.py

The committed .gltf/.glb/.bin files are the test inputs; this script exists so
they can be regenerated/audited.
"""

import base64
import json
import os
import struct

HERE = os.path.dirname(os.path.abspath(__file__))

# glTF constants
ARRAY_BUFFER = 34962
ELEMENT_ARRAY_BUFFER = 34963
FLOAT = 5126
UNSIGNED_SHORT = 5123
UNSIGNED_INT = 5125
TRIANGLES = 4


def _pad(data: bytes, align: int, fill: bytes) -> bytes:
    rem = len(data) % align
    return data if rem == 0 else data + fill * (align - rem)


class Builder:
    """Accumulates bufferViews/accessors into a single binary blob."""

    def __init__(self):
        self.blob = bytearray()
        self.buffer_views = []
        self.accessors = []

    def _view(self, raw: bytes, target: int) -> int:
        self.blob += _pad(bytes(self.blob), 4, b"\x00")[len(self.blob):]  # align start
        offset = len(self.blob)
        self.blob += raw
        self.buffer_views.append(
            {"buffer": 0, "byteOffset": offset, "byteLength": len(raw), "target": target}
        )
        return len(self.buffer_views) - 1

    def add_vec3(self, values, with_minmax=False) -> int:
        raw = b"".join(struct.pack("<3f", *v) for v in values)
        view = self._view(raw, ARRAY_BUFFER)
        acc = {"bufferView": view, "componentType": FLOAT, "count": len(values), "type": "VEC3"}
        if with_minmax:
            xs = [v[0] for v in values]; ys = [v[1] for v in values]; zs = [v[2] for v in values]
            acc["min"] = [min(xs), min(ys), min(zs)]
            acc["max"] = [max(xs), max(ys), max(zs)]
        self.accessors.append(acc)
        return len(self.accessors) - 1

    def add_vec2(self, values) -> int:
        raw = b"".join(struct.pack("<2f", *v) for v in values)
        view = self._view(raw, ARRAY_BUFFER)
        self.accessors.append(
            {"bufferView": view, "componentType": FLOAT, "count": len(values), "type": "VEC2"}
        )
        return len(self.accessors) - 1

    def add_indices(self, values) -> int:
        if max(values) > 0xFFFF:
            raw = b"".join(struct.pack("<I", i) for i in values)
            ctype = UNSIGNED_INT
        else:
            raw = b"".join(struct.pack("<H", i) for i in values)
            ctype = UNSIGNED_SHORT
        view = self._view(raw, ELEMENT_ARRAY_BUFFER)
        self.accessors.append(
            {"bufferView": view, "componentType": ctype, "count": len(values), "type": "SCALAR"}
        )
        return len(self.accessors) - 1


def base_doc(builder, meshes, materials=None, nodes=None, textures=None,
             images=None, samplers=None):
    if nodes is None:
        nodes = [{"mesh": 0}]
    doc = {
        "asset": {"version": "2.0", "generator": "forg test generator"},
        "scene": 0,
        "scenes": [{"nodes": list(range(len(nodes)))}],
        "nodes": nodes,
        "meshes": meshes,
        "accessors": builder.accessors,
        "bufferViews": builder.buffer_views,
    }
    if materials:
        doc["materials"] = materials
    if textures:
        doc["textures"] = textures
    if images:
        doc["images"] = images
    if samplers:
        doc["samplers"] = samplers
    return doc


def write_gltf_embedded(path, builder, doc):
    uri = "data:application/octet-stream;base64," + base64.b64encode(bytes(builder.blob)).decode("ascii")
    doc["buffers"] = [{"byteLength": len(builder.blob), "uri": uri}]
    with open(path, "w") as f:
        json.dump(doc, f, indent=2)
    print("wrote", os.path.relpath(path, HERE))


def write_gltf_external(path, bin_name, builder, doc):
    with open(os.path.join(HERE, bin_name), "wb") as f:
        f.write(bytes(builder.blob))
    doc["buffers"] = [{"byteLength": len(builder.blob), "uri": bin_name}]
    with open(path, "w") as f:
        json.dump(doc, f, indent=2)
    print("wrote", os.path.relpath(path, HERE), "+", bin_name)


def write_glb(path, builder, doc):
    blob = _pad(bytes(builder.blob), 4, b"\x00")
    doc["buffers"] = [{"byteLength": len(blob)}]
    json_bytes = _pad(json.dumps(doc).encode("utf-8"), 4, b" ")

    def chunk(tag, data):
        return struct.pack("<I", len(data)) + tag + data

    body = chunk(b"JSON", json_bytes) + chunk(b"BIN\x00", blob)
    header = struct.pack("<III", 0x46546C67, 2, 12 + len(body))
    with open(path, "wb") as f:
        f.write(header + body)
    print("wrote", os.path.relpath(path, HERE))


def out(name):
    return os.path.join(HERE, name)


def mat_color(rgba):
    return {"pbrMetallicRoughness": {"baseColorFactor": list(rgba)}}


# --- single triangle in the XY plane, CCW, +Z normals, with a coloured material
TRI_POS = [(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)]
TRI_NRM = [(0.0, 0.0, 1.0)] * 3
TRI_UV = [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]
TRI_IDX = [0, 1, 2]
TRI_RGBA = (0.25, 0.5, 0.75, 1.0)


def gen_triangle():
    b = Builder()
    p = b.add_vec3(TRI_POS, with_minmax=True)
    n = b.add_vec3(TRI_NRM)
    t = b.add_vec2(TRI_UV)
    i = b.add_indices(TRI_IDX)
    meshes = [{"primitives": [{"attributes": {"POSITION": p, "NORMAL": n, "TEXCOORD_0": t},
                               "indices": i, "material": 0, "mode": TRIANGLES}]}]
    doc = base_doc(b, meshes, materials=[mat_color(TRI_RGBA)])
    write_gltf_embedded(out("triangle.gltf"), b, doc)


def gen_triangle_external():
    b = Builder()
    p = b.add_vec3(TRI_POS, with_minmax=True)
    n = b.add_vec3(TRI_NRM)
    t = b.add_vec2(TRI_UV)
    i = b.add_indices(TRI_IDX)
    meshes = [{"primitives": [{"attributes": {"POSITION": p, "NORMAL": n, "TEXCOORD_0": t},
                               "indices": i, "material": 0, "mode": TRIANGLES}]}]
    doc = base_doc(b, meshes, materials=[mat_color(TRI_RGBA)])
    write_gltf_external(out("triangle_external.gltf"), "triangle_external.bin", b, doc)


def gen_translated():
    # Same triangle, but the node is translated by +10 on X. The loader must
    # bake the world transform into the vertex positions.
    b = Builder()
    p = b.add_vec3(TRI_POS, with_minmax=True)
    n = b.add_vec3(TRI_NRM)
    i = b.add_indices(TRI_IDX)
    meshes = [{"primitives": [{"attributes": {"POSITION": p, "NORMAL": n},
                               "indices": i, "mode": TRIANGLES}]}]
    nodes = [{"mesh": 0, "translation": [10.0, 0.0, 0.0]}]
    doc = base_doc(b, meshes, nodes=nodes)
    write_gltf_embedded(out("translated_triangle.gltf"), b, doc)


def gen_no_normals():
    b = Builder()
    p = b.add_vec3(TRI_POS, with_minmax=True)
    i = b.add_indices(TRI_IDX)
    meshes = [{"primitives": [{"attributes": {"POSITION": p}, "indices": i, "mode": TRIANGLES}]}]
    doc = base_doc(b, meshes)
    write_gltf_embedded(out("no_normals.gltf"), b, doc)


def gen_two_materials():
    # One mesh, two triangle primitives with distinct materials -> two subsets.
    b = Builder()
    p0 = b.add_vec3(TRI_POS, with_minmax=True)
    i0 = b.add_indices(TRI_IDX)
    pos2 = [(2.0, 0.0, 0.0), (3.0, 0.0, 0.0), (2.0, 1.0, 0.0)]
    p1 = b.add_vec3(pos2, with_minmax=True)
    i1 = b.add_indices(TRI_IDX)
    meshes = [{"primitives": [
        {"attributes": {"POSITION": p0}, "indices": i0, "material": 0, "mode": TRIANGLES},
        {"attributes": {"POSITION": p1}, "indices": i1, "material": 1, "mode": TRIANGLES},
    ]}]
    materials = [mat_color((1.0, 0.0, 0.0, 1.0)), mat_color((0.0, 1.0, 0.0, 1.0))]
    doc = base_doc(b, meshes, materials=materials)
    write_gltf_embedded(out("two_materials.gltf"), b, doc)


def gen_textured_glb():
    # Quad (two triangles), base-color texture referencing an external image.
    b = Builder()
    pos = [(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (1.0, 1.0, 0.0), (0.0, 1.0, 0.0)]
    uv = [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)]
    idx = [0, 1, 2, 0, 2, 3]
    p = b.add_vec3(pos, with_minmax=True)
    t = b.add_vec2(uv)
    i = b.add_indices(idx)
    meshes = [{"primitives": [{"attributes": {"POSITION": p, "TEXCOORD_0": t},
                               "indices": i, "material": 0, "mode": TRIANGLES}]}]
    materials = [{"pbrMetallicRoughness": {
        "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
        "baseColorTexture": {"index": 0}}}]
    textures = [{"source": 0, "sampler": 0}]
    images = [{"uri": "wood.png"}]
    samplers = [{}]
    doc = base_doc(b, meshes, materials=materials, textures=textures,
                   images=images, samplers=samplers)
    write_glb(out("quad_textured.glb"), b, doc)


def gen_grid32_glb():
    # > 65535 vertices to exercise the 32-bit index path. Only a sparse set of
    # triangles is emitted (incl. one referencing the highest index) so the
    # fixture stays small while still flipping the 32-bit flag and forcing the
    # loader to read a vertex count above 0xffff.
    width = 256
    vcount = 65540  # > 0xffff (65535)
    pos = [(float(i % width), float(i // width), 0.0) for i in range(vcount)]
    idx = []
    for x in range(width - 1):  # a strip of triangles on the first two rows
        v0 = x
        v1 = x + 1
        v2 = x + width
        v3 = v2 + 1
        idx += [v0, v1, v2, v1, v3, v2]
    idx += [vcount - 3, vcount - 2, vcount - 1]  # touch the top index
    b = Builder()
    p = b.add_vec3(pos, with_minmax=True)
    i = b.add_indices(idx)
    meshes = [{"primitives": [{"attributes": {"POSITION": p}, "indices": i, "mode": TRIANGLES}]}]
    doc = base_doc(b, meshes)
    write_glb(out("grid32.glb"), b, doc)


if __name__ == "__main__":
    gen_triangle()
    gen_triangle_external()
    gen_translated()
    gen_no_normals()
    gen_two_materials()
    gen_textured_glb()
    gen_grid32_glb()
