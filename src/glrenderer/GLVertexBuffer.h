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
