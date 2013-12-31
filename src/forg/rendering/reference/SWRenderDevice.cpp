#include "forg_pch.h"

#include "SWRenderDevice.h"
#include "SWBuffers.h"

namespace forg { namespace rendering { namespace reference {

    int bit_set(int v, int b)
    {
        return v | (1<<b);
    }

    int bit_test(int v, int b)
    {
        return v & (1<<b);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // Interpolator
    /////////////////////////////////////////////////////////////////////////////////////

    int iround(float v)
    {
        return (int)v;
    }

    int min(int a, int b, int c)
    {
        return forg::min(forg::min(a, b), c);
    }

    int max(int a, int b, int c)
    {
        return forg::max(forg::max(a, b), c);
    }

    struct TriangleInterpolator
    {
        float lineA01;
        float lineA12;
        float lineA20;

        float lineB01;
        float lineB12;
        float lineB20;

        float lineC01;
        float lineC12;
        float lineC20;

        float bary01;
        float bary12;
        float bary20;

        float bary_a;
        float bary_b;
        float bary_c;

        bool ext01;
        bool ext12;
        bool ext20;

        bool draw;

        void Initialize(const Vector4& pos0, const Vector4& pos1, const Vector4& pos2)
        {
            lineA01 = pos0.Y - pos1.Y;
            lineA12 = pos1.Y - pos2.Y;
            lineA20 = pos2.Y - pos0.Y;

            lineB01 = pos1.X - pos0.X;
            lineB12 = pos2.X - pos1.X;
            lineB20 = pos0.X - pos2.X;

            lineC01 = pos0.X*pos1.Y - pos1.X*pos0.Y;
            lineC12 = pos1.X*pos2.Y - pos2.X*pos1.Y;
            lineC20 = pos2.X*pos0.Y - pos0.X*pos2.Y;

            bary01 = line(lineA01, lineB01, lineC01, pos2.X, pos2.Y);
            bary12 = line(lineA12, lineB12, lineC12, pos0.X, pos0.Y);
            bary20 = line(lineA20, lineB20, lineC20, pos1.X, pos1.Y);

            ext12 = line(lineA12, lineB12, lineC12, -1, -1)*bary12 > 0.0f;
            ext20 = line(lineA20, lineB20, lineC20, -1, -1)*bary20>0.0f;
            ext01 = line(lineA01, lineB01, lineC01, -1, -1)*bary01>0.0f;
        }

        void SetPixel(int _x, int _y)
        {
            //TODO: could increment only, because line(x+1,y) = line(x,y)+A

            bary_a = line(lineA12, lineB12, lineC12, _x, _y)/bary12;
            bary_b = line(lineA20, lineB20, lineC20, _x, _y)/bary20;
            bary_c = line(lineA01, lineB01, lineC01, _x, _y)/bary01;

            bary_a = fabs(bary_a);
            bary_b = fabs(bary_b);
            bary_c = fabs(bary_c);

            draw = false;

            if (bary_a>=0.0f && bary_b>=0.0f && bary_c>=0.0f)
            {
                if ( (bary_a>0.0f || ext12) 
                    && (bary_b>0.0f || ext20) 
                    && (bary_c>0.0f || ext01) )
                {
                    draw = true;
                }
            }
        }

        bool CanDraw() const { return draw; }

        void Interpolate(Vector3* out, const Vector3* attribs) const
        {
            Vector3 interp = attribs[0]*bary_a +  attribs[1]*bary_b + attribs[2]*bary_c;

            *out = interp;
        }

        void Interpolate(Vector3* out, const Vector3& attribs0, const Vector3& attribs1, const Vector3& attribs2) const
        {
            Vector3 interp = attribs0*bary_a +  attribs1*bary_b + attribs2*bary_c;

            *out = interp;
        }

        void Interpolate(Vector4* out, const Vector4& attribs0, const Vector4& attribs1, const Vector4& attribs2) const
        {
            Vector4 interp = attribs0*bary_a +  attribs1*bary_b + attribs2*bary_c;

            *out = interp;
        }

        float Interpolate(float a0, float a1, float a2) const
        {
            return a0*bary_a +  a1*bary_b + a2*bary_c;
        }

        float line(float A, float B, float C, float x, float y)
        {
            return A*x + B*y + C;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // SWSampler
    /////////////////////////////////////////////////////////////////////////////////////
    SWSampler::SWSampler()
    {
        texture = 0;
    }

    SWSampler::~SWSampler()
    {
        if (texture != 0)
        {
            texture->Release();
            texture = 0;
        }
    }

    void SWSampler::SetTexture(ITexture* _texture)
    {
        if (texture != 0)
        {
            texture->Release();
        }

        if (_texture != 0)
        {
            _texture->AddRef();
        }

        texture = _texture;
    }

    uint SWSampler::Sample(float u, float v)
    {
        if (texture)
        {
            return ((forg::rendering::reference::SWTexture*)texture)->Sample(u, v);
        }

        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // SWRenderDevice
    /////////////////////////////////////////////////////////////////////////////////////
    SWRenderDevice::SWRenderDevice(HWIN handle)
        : m_vdecl(&VertexElement::VertexDeclarationEnd)
    {
        m_refCount = 1;
        m_window = handle;
        m_frame_buffer = 0;
        m_width = m_height = 0;
        m_fb_stride = 0;
        m_vp_x = m_vp_y = 0;
        m_vp_width = m_vp_height = 0;

        m_indices = 0;

        m_frame_buffer = 0;
        m_depth_buffer = 0;
    }

    SWRenderDevice::~SWRenderDevice()
    {
        SetIndices(0);
        for (int i=0; i<NUM_SAMPLERS; i++) SetTexture(i, 0);
        for (int i=0; i<NUM_STREAMS; i++) SetStreamSource(i, 0, 0, 0);

        delete [] m_frame_buffer;
        delete [] m_depth_buffer;
    }

    int SWRenderDevice::Initialize(uint _width, uint _height)
    {
        /*
        RECT rcClient;
        GetClientRect((HWND)m_window, &rcClient);

        // calculate window width/height 
        m_width = rcClient.right - rcClient.left;  
        m_height = rcClient.bottom - rcClient.top;  
        */

        m_width = _width;
        m_height = _height;

        CreateBuffers();

        return FORG_OK;
    }

    void SWRenderDevice::CreateBuffers()
    {
        if (m_frame_buffer)
        {
            delete [] m_frame_buffer;
            m_frame_buffer = 0;
        }

        if (m_depth_buffer)
        {
            delete [] m_depth_buffer;
            m_depth_buffer = 0;
        }

        m_fb_stride = m_width * 4;
        m_frame_buffer = new uint[m_width*m_height];
        m_depth_buffer = new float[m_width*m_height];
    }

    int SWRenderDevice::Reset()
    {
        /*
        RECT rcClient;
        GetClientRect((HWND)m_window, &rcClient);

        // calculate window width/height
        m_width = rcClient.right - rcClient.left;
        m_height = rcClient.bottom - rcClient.top;
        */

        CreateBuffers();

        return FORG_OK;
    }

    LPVERTEXBUFFER SWRenderDevice::CreateVertexBuffer(uint length, uint usage, uint pool)
    {
        forg::rendering::reference::SWVertexBuffer* vb = new forg::rendering::reference::SWVertexBuffer();

        vb->Create(length, usage, pool);

        return vb;
    }

    LPINDEXBUFFER SWRenderDevice::CreateIndexBuffer(uint length, uint usage, bool sixteenBitIndices, uint pool)
    {
        forg::rendering::reference::SWIndexBuffer* ib = new forg::rendering::reference::SWIndexBuffer();

        ib->Create(length, usage, sixteenBitIndices, pool);

        return ib;
    }

    LPTEXTURE SWRenderDevice::CreateTexture(uint Width, uint Height, uint Levels, uint Usage, uint Format, uint Pool)
    {
        forg::rendering::reference::SWTexture* tex = new forg::rendering::reference::SWTexture();

        tex->Create(Width, Height, Levels, Usage, Format, Pool);

        return tex;
    }

    int SWRenderDevice::SetStreamSource(int streamNumber, IVertexBuffer* streamData, int offsetInBytes, int stride)
    {
        if (streamNumber >= NUM_STREAMS)
            return FORG_INVALID_CALL;

        if (m_streams[streamNumber].streamData)
        {
            m_streams[streamNumber].streamData->Release();
        }

        m_streams[streamNumber].streamData = streamData;
        m_streams[streamNumber].offsetInBytes = offsetInBytes;
        m_streams[streamNumber].stride = stride;

        if (m_streams[streamNumber].streamData)
        {
            m_streams[streamNumber].streamData->AddRef();
        }

        return FORG_OK;
    }

    int SWRenderDevice::SetIndices(IIndexBuffer* pIndexData)
    {
        if (m_indices)
        {
            m_indices->Release();
        }

        m_indices = pIndexData;

        if (m_indices)
        {
            m_indices->AddRef();
        }

        return FORG_OK;
    }

    int SWRenderDevice::SetTexture(uint Sampler, ITexture* pTexture)
    {
        if (Sampler < NUM_SAMPLERS)
        {
            m_samplers[Sampler].SetTexture(pTexture);
        }

        return FORG_OK;
    }

    void SWRenderDevice::SetTransform(TransformType state, const Matrix4& matrix)
    {
        switch(state)
        {
        case TransformType_View:
            m_transforms[TM_VIEW] = matrix;
            break;
        case TransformType_Projection:
            m_transforms[TM_PROJECTION] = matrix;
            break;
        case TransformType_World:
            m_transforms[TM_WORLD] = matrix;
            break;
        }

        Matrix4::Multiply(m_transforms[TM_MODELVIEW], m_transforms[TM_WORLD], m_transforms[TM_VIEW]);
        Matrix4::Multiply(m_transforms[TM_MODELVIEWPROJ], m_transforms[TM_MODELVIEW], m_transforms[TM_PROJECTION]);
    }

    void SWRenderDevice::GetTransform(TransformType state, Matrix4& matrix)
    {
        switch(state)
        {
        case TransformType_View:
            matrix = m_transforms[TM_VIEW];
            break;
        case TransformType_Projection:
            matrix = m_transforms[TM_PROJECTION];
            break;
        case TransformType_World:
            matrix = m_transforms[TM_WORLD];
            break;
        }
    }

    int SWRenderDevice::SetVertexDeclaration(const VertexDeclaration* pDecl)
    { 
        if (pDecl)
        {
            m_vdecl = *pDecl;
        }

        return FORG_OK; 
    }
    
    int SWRenderDevice::SetViewport(uint X, uint Y, uint Width, uint Height, float MinZ, float MaxZ)
    {
        m_vp_x = X;
        m_vp_y = Y;
        m_vp_minz = MinZ;
        m_vp_maxz = MaxZ;
        m_vp_width = Width;
        m_vp_height = Height;

        float halfw = 0.5f*m_vp_width;
        float halfh = 0.5f*m_vp_height;
        Matrix4 viewport(
            halfw, 0.0f, 0.0f, 0.0f,
            0.0f, -halfh, 0.0f, 0.0f,
            0.0f, 0.0f, m_vp_maxz - m_vp_minz, 0.0f,
            m_vp_x + halfw, m_vp_y + halfh, m_vp_minz, 1.0f
        );

        m_transforms[TM_VIEWPORT] = viewport;

        return FORG_OK;
    }

    int SWRenderDevice::GetViewport(Viewport* viewport)
    {
        viewport->Width = m_width;
        viewport->Height = m_height;
        viewport->X = m_vp_x;
        viewport->Y = m_vp_y;
        viewport->MinZ = m_vp_minz;
        viewport->MaxZ = m_vp_maxz;

        return FORG_OK;
    }

    int SWRenderDevice::SetLight(uint Index, const Light* pLight)
    {
        if (Index < NUM_LIGHTS)
        {
            m_lights[Index] = *pLight;
            return FORG_OK;
        }

        return FORG_INVALID_CALL;
    }

    int SWRenderDevice::SetMaterial(const Material* pMaterial)
    {
        m_material = *pMaterial;

        return FORG_OK;
    }

    enum
    {
        V_POS_FLOAT3 = 1,
        V_NRM_FLOAT3 = 2,
        V_UV0_FLOAT2 = 4,
    };

    int SWRenderDevice::DrawIndexedUserPrimitives(
		    PrimitiveType primitiveType,
		    uint minVertexIndex,
		    uint numVertexIndices,
		    uint primitiveCount,
		    const void* indexData,
		    bool sixteenBitIndices,
		    const void* vertexStreamZeroData,
            uint vertexStreamZeroStride)
    {
	    uint stride = m_vdecl.GetVertexSize();
        char* position_ptr = 0;
        uint position_size = 0;
        char* normal_ptr = 0;
        char* texcoord_ptr = 0;
        uint texcoord_size = 0;
        int vertex_usage = 0;

        const VertexElement* elements = m_vdecl.GetDeclaration();
        for (uint i=0; i<m_vdecl.GetElementsCount(); i++)
        {
            vertex_usage |= (1 << elements[i].Usage);

            if (elements[i].Usage == DeclarationUsage_Position)
            {
                position_ptr = (char*)vertexStreamZeroData+elements[i].Offset;
                position_size = VertexElement::GetTypeCount(elements[i].Type);
            }
            else if (elements[i].Usage == DeclarationUsage_Normal)
            {
                normal_ptr = (char*)vertexStreamZeroData+elements[i].Offset;
            }
            else if (elements[i].Usage == DeclarationUsage_TextureCoordinate)
            {
                texcoord_ptr = (char*)vertexStreamZeroData+elements[i].Offset;
                texcoord_size = VertexElement::GetTypeCount(elements[i].Type);
            }
            else if (elements[i].Usage == DeclarationUsage_Color)
            {
            }
        }

        uint num_indices = primitiveCount;
        uint indices_step = 3;
        /// swap indices for even triangles
        bool swap_even = false;

        switch(primitiveType)
        {
        case PrimitiveType_PointList:
            num_indices = primitiveCount;
            indices_step = 1;
            break;
        case PrimitiveType_LineList:
            num_indices <<= 1;
            indices_step = 2;
            break;
        case PrimitiveType_LineStrip:
            num_indices = primitiveCount + 1;
            indices_step = 1;
            break;
        case PrimitiveType_TriangleList:
            num_indices *= 3;
            break;
        case PrimitiveType_TriangleStrip:
            num_indices = 2 + primitiveCount;
            indices_step = 1;
            swap_even = true;
            break;
        case PrimitiveType_TriangleFan:
            break;
        }

        const uint* indices32 = (const uint*)indexData;
        const ushort* indices16 = (const ushort*)indexData;

        VSInput vs_input[3];
        VSOutput vs_output[3];

        for (uint i=0; i<primitiveCount; i++)
        {
            uint idx[3];

            if (sixteenBitIndices)
            {
                idx[0] = indices16[0];
                idx[1] = indices16[1];
                idx[2] = indices16[2];

                indices16 += indices_step;
            } else
            {
                idx[0] = indices32[0];
                idx[1] = indices32[1];
                idx[2] = indices32[2];

                indices32 += indices_step;
            }

            if (swap_even && i%2!=0)
            {
                uint ti = idx[0];
                idx[0] = idx[1];
                idx[1] = ti;
            }

            //Vector3 tri_pos[3];
            //Vector3 tri_uv[3];
            bool external[3];

            // transform vertices to view space
            for (int k=0; k<3; k++)
            {
                float* pos = (float*)(position_ptr + idx[k]*stride);
                float* nrm = (float*)(normal_ptr + idx[k]*stride);
                float* uv = (float*)(texcoord_ptr + idx[k]*stride);

                // assume the size is 3
                vs_input[k].position.X = pos[0];
                vs_input[k].position.Y = pos[1];
                vs_input[k].position.Z = pos[2];
                vs_input[k].position.W = 1.0f;

                if (normal_ptr)
                {
                    vs_input[k].normal.X = nrm[0];
                    vs_input[k].normal.Y = nrm[1];
                    vs_input[k].normal.Z = nrm[2];
                    vs_input[k].normal.W = 0.0f;
                }

                if (texcoord_ptr)
                {
                    vs_input[k].texcoord0.X = uv[0];
                    vs_input[k].texcoord0.Y = uv[1];
                    vs_input[k].texcoord0.Z = 0.0f;
                }
                
                vs_output[k].texcoord0 = vs_input[k].texcoord0;

                vertex_usage = ProcessVertex(vs_input[k], vs_output[k], vertex_usage);

                // simple culling and clipping
                float w = vs_output[k].position.W;
                external[k] = vs_output[k].position.Z < 0.0f || vs_output[k].position.Z > w 
                    || vs_output[k].position.X < -w || vs_output[k].position.X > w
                    || vs_output[k].position.Y < -w || vs_output[k].position.Y > w;

                // perspective divide
                if (w!=0.0f)
                {
                    vs_output[k].position.Scale(1.0f/w);
                    //tri_uv[k].Scale(1.0f/w);
                }

                vs_output[k].position.TransformCoordinate(m_transforms[TM_VIEWPORT]);
            }
            
            if (external[0] && external[1] && external[2])
            {
            } else
            {
                DrawTriangle(vs_output, vertex_usage);
            }
        }

        //SetStreamSource(0, 0, 0, 0);
        //SetIndices(0);
	    //GLV(glDrawElements(mode, num_indices, sixteenBitIndices ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexData));	//ogl 1.1

        return FORG_OK;
    }

    int SWRenderDevice::DrawIndexedPrimitive(PrimitiveType primitiveType, int baseVertex, int minVertexIndex, int numVertices, int startIndex, int primCount)
    {
        forg::rendering::reference::SWVertexBuffer* vb = (forg::rendering::reference::SWVertexBuffer*)m_streams[0].streamData;
        forg::rendering::reference::SWIndexBuffer* ib = (forg::rendering::reference::SWIndexBuffer*)m_indices;

        return DrawIndexedUserPrimitives(primitiveType, 0, ib->GetLength(), primCount, ib->GetData(), ib->IsIndexShort(), vb->GetData(), m_vdecl.GetVertexSize());
    }

    int SWRenderDevice::Clear(uint flags, Color color, float zdepth, int stencil)
    {
        uint c = color;

        if (flags & forg::ClearFlags_Target)
        {
            for (uint p=0; p<m_width*m_height; p++)
            {
                m_frame_buffer[p] = c;
            }
        }

        if (flags & forg::ClearFlags_ZBuffer)
        {
            for (uint p=0; p<m_width*m_height; p++)
            {
                m_depth_buffer[p] = zdepth;
            }
        }
        
        //memset(fb, 0, m_width*m_height*4);
        return FORG_OK;
    }

    int SWRenderDevice::BeginScene(void)
    {
        return FORG_OK;
    }

    int SWRenderDevice::EndScene(void)
    {
        return FORG_OK;
    }

    int SWRenderDevice::Present()
    {
        /*
        // window color format: ARGB
        HWND hWnd = (HWND)m_window;

        HDC hWindowDC = GetDC(hWnd); // get the desktop device context
        HDC hMemDC = CreateCompatibleDC(hWindowDC); // create a device context to use yourself
    
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // calculate window width/height 
        ULONG ulWindowWidth = rcClient.right - rcClient.left;  
        ULONG ulWindowHeight = rcClient.bottom - rcClient.top;  

        BITMAPINFO bi; 
        ZeroMemory(&bi,sizeof(BITMAPINFO));
        bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth=ulWindowWidth;
        bi.bmiHeader.biHeight=ulWindowHeight;
        bi.bmiHeader.biPlanes=1;
        bi.bmiHeader.biBitCount= 32;//bm.bmBitsPixel;
        bi.bmiHeader.biCompression = BI_RGB;
        bi.bmiHeader.biSizeImage = ((ulWindowWidth * bi.bmiHeader.biBitCount +31)& ~31) / 8 * ulWindowHeight; 

        // create our DIB section and select the bitmap into the dc 
        VOID *pvBits;          // pointer to DIB section 
        HBITMAP hBitmap = CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
        HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

        //for (uint y = 0; y < ulWindowHeight; y++)
        //for (uint x = 0; x < ulWindowWidth; x++)
        //    ((UINT32 *)pvBits)[x + y * ulWindowWidth] = 0xff0000ff; 

        memcpy(pvBits, m_frame_buffer, ulWindowWidth*ulWindowHeight*4);

        if (!BitBlt(hWindowDC, 0, 0, ulWindowWidth, ulWindowHeight, hMemDC, 0, 0, SRCCOPY))
        //StretchBlt used to flip frame buffer
        //if (!StretchBlt(hWindowDC, 0, ulWindowHeight-1, ulWindowWidth, -1*ulWindowHeight, hMemDC, 0, 0, ulWindowWidth, ulWindowHeight, SRCCOPY))
        {
            // error
        }

        hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

        ReleaseDC((HWND)m_window, hWindowDC);
        DeleteDC(hMemDC);   
        DeleteObject(hBitmap);
        */
        return FORG_OK;
    }

    uint blend_color(uint _src, uint _dst)
    {
        uint out = _src;

        int a = _src >> 24;

        if (a == 0)
        {
            out = _dst;
        }
        else if (a < 255)
        {
            int ac = 255 - a;
            int r1 = (_src >> 16)&0xff;
            int g1 = (_src >> 8)&0xff;
            int b1 = (_src)&0xff;

            int r0 = (_dst >> 16)&0xff;
            int g0 = (_dst >> 8)&0xff;
            int b0 = (_dst)&0xff;

            r0 = ac*r0 + a*r1;
            g0 = ac*g0 + a*g1;
            b0 = ac*b0 + a*b1;

            r0 = (r0+1 + (r0>>8))>>8;
            g0 = (g0+1 + (g0>>8))>>8;
            b0 = (b0+1 + (b0>>8))>>8;

            out = 0xff000000 | (r0<<16) | (g0<<8) | b0;
        } 

        return out;
    }

    float SWRenderDevice::GetDepth(uint _x, uint _y)
    {
        _y = m_height - 1 - _y;

        uint index = _y*m_width + _x;

        float d = m_depth_buffer[index];

        return d;
    }

    void SWRenderDevice::SetPixel(uint _x, uint _y, float _z, uint _c)
    {
        _x += m_vp_x;
        _y += m_vp_y;

        _y = m_height - 1 - _y;

        uint index = _y*m_width + _x;

        float& d = m_depth_buffer[index];

        if (_z <= d)
        {
            d = _z;

            uint& p = m_frame_buffer[index];

            //Color colz(_z, _z, _z);
            //_c = colz;

            p = blend_color(_c, p);
        }
    }

    int SWRenderDevice::ProcessVertex(VSInput& _input, VSOutput& _output, int _usage)
    {
        Vector4::TransformCoordinate(_output.position, _input.position, m_transforms[TM_MODELVIEWPROJ]); 

        if (bit_test(_usage, DeclarationUsage_Normal))
        {
            Vector3* n = (Vector3*)&_input.normal;
            Vector3 dir = m_lights[0].Position;
            
            dir.X -= _input.position.X;
            dir.Y -= _input.position.Y;
            dir.Z -= _input.position.Z;
            dir.Normalize();

            float d  = (n->Dot(dir) + 1.0f)*0.5f;
            //float d  = max(0.0f, n->Dot(dir));
            _output.color.X = d * m_lights[0].Diffuse.r * m_lights[0].Ambient.r;
            _output.color.Y = d * m_lights[0].Diffuse.g * m_lights[0].Ambient.g;
            _output.color.Z = d * m_lights[0].Diffuse.b * m_lights[0].Ambient.b;
            _output.color.W = 1.0f;
            _usage = bit_set(_usage, DeclarationType_Color);
        }

        return _usage;
    }

    void SWRenderDevice::ProcessPixel(PSInput& _input, PSOutput& _output, int _usage)
    {
        Color cout(0.0f, 0.0f, 0.0f, 1.0f);

        if (bit_test(_usage, DeclarationUsage_TextureCoordinate))
        {
            Color c = Color(m_samplers[0].Sample(_input.texcoord0.X, _input.texcoord0.Y));

            cout = c;
        }

        if (bit_test(_usage, DeclarationType_Color))
        {
            cout = _input.color;
        }

        _output.color.X = cout.r;
        _output.color.Y = cout.g;
        _output.color.Z = cout.b;
        _output.color.W = cout.a;
    }
   
    // based on http://devmaster.net/forums/topic/1145-advanced-rasterization/
    // points must be in CCW order
    void SWRenderDevice::DrawTriangle(const Vector3* pos)
    {
        uint* colorBuffer = (uint*)m_frame_buffer;

        // 28.4 fixed-point coordinates

        const int Y1 = iround(16.0f * pos[0].Y);
        const int Y2 = iround(16.0f * pos[1].Y);
        const int Y3 = iround(16.0f * pos[2].Y);

        const int X1 = iround(16.0f * pos[0].X);
        const int X2 = iround(16.0f * pos[1].X);
        const int X3 = iround(16.0f * pos[2].X);

        // Deltas

        const int DX12 = X1 - X2;
        const int DX23 = X2 - X3;
        const int DX31 = X3 - X1;

        const int DY12 = Y1 - Y2;
        const int DY23 = Y2 - Y3;
        const int DY31 = Y3 - Y1;

        // Fixed-point deltas

        const int FDX12 = DX12 << 4;
        const int FDX23 = DX23 << 4;
        const int FDX31 = DX31 << 4;

        const int FDY12 = DY12 << 4;
        const int FDY23 = DY23 << 4;
        const int FDY31 = DY31 << 4;

        // Bounding rectangle

        int minx = (min(X1, X2, X3) + 0xF) >> 4;
        int maxx = (max(X1, X2, X3) + 0xF) >> 4;
        int miny = (min(Y1, Y2, Y3) + 0xF) >> 4;
        int maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;

        if (minx < 0) minx = 0;
        if (miny < 0) miny = 0;
        if (maxx > m_width) maxx = m_width;
        if (maxy > m_height) maxy = m_height;

        // Block size, standard 8x8 (must be power of two)
        const int q = 8;

        // Start in corner of 8x8 block
        minx &= ~(q - 1);
        miny &= ~(q - 1);

        (char*&)colorBuffer += miny * m_fb_stride;

        // Half-edge constants
        int C1 = DY12 * X1 - DX12 * Y1;
        int C2 = DY23 * X2 - DX23 * Y2;
        int C3 = DY31 * X3 - DX31 * Y3;

        // Correct for fill convention
        if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
        if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
        if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

        // Loop through blocks
        for(int y = miny; y < maxy; y += q)
        {
            for(int x = minx; x < maxx; x += q)
            {
                // Corners of block
                int x0 = x << 4;
                int x1 = (x + q - 1) << 4;
                int y0 = y << 4;
                int y1 = (y + q - 1) << 4;

                // Evaluate half-space functions
                bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
                bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
                bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
                bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;

                int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

                bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
                bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
                bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
                bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;

                int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

                bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
                bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
                bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
                bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;

                int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

                // Skip block when outside an edge
                if(a == 0x0 || b == 0x0 || c == 0x0) continue;

                unsigned int *buffer = colorBuffer;

                // Accept whole block when totally covered

                if(a == 0xF && b == 0xF && c == 0xF)
                {
                    for(int iy = 0; iy < q && y+iy<maxy; iy++)
                    {
                        for(int ix = x; ix < x + q && ix <maxx; ix++)
                        {
                            //buffer[ix] = 0xFF007F00; // Green
                            SetPixel(ix, y+iy, 0.0f, 0xFF007F00);
                        }

                        (char*&)buffer += m_fb_stride;
                    }
                }
                else // Partially covered block
                {
                    int CY1 = C1 + DX12 * y0 - DY12 * x0;
                    int CY2 = C2 + DX23 * y0 - DY23 * x0;
                    int CY3 = C3 + DX31 * y0 - DY31 * x0;

                    for(int iy = y; iy < y + q && iy<maxy; iy++)
                    {
                        int CX1 = CY1;
                        int CX2 = CY2;
                        int CX3 = CY3;

                        for(int ix = x; ix < x + q; ix++)
                        {
                            if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                            {
                                //buffer[ix] = 0xFF00007F; // Blue
                                SetPixel(ix, iy, 0.0f, 0xFF00007F);
                            }

                            CX1 -= FDY12;
                            CX2 -= FDY23;
                            CX3 -= FDY31;
                        }

                        CY1 += FDX12;
                        CY2 += FDX23;
                        CY3 += FDX31;

                        (char*&)buffer += m_fb_stride;
                    }
                }
            }

            (char*&)colorBuffer += q * m_fb_stride;
        }
    }

    void SWRenderDevice::DrawTriangle(const VSOutput* vertices, int usage)
    {
        TriangleInterpolator interpolator;
        PSInput ps_input;
        PSOutput ps_output;

        interpolator.Initialize(vertices[0].position, vertices[1].position, vertices[2].position);

        // 28.4 fixed-point coordinates

        const int Y1 = iround(16.0f * vertices[0].position.Y);
        const int Y2 = iround(16.0f * vertices[1].position.Y);
        const int Y3 = iround(16.0f * vertices[2].position.Y);

        const int X1 = iround(16.0f * vertices[0].position.X);
        const int X2 = iround(16.0f * vertices[1].position.X);
        const int X3 = iround(16.0f * vertices[2].position.X);

        // Deltas

        const int DX12 = X1 - X2;
        const int DX23 = X2 - X3;
        const int DX31 = X3 - X1;

        const int DY12 = Y1 - Y2;
        const int DY23 = Y2 - Y3;
        const int DY31 = Y3 - Y1;

        // Fixed-point deltas

        const int FDX12 = DX12 << 4;
        const int FDX23 = DX23 << 4;
        const int FDX31 = DX31 << 4;

        const int FDY12 = DY12 << 4;
        const int FDY23 = DY23 << 4;
        const int FDY31 = DY31 << 4;

        // Bounding rectangle

        int minx = (min(X1, X2, X3) + 0xF) >> 4;
        int maxx = (max(X1, X2, X3) + 0xF) >> 4;
        int miny = (min(Y1, Y2, Y3) + 0xF) >> 4;
        int maxy = (max(Y1, Y2, Y3) + 0xF) >> 4;

        if (minx < 0) minx = 0;
        if (miny < 0) miny = 0;
        if (maxx > m_width) maxx = m_width;
        if (maxy > m_height) maxy = m_height;

        // Block size, standard 8x8 (must be power of two)
        const int q = 8;

        // Start in corner of 8x8 block
        minx &= ~(q - 1);
        miny &= ~(q - 1);

        // Half-edge constants
        int C1 = DY12 * X1 - DX12 * Y1;
        int C2 = DY23 * X2 - DX23 * Y2;
        int C3 = DY31 * X3 - DX31 * Y3;

        // Correct for fill convention
        if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
        if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
        if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

        // Loop through blocks
        for(int y = miny; y < maxy; y += q)
        {
            for(int x = minx; x < maxx; x += q)
            {
                // Corners of block
                int x0 = x << 4;
                int x1 = (x + q - 1) << 4;
                int y0 = y << 4;
                int y1 = (y + q - 1) << 4;

                // Evaluate half-space functions
                bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
                bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
                bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
                bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;

                int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

                bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
                bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
                bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
                bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;

                int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

                bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
                bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
                bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
                bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;

                int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

                // Skip block when outside an edge
                if(a == 0x0 || b == 0x0 || c == 0x0) continue;

                // Accept whole block when totally covered

                if(a == 0xF && b == 0xF && c == 0xF)
                {
                    for(int iy = 0; iy < q && y+iy<maxy; iy++)
                    {

                        for(int ix = x; ix < x + q && ix <maxx; ix++)
                        {
                            uint c = 0xff007f00;    // green

                            interpolator.SetPixel(ix, y+iy);
                           
                            ps_input.vpos.X = ix;
                            ps_input.vpos.Y = y+iy;
                            
                            if (bit_test(usage, DeclarationType_Color))
                            {
                                interpolator.Interpolate(&ps_input.color, vertices[0].color, vertices[1].color, vertices[2].color);

                                // flat shading
                                //ps_input.color = vertices[0].color;
                            }

                            if (bit_test(usage, DeclarationUsage_TextureCoordinate))
                            {
                                interpolator.Interpolate(&ps_input.texcoord0, vertices[0].texcoord0, vertices[1].texcoord0, vertices[2].texcoord0);                                
                            }

                            ProcessPixel(ps_input, ps_output, usage);

                            float d = interpolator.Interpolate(vertices[0].position.Z, vertices[1].position.Z, vertices[2].position.Z); 

                            if (interpolator.CanDraw() && d <= GetDepth(ix, y+iy))
                            {
                                Color col = ps_output.color;
                                SetPixel(ix, y+iy, d, col);
                            }
                        }
                    }
                }
                else // Partially covered block
                {
                    int CY1 = C1 + DX12 * y0 - DY12 * x0;
                    int CY2 = C2 + DX23 * y0 - DY23 * x0;
                    int CY3 = C3 + DX31 * y0 - DY31 * x0;

                    for(int iy = y; iy < y + q && iy<maxy; iy++)
                    {
                        int CX1 = CY1;
                        int CX2 = CY2;
                        int CX3 = CY3;

                        for(int ix = x; ix < x + q; ix++)
                        {
                            if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                            {
                                uint c = 0xff00007f;

                                interpolator.SetPixel(ix, iy);

                                ps_input.vpos.X = ix;
                                ps_input.vpos.Y = y+iy;
                                
                                if (bit_test(usage, DeclarationType_Color))
                                {
                                    interpolator.Interpolate(&ps_input.color, vertices[0].color, vertices[1].color, vertices[2].color);
                                    // flat shading
                                    //ps_input.color = vertices[0].color;
                                }

                                if (bit_test(usage, DeclarationUsage_TextureCoordinate))
                                {
                                    interpolator.Interpolate(&ps_input.texcoord0, vertices[0].texcoord0, vertices[1].texcoord0, vertices[2].texcoord0);                                
                                }

                                ProcessPixel(ps_input, ps_output, usage);

                                float d = interpolator.Interpolate(vertices[0].position.Z, vertices[1].position.Z, vertices[2].position.Z); 

                                if (interpolator.CanDraw() && d<=GetDepth(ix, iy))
                                {
                                    Color col = ps_output.color;
                                    SetPixel(ix, iy, d, col);
                                }

                            }

                            CX1 -= FDY12;
                            CX2 -= FDY23;
                            CX3 -= FDY31;
                        }

                        CY1 += FDX12;
                        CY2 += FDX23;
                        CY3 += FDX31;
                    }
                }
            }
        }
    }

} } }

