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

#ifndef _FORG_RAY_H_
#define _FORG_RAY_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "math/Vector3.h"

namespace forg {

    using namespace math;

struct Ray
{
    Vector3 Origin;
    Vector3 Direction;

    bool TriangleIntersection(
        const Vector3& vert0,
        const Vector3& vert1,
        const Vector3& vert2,
        float epsilon,
        bool culling,
        float& t,
        float& u,
        float& v) const;
};

}

#endif //_FORG_RAY_H_
