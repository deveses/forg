// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "core/RefCounter.h"
#include "rendering/ISurface.h"

namespace forg {

class IRenderDevice;

/// ITexture interface
/**
 * ITexture
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class FORG_API ITexture : public core::RefCounter
{
    // 'structors
  public:
    virtual ~ITexture(void) {};

  public:
    // Public methods
    // static Texture FromFile(
    //	Device device,
    //	string srcFile,
    //	int width,
    //	int height,
    //	int mipLevels,
    //	Usage usage,
    //	Format format,
    //	Pool pool,
    //	Filter filter,
    //	Filter mipFilter,
    //	int colorKey,
    //	ref ImageInformation srcInformation
    //	);

    static ITexture* FromFile(IRenderDevice* device, const char* srcFile);

    virtual int GetLevelDesc(u32 Level,
                             SurfaceDescription* Description) const = 0;

    ISurface* GetSurfaceLevel(u32 Level);

    /// Locks a rectangle on a texture resource.
    virtual void* LockRect(u32 Level, u32 Flags) = 0;

    /// Unlocks a rectangle on a texture resource.
    virtual int UnlockRect(u32 Level) = 0;

    /// Returns the number of texture levels in a multilevel texture.
    virtual u32 GetLevelCount() = 0;

  private:
};

using LPTEXTURE = ITexture*;

} // namespace forg
