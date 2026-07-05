// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "enums.h"

namespace forg {

struct SurfaceDescription
{
    u32 Width;
    u32 Height;
    u32 Format;

    // Usage Usage;
    // Pool Pool;
    // ResourceType Type;
    // MultiSampleType MultiSampleType;
    // int MultiSampleQuality;
};

class ISurface
{
    // 'structors
  public:
    virtual ~ISurface(void) {};

    // Public Methods
  public:
    const SurfaceDescription* GetDesc() const { return 0; };
    int LockRect(
        // D3DLOCKED_RECT * pLockedRect,
        // CONST RECT * pRect,
        u32 Flags);
    int UnlockRect();
};

} // namespace forg
