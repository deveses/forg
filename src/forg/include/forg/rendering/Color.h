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

#ifndef FORG_RENDERING_COLOR_H
#define FORG_RENDERING_COLOR_H

#include "base.h"
#include "math/Vector4.h"

namespace forg {

template <typename ColorComponentType> struct TColor3
{
  public:
    ColorComponentType r{};
    ColorComponentType g{};
    ColorComponentType b{};

    constexpr TColor3() = default;

    constexpr TColor3(ColorComponentType red, ColorComponentType green,
                      ColorComponentType blue)
        : r(red), g(green), b(blue)
    {
    }

    constexpr bool operator<(const TColor3& _rhs) const
    {
        return (r < _rhs.r && g < _rhs.g && b < _rhs.b);
    }

    constexpr bool operator>(const TColor3& _rhs) const
    {
        return (r > _rhs.r && g > _rhs.g && b > _rhs.b);
    }

    constexpr TColor3 operator*(const ColorComponentType& _value) const
    {
        return TColor3(r * _value, g * _value, b * _value);
    }

    constexpr TColor3 operator/(const ColorComponentType& _value) const
    {
        return TColor3(r / _value, g / _value, b / _value);
    }

    constexpr TColor3 operator+(const TColor3& _rhs) const
    {
        return TColor3(r + _rhs.r, g + _rhs.g, b + _rhs.b);
    }

    constexpr void operator=(const ColorComponentType& _value)
    {
        r = g = b = _value;
    }
};

template <typename ColorComponentType> struct TColor4
{
  public:
    ColorComponentType r{};
    ColorComponentType g{};
    ColorComponentType b{};
    ColorComponentType a{};

    constexpr TColor4() = default;

    constexpr TColor4(ColorComponentType red, ColorComponentType green,
                      ColorComponentType blue, ColorComponentType alpha)
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    constexpr bool operator<(const TColor4& _rhs) const
    {
        return (r < _rhs.r && g < _rhs.g && b < _rhs.b && a < _rhs.a);
    }

    constexpr bool operator>(const TColor4& _rhs) const
    {
        return (r > _rhs.r && g > _rhs.g && b > _rhs.b && a > _rhs.a);
    }

    constexpr TColor4 operator*(const ColorComponentType& _value) const
    {
        return TColor4(r * _value, g * _value, b * _value, a * _value);
    }

    constexpr TColor4 operator/(const ColorComponentType& _value) const
    {
        return TColor4(r / _value, g / _value, b / _value, a / _value);
    }

    constexpr TColor4 operator+(const TColor4& _rhs) const
    {
        return TColor4(r + _rhs.r, g + _rhs.g, b + _rhs.b, a + _rhs.a);
    }

    constexpr void operator=(const ColorComponentType& _value)
    {
        r = g = b = a = _value;
    }
};

using Color3f = TColor3<float>;
using Color4f = TColor4<float>;

using Color3b = TColor3<byte>;
using Color4b = TColor4<byte>;

struct FORG_API Color
{
    float r;
    float g;
    float b;
    float a;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    //////////////////////////////////////////////////////////////////////////
    Color(uint argb);
    Color(const Color& c) = default;
    constexpr Color() noexcept : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}

    constexpr Color(float red, float green, float blue, float alpha) noexcept
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    constexpr Color(float red, float green, float blue) noexcept
        : r(red), g(green), b(blue), a(1.0f)
    {
    }

    constexpr Color(const math::Vector4& v) noexcept
        : r(v.X), g(v.Y), b(v.Z), a(v.W)
    {
    }

    // D3DXCOLOR( DWORD argb );
    // D3DXCOLOR( CONST FLOAT * );
    // D3DXCOLOR( CONST D3DXFLOAT16 * );
    // D3DXCOLOR( CONST D3DCOLORVALUE& );

    //// casting
    operator uint() const;

    operator math::Vector4() const { return math::Vector4(r, g, b, a); }

    Color& operator=(uint argb);
    Color& operator=(const Color& c) = default;

    constexpr void BlendTo(const Color& arg) noexcept
    {
        r = a * r + (1.0f - a) * arg.r;
        g = a * g + (1.0f - a) * arg.g;
        b = a * b + (1.0f - a) * arg.b;
        a = arg.a;
    }

    // operator FLOAT* ();
    // operator CONST FLOAT* () const;

    // operator D3DCOLORVALUE* ();
    // operator CONST D3DCOLORVALUE* () const;

    // operator D3DCOLORVALUE& ();
    // operator CONST D3DCOLORVALUE& () const;

    //// assignment operators
    // D3DXCOLOR& operator += ( CONST D3DXCOLOR& );
    // D3DXCOLOR& operator -= ( CONST D3DXCOLOR& );
    // D3DXCOLOR& operator *= ( FLOAT );
    // D3DXCOLOR& operator /= ( FLOAT );

    //// unary operators
    // D3DXCOLOR operator + () const;
    // D3DXCOLOR operator - () const;

    //// binary operators
    // D3DXCOLOR operator + ( CONST D3DXCOLOR& ) const;
    // D3DXCOLOR operator - ( CONST D3DXCOLOR& ) const;
    // D3DXCOLOR operator * ( FLOAT ) const;
    // D3DXCOLOR operator / ( FLOAT ) const;

    // friend D3DXCOLOR operator * ( FLOAT, CONST D3DXCOLOR& );

    // BOOL operator == ( CONST D3DXCOLOR& ) const;
    // BOOL operator != ( CONST D3DXCOLOR& ) const;
};

} // namespace forg

#endif // FORG_RENDERING_COLOR_H
