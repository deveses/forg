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

#ifndef _FORG_LIGHT_H_
#define _FORG_LIGHT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/Color.h"
#include "math/Vector3.h"

namespace forg {

    using namespace forg::math;

    /** Light properties
    * Attenuation = 1/( att0 + att1 * d + att2 * d^2)
    *
    */
    struct Light
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Attributes
        ////////////////////////////////////////////////////////////////////////////////

        uint Type;
        Color Diffuse;
        Color Specular;
        Color Ambient;
        Vector3 Position;
        Vector3 Direction;

        float Range;            ///< Distance beyond which the light has no effect.
        float Falloff;          ///< Falloff factor (spotlight attenuation speed)
        float Attenuation0;     ///< Constant attenuation factor
        float Attenuation1;     ///< Linear attenuation factor
        float Attenuation2;     ///< Quadratic attenuation factor
        float Theta;            ///< Umbra angle of spotlight in radians (inner cone)
        float Phi;              ///< Penumbra angle of spotlight in radians (outer cone)

        //////////////////////////////////////////////////////////////////////////
        // Operators
        //////////////////////////////////////////////////////////////////////////

        // casting
        operator float* () { return (float*)this; };
        operator const float* () const { return (const float*)this; };
    };
}

#endif //_FORG_LIGHT_H_

