// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include <cstddef>
#include <type_traits>

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

    /// Floating-point value specifying the sharpness of specular highlights.
    /// The higher the value, the sharper the highlight.
    float Power;

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    // casting
    operator float*() noexcept { return &Diffuse.r; };
    operator const float*() const noexcept { return &Diffuse.r; };
};

static_assert(std::is_standard_layout_v<Material>);
static_assert(offsetof(Material, Diffuse) == 0);
static_assert(offsetof(Material, Ambient) == sizeof(Color));
static_assert(offsetof(Material, Specular) == sizeof(Color) * 2);
static_assert(offsetof(Material, Emissive) == sizeof(Color) * 3);
static_assert(offsetof(Material, Power) == sizeof(Color) * 4);
} // namespace forg
