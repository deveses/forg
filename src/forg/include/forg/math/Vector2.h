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

#ifndef FORG_MATH_VECTOR2_H
#define FORG_MATH_VECTOR2_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <type_traits>

#include "forg/base.h"

namespace forg::math {

struct FORG_API Vector2
{
    static const Vector2 Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    constexpr Vector2(float x = 0.0f, float y = 0.0f) noexcept : X(x), Y(y) {}

    constexpr Vector2(const Vector2&) noexcept = default;
    constexpr Vector2& operator=(const Vector2&) noexcept = default;
    ~Vector2() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////

    float X;
    float Y;
};

static_assert(std::is_standard_layout_v<Vector2>);
static_assert(sizeof(Vector2) == sizeof(float) * 2);
static_assert(alignof(Vector2) == alignof(float));

} // namespace forg::math

#endif // FORG_MATH_VECTOR2_H
