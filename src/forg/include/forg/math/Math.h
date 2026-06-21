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

#ifndef FORG_MATH_MATH_H
#define FORG_MATH_MATH_H

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>

#include "forg/base.h"

#include "forg/math/Matrix4.h"
#include "forg/math/Quaternion.h"
#include "forg/math/Vector2.h"
#include "forg/math/Vector3.h"
#include "forg/math/Vector4.h"

namespace forg::math {

class Math
{

    ////////////////////////////////////////////////////////////////////////////////
    // Constants
    ////////////////////////////////////////////////////////////////////////////////

  public:
    static FORG_API const double PI;
    static FORG_API const double SQRT2_2;
    static FORG_API const double
        RAD2DEG; ///< Radians to degrees multiplier (180/Pi)
    static FORG_API const double
        DEG2RAD; ///< Degrees to radians multiplier (Pi/180)
    static FORG_API const float FloatMinValue;
    static FORG_API const float FloatMaxValue;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////
  public:
    static FORG_API double Acos(double d);
    static FORG_API bool IsNaN(double d);
    static FORG_API double Sqrt(double d);
    static FORG_API double Sin(double d);
    static FORG_API double Cos(double d);
    static FORG_API double Tan(double d);
    static FORG_API double Atan(double d);
    static FORG_API double Abs(double d);
    static FORG_API double Log(double d);
    template <class T> static constexpr T Max(T a, T b)
    {
        return a > b ? a : b;
    }
    template <class T> static constexpr T Min(T a, T b)
    {
        return a > b ? b : a;
    }
    static FORG_API double Floor(double d);
    static FORG_API double Log10(double d);
    static FORG_API double Pow(double a, double b);
    static FORG_API double Atan2(double y, double x);

    // FORG_API static Quaternion& RotationVectors(Quaternion& out, const
    // Vector3& source, const Vector3& target, const Vector3& axis); FORG_API
    // static Quaternion& RotationQuaternion(Quaternion& out, const Vector3&
    // target); FORG_API static Quaternion& RotationQuaternion(Quaternion&
    // out,Vector3 v1, Vector3 v2, bool normalize); FORG_API static bool
    // LineLineIntersection(const Vector3& p1, const Vector3& p2, const Vector3&
    // p3, const Vector3& p4, float eps, Vector3& pa, Vector3& pb, float& mua,
    // float& mub); FORG_API static void decompSwingTwistZ(const Quaternion& q,
    // Vector3& swing, Vector3& twist); FORG_API static float
    // constraintEllipse(float sx, float sy, float rx, float ry);
    //
    // FORG_API static Quaternion& PlaneRotation(Quaternion& out, PlaneType
    // plane);

    // FORG_API static Vector3 TransformCoordinate(Vector3& out, const Vector3
    // source, const Quaternion sourceQuaternion);

    ////////////////////////////////////////////////////////////////////////////////

    /// -1, 0, 1
    static constexpr int bit_sign(int value) noexcept
    {
        return value < 0 ? -1 : 1;
    }

    static constexpr int bit_min(int x, int y) noexcept
    {
        return std::min(x, y);
    }

    static constexpr int bit_max(int x, int y) noexcept
    {
        return std::max(x, y);
    }

    static constexpr int bit_avarage(int x, int y) noexcept
    {
        return static_cast<int>((static_cast<std::int64_t>(x) + y) / 2);
    }

    static constexpr bool is_pow2(int v) noexcept
    {
        return v > 0 && std::has_single_bit(static_cast<unsigned int>(v));
    }

    /*
    static void cond_mask_or_clear(bool cond, unsigned int mask, unsigned int&
    val)
    {
        val ^= (-cond ^ val) & mask;
    }*/

    static constexpr unsigned int count_bits_set(unsigned int v) noexcept
    {
        return std::popcount(v);
    }

    static constexpr unsigned int bit_log2(unsigned int v) noexcept
    {
        return v == 0 ? 0U : std::bit_width(v) - 1U;
    }

    static constexpr int count_zeros_trail(unsigned int v) noexcept
    {
        return static_cast<int>(std::countr_zero(v));
    }

    static constexpr int next_pow2(unsigned int v) noexcept
    {
        if (v == 0 || v > (1U << 31U))
            return 0;
        return static_cast<int>(std::bit_ceil(v));
    }

    static constexpr int first_bit_num(unsigned int value) noexcept
    {
        return static_cast<int>(std::countr_zero(value));
    }

    // period 2^96-1
    unsigned int rand_xorshf96(void)
    {
        static unsigned int x = 123456789, y = 362436069, z = 521288629;

        unsigned int t;

        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;

        return z;
    }

    // period 3*2^31
    unsigned int rand_fib32(void)
    {
        static unsigned int a = 9983651, b = 95746118;

        b = a + b;
        a = b - a;

        return a;
    }

    // period about 2^60
    unsigned int rand_mwc60(void)
    {
        static unsigned int z = 12345, w = 65435;

        z = 36969 * (z & 65535) + (z >> 16);

        w = 18000 * (w & 65535) + (w >> 16);

        return ((z << 16) + w);
    }
};

} // namespace forg::math

#endif // FORG_MATH_MATH_H
