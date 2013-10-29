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

#ifndef _FORG_MATH_VECTOR4_H_
#define _FORG_MATH_VECTOR4_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
//#include "Matrix4.h"

namespace forg { namespace math {

struct Matrix4;

/// Describes a vector in three-dimensional (3D) space.
struct FORG_API Vector4
{
    static const Vector4 Empty;

    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////
    explicit Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        : X(x), Y(y), Z(z), W(w)
    {}

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
    operator float* () { return (float*)this; };
    operator const float* () const { return (const float*)this; };

	// assignment operators
	Vector4& operator += ( const Vector4& value);
	Vector4& operator -= ( const Vector4& value);
	Vector4& operator *= ( float value);
	Vector4& operator /= ( float value);

	// unary operators
    Vector4 operator + () const { return Vector4(*this); };
    Vector4 operator - () const { return Vector4(-X, -Y, -Z); };

	// binary operators
	Vector4 operator + ( const Vector4& value) const;
	Vector4 operator - ( const Vector4& value) const;
	Vector4 operator * ( float value) const;
	Vector4 operator / ( float value) const;

	float operator * (const Vector4& value) const;	//Dot

	friend Vector4 operator * ( float scalar, const Vector4& value);

	bool operator == ( const Vector4& value) const;
	bool operator != ( const Vector4& value) const;

    ////////////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////////////////////////
	static void Add(Vector4& vOut, const Vector4& vLeft, const Vector4& vRight);
	static void Substract(Vector4& vOut, const Vector4& vLeft, const Vector4& vRight);
	static float Dot( const Vector4& vLeft, const Vector4& vRight );
	static Vector4& Normalize(Vector4& vOut, const Vector4& vSource);
    /// Transforms a 3D vector by a given matrix, projecting the result back into w = 1.
	static Vector4& TransformCoordinate(Vector4& vOut, const Vector4& vSource, const Matrix4& mTransformation);

    static Vector4& CatmullRom(Vector4& vOut, const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3, float s);

	void Scale(float fScale);
    float Dot( const Vector4& v ) const;
    float LengthSq() const;
    float Length() const;
    Vector4& Normalize();
    /// Transforms a 3D vector by a given matrix, projecting the result back into w = 1.
	Vector4& TransformCoordinate(const Matrix4& mTransformation);

    void Zero() { X = Y = Z = W = 0.0f; }
};


}}

#endif // _FORG_MATH_VECTOR4_H_

