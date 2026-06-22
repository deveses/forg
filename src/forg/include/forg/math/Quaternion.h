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

#ifndef FORG_MATH_QUATERNION_H
#define FORG_MATH_QUATERNION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstddef>
#include <type_traits>

#include "forg/base.h"
#include "forg/math/Vector3.h"

namespace forg::math {

struct FORG_API Quaternion
{
    static const Quaternion Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    constexpr Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f,
                         float w = 0.0f) noexcept
        : v(x, y, z), s(w)
    {
    }

    constexpr Quaternion(const Quaternion&) noexcept = default;
    constexpr Quaternion& operator=(const Quaternion&) noexcept = default;
    ~Quaternion() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////
    Vector3 v;
    float s;

    ////////////////////////////////////////////////////////////////////////////////
    // Properties
    ////////////////////////////////////////////////////////////////////////////////
    constexpr float& X() noexcept { return v.X; }
    constexpr float& Y() noexcept { return v.Y; }
    constexpr float& Z() noexcept { return v.Z; }
    constexpr float& W() noexcept { return s; }
    [[nodiscard]] constexpr const float& X() const noexcept { return v.X; }
    [[nodiscard]] constexpr const float& Y() const noexcept { return v.Y; }
    [[nodiscard]] constexpr const float& Z() const noexcept { return v.Z; }
    [[nodiscard]] constexpr const float& W() const noexcept { return s; }

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    // casting
    operator float*() noexcept { return &v.X; };
    operator const float*() const noexcept { return &v.X; };

    // assignment operators
    Quaternion& operator+=(const Quaternion& value);
    Quaternion& operator-=(const Quaternion& value);
    Quaternion& operator*=(const Quaternion& value);
    Quaternion& operator*=(float value);

    // unary operators

    // binary operators
    Quaternion operator+(const Quaternion& value) const;
    Quaternion operator-(const Quaternion& value) const;

    bool operator==(const Quaternion& value) const;
    bool operator!=(const Quaternion& value) const;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////

    constexpr void Zero() noexcept { v.X = v.Y = v.Z = s = 0.0f; }

    Quaternion& Multiply(Quaternion& q)
    {
        return Quaternion::Multiply(*this, *this, q);
    };

    Quaternion& Inverse() { return Quaternion::Inverse(*this, *this); }

    // Statics

    static void Add(Quaternion& out, const Quaternion& left,
                    const Quaternion& right);

    static void Substract(Quaternion& out, const Quaternion& left,
                          const Quaternion& right);

    Quaternion& Multiply(float scalar);

    static Quaternion& Multiply(Quaternion& out, const Quaternion& left,
                                const Quaternion& right);

    static Quaternion& Conjugate(Quaternion& out, const Quaternion& source);

    static float Length(const Quaternion& source);

    static float LengthSq(const Quaternion& source);

    static float Dot(const Quaternion& left, const Quaternion& right);

    static Quaternion& Normalize(Quaternion& out, const Quaternion& source);

    /// Conjugates and renormalizes a quaternion.
    static Quaternion& Inverse(Quaternion& out, const Quaternion& src);

    /// Calculates the natural logarithm.
    static Quaternion& Ln(Quaternion& out, const Quaternion& source);

    /// Calculates the exponential.
    static Quaternion& Exp(Quaternion& out, const Quaternion& source);

    static Quaternion& RotationAxis(Quaternion& out, const Vector3& axis,
                                    float angle);
};

static_assert(std::is_standard_layout_v<Quaternion>);
static_assert(sizeof(Quaternion) == sizeof(float) * 4);
static_assert(alignof(Quaternion) == alignof(float));
static_assert(offsetof(Quaternion, s) == sizeof(float) * 3);

inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
    Quaternion r;
    Quaternion::Multiply(r, lhs, rhs);
    return r;
}

} // namespace forg::math

#endif // FORG_MATH_QUATERNION_H
