/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2012  Slawomir Strumecki

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

#ifndef _SW_RENDER_DEVICE_H_
#define _SW_RENDER_DEVICE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/reference/SWRenderDevice.h"
#include "rendering/VertexDeclaration.h"
#include "math/Vector4.h"
#include "math/Vector2.h"

namespace forg {

class SWRenderDevice
	: public forg::rendering::reference::SWRenderDevice
{
    typedef forg::rendering::reference::SWRenderDevice super;

public:
    SWRenderDevice(HWIN handle);
	virtual ~SWRenderDevice();

    // IRenderDevice implementation
public:
    virtual int Present();
    virtual int Reset();
};

}

#endif
