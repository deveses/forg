/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2012  Slawomir Strumecki

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

#pragma once

#include "forg.h"
#include "rendering/IRenderDevice.h"
#include "rendering/VertexDeclaration.h"
#include "math/Vector4.h"
#include "math/Vector2.h"
#include "core/core.h"

#include <amp.h>

namespace forg {

struct VSInput
{
    Vector4 position;
    Vector4 color;
    Vector3 texcoord0;
    Vector4 normal;
    Vector4 tangent;
};

// TODO: change to cl_ types for proper alignment
struct VSOutput
{
    Vector4 position;
    Vector4 color;
    Vector3 texcoord0;
    float unused;
};

struct PSInput
{
    Vector4 color;
    Vector3 texcoord0;
    Vector2 vpos;
    float vface;
};

struct PSOutput
{
    Vector4 color;
    float depth;
};

struct SWSampler
{
    ITexture* texture;
    int mipfilter;

    SWSampler();
    ~SWSampler();

    void SetTexture(ITexture* _texture);
    uint Sample(float u, float v);
};

struct AMPBuffers
{
};

class SWRenderDevice
	: public IRenderDevice
{
    enum 
    {
        TM_VIEW = 0,
        TM_PROJECTION,
        TM_MODELVIEW,
        TM_MODELVIEWPROJ,
        TM_VIEWPORT,
        TM_WORLD,
        TM_WORLD1,
        TM_WORLD2,
        TM_WORLD3,
        TM_TEXTURE0,
        TM_TEXTURE1,
        TM_TEXTURE2,
        TM_TEXTURE3,
        TM_TEXTURE4,
        TM_TEXTURE5,
        TM_TEXTURE6,
        TM_TEXTURE7,
        _TM_COUNT
    };

    enum { NUM_SAMPLERS = 16};
    enum { NUM_STREAMS = 8};
    enum { NUM_LIGHTS = 8};

    struct SStreamSource
    {
        IVertexBuffer* streamData;
        int offsetInBytes;
        int stride;

        SStreamSource()
        {
            Clear();
        }

        void Clear()
        {
            streamData = 0;
            offsetInBytes = 0;
            stride = 0;
        }

        bool IsData() const { return (streamData!=0); }
    };

    int m_refCount;
    HWIN m_window;

    // Frame buffer: ARGB, each component is an uint8 (32bits per pixel)
    uint* m_frame_buffer;
    float* m_depth_buffer;
    uint m_fb_size;
    uint m_fb_pitch;
    uint m_zb_pitch;
    uint m_width;
    uint m_height;

    uint m_vp_x;
    uint m_vp_y;
    uint m_vp_width;
    uint m_vp_height;
    float m_vp_minz;
    float m_vp_maxz;

    VertexDeclaration m_vdecl;

    Matrix4 m_transforms[_TM_COUNT];

    Material m_material;
    Light    m_lights[NUM_LIGHTS];

    // Associations
    IIndexBuffer* m_indices;
    SStreamSource m_streams[NUM_STREAMS];
    //ITexture* m_textures[NUM_SAMPLERS];
    SWSampler m_samplers[NUM_SAMPLERS];

public:
    SWRenderDevice(HWIN handle);
	virtual ~SWRenderDevice();

    int Initialize(uint _width, uint _height);
    
    // Helpers
private:
    bool InitializeCL();
    void CreateBuffers();

    int ProcessVertex(VSInput& _input, VSOutput& _output, int _usage);
    void ProcessPixel(PSInput& _input, PSOutput& _output, int _usage);

    float GetDepth(uint _x, uint _y);
    // Rasterisation
    void SetPixel(uint _x, uint _y, float _z, uint _c);
    void DrawTriangle(const Vector3* pos);
    void DrawTriangle(const VSOutput* vertices, int usage);
    void DrawTriangleArray(const VSOutput* vertices, uint num_triangles, int usage);
    void DrawTriangleArrayAMP(VSOutput* vertices, uint num_triangles, int usage);

    // IRenderDevice implementation
public:
    virtual int BeginScene(void);
    virtual int EndScene(void);
    virtual int Clear(uint flags, Color color, float zdepth, int stencil);
    virtual int Present();
    virtual int Reset();
    
    virtual LPVERTEXDECLARATION CreateVertexDeclaration(const VertexElement* pVertexElements) { return 0; }
	virtual LPVERTEXBUFFER CreateVertexBuffer(
		uint length,
		uint usage,
		uint pool
        );
	virtual	LPINDEXBUFFER CreateIndexBuffer(
			uint length,
			uint usage,
			bool sixteenBitIndices,
			uint pool
            );

	virtual LPTEXTURE CreateTexture(
		uint Width,
		uint Height,
		uint Levels,
		uint Usage,
		uint Format,
		uint Pool
        );

	virtual LPTEXTURE CreateTextureFromFile(
		const char* filename,
		uint Width,
		uint Height,
		uint Levels,
		uint Usage,
		uint Format,
		uint Pool
        ) { return 0; }

	virtual int DrawIndexedPrimitive(
		PrimitiveType primitiveType,
		int baseVertex,
		int minVertexIndex,
		int numVertices,
		int startIndex,
        int primCount);

	virtual int DrawIndexedUserPrimitives(
		PrimitiveType primitiveType,
		uint minVertexIndex,
		uint numVertexIndices,
		uint primitiveCount,
		const void* indexData,
		bool sixteenBitIndices,
		const void* vertexStreamZeroData,
        uint vertexStreamZeroStride);

    virtual void SetTransform(TransformType state, const Matrix4& matrix);

    virtual void GetTransform(TransformType state, Matrix4& matrix);

    virtual int SetVertexDeclaration(const VertexDeclaration* pDecl);

	virtual int SetStreamSource(
		int streamNumber,
		IVertexBuffer* streamData,
		int offsetInBytes,
        int stride);

	virtual int SetIndices(
		IIndexBuffer* pIndexData
        );

    virtual int SetViewport(uint X, uint Y, uint Width, uint Height, float MinZ = 0.0f, float MaxZ = 1.0f);

    virtual int GetViewport(Viewport* viewport);

    virtual int SetRenderState(uint state, uint value) { return 0; };

	virtual int SetTexture(uint Sampler, ITexture* pTexture);

    virtual int SetLight(uint Index, const Light* pLight);

    virtual int LightEnable(
        uint LightIndex,
        bool bEnable
        ) { return 0; };

    virtual int SetMaterial(const Material* pMaterial);

};

}

