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

#pragma once
#include "GLDeviceCaps.h"
#include "base.h"
#include "rendering/IRenderDevice.h"
#include "rendering/VertexDeclaration.h"

namespace forg {

#ifdef _DEBUG
extern int GLErrorCheck(int line, LPCSTR file, LPCSTR func);

#define GLV(func)                                                              \
    func;                                                                      \
    GLErrorCheck(__LINE__, __FILE__, #func)
#define GLV_RETURNV(func, rvalue)                                              \
    func;                                                                      \
    if (GLErrorCheck(__LINE__, __FILE__, #func) != GL_NO_ERROR)                \
    return rvalue
#define GLV_RETURN(func)                                                       \
    func;                                                                      \
    if (GLErrorCheck(__LINE__, __FILE__, #func) != GL_NO_ERROR)                \
    return

#else // RELEASE

#define GLV(func)                                                              \
    {                                                                          \
        func;                                                                  \
    }
#define GLV_RETURN(func)                                                       \
    {                                                                          \
        func;                                                                  \
        if (glGetError() != GL_NO_ERROR)                                       \
            return;                                                            \
    }
#define GLV_RETURNV(func, rvalue)                                              \
    func;                                                                      \
    if (glGetError() != GL_NO_ERROR)                                           \
    {                                                                          \
        return rvalue;                                                         \
    }

#endif

struct SRenderStates
{
    u32 SourceBlend;
    u32 DestinationBlend;
};

class IVertexShader
{
};

typedef IVertexShader* LPVERTEXSHADER;

class IPixelShader
{
};

typedef IPixelShader* LPPIXELSHADER;

class GLRenderDevice : public IRenderDevice
{
    friend class GLRenderer;

  private:
    GLRenderDevice();

  public:
    ~GLRenderDevice(void);

    // Attributes
  private:
    int m_refCount;
    HWIN m_hWnd;
    HANDLE m_hDC;
    HANDLE m_hRC;

    Matrix4 m_matView;       // World to Camera Transformation
    Matrix4 m_matWorld;      // Object to World Transformation
    Matrix4 m_matProjection; // Camera to Screen Transformation
    Matrix4 m_matModelView;  // Object to Camera Transformation

    VertexElement m_internal_decl[255];
    LPVERTEXBUFFER m_vertex_buffer;
    LPINDEXBUFFER m_index_buffer;
    const VertexDeclaration* m_pVertexDeclaration;

    SRenderStates m_render_states;
    GLDeviceCaps m_caps;

    bool m_use_shaders;

    // IRenderDevice methods
  public:
    int BeginScene(void);
    int EndScene(void);

    int Clear(u32 flags, Color color, float zdepth, int stencil);
    int Present();
    int Reset();

    LPVERTEXDECLARATION
    CreateVertexDeclaration(const VertexElement* pVertexElements);

    LPVERTEXBUFFER CreateVertexBuffer(u32 length, u32 usage, u32 pool
                                      // IVertexBuffer** ppVertexBuffer
    );

    LPINDEXBUFFER CreateIndexBuffer(u32 length, u32 usage,
                                    bool sixteenBitIndices, u32 pool
                                    // IIndexBuffer** ppIndexBuffer
    );

    LPVERTEXSHADER CreateVertexShader(const void* pFunction
                                      // IDirect3DVertexShader9 **ppShader
    );

    LPPIXELSHADER CreatePixelShader(const void* pFunction
                                    // IDirect3DVertexShader9 **ppShader
    );

    LPTEXTURE CreateTexture(u32 Width, u32 Height, u32 Levels, u32 Usage,
                            u32 Format, u32 Pool);

    LPTEXTURE CreateTextureFromFile(const char* filename, u32 Width, u32 Height,
                                    u32 Levels, u32 Usage, u32 Format,
                                    u32 Pool);

    int DrawIndexedPrimitive(PrimitiveType primitiveType, int baseVertex,
                             int minVertexIndex, int numVertices,
                             int startIndex, int primCount);

    int DrawIndexedUserPrimitives(PrimitiveType primitiveType,
                                  u32 minVertexIndex, u32 numVertexIndices,
                                  u32 primitiveCount, const void* indexData,
                                  bool sixteenBitIndices,
                                  const void* vertexStreamZeroData,
                                  u32 VertexStreamZeroStride);

    int SetRenderState(u32 state, u32 value);

    int SetViewport(u32 X, u32 Y, u32 Width, u32 Height, float MinZ = 0.0f,
                    float MaxZ = 1.0f);

    int GetViewport(Viewport* viewport);

    void SetTransform(TransformType state, const Matrix4& matrix);

    void GetTransform(TransformType state, Matrix4& matrix);

    int SetVertexDeclaration(const VertexDeclaration* pDecl);

    int SetVertexShader(IVertexShader* shader);

    int SetPixelShader(IPixelShader* shader);

    int SetStreamSource(int streamNumber, IVertexBuffer* streamData,
                        int offsetInBytes, int stride);

    int SetIndices(IIndexBuffer* pIndexData);

    int SetTexture(u32 Sampler, ITexture* pTexture);

    int SetLight(u32 Index, const Light* pLight);

    int LightEnable(u32 LightIndex, bool bEnable);

    int SetMaterial(const Material* pMaterial);

    const GLDeviceCaps* get_DeviceCaps() const;

    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
  private:
    int SetRenderState_CullMode(u32 value);
    int SetRenderState_ShadeMode(u32 value);
    int SetRenderState_FillMode(u32 value);

    int DrawIndexedUserPrimitives_Slow(
        PrimitiveType primitiveType, u32 minVertexIndex, u32 numVertexIndices,
        u32 primitiveCount, const void* indexData, bool sixteenBitIndices,
        const void* vertexStreamZeroData, u32 VertexStreamZeroStride);
};

} // namespace forg
