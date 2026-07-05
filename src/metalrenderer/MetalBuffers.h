// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 Slawomir Strumecki

#pragma once
#include "base.h"
#include "rendering/IIndexBuffer.h"
#include "rendering/IVertexBuffer.h"

namespace forg {

// Both buffers wrap a single MTLBuffer (storageModeShared) so the CPU pointer
// returned by Lock() points straight at GPU-visible memory; Unlock() is a
// no-op. The MTLBuffer handle is stored as void* so this header stays
// includable from plain C++ (the factory in MetalRenderer.cpp never sees an
// ObjC type).

class MetalVertexBuffer : public IVertexBuffer
{
  public:
    MetalVertexBuffer();
    virtual ~MetalVertexBuffer();

    // mtlDevice is an id<MTLDevice>; allocates a `length`-byte shared buffer.
    int Create(void* mtlDevice, u32 length);

    void* GetMTLBuffer() const { return m_buffer; }
    u32 GetLength() const { return m_length; }

    virtual int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                     u32 flags);
    virtual int Unlock();

  private:
    void* m_buffer; // id<MTLBuffer>, retained
    u32 m_length;
};

class MetalIndexBuffer : public IIndexBuffer
{
  public:
    MetalIndexBuffer();
    virtual ~MetalIndexBuffer();

    int Create(void* mtlDevice, u32 length, bool sixteenBitIndices);

    void* GetMTLBuffer() const { return m_buffer; }
    u32 GetLength() const { return m_length; }
    bool IsIndexShort() const { return m_short; }

    virtual int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                     u32 flags);
    virtual int Unlock();

  private:
    void* m_buffer; // id<MTLBuffer>, retained
    u32 m_length;
    bool m_short;
};

} // namespace forg
