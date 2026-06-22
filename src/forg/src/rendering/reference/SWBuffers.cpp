#include "forg_pch.h"

#include "rendering/reference/SWBuffers.h"

namespace forg::rendering::reference {

/////////////////////////////////////////////////////////////////////////////////////
// SWTexture
/////////////////////////////////////////////////////////////////////////////////////

SWTexture::SWTexture() { m_refCount = 1; }

SWTexture::~SWTexture() = default;

uint SWTexture::GetLevelCount() { return m_Levels; }

int SWTexture::GetLevelDesc(uint level, SurfaceDescription* description) const
{
    if (level >= m_Levels || description == nullptr)
        return FORG_INVALID_CALL;

    description->Width = m_Width;
    description->Height = m_Height;
    description->Format = m_Format;

    return FORG_OK;
}

void* SWTexture::LockRect(uint level, uint)
{
    return level < m_Levels ? m_data.get() : nullptr;
}

int SWTexture::UnlockRect(uint level)
{
    return level < m_Levels ? FORG_OK : FORG_INVALID_CALL;
}

int SWTexture::Create(uint Width, uint Height, uint, uint Usage, uint Format,
                      uint Pool)
{
    m_Levels = 1;
    m_Width = Width;
    m_Height = Height;
    m_Usage = Usage;
    m_Format = Format;
    m_Pool = Pool;

    uint stride = Width * 4;
    uint size = stride * Height;

    m_data = std::make_unique<char[]>(size);

    return FORG_OK;
}

uint SWTexture::Sample(float u, float v)
{
    uint x = u * m_Width;
    uint y = v * m_Height;

    if (x < m_Width && y < m_Height)
    {
        uint* buf_argb = reinterpret_cast<uint*>(m_data.get());

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

int SWVertexBuffer::Create(uint length, uint usage, uint pool)
{
    m_length = length;
    m_usage = usage;
    m_pool = pool;

    m_data = std::make_unique<char[]>(length);

    return FORG_OK;
}

int SWVertexBuffer::Lock(uint offsetToLock, uint sizeToLock, void** ppbData,
                         uint)
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

int SWIndexBuffer::Create(uint length, uint usage, bool sixteenBitIndices,
                          uint pool)
{
    m_length = length;
    m_usage = usage;
    m_pool = pool;
    m_short = sixteenBitIndices;

    m_data = std::make_unique<char[]>(length);

    return FORG_OK;
}

int SWIndexBuffer::Lock(uint offsetToLock, uint sizeToLock, void** ppbData,
                        uint)
{
    if (ppbData == nullptr || offsetToLock > m_length ||
        (sizeToLock != 0 && sizeToLock > m_length - offsetToLock))
        return FORG_INVALID_CALL;

    *ppbData = m_data.get() + offsetToLock;

    return FORG_OK;
}

int SWIndexBuffer::Unlock() { return FORG_OK; }

} // namespace forg::rendering::reference
