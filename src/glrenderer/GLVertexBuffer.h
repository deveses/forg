// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "GLRenderDevice.h"
#include <forg.h>

namespace forg {

class GLVertexBuffer : public IVertexBuffer
{
  public:
    GLVertexBuffer(GLRenderDevice* device, u32 sizeOfBufferInBytes, u32 usage,
                   u32 pool);
    ~GLVertexBuffer(void);

  public:
    int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData, u32 flags);

    int Unlock();

    u32 m_buffer_id;

  private:
    GLRenderDevice* m_device;
    const GLDeviceCaps* m_caps;

    u32 m_usage;
    u32 m_pool;
    u32 m_type;
    bool m_created;
};

} // namespace forg
