/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2024  Slawomir Strumecki

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

#ifndef _METAL_RENDER_DEVICE_H_
#define _METAL_RENDER_DEVICE_H_

#include <string>
#include <string_view>

#include "base.h"
#include "rendering/IRenderDevice.h"
#include "rendering/VertexDeclaration.h"

namespace forg {

// All Metal/ObjC objects live in this opaque struct (defined in the .mm) so
// this header is includable from plain C++ (e.g. the factory in
// MetalRenderer.cpp).
struct MetalImpl;

/// Hardware render device implemented on Apple Metal.
/**
 * Implements the subset of IRenderDevice that the macapp demo exercises,
 * hosting a CAMetalLayer in the NSView passed as HWIN. See
 * docs/metal-renderer-plan.md.
 */
class MetalRenderDevice : public IRenderDevice
{
  public:
    MetalRenderDevice(HWIN handle);
    virtual ~MetalRenderDevice();

    // Brings up the Metal device, command queue, shaders and presentation
    // layer.
    int Initialize(u32 width, u32 height);

    // IRenderDevice implementation
  public:
    virtual int BeginScene(void);
    virtual int EndScene(void);
    virtual int Clear(u32 flags, Color color, float zdepth, int stencil);
    virtual int Present();
    virtual int SaveBackBuffer(std::string_view filename);
    virtual int GetBackBuffer(BackBuffer& backBuffer);
    virtual int Reset();

    virtual LPVERTEXDECLARATION
    CreateVertexDeclaration(const VertexElement* pVertexElements);
    virtual LPVERTEXBUFFER CreateVertexBuffer(u32 length, u32 usage,
                                              u32 pool);
    virtual LPINDEXBUFFER CreateIndexBuffer(u32 length, u32 usage,
                                            bool sixteenBitIndices, u32 pool);
    virtual LPTEXTURE CreateTexture(u32 Width, u32 Height, u32 Levels,
                                    u32 Usage, u32 Format, u32 Pool);
    virtual LPTEXTURE CreateTextureFromFile(const char* filename, u32 Width,
                                            u32 Height, u32 Levels,
                                            u32 Usage, u32 Format, u32 Pool);

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
    virtual int SetRenderState(u32 state, u32 value);
    virtual int SetTexture(u32 Sampler, ITexture* pTexture);
    virtual int SetLight(u32 Index, const Light* pLight);
    virtual int LightEnable(u32 LightIndex, bool bEnable);
    virtual int SetMaterial(const Material* pMaterial);

    HWIN GetHWIN() { return m_window; }

  private:
    // (Re)creates the CAMetalLayer (once) and depth texture (on size change) to
    // match the hosting view's current backing-pixel size.
    void EnsureSurfaces();

    HWIN m_window;
    MetalImpl* m_impl;

    // Backing-store (pixel) dimensions of the drawable / depth buffer.
    u32 m_width;
    u32 m_height;

    Matrix4 m_world;
    Matrix4 m_view;
    Matrix4 m_proj;
    ITexture* m_texture0;

    // Recorded by Clear(); consumed when the render pass begins.
    Color m_clear_color;
    float m_clear_z;
    u32 m_clear_flags;

    enum
    {
        NUM_LIGHTS = 8
    };
    Light m_lights[NUM_LIGHTS];
    bool m_light_enabled[NUM_LIGHTS];
    Material m_material;

    VertexDeclaration m_vdecl;

    IVertexBuffer* m_stream0;
    int m_stream0_offset;
    int m_stream0_stride;
    IIndexBuffer* m_indices;

    u32 m_cull;
    u32 m_fill;
    bool m_lighting;
    std::string m_capture_path;
    BackBuffer m_last_back_buffer;
    bool m_has_back_buffer;
};

} // namespace forg

#endif //_METAL_RENDER_DEVICE_H_
