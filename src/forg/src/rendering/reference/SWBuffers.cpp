#include "forg_pch.h"

#include "rendering/reference/SWBuffers.h"

namespace forg::rendering::reference {

/////////////////////////////////////////////////////////////////////////////////////
// SWTexture
/////////////////////////////////////////////////////////////////////////////////////

SWTexture::SWTexture() { m_refCount = 1; }

SWTexture::~SWTexture() = default;

u32 SWTexture::GetLevelCount() { return m_Levels; }

int SWTexture::GetLevelDesc(u32 level, SurfaceDescription* description) const
{
    if (level >= m_Levels || description == nullptr)
        return FORG_INVALID_CALL;

    description->Width = m_Width;
    description->Height = m_Height;
    description->Format = m_Format;

    return FORG_OK;
}

void* SWTexture::LockRect(u32 level, u32)
{
    return level < m_Levels ? m_data.get() : nullptr;
}

int SWTexture::UnlockRect(u32 level)
{
    return level < m_Levels ? FORG_OK : FORG_INVALID_CALL;
}

int SWTexture::Create(u32 Width, u32 Height, u32, u32 Usage, u32 Format,
                      u32 Pool)
{
    m_Levels = 1;
    m_Width = Width;
    m_Height = Height;
    m_Usage = Usage;
    m_Format = Format;
    m_Pool = Pool;

    u32 stride = Width * 4;
    u32 size = stride * Height;

    m_data = std::make_unique<char[]>(size);

    return FORG_OK;
}

u32 SWTexture::Sample(float u, float v)
{
    u32 x = u * m_Width;
    u32 y = v * m_Height;

    if (x < m_Width && y < m_Height)
    {
        u32* buf_argb = reinterpret_cast<u32*>(m_data.get());

        return buf_argb[y * m_Width + x];
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
}

SWVertexBuffer::~SWVertexBuffer() = default;

int SWVertexBuffer::Create(u32 length, u32 usage, u32 pool)
{
    m_length = length;
    m_usage = usage;
    m_pool = pool;

    m_data = std::make_unique<char[]>(length);

    return FORG_OK;
}

int SWVertexBuffer::Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                         u32)
{
    if (ppbData == nullptr || offsetToLock > m_length ||
        (sizeToLock != 0 && sizeToLock > m_length - offsetToLock))
        return FORG_INVALID_CALL;

    *ppbData = m_data.get() + offsetToLock;

    return FORG_OK;
}

int SWVertexBuffer::Unlock() { return FORG_OK; }

/////////////////////////////////////////////////////////////////////////////////////
// SWIndexBuffer
/////////////////////////////////////////////////////////////////////////////////////

SWIndexBuffer::SWIndexBuffer()
{
    m_length = 0;
    m_usage = 0;
    m_pool = 0;
    m_short = false;
}

SWIndexBuffer::~SWIndexBuffer() = default;

int SWIndexBuffer::Create(u32 length, u32 usage, bool sixteenBitIndices,
                          u32 pool)
{
    m_length = length;
    m_usage = usage;
    m_pool = pool;
    m_short = sixteenBitIndices;

    m_data = std::make_unique<char[]>(length);

    return FORG_OK;
}

int SWIndexBuffer::Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                        u32)
{
    if (ppbData == nullptr || offsetToLock > m_length ||
        (sizeToLock != 0 && sizeToLock > m_length - offsetToLock))
        return FORG_INVALID_CALL;

    *ppbData = m_data.get() + offsetToLock;

    return FORG_OK;
}

int SWIndexBuffer::Unlock() { return FORG_OK; }

} // namespace forg::rendering::reference
