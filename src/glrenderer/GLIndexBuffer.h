// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "rendering/IIndexBuffer.h"

namespace forg {

class GLRenderDevice;
class GLDeviceCaps;

class GLIndexBuffer : public IIndexBuffer
{
  public:
    GLIndexBuffer(GLRenderDevice* device, u32 sizeOfBufferInBytes, u32 usage,
                  u32 pool, bool sixteenBitIndices);

    ~GLIndexBuffer(void);

  public:
    int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData, u32 flags);

    int Unlock();

    u32 m_buffer_id;
    bool m_sixteen;
    u32 m_size;

  private:
    GLRenderDevice* m_device;
    const GLDeviceCaps* m_caps;

    u32 m_usage;
    u32 m_pool;
    u32 m_type;
};

} // namespace forg
