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

#ifndef _FORG_MATH_QUATERNION_H_
#define _FORG_MATH_QUATERNION_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "math/Vector3.h"

namespace forg { namespace math {

struct FORG_API Quaternion
{
    static const Quaternion Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        : v(x, y, z), s(w)
    {}

    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////
    Vector3 v;
    float s;

    ////////////////////////////////////////////////////////////////////////////////
    // Properties
    ////////////////////////////////////////////////////////////////////////////////
    float& X() { return v.X; }
    float& Y() { return v.Y; }
    float& Z() { return v.Z; }
    float& W() { return s; }

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

	// casting
    operator float* () { return (float*)this; };
    operator const float* () const { return (const float*)this; };

	// assignment operators
	Quaternion& operator += ( const Quaternion& value);
	Quaternion& operator -= ( const Quaternion& value);
    Quaternion& operator *= ( const Quaternion& value);
    Quaternion& operator *= ( float value);

	// unary operators

	// binary operators
	Quaternion operator + ( const Quaternion& value) const;
	Quaternion operator - ( const Quaternion& value) const;

	bool operator == ( const Quaternion& value) const;
	bool operator != ( const Quaternion& value) const;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////

    void Zero() { v.X = v.Y = v.Z = s = 0.0f; }

    Quaternion& Multiply(Quaternion& q) { return Quaternion::Multiply(*this, *this, q); };

    Quaternion& Inverse() { return Quaternion::Inverse(*this, *this); }

    // Statics

	static void Add(Quaternion& out, const Quaternion& left, const Quaternion& right);

	static void Substract(Quaternion& out, const Quaternion& left, const Quaternion& right);

    Quaternion& Multiply(float scalar);

    static Quaternion& Multiply(Quaternion& out, const Quaternion& left, const Quaternion& right);

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

    static Quaternion& RotationAxis(Quaternion& out, const Vector3& axis, float angle);
};

inline Quaternion operator *(const Quaternion& lhs, const Quaternion& rhs)
{
    Quaternion r;
    Quaternion::Multiply(r, lhs, rhs);
    return r;
}

}}

#endif // _FORG_MATH_QUATERNION_H_

