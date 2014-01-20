#include "SWRenderDevice.h"
#include "SWBuffers.h"
#include "forg.h"

#define NOMINMAX
#include <Windows.h>

enum { USE_SOFTWARE_MODE = 0 };

namespace forg
{
    int bit_set(int v, int b)
    {
        return v | (1 << b);
    }

    int bit_test(int v, int b)
    {
        return v & (1 << b);
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
            int r1 = (_src >> 16) & 0xff;
            int g1 = (_src >> 8) & 0xff;
            int b1 = (_src)& 0xff;

            int r0 = (_dst >> 16) & 0xff;
            int g0 = (_dst >> 8) & 0xff;
            int b0 = (_dst)& 0xff;

            r0 = ac*r0 + a*r1;
            g0 = ac*g0 + a*g1;
            b0 = ac*b0 + a*b1;

            r0 = (r0 + 1 + (r0 >> 8)) >> 8;
            g0 = (g0 + 1 + (g0 >> 8)) >> 8;
            b0 = (b0 + 1 + (b0 >> 8)) >> 8;

            out = 0xff000000 | (r0 << 16) | (g0 << 8) | b0;
        }

        return out;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // OpenCL helpers
    /////////////////////////////////////////////////////////////////////////////////////

    bool LoadKernel(const char* _name, forg::core::vector<char>& _out_kernel)
    {
        forg::os::File kf;

        if (kf.Open(_name))
        {
            forg::uint fsize = 0;

            kf.GetSize(fsize);

            _out_kernel.resize(fsize);
            return (fsize == kf.Read(_out_kernel.data(), fsize));
        }

        return false;
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
        return min(min(a, b), c);
    }

    int max(int a, int b, int c)
    {
        return max(max(a, b), c);
    }

    struct float2
    {
        float x;
        float y;

        float2() {}
        float2(float _x, float _y) { x = _x; y = _y; }
    };

    struct float4
    {
        float x;
        float y;
        float z;
        float w;

        float4() {}
        float4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
    };

    float dot(float2 a, float2 b)
    {
        return a.x*b.x + a.y*b.y;
    }

    float linear_eq(float2 A, float C, float2 x)
    {
        // could be fma or mad (faster)
        return dot(A, x) + C;

        // return A*x + B*y + C;
    }

    float det(float2 col1, float2 col2)
    {
        // area of the parallelogram with the two vectors for sides
        return col1.x*col2.y - col2.x*col1.y;
    }

    struct TriangleInterpolator
    {
        float2 lineAB01;
        float2 lineAB12;
        float2 lineAB20;

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

        void Initialize(const float2& pos0, const float2& pos1, const float2& pos2)
        {
            lineAB01 = float2(pos0.y - pos1.y, pos1.x - pos0.x);
            lineAB12 = float2(pos1.y - pos2.y, pos2.x - pos1.x);
            lineAB20 = float2(pos2.y - pos0.y, pos0.x - pos2.x);

            //lineA01 = pos0.y - pos1.y;
            //lineA12 = pos1.y - pos2.y;
            //lineA20 = pos2.y - pos0.y;

            //lineB01 = pos1.x - pos0.x;
            //lineB12 = pos2.x - pos1.x;
            //lineB20 = pos0.x - pos2.x;

            lineC01 = det(pos0, pos1);      // pos0.x*pos1.y - pos1.x*pos0.y;
            lineC12 = det(pos1, pos2);      // pos1.x*pos2.y - pos2.x*pos1.y;
            lineC20 = det(pos2, pos0);      // pos2.x*pos0.y - pos0.x*pos2.y;

            bary01 = linear_eq(lineAB01, lineC01, pos2);
            bary12 = linear_eq(lineAB12, lineC12, pos0);
            bary20 = linear_eq(lineAB20, lineC20, pos1);

            //bary01 = line(lineA01, lineB01, lineC01, pos2.x, pos2.y);
            //bary12 = line(lineA12, lineB12, lineC12, pos0.x, pos0.y);
            //bary20 = line(lineA20, lineB20, lineC20, pos1.x, pos1.y);

            float2 xp = float2(-1, -1);

            ext01 = linear_eq(lineAB01, lineC01, xp)*bary01 > 0.0f;
            ext12 = linear_eq(lineAB12, lineC12, xp)*bary12 > 0.0f;
            ext20 = linear_eq(lineAB20, lineC20, xp)*bary20 > 0.0f;

            //ext12 = line(lineA12, lineB12, lineC12, -1, -1)*bary12 > 0.0f;
            //ext20 = line(lineA20, lineB20, lineC20, -1, -1)*bary20 > 0.0f;
            //ext01 = line(lineA01, lineB01, lineC01, -1, -1)*bary01 > 0.0f;
        }

        void SetPixel(int _x, int _y)
        {
            //TODO: could increment only, because line(x+1,y) = line(x,y)+A

            float2 pos(_x, _y);

            bary_a = linear_eq(lineAB12, lineC12, pos) / bary12;
            bary_b = linear_eq(lineAB20, lineC20, pos) / bary20;
            bary_c = linear_eq(lineAB01, lineC01, pos) / bary01;

            bary_a = fabs(bary_a);
            bary_b = fabs(bary_b);
            bary_c = fabs(bary_c);

            draw = false;

            if (bary_a >= 0.0f && bary_b >= 0.0f && bary_c >= 0.0f)
            {
                if ((bary_a > 0.0f || ext12)
                    && (bary_b > 0.0f || ext20)
                    && (bary_c > 0.0f || ext01))
                {
                    draw = true;
                }
            }
        }
        
        float line(float A, float B, float C, float x, float y)
        {
            return A*x + B*y + C;
        }

        bool CanDraw() const { return draw; }

        void Interpolate(Vector3* out, const Vector3* attribs) const
        {
            Vector3 interp = attribs[0] * bary_a + attribs[1] * bary_b + attribs[2] * bary_c;

            *out = interp;
        }

        void Interpolate(Vector3* out, const Vector3& attribs0, const Vector3& attribs1, const Vector3& attribs2) const
        {
            Vector3 interp = attribs0*bary_a + attribs1*bary_b + attribs2*bary_c;

            *out = interp;
        }

        void Interpolate(Vector4* out, const Vector4& attribs0, const Vector4& attribs1, const Vector4& attribs2) const
        {
            Vector4 interp = attribs0*bary_a + attribs1*bary_b + attribs2*bary_c;

            *out = interp;
        }

        float Interpolate(float a0, float a1, float a2) const
        {
            return a0*bary_a + a1*bary_b + a2*bary_c;
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
            return ((SWTexture*)texture)->Sample(u, v);
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
        for (int i = 0; i < NUM_SAMPLERS; i++) SetTexture(i, 0);
        for (int i = 0; i < NUM_STREAMS; i++) SetStreamSource(i, 0, 0, 0);

        delete[] m_frame_buffer;
        delete[] m_depth_buffer;
    }

    void kernel_test(float* x, float* y, float a, size_t length)
    {
        for (size_t i = 0; i < length; i++)
        {
            y[i] += a * x[i];
        }
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

        if (!InitializeCL())
        {
            return FORG_INVALID_CALL;
        }

        CreateBuffers();

        return FORG_OK;
    }

    bool SWRenderDevice::InitializeCL()
    {
        if (!m_opencl.Initialize())
        {
            return FORG_INVALID_CALL;
        }

        // Device -> Compute units -> Processing elements

        OpenCL::CLPlatform* platform = m_opencl.GetPlatform(0);
        OpenCL::CLDevice* device = platform->GetDevice(0);

        forg::PerformanceCounter job_timer;

        DBG_MSG("Create context...\n");
        m_context.Create(platform->GetPlatformId(), device->GetDeviceId());

        DBG_MSG("Load kernel...\n");
        forg::core::vector<char> kernel_src;
        if (!LoadKernel("kernels.cl", kernel_src))
        {
            DBG_MSG("Failed to load kernel file!\n");
            return false;
        }

        DBG_MSG("Create program...\n");
        m_program.CreateProgramWithSource(m_context.GetContext(), kernel_src.data(), kernel_src.size());

        DBG_MSG("Build program...\n");
        ASSERT( m_program.BuildProgram(device->GetDeviceId()) );

        DBG_MSG("Create kernel...\n");

        m_queue.Create(m_context.GetContext(), device->GetDeviceId(), 0);

        m_kPrepareBlock.Create(m_program.GetProgram(), "PrepareBlock");
        m_kDrawBlock.Create(m_program.GetProgram(), "DrawBlock");
        m_kClearScreenBuffer.Create(m_program.GetProgram(), "ClearScreenBuffer");

        OpenCL::CLKernelWorkGroupInfo kwginfo;
        m_kDrawBlock.GetKernelWorkGroupInfo(device->GetDeviceId(), kwginfo);
        DBG_MSG("kernel DrawBlock:\n");
        DBG_MSG("\tWork group size: %d\n", kwginfo.WorkGroupSize);
        DBG_MSG("\tPreferred Work group size multiple: %d\n", kwginfo.PreferredWorkGroupSizeMultiple);
        DBG_MSG("\tLocal mem size: %llu\n", kwginfo.LocalMemSize);
        DBG_MSG("\tPrivate mem size: %llu\n", kwginfo.PrivateMemSize);

        return true;
    }

    void SWRenderDevice::CreateBuffers()
    {
        DBG_MSG("Create buffers...\n");
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
        m_zb_stride = m_width * 4;
        m_frame_buffer = new uint[m_width*m_height];
        m_depth_buffer = new float[m_width*m_height];

        cl_image_format img_format;
        img_format.image_channel_data_type = CL_UNSIGNED_INT8;
        img_format.image_channel_order = CL_RGBA; //CL_ARGB;
        
        m_fbuffer.Release();
        if (!m_fbuffer.CreateImage2D(m_context.GetContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &img_format, m_width, m_height, m_fb_stride, m_frame_buffer))
        {
            DBG_MSG("Failed to create OpenCL image buffer!");
        }

        img_format.image_channel_data_type = CL_FLOAT;
        img_format.image_channel_order = CL_A; //CL_ARGB;

        m_zbuffer.Release();
        if (!m_zbuffer.CreateBuffer(m_context.GetContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,  m_zb_stride*m_height, m_depth_buffer))
        {
            DBG_MSG("Failed to create OpenCL image buffer!");
        }

    }

    int SWRenderDevice::Reset()
    {
        RECT rcClient;
        GetClientRect((HWND)m_window, &rcClient);

        // calculate window width/height 
        m_width = rcClient.right - rcClient.left;  
        m_height = rcClient.bottom - rcClient.top;  

        CreateBuffers();

        return FORG_OK;
    }

    LPVERTEXBUFFER SWRenderDevice::CreateVertexBuffer(uint length, uint usage, uint pool)
    {
        SWVertexBuffer* vb = new SWVertexBuffer();

        vb->Create(length, usage, pool);

        return vb;
    }

    LPINDEXBUFFER SWRenderDevice::CreateIndexBuffer(uint length, uint usage, bool sixteenBitIndices, uint pool)
    {
        SWIndexBuffer* ib = new SWIndexBuffer();

        ib->Create(length, usage, sixteenBitIndices, pool);

        return ib;
    }

    LPTEXTURE SWRenderDevice::CreateTexture(uint Width, uint Height, uint Levels, uint Usage, uint Format, uint Pool)
    {
        SWTexture* tex = new SWTexture();

        tex->Create(m_context, m_queue, Width, Height, Levels, Usage, Format, Pool);

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

    enum
    {
        MAX_VERTEX_BATCH_SIZE = 3 * 512
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

        VSOutput vs_batch[MAX_VERTEX_BATCH_SIZE];
        VSInput vs_input[3];
        VSOutput* vs_output = vs_batch;

        uint tri_count = 0;

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

                // TODO: what with per vertex usage?
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
                //DrawTriangle(vs_output, vertex_usage);

                vs_output += 3;
                tri_count++;
            }

            if (vs_output - vs_batch == MAX_VERTEX_BATCH_SIZE)
            {
                DrawTriangleArrayCL(vs_batch, tri_count, vertex_usage);

                vs_output = vs_batch;
                tri_count = 0;
            }
        }

        if (tri_count > 0)
            DrawTriangleArrayCL(vs_batch, tri_count, vertex_usage);                

        //SetStreamSource(0, 0, 0, 0);
        //SetIndices(0);
	    //GLV(glDrawElements(mode, num_indices, sixteenBitIndices ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexData));	//ogl 1.1

        return FORG_OK;
    }

    int SWRenderDevice::DrawIndexedPrimitive(PrimitiveType primitiveType, int baseVertex, int minVertexIndex, int numVertices, int startIndex, int primCount)
    {
        SWVertexBuffer* vb = (SWVertexBuffer*)m_streams[0].streamData;
        SWIndexBuffer* ib = (SWIndexBuffer*)m_indices;

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

            size_t origin[3] = { 0, 0, 0 };
            size_t region[3] = { m_width, m_height, 1 };
            m_queue.EnqueueWriteImage(m_fbuffer.GetMemObject(), CL_FALSE, origin, region, m_fb_stride, 0, m_frame_buffer);
        }

        if (flags & forg::ClearFlags_ZBuffer)
        {
            for (uint p=0; p<m_width*m_height; p++)
            {
                m_depth_buffer[p] = zdepth;
            }

            const size_t globalWorkSize[] = { m_width, m_height, 0 };

            m_kClearScreenBuffer.SetKernelArg(0, m_zbuffer);
            m_kClearScreenBuffer.SetKernelArg(1, sizeof(float), &zdepth);

            m_queue.EnqueueNDRangeKernel(m_kClearScreenBuffer.GetKernel(), 2,
                nullptr, globalWorkSize, nullptr);
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

    /// returns r where r >= v and r = x * 2^pow2
    template <uint pow2>
    uint next_mul(uint v)
    {
        const uint c = (1 << pow2) - 1;
        //uint a = (v & c) ^ c;
        //v += a + 1;
       

        v += c;
        v &= ~c;

        return v;
    }

    int SWRenderDevice::Present()
    {
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
        //bi.bmiHeader.biSizeImage = ((ulWindowWidth * bi.bmiHeader.biBitCount +31)& ~31) / 8 * ulWindowHeight; 
        bi.bmiHeader.biSizeImage = next_mul<5>(ulWindowWidth * bi.bmiHeader.biBitCount) / 8 * ulWindowHeight;

        // create our DIB section and select the bitmap into the dc 
        VOID *pvBits;          // pointer to DIB section 
        HBITMAP hBitmap = CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
        HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

        //for (uint y = 0; y < ulWindowHeight; y++)
        //for (uint x = 0; x < ulWindowWidth; x++)
        //    ((UINT32 *)pvBits)[x + y * ulWindowWidth] = 0xff0000ff; 

        if (USE_SOFTWARE_MODE)
        {
            memcpy(pvBits, m_frame_buffer, ulWindowWidth*ulWindowHeight * 4);
        }
        else
        {
            // Get the results back to the host
            size_t origin[3] = { 0, 0, 0 };
            size_t region[3] = { m_width, m_height, 1 };
            m_queue.EnqueueReadImage(m_fbuffer.GetMemObject(), CL_TRUE,
                origin, region, m_fb_stride, 0, pvBits);

            m_mem_buffers.clear();

            m_queue.Finish();
        }

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

        return FORG_OK;
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

    void SWRenderDevice::DrawTriangleArray(const VSOutput* vertices, uint num_triangles, int usage)
    {
        for (uint i = 0; i < num_triangles; i++)
        {
            DrawTriangle(vertices + i*3, usage);
        }
    }

    void SWRenderDevice::DrawTriangleArrayCL(VSOutput* vertices, uint num_triangles, int usage)
    {
        if (num_triangles == 0)
            return;

        OpenCL::CLPlatform* platform = m_opencl.GetPlatform(0);
        OpenCL::CLDevice* device = platform->GetDevice(0);

        // Prepare buffers

        m_mem_buffers.push_back(OpenCL::CLMemObject());
        OpenCL::CLMemObject& tri_buffer = m_mem_buffers.back();
        tri_buffer.CreateBuffer(m_context.GetContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(VSOutput)*num_triangles*3, vertices);

        // Set kernel attributes

        m_kDrawBlock.SetKernelArg(0, m_fbuffer);
        m_kDrawBlock.SetKernelArg(1, m_zbuffer);
        m_kDrawBlock.SetKernelArg(2, tri_buffer);
        m_kDrawBlock.SetKernelArg(3, sizeof(num_triangles), &num_triangles);
        m_kDrawBlock.SetKernelArg(4, sizeof(usage), &usage);
        if (bit_test(usage, DeclarationUsage_TextureCoordinate) && m_samplers[0].texture)
        {
            m_kDrawBlock.SetKernelArg(5, static_cast<SWTexture*>(m_samplers[0].texture)->GetBuffer());
        }
        else
        {
            m_kDrawBlock.SetKernelArg(5, m_fbuffer);
        }

        // Set up work groups

        uint xmax = 0;
        uint xmin = m_width;
        uint ymax = 0;
        uint ymin = m_height;
        uint xtreme_found = 0;

        for (size_t i = 0; i < num_triangles*3 && xtreme_found < 4; i++)
        {
            int screenx = iround(vertices[i].position.X);
            int screeny = iround(vertices[i].position.Y);

            if (screenx < xmin && xmin > 0)
            {
                xmin = screenx;

                if (xmin <= 0)
                {
                    xmin = 0;
                    xtreme_found++;
                }
            }
            else if (screenx > xmax && xmax < m_width)
            {
                xmax = screenx;

                if (xmax >= m_width)
                {
                    xmax = m_width;
                    xtreme_found++;
                }
            }

            if (screeny < ymin && ymin > 0)
            {
                ymin = screeny;

                if (ymin <= 0)
                {
                    ymin = 0;
                    xtreme_found++;
                }
            }
            else if (screeny > ymax && ymax < m_height)
            {
                ymax = screeny;

                if (ymax >= m_height)
                {
                    ymax = m_height;
                    xtreme_found++;
                }
            }
        }

        /*
        xmin = 0;
        xmax = m_width;
        ymin = 0;
        ymax = m_height;
        */

        xmin &= ~7;
        ymin &= ~7;
        uint xsize = next_mul<3>(xmax-xmin);
        uint ysize = next_mul<3>(ymax-ymin);
        const size_t globalWorkOffset[] = { xmin, ymin, 0 };
        const size_t globalWorkSize[] = { xsize, ysize, 0 };
        const size_t localWorkSize[] = { 8, 8, 0 };

        // Enqueue job 

        // we need one loop to iterate through triangles
        m_queue.EnqueueNDRangeKernel(m_kDrawBlock.GetKernel(), 2,
            globalWorkOffset, globalWorkSize, localWorkSize);
    }

    /*
    void SWRenderDevice::DrawTriangleArrayCL(VSOutput* vertices, uint num_triangles, int usage)
    {
        if (num_triangles == 0)
            return;

        OpenCL::CLPlatform* platform = m_opencl.GetPlatform(0);
        OpenCL::CLDevice* device = platform->GetDevice(0);

        OpenCL::CLMemObject tri_buffer;
        tri_buffer.CreateBuffer(m_context.GetContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(VSOutput)*num_triangles, vertices);

        OpenCL::CLCommandQueue queue;
        queue.Create(m_context.GetContext(), device->GetDeviceId(), 0);

        m_kRasterize.SetKernelArg(0, tri_buffer);
        m_kRasterize.SetKernelArg(1, m_fbuffer);
        int vertex_size = sizeof(VSOutput);
        m_kRasterize.SetKernelArg(2, sizeof(vertex_size), &vertex_size);

        const size_t globalWorkSize[] = { num_triangles, 0, 0 };

        // we need one loop to iterate through triangles
        queue.EnqueueNDRangeKernel(m_kRasterize.GetKernel(), 1,
            nullptr, globalWorkSize, nullptr);
    }*/

    void SWRenderDevice::DrawTriangleCL(const VSOutput* vertices, int usage)
    {

    }

    struct KernelTest
    {
        uint sx;
        uint sy;

        uint width;
        uint height;

        struct int2
        {
            int x;
            int y;

            int2(int _x, int _y) { x = _x; y = _y; }
        };

        struct uint4
        {
            uint x;
            uint y;
            uint z;
            uint w;

            uint4(uint _x, uint _y, uint _z, uint _w) { x = _x; y = _y; z = _z; w = _w; }
        };


        int convert_int(float f)
        {
            return (int)f;
        }

        int min3i(int a, int b, int c)
        {
            return min(min(a, b), c);
        }

        int max3i(int a, int b, int c)
        {
            return max(max(a, b), c);
        }

        void write_imageui(uint* buffer, int2& coords, uint4& col)
        {
            uint index = coords.y*width + coords.x;

            uint& p = buffer[index];

            // ZYXW
            p = (col.z << 24) | (col.y << 16) | (col.x << 8) | col.w;
        }

        void DrawBlock(uint* buffer, VSOutput* vertices, uint vertex_size, uint num_triangles)
        {
            //int width = get_image_width(buffer);
            //int height = get_image_height(buffer);
            int stride = vertex_size / 4;

            //int sx = get_global_id(0);
            //int sy = get_global_id(1);

            //int lx = get_local_id(0);
            //int ly = get_local_id(1);

            // GRAB?
            //uint4 col = (uint4)(lx*20, ly*20, 255, 0);  
            uint4 col = uint4(0, 0, 255, 0);
            uint4 colIn = uint4(255, 255, 255, 255);
            uint4 colEdge = uint4(255, 0, 255, 0);

            for (uint t = 0; t<num_triangles; t++)
            {
                int tri0 = t * 3 ;
                int tri1 = t * 3  + 1;
                int tri2 = t * 3  + 2;

                const int Y1 = convert_int(16.0f * vertices[tri0].position.Y);
                const int Y2 = convert_int(16.0f * vertices[tri1].position.Y);
                const int Y3 = convert_int(16.0f * vertices[tri2].position.Y);

                const int X1 = convert_int(16.0f * vertices[tri0].position.X);
                const int X2 = convert_int(16.0f * vertices[tri1].position.X);
                const int X3 = convert_int(16.0f * vertices[tri2].position.X);

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

                int minx = (min3i(X1, X2, X3) + 0xF) >> 4;
                int maxx = (max3i(X1, X2, X3) + 0xF) >> 4;
                int miny = (min3i(Y1, Y2, Y3) + 0xF) >> 4;
                int maxy = (max3i(Y1, Y2, Y3) + 0xF) >> 4;

                if (sx < minx || sx > maxx)
                    continue;

                if (sy < miny || sy > maxy)
                    continue;

                if (minx < 0) minx = 0;
                if (miny < 0) miny = 0;
                if (maxx > width) maxx = width;
                if (maxy > height) maxy = height;

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
                if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
                if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
                if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

                // Loop through blocks
                int y = sy & ~(q - 1);
                int x = sx & ~(q - 1);

                //for(int y = miny; y < maxy; y += q)
                if (y >= miny && y < maxy)
                {
                    //for(int x = minx; x < maxx; x += q)
                    if (x >= minx && x < maxx)
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
                        if (a == 0x0 || b == 0x0 || c == 0x0) //continue;
                        {
                        }
                        // Accept whole block when totally covered
                        else if (a == 0xF && b == 0xF && c == 0xF)
                        {
                            //for(int iy = 0; iy < q && y+iy<maxy; iy++)
                            {

                                //for(int ix = x; ix < x + q && ix <maxx; ix++)
                                {
                                    col = uint4(255, 255, 255, 255);

                                    //write_imageui(buffer, (int2)(ix, y+iy), c);

                                    if (sx < width && sy < height) write_imageui(buffer, int2(sx, height - 1 - sy), colIn);
                                }
                            }
                        }
                        else // Partially covered block
                        {
                            int CY1 = C1 + DX12 * y0 - DY12 * x0;
                            int CY2 = C2 + DX23 * y0 - DY23 * x0;
                            int CY3 = C3 + DX31 * y0 - DY31 * x0;

                            int iy = sy;
                            //for(int iy = y; iy < y + q && iy<maxy; iy++)
                            {
                                int CX1 = CY1 + FDX12*(sy - y) - FDY12*(sx - x);
                                int CX2 = CY2 + FDX23*(sy - y) - FDY23*(sx - x);
                                int CX3 = CY3 + FDX31*(sy - y) - FDY31*(sx - x);

                                int ix = sx;
                                //for(int ix = x; ix < x + q; ix++)
                                {
                                    if (CX1 > 0 && CX2 > 0 && CX3 > 0)
                                    {
                                        col = uint4(255, 0, 255, 0);
                                        //write_imageui(buffer, (int2)(ix, iy), c);

                                        if (sx < width && sy < height) write_imageui(buffer, int2(sx, height - 1 - sy), colEdge);
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
                // end of blocks checking loop    
            }

            if (sx < width && sy < height)
            {
                //write_imageui(buffer, (int2)(sx, height - 1 - sy), col);
            }
        }
    };

    void SWRenderDevice::DrawTriangleArrayCLTest(VSOutput* vertices, uint num_triangles, int usage)
    {
        if (num_triangles == 0)
            return;

        uint xmax = 0;
        uint xmin = m_width;
        uint ymax = 0;
        uint ymin = m_height;
        uint xtreme_found = 0;

        for (size_t i = 0; i < num_triangles * 3 && xtreme_found < 4; i++)
        {
            int screenx = iround(vertices[i].position.X);
            int screeny = iround(vertices[i].position.Y);

            if (screenx < xmin && xmin > 0)
            {
                xmin = screenx;

                if (xmin <= 0)
                {
                    xmin = 0;
                    xtreme_found++;
                }
            }
            else if (screenx > xmax && xmax < m_width)
            {
                xmax = screenx;

                if (xmax >= m_width)
                {
                    xmax = m_width;
                    xtreme_found++;
                }
            }

            if (screeny < ymin && ymin > 0)
            {
                ymin = screeny;

                if (ymin <= 0)
                {
                    ymin = 0;
                    xtreme_found++;
                }
            }
            else if (screeny > ymax && ymax < m_height)
            {
                ymax = screeny;

                if (ymax >= m_height)
                {
                    ymax = m_height;
                    xtreme_found++;
                }
            }
        }

        /*
        xmin = 0;
        xmax = m_width;
        ymin = 0;
        ymax = m_height;
        */

        uint xsize = next_mul<3>(xmax);
        uint ysize = next_mul<3>(ymax);
        const size_t globalWorkOffset[] = { xmin, ymin, 0 };
        const size_t globalWorkSize[] = { xsize, ysize, 0 };
        const size_t localWorkSize[] = { 8, 8, 0 };

        KernelTest kt;

        for (uint sy = 0; sy < m_height; sy++)
        {
            for (uint sx = 0; sx < m_width; sx++)
            {
                kt.sx = sx;
                kt.sy = sy;
                kt.width = m_width;
                kt.height = m_height;
                kt.DrawBlock(m_frame_buffer, vertices, sizeof(VSOutput), num_triangles);
            }
        }
    }

    /*struct int2
    {
        int x;
        int y;

        int2() {}
        int2(int _x, int _y) { x = _x; y = _y; }
    };

    void SWRenderDevice::DrawTriangleCL(const VSOutput* vertices, int usage)
    {
        forg::core::vector<int2> pixels;
        pixels.reserve(4096);

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
        if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
        if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
        if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

        // Loop through blocks
        for (int y = miny; y < maxy; y += q)
        {
            for (int x = minx; x < maxx; x += q)
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
                if (a == 0x0 || b == 0x0 || c == 0x0) continue;

                // Accept whole block when totally covered

                if (a == 0xF && b == 0xF && c == 0xF)
                {
                    for (int iy = 0; iy < q && y + iy<maxy; iy++)
                    {
                        for (int ix = x; ix < x + q && ix <maxx; ix++)
                        {
                            pixels.push_back(int2(ix, m_height - 1 - (y + iy)));
                        }
                    }
                }
                else // Partially covered block
                {
                    int CY1 = C1 + DX12 * y0 - DY12 * x0;
                    int CY2 = C2 + DX23 * y0 - DY23 * x0;
                    int CY3 = C3 + DX31 * y0 - DY31 * x0;

                    for (int iy = y; iy < y + q && iy<maxy; iy++)
                    {
                        int CX1 = CY1;
                        int CX2 = CY2;
                        int CX3 = CY3;

                        for (int ix = x; ix < x + q; ix++)
                        {
                            if (CX1 > 0 && CX2 > 0 && CX3 > 0)
                            {                                
                                pixels.push_back(int2(ix, m_height - 1 - iy));
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

        // send to cl
        if (pixels.size() > 0)
        {
            OpenCL::CLPlatform* platform = m_opencl.GetPlatform(0);
            OpenCL::CLDevice* device = platform->GetDevice(0);

            m_mem_buffers.push_back(OpenCL::CLMemObject());
            OpenCL::CLMemObject& pixel_buffer = m_mem_buffers.back();
            pixel_buffer.CreateBuffer(m_context.GetContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(int2)*pixels.size(), pixels.data());

            m_kDrawPixels.SetKernelArg(0, pixel_buffer);
            m_kDrawPixels.SetKernelArg(1, m_fbuffer);

            const size_t globalWorkSize[] = { pixels.size(), 0, 0 };

            // we need one loop to iterate through triangles
            m_queue.EnqueueNDRangeKernel(m_kDrawPixels.GetKernel(), 1,
                nullptr, globalWorkSize, nullptr);
        }
    }*/

    void SWRenderDevice::DrawTriangle(const VSOutput* vertices, int usage)
    {
        TriangleInterpolator interpolator;
        PSInput ps_input;
        PSOutput ps_output;

        float2 pos0(vertices[0].position.X, vertices[0].position.Y);
        float2 pos1(vertices[1].position.X, vertices[1].position.Y);
        float2 pos2(vertices[2].position.X, vertices[2].position.Y);

        interpolator.Initialize(pos0, pos1, pos2);

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

}

