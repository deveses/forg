// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2012 Slawomir Strumecki

#pragma once
#include "base.h"
#include "math/Vector2.h"
#include "math/Vector4.h"
#include "rendering/VertexDeclaration.h"
#include "rendering/reference/SWRenderDevice.h"

namespace forg {

class SWRenderDevice : public forg::rendering::reference::SWRenderDevice
{
    typedef forg::rendering::reference::SWRenderDevice super;

  public:
    SWRenderDevice(HWIN handle);
    virtual ~SWRenderDevice();

    // IRenderDevice implementation
  public:
    virtual int Present();
    virtual int Reset();

  private:
    struct Impl;
    Impl* m_impl = nullptr;
};

} // namespace forg
