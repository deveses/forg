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

#ifndef _FORG_MATH_MATRIX4_H_
#define _FORG_MATH_MATRIX4_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
//#include "vector3.h"

namespace forg { namespace math {

struct Vector3;
struct Vector4;
struct Quaternion;


/// Describes a matrix.
/**
* Access by [row][col].
* In memory layout - row by row.
* Multiplication from left to right (like in DirectX).
*/
struct FORG_API Matrix4
{
    //////////////////////////////////////////////////////////////////////////
    // Constructors
    //////////////////////////////////////////////////////////////////////////

    Matrix4()
        : M11(1.0f), M12(0.0f), M13(0.0f), M14(0.0f)
        , M21(0.0f), M22(1.0f), M23(0.0f), M24(0.0f)
        , M31(0.0f), M32(0.0f), M33(1.0f), M34(0.0f)
        , M41(0.0f), M42(0.0f), M43(0.0f), M44(1.0f)
    {}

    Matrix4( const float * m);
    Matrix4( const Matrix4& m);
    Matrix4(
        float _11, float _12, float _13, float _14,
        float _21, float _22, float _23, float _24,
        float _31, float _32, float _33, float _34,
        float _41, float _42, float _43, float _44 )
        : M11(_11), M12(_12), M13(_13), M14(_14)
        , M21(_21), M22(_22), M23(_23), M24(_24)
        , M31(_31), M32(_32), M33(_33), M34(_34)
        , M41(_41), M42(_42), M43(_43), M44(_44)
    {}

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////////

    float M11;
	float M12;
	float M13;
	float M14;
	float M21;
	float M22;
	float M23;
	float M24;
	float M31;
	float M32;
	float M33;
	float M34;
	float M41;
	float M42;
	float M43;
	float M44;

	static const Matrix4 Identity;
	static const Matrix4 Zero;

    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////

    /// casting to float array
    operator float* () const { return (float*)this; }

    /// multiply by scalar
    Matrix4& operator *= (float fScalar);


	//// access grants
	//FLOAT& operator () ( UINT Row, UINT Col );
	//FLOAT  operator () ( UINT Row, UINT Col ) const;

	//// casting operators
	//operator FLOAT* ();
	//operator CONST FLOAT* () const;

	//// assignment operators
	//D3DXMATRIX& operator *= ( CONST D3DXMATRIX& );
	//D3DXMATRIX& operator += ( CONST D3DXMATRIX& );
	//D3DXMATRIX& operator -= ( CONST D3DXMATRIX& );
	//D3DXMATRIX& operator *= ( FLOAT );
	//D3DXMATRIX& operator /= ( FLOAT );

	//// unary operators
	//D3DXMATRIX operator + () const;
	//D3DXMATRIX operator - () const;

	//// binary operators
	//D3DXMATRIX operator * ( CONST D3DXMATRIX& ) const;
	//D3DXMATRIX operator + ( CONST D3DXMATRIX& ) const;
	//D3DXMATRIX operator - ( CONST D3DXMATRIX& ) const;
	//D3DXMATRIX operator * ( FLOAT ) const;
	//D3DXMATRIX operator / ( FLOAT ) const;

	//friend D3DXMATRIX operator * ( FLOAT, CONST D3DXMATRIX& );

	//BOOL operator == ( CONST D3DXMATRIX& ) const;
	//BOOL operator != ( CONST D3DXMATRIX& ) const;

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    //////////////////////////////////////////////////////////////////////////

    Matrix4& SetPosition(float _x, float _y, float _z);
    Matrix4& SetPosition(const Vector3& _pos);
    void GetPosition(Vector3& _pos);

    Matrix4& SetColumn(const Vector3& _col, int index);
    Vector3& GetColumn(Vector3& _col, int index) const;

    Matrix4& SetRow(const Vector3& _row, int index);
    Vector3& GetRow(Vector3& _row, int index) const;

	Matrix4& Transpose();
	Matrix4& Transpose(const Matrix4& source);
    Matrix4& Translate(float x, float y, float z);

    Matrix4& RotateX(float _angle);
    Matrix4& RotateY(float _angle);
    Matrix4& RotateZ(float _angle);

    /// Determines the product of two matrices.
    /* Determines the product of two matrices.
    * 64 muls, 48 adds
    */
    Matrix4& Multiply(const Matrix4& source);

    /// Scales the matrix along the x-axis, y-axis, and z-axis.
    Matrix4& Scale(float x, float y, float z);

    /// Returns the determinant of a matrix.
    float Determinant() const;

    void Decompose(Vector3* _scale, Quaternion* _rotation, Vector3* _translation) const;
    // Static

	static Matrix4& RotationAxis(Matrix4& mOut, const Vector3& vAxis, float fAngle);
    static Matrix4& RotationX(Matrix4& mOut, float fAngle);
    static Matrix4& RotationY(Matrix4& mOut, float fAngle);
    static Matrix4& RotationZ(Matrix4& mOut, float fAngle);
	static Matrix4& LookAtLH(Matrix4& mOut, const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector);
	static Matrix4& LookAtRH(Matrix4& mOut, const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector);
	static Matrix4& PerspectiveFovLH(Matrix4& mOut, float fieldOfViewY, float aspectRatio, float znearPlane, float zfarPlane);
	static Matrix4& PerspectiveFovRH(Matrix4& mOut, float fieldOfViewY, float aspectRatio, float znearPlane, float zfarPlane);
    static Matrix4& OrthoLH(Matrix4& mOut, float width, float height, float znearPlane, float zfarPlane);
    static Matrix4& OrthoRH(Matrix4& mOut, float width, float height, float znearPlane, float zfarPlane);
    static Matrix4& OrthoOffCenterLH(Matrix4& mOut, float xMin, float xMax, float yMin, float yMax, float znearPlane, float zfarPlane); 
    static Matrix4& OrthoOffCenterRH(Matrix4& mOut, float xMin, float xMax, float yMin, float yMax, float znearPlane, float zfarPlane); 
    static Matrix4& Translation(Matrix4& mOut, float x, float y, float z);
    static Matrix4& Scaling(Matrix4& mOut, float x, float y, float z);

    /// Determines the product of two matrices.
    /**
    * The result represents the transformation M1 (left) followed by the transformation M2 (right) (Out = M1 * M2).
    */
    static Matrix4& Multiply(Matrix4& mOut, const Matrix4& left, const Matrix4& right);

    static Matrix4& Inverse(Matrix4& mOut, const Matrix4& src, float* pDeterminant);

};

}}


#endif //_FORG_MATH_MATRIX4_H_

