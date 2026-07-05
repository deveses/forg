// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#include "forg_pch.h"

#include "mesh/GLTFLoader.h"

#include "debug/dbg.h"
#include "math/Math.h"
#include "rendering/IRenderDevice.h"
#include "rendering/Vertex.h"

#include <cstring>
#include <string>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace forg::gltf {

namespace {

// glTF / cgltf node matrices are column-major: element (row, col) is m[col*4 +
// row].

// Transform a position (w = 1) by a column-major 4x4 matrix.
inline Vector3 TransformPoint(const float* m, float x, float y, float z)
{
    return Vector3(m[0] * x + m[4] * y + m[8] * z + m[12],
                   m[1] * x + m[5] * y + m[9] * z + m[13],
                   m[2] * x + m[6] * y + m[10] * z + m[14]);
}

// Transform a direction (normal) by the upper-3x3 of a column-major 4x4 matrix.
// This is exact for rigid and uniform-scale transforms; non-uniform scale would
// require the inverse-transpose, which v1 does not handle. Normals are
// re-normalized by the caller.
inline Vector3 TransformDir(const float* m, float x, float y, float z)
{
    return Vector3(m[0] * x + m[4] * y + m[8] * z,
                   m[1] * x + m[5] * y + m[9] * z,
                   m[2] * x + m[6] * y + m[10] * z);
}

const cgltf_accessor* FindAttribute(const cgltf_primitive* prim,
                                    cgltf_attribute_type type, cgltf_int index)
{
    for (cgltf_size a = 0; a < prim->attributes_count; ++a)
    {
        if (prim->attributes[a].type == type &&
            prim->attributes[a].index == index)
            return prim->attributes[a].data;
    }
    return 0;
}

// Map a glTF material onto FORG's fixed-function material model. PBR is
// approximated: base color becomes the diffuse/ambient colour and the
// base-color texture URI becomes the (relative) texture filename.
ExtendedMaterial MapMaterial(const cgltf_material* mat)
{
    ExtendedMaterial em;

    em.Material3D.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
    em.Material3D.Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);
    em.Material3D.Specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
    em.Material3D.Emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    em.Material3D.Power = 0.0f;

    if (mat == 0)
        return em;

    if (mat->has_pbr_metallic_roughness)
    {
        const cgltf_float* bc = mat->pbr_metallic_roughness.base_color_factor;
        em.Material3D.Diffuse = Color(bc[0], bc[1], bc[2], bc[3]);
        em.Material3D.Ambient = em.Material3D.Diffuse;

        const cgltf_texture* tex =
            mat->pbr_metallic_roughness.base_color_texture.texture;
        if (tex != 0 && tex->image != 0 && tex->image->uri != 0)
        {
            const char* uri = tex->image->uri;
            // Skip embedded data: URIs - there is no usable file path.
            if (std::strncmp(uri, "data:", 5) != 0)
            {
                std::string decoded(uri);
                if (!decoded.empty())
                {
                    cgltf_size n = cgltf_decode_uri(&decoded[0]);
                    decoded.resize(n);
                }
                // Stored relative to the model file, like the X loader; the
                // caller (e.g. scene::Model::Load) prepends the model
                // directory.
                em.TextureFilename = forg::core::string(decoded.c_str());
            }
        }
    }

    em.Material3D.Emissive =
        Color(mat->emissive_factor[0], mat->emissive_factor[1],
              mat->emissive_factor[2], 1.0f);

    return em;
}

// Recompute per-vertex normals over the vertex range [vStart, vStart+vCount)
// using the faces [fStart, fStart+fCount). Each glTF primitive owns a disjoint
// vertex range, so this is self-contained per primitive.
void ComputeNormals(GltfLoader::CpuMesh& m, u32 vStart, u32 vCount, u32 fStart,
                    u32 fCount)
{
    for (u32 i = 0; i < vCount; ++i)
        m.vertices[vStart + i].Normal.Zero();

    for (u32 f = 0; f < fCount; ++f)
    {
        u32 i0 = m.indices[(fStart + f) * 3 + 0];
        u32 i1 = m.indices[(fStart + f) * 3 + 1];
        u32 i2 = m.indices[(fStart + f) * 3 + 2];

        Vector3 e0 = m.vertices[i1].Position - m.vertices[i0].Position;
        Vector3 e1 = m.vertices[i2].Position - m.vertices[i0].Position;

        Vector3 normal;
        Vector3::Cross(normal, e0, e1);

        m.vertices[i0].Normal += normal;
        m.vertices[i1].Normal += normal;
        m.vertices[i2].Normal += normal;
    }

    for (u32 i = 0; i < vCount; ++i)
        m.vertices[vStart + i].Normal.Normalize();
}

void AppendPrimitive(const cgltf_primitive* prim, const float* world,
                     GltfLoader::CpuMesh& out)
{
    if (prim->type != cgltf_primitive_type_triangles)
    {
        DBG_MSG("[GltfLoader] Skipping non-triangle primitive (type %d)\n",
                (int)prim->type);
        return;
    }

    const cgltf_accessor* pos =
        FindAttribute(prim, cgltf_attribute_type_position, 0);
    if (pos == 0 || pos->count == 0)
    {
        DBG_MSG("[GltfLoader] Primitive has no POSITION attribute; skipping\n");
        return;
    }

    const cgltf_accessor* nor =
        FindAttribute(prim, cgltf_attribute_type_normal, 0);
    const cgltf_accessor* uv =
        FindAttribute(prim, cgltf_attribute_type_texcoord, 0);

    const u32 vStart = out.vertices.size();
    const u32 vCount = (u32)pos->count;

    for (u32 i = 0; i < vCount; ++i)
    {
        PositionNormalTextured v;

        float p[3] = {0.0f, 0.0f, 0.0f};
        cgltf_accessor_read_float(pos, i, p, 3);
        v.Position = TransformPoint(world, p[0], p[1], p[2]);

        if (nor != 0)
        {
            float n[3] = {0.0f, 0.0f, 0.0f};
            cgltf_accessor_read_float(nor, i, n, 3);
            v.Normal = TransformDir(world, n[0], n[1], n[2]);
            v.Normal.Normalize();
        }
        else
        {
            v.Normal.Zero();
        }

        if (uv != 0)
        {
            float t[2] = {0.0f, 0.0f};
            cgltf_accessor_read_float(uv, i, t, 2);
            v.Tu = t[0];
            v.Tv = t[1];
        }
        else
        {
            v.Tu = 0.0f;
            v.Tv = 0.0f;
        }

        out.vertices.push_back(v);
    }

    const u32 fStart = out.indices.size() / 3;

    if (prim->indices != 0 && prim->indices->count > 0)
    {
        const cgltf_size ic = prim->indices->count;

        std::vector<u32> tmp(ic);
        cgltf_accessor_unpack_indices(prim->indices, tmp.data(), sizeof(u32),
                                      ic);

        for (cgltf_size k = 0; k < ic; ++k)
            out.indices.push_back(tmp[(u32)k] + vStart);
    }
    else
    {
        // Non-indexed primitive: emit a sequential triangle list.
        for (u32 k = 0; k < vCount; ++k)
            out.indices.push_back(vStart + k);
    }

    const u32 fCount = (out.indices.size() / 3) - fStart;

    if (nor == 0)
        ComputeNormals(out, vStart, vCount, fStart, fCount);

    AttributeRange ar;
    ar.AttribId = out.subsets.size();
    ar.FaceStart = fStart;
    ar.FaceCount = fCount;
    ar.VertexStart = vStart;
    ar.VertexCount = vCount;
    out.subsets.push_back(ar);

    out.materials.push_back(MapMaterial(prim->material));
}

void ProcessNode(const cgltf_node* node, GltfLoader::CpuMesh& out)
{
    float world[16];
    cgltf_node_transform_world(node, world);

    if (node->mesh != 0)
    {
        for (cgltf_size p = 0; p < node->mesh->primitives_count; ++p)
            AppendPrimitive(&node->mesh->primitives[p], world, out);
    }

    for (cgltf_size c = 0; c < node->children_count; ++c)
        ProcessNode(node->children[c], out);
}

} // anonymous namespace

bool GltfLoader::Flatten(const char* filename, CpuMesh& out)
{
    cgltf_options options;
    std::memset(&options, 0, sizeof(options));

    cgltf_data* data = 0;
    if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success)
    {
        DBG_MSG("[GltfLoader] Failed to parse <%s>\n", filename);
        return false;
    }

    if (cgltf_load_buffers(&options, data, filename) != cgltf_result_success)
    {
        DBG_MSG("[GltfLoader] Failed to load buffers for <%s>\n", filename);
        cgltf_free(data);
        return false;
    }

    const cgltf_scene* scene =
        data->scene ? data->scene : (data->scenes_count ? &data->scenes[0] : 0);

    if (scene != 0)
    {
        for (cgltf_size i = 0; i < scene->nodes_count; ++i)
            ProcessNode(scene->nodes[i], out);
    }
    else
    {
        // No scene defined: walk every root node (children are visited by
        // recursion, world transforms resolve the hierarchy).
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
        {
            if (data->nodes[i].parent == 0)
                ProcessNode(&data->nodes[i], out);
        }
    }

    cgltf_free(data);

    if (out.vertices.size() == 0 || out.indices.size() == 0)
    {
        DBG_MSG("[GltfLoader] <%s> produced no triangle geometry\n", filename);
        return false;
    }

    out.use32bit = (out.vertices.size() > 0xffff);
    return true;
}

Mesh::MeshPtr GltfLoader::Load(const char* filename, u32 /*options*/,
                               IRenderDevice* device,
                               Mesh::ExtendedMaterialVec& materials)
{
    CpuMesh cpu;
    if (!Flatten(filename, cpu))
        return nullptr;

    const u32 numFaces = cpu.indices.size() / 3;
    const u32 numVerts = cpu.vertices.size();

    Mesh::MeshPtr m(new Mesh(numFaces, numVerts,
                             cpu.use32bit ? MeshFlags::Use32Bit : 0,
                             PositionNormalTextured::Declaration, device));

    PositionNormalTextured* vb = 0;
    if (m && m->LockVertexBuffer(0, (void**)&vb) == FORG_OK)
    {
        std::memcpy(vb, cpu.vertices.data(),
                    numVerts * sizeof(PositionNormalTextured));
        m->UnlockVertexBuffer();
    }

    void* ib = 0;
    if (m && m->LockIndexBuffer(0, &ib) == FORG_OK)
    {
        if (cpu.use32bit)
        {
            std::memcpy(ib, cpu.indices.data(), numFaces * 3 * sizeof(u32));
        }
        else
        {
            unsigned short* dst = (unsigned short*)ib;
            for (u32 i = 0; i < numFaces * 3; ++i)
                dst[i] = (unsigned short)cpu.indices[i];
        }
        m->UnlockIndexBuffer();
    }

    m->SetAttributeTable(cpu.subsets.data(),
                         static_cast<u32>(cpu.subsets.size()));

    materials.clear();
    for (u32 i = 0; i < cpu.materials.size(); ++i)
        materials.push_back(cpu.materials[i]);

    return m;
}

} // namespace forg::gltf
