// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "rendering/IIndexBuffer.h"
#include "rendering/ITexture.h"
#include "rendering/IVertexBuffer.h"

#include <memory>

namespace forg::rendering::reference {

/////////////////////////////////////////////////////////////////////////////////////
// SWTexture
/////////////////////////////////////////////////////////////////////////////////////

class FORG_API SWTexture : public ITexture
{
    int m_refCount;
    std::unique_ptr<char[]> m_data;
    u32 m_Width;
    u32 m_Height;
    u32 m_Levels;
    u32 m_Usage;
    u32 m_Format;
    u32 m_Pool;

  public:
    SWTexture();
    virtual ~SWTexture();

    int Create(u32 Width, u32 Height, u32 Levels, u32 Usage, u32 Format,
               u32 Pool);

    u32 Sample(float u, float v);

    // ITexture implementation
  public:
    u32 GetLevelCount();

    int GetLevelDesc(u32 Level, SurfaceDescription* Description) const;

    void* LockRect(u32 Level, u32 Flags);

    int UnlockRect(u32 Level);
};

/////////////////////////////////////////////////////////////////////////////////////
// SWVertexBuffer
/////////////////////////////////////////////////////////////////////////////////////
class FORG_API SWVertexBuffer : public IVertexBuffer
{
    std::unique_ptr<char[]> m_data;

    u32 m_length;
    u32 m_usage;
    u32 m_pool;

  public:
    SWVertexBuffer();
    virtual ~SWVertexBuffer();

    char* GetData() { return m_data.get(); }

    int Create(u32 length, u32 usage, u32 pool);

  public:
    virtual int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                     u32 flags);

    virtual int Unlock();
};

/////////////////////////////////////////////////////////////////////////////////////
// SWIndexBuffer
/////////////////////////////////////////////////////////////////////////////////////
class FORG_API SWIndexBuffer : public IIndexBuffer
{
    std::unique_ptr<char[]> m_data;

    u32 m_length;
    u32 m_usage;
    bool m_short;
    u32 m_pool;

  public:
    SWIndexBuffer();
    virtual ~SWIndexBuffer();

    char* GetData() { return m_data.get(); }

    u32 GetLength() const { return m_length; }

    int GetIndexSize() const { return (m_short ? 2 : 4); }

    bool IsIndexShort() const { return m_short; }

    int Create(u32 length, u32 usage, bool sixteenBitIndices, u32 pool);

  public:
    virtual int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                     u32 flags);

    virtual int Unlock();
};
} // namespace forg::rendering::reference
