#include "SWBuffers.h"

namespace forg {

    /////////////////////////////////////////////////////////////////////////////////////
    // SWTexture
    /////////////////////////////////////////////////////////////////////////////////////

    SWTexture::SWTexture()
    {
        m_refCount = 1;
        m_data = 0;
    }

    SWTexture::~SWTexture()
    {
        if (m_data)
        {
            delete [] m_data;
            m_data = 0;
        }
    }

    u32 SWTexture::GetLevelCount()
    {
        return m_Levels;
    }

    int SWTexture::GetLevelDesc(u32 Level, SurfaceDescription* Description) const
    {
        Description->Width = m_Width;
        Description->Height = m_Height;
        Description->Format = m_Format;

        return FORG_OK;
    }

    void* SWTexture::LockRect(u32 Level, u32 Flags)
    {
        return m_data;
    }

    int SWTexture::UnlockRect(u32 Level)
    {
        u32 stride = m_Width * 4;
        size_t origin[3] = { 0, 0, 0 };
        size_t region[3] = { m_Width, m_Height, 1 };
        m_queue.EnqueueWriteImage(m_buffer.GetMemObject(), CL_TRUE, origin, region, stride, 0, m_data);

        return FORG_OK;
    }

    int SWTexture::Create(OpenCL::CLContext& context, OpenCL::CLCommandQueue& queue, u32 Width, u32 Height, u32 Levels, u32 Usage, u32 Format, u32 Pool)
    {
        m_Levels = 1;
        m_Width = Width;
        m_Height = Height;
        m_Usage = Usage;
        m_Format = Format;
        m_Pool = Pool;

        m_queue.Create(queue);

        u32 stride = Width*4;
        u32 size = stride * Height;

        m_data = new char[size];

        cl_image_format img_format;
        img_format.image_channel_data_type = CL_UNSIGNED_INT8;
        img_format.image_channel_order = CL_RGBA; // always supported;

        if (!m_buffer.CreateImage2D(context.GetContext(), CL_MEM_READ_WRITE, &img_format, m_Width, m_Height, 0, nullptr))
        {
            return FORG_INVALID_CALL;
        }

        return FORG_OK;
    }

    u32 SWTexture::Sample(float u, float v)
    {
        u32 x = u32(u*m_Width);
        u32 y = u32(v*m_Height);

        if (x < m_Width && y < m_Height)
        {
            u32* buf_argb = (u32*)m_data;

            return buf_argb[y*m_Width + x];
        }

        return 0xffffffff;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // SWVertexBuffer
    /////////////////////////////////////////////////////////////////////////////////////

    SWVertexBuffer::SWVertexBuffer()
    {
        m_length = 0;
        m_usage = 0;
        m_pool = 0;
        m_data = 0;
    }

    SWVertexBuffer::~SWVertexBuffer()
    {
        delete [] m_data;
    }

    int SWVertexBuffer::Create(u32 length, u32 usage, u32 pool)
    {
        m_length = length;
        m_usage = usage;
        m_pool = pool;

        m_data = new char[length];

        return FORG_OK;
    }

	int SWVertexBuffer::Lock(u32 offsetToLock, u32 sizeToLock, void ** ppbData, u32 flags)
    {
        *ppbData = m_data;

        return FORG_OK;
    }

	int SWVertexBuffer::Unlock()
    {
        return FORG_OK;
    }


    /////////////////////////////////////////////////////////////////////////////////////
    // SWIndexBuffer
    /////////////////////////////////////////////////////////////////////////////////////

    SWIndexBuffer::SWIndexBuffer()
    {
        m_length = 0;
        m_usage = 0;
        m_pool = 0;
        m_short = false;
        m_data = 0;
    }

    SWIndexBuffer::~SWIndexBuffer()
    {
        delete [] m_data;
    }

    int SWIndexBuffer::Create(u32 length, u32 usage, bool sixteenBitIndices, u32 pool)
    {
        m_length = length;
        m_usage = usage;
        m_pool = pool;
        m_short = sixteenBitIndices;

        m_data = new char[length];

        return FORG_OK;
    }

	int SWIndexBuffer::Lock(u32 offsetToLock, u32 sizeToLock, void ** ppbData, u32 flags)
    {
        *ppbData = m_data;

        return FORG_OK;
    }

	int SWIndexBuffer::Unlock()
    {
        return FORG_OK;
    }
}
