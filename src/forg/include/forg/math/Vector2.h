// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
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
