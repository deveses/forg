/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef FORG_RENDERING_MESH_H
#define FORG_RENDERING_MESH_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "rendering/ExtendedMaterial.h"
#include "rendering/IRenderer.h"

#include "core/RefPtr.h"

#include <memory>
#include <vector>

namespace forg::geometry {

struct AttributeRange
{
    u32 AttribId;
    u32 FaceStart;
    u32 FaceCount;
    u32 VertexStart;
    u32 VertexCount;
};

/// Mesh class
/**
 * Mesh
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class FORG_API Mesh
{
    //////////////////////////////////////////////////////////////////////
    // Nested
    //////////////////////////////////////////////////////////////////////
  public:
    using UniqueMeshPtr = std::unique_ptr<Mesh>;
    using MeshPtr = UniqueMeshPtr;
    using VertexBufferPtr = core::RefPtr<IVertexBuffer>;
    using IndexBufferPtr = core::RefPtr<IIndexBuffer>;
    using ExtendedMaterialVec = std::vector<ExtendedMaterial>;
    using AttributeRangeVec = std::vector<AttributeRange>;

    //////////////////////////////////////////////////////////////////////
    // 'structors
    //////////////////////////////////////////////////////////////////////
  public:
    Mesh(u32 NumFaces, u32 NumVertices, u32 Options,
         const VertexElement* pDeclaration, LPRENDERDEVICE pDevice);
    virtual ~Mesh(void);

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////////
  private:
    core::RefPtr<IVertexBuffer> m_vertex_buffer;
    core::RefPtr<IIndexBuffer> m_index_buffer;
    VertexDeclaration m_vertex_declaration;
    AttributeRangeVec m_attribtab;

    u32 m_num_faces;
    u32 m_num_vertices;
    u32 m_stride_size;
    u32 m_options;
    PrimitiveType m_primitive_type = PrimitiveType_TriangleList;

    //////////////////////////////////////////////////////////////////////////
    // Association
    //////////////////////////////////////////////////////////////////////////
  private:
    LPRENDERDEVICE m_device;

  public:
    /// Uses a left-handed coordinate system to create a mesh that contains an
    /// axis-aligned box.
    /**
     * Uses a left-handed coordinate system to create a mesh that contains an
     * axis-aligned box.
     * @param device
     * A IRenderDevice object.
     * @param width
     * Width of the box along the x-axis.
     * @param height
     * Height of the box along the y-axis.
     * @param depth
     * Depth of the box along the z-axis.
     * @return A Mesh object that contains the box.
     */
    static MeshPtr Box(IRenderDevice* device, float width, float height,
                       float depth);
    static UniqueMeshPtr MakeBox(IRenderDevice* device, float width,
                                 float height, float depth);

    /// Uses a left-handed coordinate system to create a mesh that contains a
    /// sphere.
    static MeshPtr Sphere(IRenderDevice* device, float radius, int slices,
                          int stacks);
    static UniqueMeshPtr MakeSphere(IRenderDevice* device, float radius,
                                    int slices, int stacks);

    /// Uses a left-handed coordinate system to create a mesh that contains a
    /// cylinder.
    static MeshPtr Cylinder(IRenderDevice* device, float radius1, float radius2,
                            float length, int slices, int stacks);
    static UniqueMeshPtr MakeCylinder(IRenderDevice* device, float radius1,
                                      float radius2, float length, int slices,
                                      int stacks);

    static MeshPtr Torus(IRenderDevice* device, float innerRadius,
                         float outerRadius, int sides, int rings);

    static MeshPtr Pyramid(IRenderDevice* device, u32 numAngles, float radius,
                           float height);
    static UniqueMeshPtr MakePyramid(IRenderDevice* device, u32 numAngles,
                                     float radius, float height);

    static MeshPtr Grid(IRenderDevice* device, float sizeX, float sizeY,
                        int color, u32 subgrid);
    static UniqueMeshPtr MakeGrid(IRenderDevice* device, float sizeX,
                                  float sizeY, int color, u32 subgrid);

    static MeshPtr Landscape(IRenderDevice* _device, const Vector3& _span,
                             const float* _hmap, unsigned int _sizex,
                             unsigned int _sizey);
    static UniqueMeshPtr MakeLandscape(IRenderDevice* _device,
                                       const Vector3& _span, const float* _hmap,
                                       unsigned int _sizex,
                                       unsigned int _sizey);

    static MeshPtr FromFile(const char* filename, u32 options,
                            IRenderDevice* device);
    static UniqueMeshPtr LoadFromFile(const char* filename, u32 options,
                                      IRenderDevice* device);

    static MeshPtr FromFile(const char* filename, u32 options,
                            IRenderDevice* device,
                            ExtendedMaterialVec& materials);
    static UniqueMeshPtr LoadFromFile(const char* filename, u32 options,
                                      IRenderDevice* device,
                                      ExtendedMaterialVec& materials);

    LPVERTEXBUFFER GetVertexBuffer() const;

    LPINDEXBUFFER GetIndexBuffer() const;

    const VertexDeclaration* GetVertexDeclaration() const;

    u32 GetNumVertices() const;

    u32 GetNumFaces() const;

    u32 GetNumBytesPerVertex() const;

    u32 GetOptions() const;

    void SetPrimitiveType(PrimitiveType primitiveType);
    PrimitiveType GetPrimitiveType() const;

    /// Sets the attribute table for a mesh and the number of entries stored in
    /// the table.
    /**
     * Sets the attribute table for a mesh and the number of entries stored in
     * the table.
     */
    int SetAttributeTable(const AttributeRange* pAttribTable,
                          u32 cAttribTableSize);

    /// Locks a vertex buffer and obtains a pointer to the vertex buffer memory.
    /**
     * Locks a vertex buffer and obtains a pointer to the vertex buffer memory.
     */
    int LockVertexBuffer(u32 Flags, void** ppData);

    /// Locks an index buffer and obtains a pointer to the index buffer memory.
    /**
     * Locks an index buffer and obtains a pointer to the index buffer memory.
     */
    int LockIndexBuffer(u32 Flags, void** ppData);

    /// Unlocks a vertex buffer.
    /**
     * Unlocks a vertex buffer.
     */
    int UnlockVertexBuffer();

    /// Unlocks an index buffer.
    /**
     * Unlocks an index buffer.
     */
    int UnlockIndexBuffer();

    /// Draws a subset of a mesh.
    /**
     * Draws a subset of a mesh.
     */
    int DrawSubset(u32 attributeID);

    /**
     * Performs tangent frame computations on a mesh. Tangent, binormal, and
     * optionally normal vectors are generated. Singularities are handled as
     * required by grouping edges and splitting vertices.
     */
    void ComputeTangentFrame(u32 options);

    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
  private:
    static MeshPtr FromPly(const char* filename, u32 options,
                           IRenderDevice* device);
    static MeshPtr FromX(const char* filename, u32 options,
                         IRenderDevice* device, ExtendedMaterialVec& materials);
    static MeshPtr FromGltf(const char* filename, u32 options,
                            IRenderDevice* device,
                            ExtendedMaterialVec& materials);
};

} // namespace forg::geometry

#endif // FORG_RENDERING_MESH_H
