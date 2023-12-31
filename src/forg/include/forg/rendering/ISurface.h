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

#ifndef _FORG_ISURFACE_H_
#define _FORG_ISURFACE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "enums.h"

namespace forg{

	struct SurfaceDescription
	{
		uint Width;
		uint Height;
        uint Format;

		//Usage Usage;
		//Pool Pool;
		//ResourceType Type;
		//MultiSampleType MultiSampleType;
		//int MultiSampleQuality;
	};


	class ISurface
	{
	// 'structors
	public:
		virtual ~ISurface(void){};

	// Public Methods
	public:
		const SurfaceDescription* GetDesc() const {return 0;};
		int LockRect(
			//D3DLOCKED_RECT * pLockedRect,
			//CONST RECT * pRect,
			uint Flags
			);
		int UnlockRect();

	};

}

#endif //_FORG_ISURFACE_H_
