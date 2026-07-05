// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "core/RefCounter.h"

namespace forg {
/// IVertexBuffer interface
/**
 * IVertexBuffer
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class IVertexBuffer : public core::RefCounter
{
  public:
    virtual ~IVertexBuffer() {}

  public:
    virtual int Lock(u32 offsetToLock, u32 sizeToLock, void** ppbData,
                     u32 flags) = 0;

    virtual int Unlock() = 0;
};

using LPVERTEXBUFFER = IVertexBuffer*;

} // namespace forg
