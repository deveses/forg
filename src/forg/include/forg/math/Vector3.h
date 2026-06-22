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

#ifndef FORG_MATH_VECTOR3_H
#define FORG_MATH_VECTOR3_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstddef>
#include <type_traits>

#include "forg/base.h"
// #include "Matrix4.h"

namespace forg::math {

struct Matrix4;
struct Vector4;
struct Quaternion;

/// Describes a vector in three-dimensional (3D) space.
struct FORG_API Vector3
{
    static const Vector3 Empty;
    static const Vector3 XAxis;
    static const Vector3 YAxis;
    static const Vector3 ZAxis;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    constexpr Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) noexcept
        : X(x), Y(y), Z(z)
    {
    }

    constexpr Vector3(const Vector3&) noexcept = default;
    constexpr Vector3& operator=(const Vector3&) noexcept = default;
    ~Vector3() = default;

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////
    float X;
    float Y;
    float Z;

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    // casting
    operator float*() noexcept { return &X; };
    operator const float*() const noexcept { return &X; };

    // assignment operators
    Vector3& operator+=(const Vector3& value);
    Vector3& operator-=(const Vector3& value);
    Vector3& operator*=(float value);
    Vector3& operator/=(float value);

    Vector3& operator=(const Vector4& value);

    // unary operators
    [[nodiscard]] constexpr Vector3 operator+() const noexcept
    {
        return Vector3(*this);
    };
    [[nodiscard]] constexpr Vector3 operator-() const noexcept
    {
        return Vector3(-X, -Y, -Z);
    };

    // binary operators
    Vector3 operator+(const Vector3& value) const;
    Vector3 operator-(const Vector3& value) const;
    Vector3 operator*(float value) const;
    Vector3 operator/(float value) const;

    float operator*(const Vector3& value) const; // Dot

    Vector3 operator%(const Vector3& value) const;

    friend Vector3 operator*(float scalar, const Vector3& value);

    bool operator==(const Vector3& value) const;
    bool operator!=(const Vector3& value) const;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////
    static void Add(Vector3& vOut, const Vector3& vLeft, const Vector3& vRight);
    static void Substract(Vector3& vOut, const Vector3& vLeft,
                          const Vector3& vRight);
    static float Dot(const Vector3& vLeft, const Vector3& vRight);
    static void Cross(Vector3& out, const Vector3& v1, const Vector3& v2);
    static Vector3& Normalize(Vector3& vOut, const Vector3& vSource);
    /// Transforms a 3D vector by a given matrix, projecting the result back
    /// into w = 1.
    static Vector3& TransformCoordinate(Vector3& vOut, const Vector3& vSource,
                                        const Matrix4& mTransformation);
    static Vector3& TransformCoordinate(Vector3& vOut, const Vector3& vSource,
                                        const Quaternion& qRotation);

    /// Transforms the 3D vector normal by the given matrix.
    static Vector3& TransformNormal(Vector3& vOut, const Vector3& vSource,
                                    const Matrix4& mTransformation);

    static Vector3& CatmullRom(Vector3& vOut, const Vector3& v0,
                               const Vector3& v1, const Vector3& v2,
                               const Vector3& v3, float s);

    void Scale(float fScale);
    float Dot(const Vector3& v) const;
    void Cross(const Vector3& v1, const Vector3& v2);
    float LengthSq() const;
    float Length() const;
    Vector3& Normalize();
    /// Transforms a 3D vector by a given matrix, projecting the result back
    /// into w = 1.
    Vector3& TransformCoordinate(const Matrix4& mTransformation);
    /// Transforms the 3D vector normal by the given matrix.
    Vector3& TransformNormal(const Matrix4& mTransformation);

    constexpr void Zero() noexcept { X = Y = Z = 0.0f; }
};

static_assert(std::is_standard_layout_v<Vector3>);
static_assert(sizeof(Vector3) == sizeof(float) * 3);
static_assert(alignof(Vector3) == alignof(float));
static_assert(offsetof(Vector3, Y) == sizeof(float));
static_assert(offsetof(Vector3, Z) == sizeof(float) * 2);

} // namespace forg::math

#endif // FORG_MATH_VECTOR3_H
