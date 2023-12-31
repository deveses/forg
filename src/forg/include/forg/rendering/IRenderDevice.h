/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005-2008  Slawomir Strumecki

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

#ifndef _FORG_IRENDERDEVICE_H_
#define _FORG_IRENDERDEVICE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "core/RefCounter.h"
#include "rendering/Color.h"
#include "rendering/Light.h"
#include "rendering/Material.h"
#include "math/Matrix4.h"
#include "rendering/IVertexBuffer.h"
#include "rendering/IIndexBuffer.h"
#include "rendering/VertexDeclaration.h"
#include "rendering/ITexture.h"
#include "enums.h"

namespace forg{

using namespace forg::math;

/// Viewport
struct Viewport {
    uint X;
    uint Y;
    uint Width;
    uint Height;
    float MinZ;
    float MaxZ;
};


/// IRenderDevice interface
/**
* IRenderDevice
* @author eses
* @version 1.0
* @date 07-2005
* @todo
* @bug
* @warning
*/
class IRenderDevice : public core::RefCounter
{
//Nested
public:

//Construction / Deconstruction
public:
	virtual ~IRenderDevice(){};


//Public Methods
public:
	/// Begins a scene.
	/**
	* Begins a scene.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
	virtual int BeginScene(void) = 0;

	/// Ends a scene that was begun by calling BeginScene.
	/**
	* Ends a scene that was begun by calling BeginScene.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
	virtual int EndScene(void) = 0;

	/// Clears one or more surfaces.
	/**
	* Clears one or more surfaces such as a render target, multiple render targets,
	* a stencil buffer, and a depth buffer.
	* @param flags
	* Combination of one or more ClearFlags flags that specify the surface(s) that will be cleared.
	* @param color
	* Clear a render target to this ARGB color.
	* @param zdepth
	* Clear the depth buffer to this new z value which ranges from 0 to 1. See remarks.
	* @param stencil
	* Clear the stencil buffer to this new value which ranges from 0 to 2n - 1 (n is the bit depth of the stencil buffer).
	* @return If the method succeeds, the return value is FORG_OK.
	* @see ClearFlags
	*/
	virtual int Clear(
		uint flags,
		Color color,
		float zdepth,
		int stencil) = 0;

	/// Presents the contents of the next buffer in the sequence of back buffers owned by the device.
	/**
	* Presents the contents of the next buffer in the sequence of back buffers owned by the device.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
	virtual int Present() = 0;

	/// Resets the type, size, and format of the swap chain.
	/**
	* Resets the type, size, and format of the swap chain.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
	virtual int Reset() = 0;

	/// Create a vertex shader declaration from the device and the vertex elements.
	/**
	* Create a vertex shader declaration from the device and the vertex elements.
	* @param pVertexElements
	* An array of VertexElement.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see VertexElement
	*/
	virtual LPVERTEXDECLARATION CreateVertexDeclaration(const VertexElement* pVertexElements) = 0;

	/// Creates a vertex buffer.
	/**
	* Creates a vertex buffer.
	* @param length
	* Size of the vertex buffer, in bytes.
	* @param usage
	* Usage can be 0, which indicates no usage value.
	* However, if usage is desired, use a combination of one or more Usage constants.
	* It is good practice to match the usage parameter in CreateVertexBuffer with the behavior flags
	* in CreateDevice.
	* @param pool
	* Member of the Pool enumerated type, describing a valid memory class into which to place the resource.
	* Do not set to Pool_Scratch.
	* @return Pointer to an IVertexBuffer interface, representing the created vertex buffer resource.
	* @see Usage, Pool
	*/
	virtual LPVERTEXBUFFER CreateVertexBuffer(
		uint length,
		uint usage,
		uint pool
		) = 0;

	/// Creates an index buffer.
	/**
	* Creates an index buffer.
	* @param length
	* Size of the index buffer, in bytes.
	* @param usage
	* Usage can be 0, which indicates no usage value. However, if usage is desired,
	* use a combination of one or more Usage constants. It is good practice to match
	* the usage parameter in CreateIndexBuffer with the behavior flags in CreateDevice.
	* @param sixteenBitIndices
	* Set to true if the index buffer contains 16-bit indices.
	* Set to false if the index buffer contains 32-bit indices.
	* @param pool
	* Member of the Pool enumerated type, describing a valid memory class into which to place the resource.
	* @return Pointer to an IIndexBuffer interface, representing the created index buffer resource.
	*/
	virtual	LPINDEXBUFFER CreateIndexBuffer(
			uint length,
			uint usage,
			bool sixteenBitIndices,
			uint pool
			) = 0;

	virtual LPTEXTURE CreateTexture(
		uint Width,
		uint Height,
		uint Levels,
		uint Usage,
		uint Format,
		uint Pool
		) = 0;

	virtual LPTEXTURE CreateTextureFromFile(
		const char* filename,
		uint Width,
		uint Height,
		uint Levels,
		uint Usage,
		uint Format,
		uint Pool
		) = 0;

	/// Based on indexing, renders the specified geometric primitive into an array of vertices.
	/**
	* Based on indexing, renders the specified geometric primitive into an array of vertices.
	* @param primitiveType
	* Member of the PrimitiveType enumerated type, describing the type of primitive to render.
	* @param baseVertex
	* Offset from the start of the vertex buffer to the first vertex.
    * BaseVertexIndex is a value that's effectively added to every VB Index stored in the index buffer
	* @param minVertexIndex
	* Minimum vertex index for vertices used during this call. This is a zero based index relative to baseVertex.
	* @param numVertices
	* Number of vertices used during this call. The first vertex is located at index: baseVertex + minVertexIndex.
	* @param startIndex
	* Index of the first index to use when accesssing the vertex buffer. Beginning at StartIndex to index vertices
	* from the vertex buffer.
	* @param primCount
	* Number of primitives to render. The number of vertices used is a function of the primitive count and the primitive type.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
	virtual int DrawIndexedPrimitive(
		PrimitiveType primitiveType,
		int baseVertex,
		int minVertexIndex,
		int numVertices,
		int startIndex,
		int primCount) = 0;

	/// Renders the specified geometric primitive with data specified by a user memory pointer.
	/**
	* Renders the specified geometric primitive with data specified by a user memory pointer.
	* @param primitiveType
	* Member of the PrimitiveType enumerated type, describing the type of primitive to render.
	* @param minVertexIndex
	* Minimum vertex index. This is a zero-based index.
	* @param numVertexIndices
	* Number of vertices used during this call. The first vertex is located at index: minVertexIndex.
	* @param primitiveCount
	* Number of primitives to render.
	* @param indexData
	* User memory pointer to the index data.
	* @param sixteenBitIndices
	* Set to true to indicate 16-bit indices. Set to false if to indicate 32-bit indices.
	* @param vertexStreamZeroData
	* User memory pointer to the vertex data. The vertex data must be in stream 0.
	* @param vertexStreamZeroStride
	* The number of bytes of data for each vertex. This value may not be 0.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see PrimitiveType
	*/
	virtual int DrawIndexedUserPrimitives(
		PrimitiveType primitiveType,
		uint minVertexIndex,
		uint numVertexIndices,
		uint primitiveCount,
		const void* indexData,
		bool sixteenBitIndices,
		const void* vertexStreamZeroData,
		uint vertexStreamZeroStride) = 0;

	/// Sets a single device transformation-related state.
	/**
	* Sets a single device transformation-related state.
	* @param state
	* Device-state variable that is being modified. This parameter can be any member of
	* the TransformType enumerated type.
	* @param matrix
	* Reference to a Matrix4 structure that modifies the current transformation.
	*/
	virtual void SetTransform(TransformType state, const Matrix4& matrix) = 0;

    /// Retrieves a matrix describing a transformation state.
    virtual void GetTransform(TransformType state, Matrix4& matrix) = 0;

	/// Sets a Vertex Declaration.
	/**
	* Sets a Vertex Declaration.
	* @param pDecl
	* Pointer to an VertexDeclaration object, which contains the vertex declaration.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see VertexDeclaration
	*/
	virtual int SetVertexDeclaration(const VertexDeclaration* pDecl) = 0;

	/// Binds a vertex buffer to a device data stream.
	/**
	* Binds a vertex buffer to a device data stream.
	* @param streamNumber
	* Specifies the data stream, in the range from 0 to the maximum number of streams -1.
	* @param streamData
	* Pointer to an IVertexBuffer interface, representing the vertex buffer to bind to the specified data stream.
	* @param offsetInBytes
	* Offset from the beginning of the stream to the beginning of the vertex data, in bytes.
	* @param stride
	* Stride of the component, in bytes.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see IVertexBuffer
	*/
	virtual int SetStreamSource(
		int streamNumber,
		IVertexBuffer* streamData,
		int offsetInBytes,
		int stride) = 0;

	/// Sets index data.
	/**
	* Sets index data.
	* @param pIndexData
	* Pointer to an IIndexBuffer interface, representing the index data to be set.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see IIndexBuffer
	*/
	virtual int SetIndices(
		IIndexBuffer* pIndexData
		) = 0;

	/// Sets the viewport parameters for the device.
	/**
	* Sets the viewport parameters for the device.
	* @param X
	* Pixel coordinate of the upper-left corner of the viewport on the render-target surface.
	* Unless you want to render to a subset of the surface, this member can be set to 0.
	* @param Y
	* Pixel coordinate of the upper-left corner of the viewport on the render-target surface.
	* Unless you want to render to a subset of the surface, this member can be set to 0.
	* @param Width
	* Width dimension of the clip volume, in pixels. Unless you are rendering only to a subset of the surface,
	* this member should be set to the width dimension of the render-target surface.
	* @param Height
	* Height dimension of the clip volume, in pixels. Unless you are rendering only to a subset of the surface,
	* this member should be set to the height dimension of the render-target surface.
	* @param MinZ
	* Together with MaxZ, value describing the range of depth values into which a scene is to be rendered,
	* the minimum and maximum values of the clip volume. Most applications set this value to 0.0.
	* Clipping is performed after applying the projection matrix.
	* @param MaxZ
	* Together with MinZ, value describing the range of depth values into which a scene is to be rendered,
	* the minimum and maximum values of the clip volume. Most applications set this value to 1.0.
	* Clipping is performed after applying the projection matrix.
	* @return If the method succeeds, the return value is FORG_OK.
	*/
    virtual int SetViewport(uint X, uint Y, uint Width, uint Height, float MinZ = 0.0f, float MaxZ = 1.0f) = 0;

    /// Retrieves the viewport parameters currently set for the device.
    /**
    * Retrieves the viewport parameters currently set for the device.
    * @param viewport
    * [out] Pointer to a Viewport structure, representing the returned viewport parameters
    */
    virtual int GetViewport(Viewport* viewport) = 0;

	/// Sets a single device render-state parameter.
	/**
	* Sets a single device render-state parameter.
	* @param state
	* Device state variable that is being modified. This parameter can be any member of
	* the RenderStates enumerated type.
	* @param value
	* New value for the device render state to be set. The meaning of this parameter
	* is dependent on the value specified for State. For example, if State were RenderStates_ShadeMode,
	* the second parameter would be one member of the ShadeMode enumerated type.
	* @return If the method succeeds, the return value is FORG_OK.
	* @see RenderStates
	*/
	virtual int SetRenderState(uint state, uint value) = 0;

	virtual int SetTexture(
		uint Sampler,
		ITexture* pTexture
		) = 0;

    virtual int SetLight(
        uint Index,
        const Light* pLight
        ) = 0;

    virtual int LightEnable(
        uint LightIndex,
        bool bEnable
        ) = 0;

    virtual int SetMaterial(
        const Material* pMaterial
        ) = 0;
};

typedef IRenderDevice* LPRENDERDEVICE;

}

#endif  //_FORG_IRENDERDEVICE_H_
