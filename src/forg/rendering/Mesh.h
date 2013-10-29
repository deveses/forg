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

#ifndef _FORG_MESH_H_
#define _FORG_MESH_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "rendering/IRenderer.h"
#include "rendering/ExtendedMaterial.h"

#include "core/auto_ptr.hpp"
#include "core/vector.hpp"

namespace forg { namespace geometry {

using namespace forg::core;

struct AttributeRange {
    uint AttribId;
    uint FaceStart;
    uint FaceCount;
    uint VertexStart;
    uint VertexCount;
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
    typedef auto_ptr<Mesh> MeshPtr;
    typedef auto_ptr<IVertexBuffer, RefCountOwner<IVertexBuffer> > VertexBufferPtr;
    typedef auto_ptr<IIndexBuffer, RefCountOwner<IIndexBuffer> > IndexBufferPtr;

    typedef core::vector<ExtendedMaterial> ExtendedMaterialVec;
    typedef core::vector<AttributeRange> AttributeRangeVec;

    //////////////////////////////////////////////////////////////////////
    // 'structors
    //////////////////////////////////////////////////////////////////////
public:
	 Mesh(
		uint NumFaces,
		uint NumVertices,
		uint Options,
		const VertexElement* pDeclaration,
		LPRENDERDEVICE pDevice
		);
	virtual ~Mesh(void);

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////////
private:
	VertexBufferPtr m_vertex_buffer;
	IndexBufferPtr m_index_buffer;
	VertexDeclaration m_vertex_declaration;
    AttributeRangeVec m_attribtab;

	uint m_num_faces;
	uint m_num_vertices;
	uint m_stride_size;
	uint m_options;

    //////////////////////////////////////////////////////////////////////////
    // Association
    //////////////////////////////////////////////////////////////////////////
private:
    LPRENDERDEVICE m_device;


public:
	/// Uses a left-handed coordinate system to create a mesh that contains an axis-aligned box.
	/**
	* Uses a left-handed coordinate system to create a mesh that contains an axis-aligned box.
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
	static MeshPtr Box(
		IRenderDevice* device,
		float width,
		float height,
		float depth);

    /// Uses a left-handed coordinate system to create a mesh that contains a sphere.
    static MeshPtr Sphere(
        IRenderDevice* device,
        float radius,
        int slices,
        int stacks);

    /// Uses a left-handed coordinate system to create a mesh that contains a cylinder.
	static MeshPtr Cylinder(
		IRenderDevice* device,
		float radius1,
		float radius2,
		float length,
		int slices,
		int stacks
		);

    static MeshPtr  Torus(
        IRenderDevice* device,
        float innerRadius,
        float outerRadius,
        int sides,
        int rings
        );

	static MeshPtr Pyramid(
		IRenderDevice* device,
		uint numAngles,
		float radius,
		float height
		);

	static MeshPtr Grid(
		IRenderDevice* device,
		float sizeX,
        float sizeY,
        int color,
        uint subgrid
		);

    static MeshPtr Landscape(
        IRenderDevice* _device,
        const Vector3& _span,
        const float* _hmap,
        unsigned int _sizex, unsigned int _sizey
        );

    static MeshPtr FromFile(
        const char* filename,
        uint options,
        IRenderDevice* device
        );

	static MeshPtr FromFile(
		const char* filename,
		uint options,
		IRenderDevice* device,
        ExtendedMaterialVec& materials
		);

	LPVERTEXBUFFER GetVertexBuffer() const;

	LPINDEXBUFFER GetIndexBuffer() const;

	const VertexDeclaration* GetVertexDeclaration() const;

	uint GetNumVertices() const;

	uint GetNumFaces() const;

	uint GetNumBytesPerVertex() const;

	uint GetOptions() const;

    /// Sets the attribute table for a mesh and the number of entries stored in the table.
    /**
    * Sets the attribute table for a mesh and the number of entries stored in the table.
    */
    int SetAttributeTable(const AttributeRange* pAttribTable, uint cAttribTableSize);

    /// Locks a vertex buffer and obtains a pointer to the vertex buffer memory.
    /**
    * Locks a vertex buffer and obtains a pointer to the vertex buffer memory.
    */
	int LockVertexBuffer(
		uint Flags,
		void** ppData
		);

    /// Locks an index buffer and obtains a pointer to the index buffer memory.
    /**
    * Locks an index buffer and obtains a pointer to the index buffer memory.
    */
	int LockIndexBuffer(
		uint Flags,
		void** ppData
		);

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
    int DrawSubset(uint attributeID);

    /**
    * Performs tangent frame computations on a mesh. Tangent, binormal, and optionally normal vectors are generated.
    * Singularities are handled as required by grouping edges and splitting vertices.
    */
    void ComputeTangentFrame(uint options);


    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
    private:
    static MeshPtr FromPly(
        const char* filename,
        uint options,
        IRenderDevice* device
        );
    static MeshPtr FromX(
        const char* filename,
        uint options,
        IRenderDevice* device,
        ExtendedMaterialVec& materials
        );
};

}}

#endif //_FORG_MESH_H_
