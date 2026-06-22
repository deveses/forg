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

#ifndef FORG_MATH_VECTOR4_H
#define FORG_MATH_VECTOR4_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstddef>
#include <type_traits>

#include "forg/base.h"
// #include "Matrix4.h"

namespace forg::math {

struct Matrix4;

/// Describes a vector in three-dimensional (3D) space.
struct FORG_API Vector4
{
    static const Vector4 Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    explicit constexpr Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f,
                               float w = 0.0f) noexcept
        : X(x), Y(y), Z(z), W(w)
    {
    }

    constexpr Vector4(const Vector4&) noexcept = default;
    constexpr Vector4& operator=(const Vector4&) noexcept = default;
    ~Vector4() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////
    float X;
    float Y;
    float Z;
    float W;

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    // casting
    operator float*() noexcept { return &X; };
    operator const float*() const noexcept { return &X; };

    // assignment operators
    Vector4& operator+=(const Vector4& value);
    Vector4& operator-=(const Vector4& value);
    Vector4& operator*=(float value);
    Vector4& operator/=(float value);

    // unary operators
    [[nodiscard]] constexpr Vector4 operator+() const noexcept
    {
        return Vector4(*this);
    };
    [[nodiscard]] constexpr Vector4 operator-() const noexcept
    {
        return Vector4(-X, -Y, -Z, -W);
    };

    // binary operators
    Vector4 operator+(const Vector4& value) const;
    Vector4 operator-(const Vector4& value) const;
    Vector4 operator*(float value) const;
    Vector4 operator/(float value) const;

    float operator*(const Vector4& value) const; // Dot

    friend Vector4 operator*(float scalar, const Vector4& value);

    bool operator==(const Vector4& value) const;
    bool operator!=(const Vector4& value) const;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////
    static void Add(Vector4& vOut, const Vector4& vLeft, const Vector4& vRight);
    static void Substract(Vector4& vOut, const Vector4& vLeft,
                          const Vector4& vRight);
    static float Dot(const Vector4& vLeft, const Vector4& vRight);
    static Vector4& Normalize(Vector4& vOut, const Vector4& vSource);
    /// Transforms a 3D vector by a given matrix, projecting the result back
    /// into w = 1.
    static Vector4& TransformCoordinate(Vector4& vOut, const Vector4& vSource,
                                        const Matrix4& mTransformation);

    static Vector4& CatmullRom(Vector4& vOut, const Vector4& v0,
                               const Vector4& v1, const Vector4& v2,
                               const Vector4& v3, float s);

    void Scale(float fScale);
    float Dot(const Vector4& v) const;
    float LengthSq() const;
    float Length() const;
    Vector4& Normalize();
    /// Transforms a 3D vector by a given matrix, projecting the result back
    /// into w = 1.
    Vector4& TransformCoordinate(const Matrix4& mTransformation);

    constexpr void Zero() noexcept { X = Y = Z = W = 0.0f; }
};

static_assert(std::is_standard_layout_v<Vector4>);
static_assert(sizeof(Vector4) == sizeof(float) * 4);
static_assert(alignof(Vector4) == alignof(float));
static_assert(offsetof(Vector4, Y) == sizeof(float));
static_assert(offsetof(Vector4, Z) == sizeof(float) * 2);
static_assert(offsetof(Vector4, W) == sizeof(float) * 3);

} // namespace forg::math

#endif // FORG_MATH_VECTOR4_H
