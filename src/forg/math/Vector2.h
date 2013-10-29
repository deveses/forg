/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

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

#ifndef _FORG_MATH_VECTOR2_H_
#define _FORG_MATH_VECTOR2_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

namespace forg { namespace math {

struct FORG_API Vector2
{
    static const Vector2 Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    Vector2(float x = 0.0f, float y = 0.0f)
        : X(x), Y(y)
    {}

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////

	float X;
	float Y;
};

}}

#endif // _FORG_MATH_VECTOR2_H_
