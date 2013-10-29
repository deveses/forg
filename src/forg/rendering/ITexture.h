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

#ifndef _FORG_ITEXTURE_H_
#define _FORG_ITEXTURE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/RefCounter.h"
#include "rendering/ISurface.h"

namespace forg{

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
	virtual ~ITexture(void){};

public:
// Public methods
	//static Texture FromFile(
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

    static ITexture* FromFile( IRenderDevice* device, const char* srcFile );

	virtual int GetLevelDesc(uint Level, SurfaceDescription* Description) const = 0;

	ISurface* GetSurfaceLevel(uint Level);

    /// Locks a rectangle on a texture resource.
	virtual void* LockRect(uint Level, uint Flags) = 0;

    /// Unlocks a rectangle on a texture resource.
	virtual int UnlockRect(uint Level) = 0;

    /// Returns the number of texture levels in a multilevel texture.
    virtual uint GetLevelCount() = 0;

private:

};

typedef ITexture* LPTEXTURE;

}

#endif //_FORG_ITEXTURE_H_
