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

#ifndef _FORG_IVERTEXDECLARATION_H_
#define _FORG_IVERTEXDECLARATION_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/VertexElement.h"

namespace forg{

/// VertexDeclaration class
/**
* VertexDeclaration
* @author eses
* @version 1.0
* @date 07-2005
* @todo
* @bug
* @warning
*/
class FORG_API VertexDeclaration
{
public:
	VertexDeclaration(const VertexElement* pDecl);
	virtual ~VertexDeclaration(void);

private:
	VertexElement elements[256];
	uint m_nElementsCount;
	uint m_nVertexSize;

public:
	const VertexElement* GetDeclaration() const;
	uint GetElementsCount() const;
	uint GetVertexSize() const;
};

typedef VertexDeclaration* LPVERTEXDECLARATION;

}

#endif //_FORG_IVERTEXDECLARATION_H_
