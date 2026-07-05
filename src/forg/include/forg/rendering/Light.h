// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
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

    u32 Type;
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
static_assert(offsetof(Light, Diffuse) >= sizeof(u32));
static_assert(offsetof(Light, Direction) > offsetof(Light, Position));
static_assert(offsetof(Light, Phi) > offsetof(Light, Theta));
} // namespace forg
