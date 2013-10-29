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

#ifndef _FORG_IVERTEXBUFFER_H_
#define _FORG_IVERTEXBUFFER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "core/RefCounter.h"

namespace forg
{
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
	virtual ~IVertexBuffer(){}

public:
	virtual int Lock(
		uint offsetToLock,
		uint sizeToLock,
		void ** ppbData,
		uint flags
		) = 0;

	virtual int Unlock() = 0;
};

typedef IVertexBuffer* LPVERTEXBUFFER;

}

#endif //_FORG_IVERTEXBUFFER_H_
