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

#ifndef _FORG_MATH_H_
#define _FORG_MATH_H_

#include "base.h"

#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4.h"
#include "math/Quaternion.h"

namespace forg { namespace math {

	class Math
	{

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

	public:
		static FORG_API const double PI;
		static FORG_API const double SQRT2_2;
        static FORG_API const double RAD2DEG;   ///< Radians to degrees multiplier (180/Pi)
        static FORG_API const double DEG2RAD;   ///< Degrees to radians multiplier (Pi/180)
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
		template <class T>
			static T Max(T a, T b)
		{
			return a > b ? a : b;
		}
		template <class T>
			static T Min(T a, T b)
		{
			return a > b ? b : a;
		}
		static FORG_API double Floor(double d);
		static FORG_API double Log10(double d);
		static FORG_API double Pow(double a, double b);
		static FORG_API double Atan2(double y, double x);


		//FORG_API static Quaternion& RotationVectors(Quaternion& out, const Vector3& source, const Vector3& target, const Vector3& axis);
		//FORG_API static Quaternion& RotationQuaternion(Quaternion& out, const Vector3& target);
		//FORG_API static Quaternion& RotationQuaternion(Quaternion& out,Vector3 v1, Vector3 v2, bool normalize);
		//FORG_API static bool LineLineIntersection(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, float eps, Vector3& pa, Vector3& pb, float& mua, float& mub);
		//FORG_API static void decompSwingTwistZ(const Quaternion& q, Vector3& swing, Vector3& twist);
		//FORG_API static float constraintEllipse(float sx, float sy, float rx, float ry);
		//
		//FORG_API static Quaternion& PlaneRotation(Quaternion& out, PlaneType plane);

		//FORG_API static Vector3 TransformCoordinate(Vector3& out, const Vector3 source, const Quaternion sourceQuaternion);

////////////////////////////////////////////////////////////////////////////////

        /// -1, 0, 1
        static int bit_sign(int _value)
        {
            // shifting preserve sign bit
            return (1 | (_value >> 31));
        }

        static int bit_min(int x, int y)
        {
            // if x < y, then -(x < y) will be all ones, so r = y + (x - y) & ~0 = y + x - y = x.
            // Otherwise, if x >= y, then -(x < y) will be all zeros, so r = y + (x - y) & 0 = y
            return (y + ((x - y) & -(x < y)));
        }

        static int bit_max(int x, int y)
        {
            return (x - ((x - y) & -(x < y)));
        }

        static int bit_avarage(int x, int y)
        {
            return (x&y)+((x^y)/2);
        }

        static bool is_pow2(int v)
        {
            return (!(v & (v - 1)) && v);
        }

        /*
        static void cond_mask_or_clear(bool cond, unsigned int mask, unsigned int& val)
        {
            val ^= (-cond ^ val) & mask;
        }*/


        static unsigned int count_bits_set(unsigned int v)
        {
            unsigned int c;
            const int S[] = {1, 2, 4, 8, 16};
            const int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};

        //     B[0] = 01010101 01010101 01010101 01010101
        //     B[1] = 00110011 00110011 00110011 00110011
        //     B[2] = 00001111 00001111 00001111 00001111
        //     B[3] = 00000000 11111111 00000000 11111111
        //     B[4] = 00000000 00000000 11111111 11111111

            c = v;
            c = ((c >> S[0]) & B[0]) + (c & B[0]);
            c = ((c >> S[1]) & B[1]) + (c & B[1]);
            c = ((c >> S[2]) & B[2]) + (c & B[2]);
            c = ((c >> S[3]) & B[3]) + (c & B[3]);
            c = ((c >> S[4]) & B[4]) + (c & B[4]);

            return c;
        }

        static unsigned int bit_log2(unsigned int v)
        {
            unsigned int r = 0;
            unsigned int shift = 0;

            shift = ( ( v & 0xFFFF0000 ) != 0 ) << 4; v >>= shift; r |= shift;
            shift = ( ( v & 0xFF00     ) != 0 ) << 3; v >>= shift; r |= shift;
            shift = ( ( v & 0xF0       ) != 0 ) << 2; v >>= shift; r |= shift;
            shift = ( ( v & 0xC        ) != 0 ) << 1; v >>= shift; r |= shift;
            shift = ( ( v & 0x2        ) != 0 ) << 0; v >>= shift; r |= shift;

            return r;
        }

        static int count_zeros_trail(unsigned int v)
        {
            unsigned int c = 32;
            const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
            const unsigned int S[] = {1, 2, 4, 8, 16};

            if (v & B[4])
            {
                v <<= S[4];
                c -= S[4];
            }
            if (v & B[3])
            {
                v <<= S[3];
                c -= S[3];
            }
            if (v & B[2])
            {
                v <<= S[2];
                c -= S[2];
            }
            if (v & B[1])
            {
                v <<= S[1];
                c -= S[1];
            }
            if (v & B[0])
            {
                v <<= S[0];
                c -= S[0];
            }
            if (v)
            {
                c--;
            }

            return c;
        }

        static int next_pow2(unsigned int v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v++;

            return v;
        }

        static int first_bit_num(unsigned int _value)
        {
            unsigned int r = 32;

            // result is in 0-31, so we need 5 pow2 numbers to write it
            // 1,2,4,8,16 - with these, we can construct any number from range 0-31

            // we zeroing from right to left by shifting

            // if there is something to zero
            if (_value & 0x0000FFFF)    // 00000000 00000000 11111111 11111111
            {
                _value <<= 16; r -= 16;
                // value = XXXXXXXX XXXXXXXX 00000000 00000000
            }
            // else there are zeros already

            if (_value & 0x00FF00FF)    // 00000000 11111111 00000000 11111111
            {
                _value <<= 8; r -= 8;
                // value = XXXXXXXX 00000000 XXXXXXXX 00000000
            }

            if (_value & 0x0F0F0F0F)    // 00001111 00001111 00001111 00001111
            {
                _value <<= 4; r -= 4;
                // value = XXXX0000 XXXX0000 XXXX0000 XXXX0000
            }

            if (_value & 0x33333333)    // 00110011 00110011 00110011 00110011
            {
                _value <<= 2; r -= 2;
                // value = XX00XX00 XX00XX00 XX00XX00 XX00XX00
            }

            if (_value & 0x55555555)    // 01010101 01010101 01010101 01010101
            {
                _value <<= 1; r -= 1;
                // value = X0X0X0X0 X0X0X0X0 X0X0X0X0 X0X0X0X0
            }

            if (_value)
                r--;


            return r;
        }

           // period 2^96-1
        unsigned int rand_xorshf96(void) 
        {          
            static unsigned int x=123456789, y=362436069, z=521288629;

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
            static unsigned int a = 9983651, b =95746118;

            b=a+b;
            a=b-a;

            return a;
        }

        // period about 2^60
        unsigned int rand_mwc60(void)
        {
            static unsigned int z = 12345, w = 65435;
   
            z=36969*(z&65535)+(z>>16);

            w=18000*(w&65535)+(w>>16);

            return ((z<<16)+w);
        }
	};

}
}

#endif // _FORG_MATH_H_
