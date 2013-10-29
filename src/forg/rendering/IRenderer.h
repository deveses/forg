/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005-2008  Slawomir Strumecki

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

#ifndef _FORG_IRENDERER_H_
#define _FORG_IRENDERER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/IRenderDevice.h"

namespace forg {

struct RENDER_PARAMETERS
{
	uint BackBufferWidth;
	uint BackBufferHeight;
	uint PresentationInterval;
};

class IRenderer
{
public:

	virtual IRenderDevice* CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters) = 0;

	virtual LPCTSTR get_Name() = 0;
};

typedef IRenderer* LPRENDERER;

typedef IRenderer* (*PFCREATERENDERER)(void);

}

#endif //_FORG_IRENDERER_H_

