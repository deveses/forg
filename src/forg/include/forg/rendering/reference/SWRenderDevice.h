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

#ifndef FORG_RENDERING_REFERENCE_SWRENDERDEVICE_H
#define FORG_RENDERING_REFERENCE_SWRENDERDEVICE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "math/Vector2.h"
#include "math/Vector4.h"
#include "rendering/IRenderDevice.h"
#include "rendering/VertexDeclaration.h"

#include <memory>

namespace forg::rendering::reference {

struct VSInput
{
    Vector4 position;
    Vector4 color;
    Vector3 texcoord0;
    Vector4 normal;
    Vector4 tangent;
};

struct VSOutput
{
    Vector4 position;
    Vector4 color;
    Vector3 texcoord0;
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
    u32 Sample(float u, float v);
};

class FORG_API SWRenderDevice : public IRenderDevice
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

    enum
    {
        NUM_SAMPLERS = 16
    };
    enum
    {
        NUM_STREAMS = 8
    };
    enum
    {
        NUM_LIGHTS = 8
    };

    struct SStreamSource
    {
        IVertexBuffer* streamData;
        int offsetInBytes;
        int stride;

        SStreamSource() { Clear(); }

        void Clear()
        {
            streamData = 0;
            offsetInBytes = 0;
            stride = 0;
        }

        bool IsData() const { return (streamData != 0); }
    };

    int m_refCount;
    HWIN m_window;

    std::unique_ptr<u32[]> m_frame_buffer;
    std::unique_ptr<float[]> m_depth_buffer;
    u32 m_fb_stride;
    u32 m_width;
    u32 m_height;

    u32 m_vp_x;
    u32 m_vp_y;
    u32 m_vp_width;
    u32 m_vp_height;
    float m_vp_minz;
    float m_vp_maxz;

    VertexDeclaration m_vdecl;

    Matrix4 m_transforms[_TM_COUNT];

    Material m_material;
    Light m_lights[NUM_LIGHTS];

    // Associations
    IIndexBuffer* m_indices;
    SStreamSource m_streams[NUM_STREAMS];
    // ITexture* m_textures[NUM_SAMPLERS];
    SWSampler m_samplers[NUM_SAMPLERS];

  public:
    SWRenderDevice(HWIN handle);
    virtual ~SWRenderDevice();

    int Initialize(u32 _width, u32 _height);
    void CreateBuffers();

    int ProcessVertex(VSInput& _input, VSOutput& _output, int _usage);
    void ProcessPixel(PSInput& _input, PSOutput& _output, int _usage);

    float GetDepth(u32 _x, u32 _y);
    // Rasterisation
    void SetPixel(u32 _x, u32 _y, float _z, u32 _c);
    void DrawTriangle(const Vector3* pos);
    void DrawTriangle(const VSOutput* vertices, int usage);

    u32* GetBuffer() { return m_frame_buffer.get(); }
    void SetBufferSize(u32 _width, u32 _height)
    {
        m_width = _width;
        m_height = _height;
    }

    HWIN GetHWIN() { return m_window; }
    u32 GetWidth() { return m_width; }
    u32 GetHeight() { return m_height; }

    // IRenderDevice implementation
  public:
    virtual int BeginScene(void);
    virtual int EndScene(void);
    virtual int Clear(u32 flags, Color color, float zdepth, int stencil);
    virtual int Present();
    virtual int GetBackBuffer(BackBuffer& backBuffer);
    virtual int Reset();

    virtual LPVERTEXDECLARATION
    CreateVertexDeclaration(const VertexElement* /*pVertexElements*/)
    {
        return 0;
    }
    virtual LPVERTEXBUFFER CreateVertexBuffer(u32 length, u32 usage, u32 pool);
    virtual LPINDEXBUFFER CreateIndexBuffer(u32 length, u32 usage,
                                            bool sixteenBitIndices, u32 pool);

    virtual LPTEXTURE CreateTexture(u32 Width, u32 Height, u32 Levels,
                                    u32 Usage, u32 Format, u32 Pool);

    virtual LPTEXTURE CreateTextureFromFile(const char* /*filename*/,
                                            u32 /*Width*/, u32 /*Height*/,
                                            u32 /*Levels*/, u32 /*Usage*/,
                                            u32 /*Format*/, u32 /*Pool*/
    )
    {
        return 0;
    }

    virtual int DrawIndexedPrimitive(PrimitiveType primitiveType,
                                     int baseVertex, int minVertexIndex,
                                     int numVertices, int startIndex,
                                     int primCount);

    virtual int DrawIndexedUserPrimitives(
        PrimitiveType primitiveType, u32 minVertexIndex, u32 numVertexIndices,
        u32 primitiveCount, const void* indexData, bool sixteenBitIndices,
        const void* vertexStreamZeroData, u32 vertexStreamZeroStride);

    virtual void SetTransform(TransformType state, const Matrix4& matrix);

    virtual void GetTransform(TransformType state, Matrix4& matrix);

    virtual int SetVertexDeclaration(const VertexDeclaration* pDecl);

    virtual int SetStreamSource(int streamNumber, IVertexBuffer* streamData,
                                int offsetInBytes, int stride);

    virtual int SetIndices(IIndexBuffer* pIndexData);

    virtual int SetViewport(u32 X, u32 Y, u32 Width, u32 Height,
                            float MinZ = 0.0f, float MaxZ = 1.0f);

    virtual int GetViewport(Viewport* viewport);

    virtual int SetRenderState(u32 /*state*/, u32 /*value*/) { return 0; };

    virtual int SetTexture(u32 Sampler, ITexture* pTexture);

    virtual int SetLight(u32 Index, const Light* pLight);

    virtual int LightEnable(u32 /*LightIndex*/, bool /*bEnable*/
    )
    {
        return 0;
    };

    virtual int SetMaterial(const Material* pMaterial);
};

} // namespace forg::rendering::reference

#endif // FORG_RENDERING_REFERENCE_SWRENDERDEVICE_H
