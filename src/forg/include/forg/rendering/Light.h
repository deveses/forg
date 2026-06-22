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

#ifndef FORG_RENDERING_LIGHT_H
#define FORG_RENDERING_LIGHT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstddef>
#include <type_traits>

#include "base.h"
#include "math/Vector3.h"
#include "rendering/Color.h"

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

    float Range;        ///< Distance beyond which the light has no effect.
    float Falloff;      ///< Falloff factor (spotlight attenuation speed)
    float Attenuation0; ///< Constant attenuation factor
    float Attenuation1; ///< Linear attenuation factor
    float Attenuation2; ///< Quadratic attenuation factor
    float Theta;        ///< Umbra angle of spotlight in radians (inner cone)
    float Phi;          ///< Penumbra angle of spotlight in radians (outer cone)

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    // casting
    operator float*() noexcept { return reinterpret_cast<float*>(this); };
    operator const float*() const noexcept
    {
        return reinterpret_cast<const float*>(this);
    };
};

static_assert(std::is_standard_layout_v<Light>);
static_assert(offsetof(Light, Type) == 0);
static_assert(offsetof(Light, Diffuse) >= sizeof(uint));
static_assert(offsetof(Light, Direction) > offsetof(Light, Position));
static_assert(offsetof(Light, Phi) > offsetof(Light, Theta));
} // namespace forg

#endif // FORG_RENDERING_LIGHT_H
