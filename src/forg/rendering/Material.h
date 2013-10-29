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

#ifndef _FORG_MATERIAL_H_
#define _FORG_MATERIAL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/Color.h"

namespace forg {

    using namespace forg::math;

    /**
    *
    */
    struct Material
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Attributes
        ////////////////////////////////////////////////////////////////////////////////
        Color Diffuse;
        Color Ambient;
        Color Specular;
        Color Emissive;

        /// Floating-point value specifying the sharpness of specular highlights. The higher the value, the sharper the highlight.
        float Power;

        //////////////////////////////////////////////////////////////////////////
        // Operators
        //////////////////////////////////////////////////////////////////////////

        // casting
        operator float* () { return (float*)this; };
        operator const float* () const { return (const float*)this; };
    };
}

#endif //_FORG_MATERIAL_H_

