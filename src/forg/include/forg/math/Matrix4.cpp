#include "forg_pch.h"

#include "math/Math.h"
#include "math/Vector3.h"
#include "math/Matrix4.h"
#include "math/Quaternion.h"
#include <string.h>

namespace forg { namespace math {

	const Matrix4 Matrix4::Identity = Matrix4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	const Matrix4 Matrix4::Zero = Matrix4(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
		);


	Matrix4::Matrix4( const float * m)
	{
		memcpy(this, m, sizeof(Matrix4));
	}

	Matrix4::Matrix4( const Matrix4& m)
	{
		memcpy(this, &m, sizeof(Matrix4));
	}

    Matrix4& Matrix4::operator*=(float fScalar)
    {
        M11 *= fScalar;
        M12 *= fScalar;
        M13 *= fScalar;
        M14 *= fScalar;

        M21 *= fScalar;
        M22 *= fScalar;
        M23 *= fScalar;
        M24 *= fScalar;

        M31 *= fScalar;
        M32 *= fScalar;
        M33 *= fScalar;
        M34 *= fScalar;

        M41 *= fScalar;
        M42 *= fScalar;
        M43 *= fScalar;
        M44 *= fScalar;

        return *this;
    }

    Matrix4& Matrix4::SetPosition(float _x, float _y, float _z)
    {
        M41 = _x;
        M42 = _y;
        M43 = _z;

        return *this;
    }

    Matrix4& Matrix4::SetPosition(const Vector3& _pos)
    {
        M41 = _pos.X;
        M42 = _pos.Y;
        M43 = _pos.Z;

        return *this;
    }

    void Matrix4::GetPosition(Vector3& _pos)
    {
        _pos.X = M41;
        _pos.Y = M42;
        _pos.Z = M43;
    }

    Matrix4& Matrix4::SetColumn(const Vector3& _col, int index)
    {
        float* fl = (float*)this;

        fl[index] = _col.X;
        fl[index+4] = _col.Y;
        fl[index+8] = _col.Z;

        return *this;
    }

    Vector3& Matrix4::GetColumn(Vector3& _col, int index) const
    {
        float* fl = (float*)this;

        _col.X = fl[index];
        _col.Y = fl[index+4];
        _col.Z = fl[index+8];

        return _col;
    }

    Matrix4& Matrix4::SetRow(const Vector3& _row, int index)
    {
        float* fl = (float*)this;

        index <<= 2; // multiply by row size in floats

        fl[index] = _row.X;
        fl[index+1] = _row.Y;
        fl[index+2] = _row.Z;

        return *this;
    }

    Vector3& Matrix4::GetRow(Vector3& _row, int index) const
    {
        float* fl = (float*)this;

        index <<= 2; // multiply by row size in floats

        _row.X = fl[index];
        _row.Y = fl[index+1];
        _row.Z = fl[index+2];

        return _row;
    }

	Matrix4& Matrix4::Transpose()
	{
		return Transpose(*this);
	}

	Matrix4& Matrix4::Transpose(const Matrix4& source)
	{
		Matrix4 m(
			source.M11, source.M21, source.M31, source.M41,
			source.M12, source.M22, source.M32, source.M42,
			source.M13, source.M23, source.M33, source.M43,
			source.M14, source.M24, source.M34, source.M44
			);

		*this = m;

		return *this;
	}

    Matrix4& Matrix4::Translate(float x, float y, float z)
    {
        M41 += x;
        M42 += y;
        M43 += z;

        return *this;
    }

    Matrix4& Matrix4::Scale(float x, float y, float z)
    {
        M11 *= x;
        M22 *= y;
        M33 *= z;

        return *this;
    }

    Matrix4& Matrix4::RotateX(float _angle)
    {
        Matrix4 rot;

        Matrix4::RotationX(rot, _angle);
        Multiply(rot);

        return *this;
    }

    Matrix4& Matrix4::RotateY(float _angle)
    {
        Matrix4 rot;

        Matrix4::RotationY(rot, _angle);
        Multiply(rot);
        return *this;
    }

    Matrix4& Matrix4::RotateZ(float _angle)
    {
        Matrix4 rot;

        Matrix4::RotationZ(rot, _angle);
        Multiply(rot);
        return *this;
    }

    Matrix4& Matrix4::Multiply(const Matrix4& source)
    {
        return Matrix4::Multiply(*this, *this, source);
    }

    float Matrix4::Determinant() const
    {
       float value =
       M14 * M23 * M32 * M41 - M13 * M24 * M32 * M41 - M14 * M22 * M33 * M41 + M12 * M24 * M33 * M41 +
       M13 * M22 * M34 * M41 - M12 * M23 * M34 * M41 - M14 * M23 * M31 * M42 + M13 * M24 * M31 * M42 +
       M14 * M21 * M33 * M42 - M11 * M24 * M33 * M42 - M13 * M21 * M34 * M42 + M11 * M23 * M34 * M42 +
       M14 * M22 * M31 * M43 - M12 * M24 * M31 * M43 - M14 * M21 * M32 * M43 + M11 * M24 * M32 * M43 +
       M12 * M21 * M34 * M43 - M11 * M22 * M34 * M43 - M13 * M22 * M31 * M44 + M12 * M23 * M31 * M44 +
       M13 * M21 * M32 * M44 - M11 * M23 * M32 * M44 - M12 * M21 * M33 * M44 + M11 * M22 * M33 * M44;

       return value;
    }

    void Matrix4::Decompose(Vector3* _scale, Quaternion* _rotation, Vector3* _translation) const
    {
        if (_scale)
        {
            Vector3 c;

            GetColumn(c, 0);
            _scale->X = c.Length();
            GetColumn(c, 1);
            _scale->Y = c.Length();
            GetColumn(c, 2);
            _scale->Z = c.Length();
        }

        if (_translation)
        {
            GetRow(*_translation, 3);
        }

        // TODO: rotation
        if (_rotation)
        {
        }
    }

    /***************************************************************************
    * Statics
    ***************************************************************************/

    Matrix4& Matrix4::Translation(Matrix4& mOut, float x, float y, float z)
    {
        Matrix4 m;

        m.Translate(x, y, z);

        return (mOut=m);
    }

    Matrix4& Matrix4::Scaling(Matrix4& mOut, float x, float y, float z)
    {
        mOut = Matrix4::Identity;

        mOut.M11 *= x;
        mOut.M22 *= y;
        mOut.M33 *= z;

        return mOut;
    }

    Matrix4& Matrix4::Multiply(Matrix4& mOut, const Matrix4& left, const Matrix4& right)
    {
        Matrix4 m;

        m.M11 = (left.M11 * right.M11) + (left.M12 * right.M21) + (left.M13 * right.M31) + (left.M14 * right.M41);
        m.M12 = (left.M11 * right.M12) + (left.M12 * right.M22) + (left.M13 * right.M32) + (left.M14 * right.M42);
        m.M13 = (left.M11 * right.M13) + (left.M12 * right.M23) + (left.M13 * right.M33) + (left.M14 * right.M43);
        m.M14 = (left.M11 * right.M14) + (left.M12 * right.M24) + (left.M13 * right.M34) + (left.M14 * right.M44);

        m.M21 = (left.M21 * right.M11) + (left.M22 * right.M21) + (left.M23 * right.M31) + (left.M24 * right.M41);
        m.M22 = (left.M21 * right.M12) + (left.M22 * right.M22) + (left.M23 * right.M32) + (left.M24 * right.M42);
        m.M23 = (left.M21 * right.M13) + (left.M22 * right.M23) + (left.M23 * right.M33) + (left.M24 * right.M43);
        m.M24 = (left.M21 * right.M14) + (left.M22 * right.M24) + (left.M23 * right.M34) + (left.M24 * right.M44);

        m.M31 = (left.M31 * right.M11) + (left.M32 * right.M21) + (left.M33 * right.M31) + (left.M34 * right.M41);
        m.M32 = (left.M31 * right.M12) + (left.M32 * right.M22) + (left.M33 * right.M32) + (left.M34 * right.M42);
        m.M33 = (left.M31 * right.M13) + (left.M32 * right.M23) + (left.M33 * right.M33) + (left.M34 * right.M43);
        m.M34 = (left.M31 * right.M14) + (left.M32 * right.M24) + (left.M33 * right.M34) + (left.M34 * right.M44);

        m.M41 = (left.M41 * right.M11) + (left.M42 * right.M21) + (left.M43 * right.M31) + (left.M44 * right.M41);
        m.M42 = (left.M41 * right.M12) + (left.M42 * right.M22) + (left.M43 * right.M32) + (left.M44 * right.M42);
        m.M43 = (left.M41 * right.M13) + (left.M42 * right.M23) + (left.M43 * right.M33) + (left.M44 * right.M43);
        m.M44 = (left.M41 * right.M14) + (left.M42 * right.M24) + (left.M43 * right.M34) + (left.M44 * right.M44);

        return (mOut=m);
    }

    Matrix4& Matrix4::RotationX(Matrix4& mOut, float fAngle)
    {
        float cos = Math::Cos(fAngle);
        float sin = Math::Sin(fAngle);
        const Matrix4 rot(
		    1.0f, 0.0f, 0.0f, 0.0f,
		    0.0f,  cos,  sin, 0.0f,
		    0.0f, -sin,  cos, 0.0f,
		    0.0f, 0.0f, 0.0f, 1.0f
	    );

        return (mOut=rot);
    }

    Matrix4& Matrix4::RotationY(Matrix4& mOut, float fAngle)
    {
        float cos = Math::Cos(fAngle);
        float sin = Math::Sin(fAngle);
        const Matrix4 rot(
		     cos, 0.0f, -sin, 0.0f,
		    0.0f, 1.0f, 0.0f, 0.0f,
		     sin, 0.0f,  cos, 0.0f,
		    0.0f, 0.0f, 0.0f, 1.0f
	    );

        return (mOut=rot);
    }

    Matrix4& Matrix4::RotationZ(Matrix4& mOut, float fAngle)
    {
        float cos = Math::Cos(fAngle);
        float sin = Math::Sin(fAngle);
        const Matrix4 rot(
		     cos,  sin, 0.0f, 0.0f,
		    -sin,  cos, 0.0f, 0.0f,
		    0.0f, 0.0f, 1.0f, 0.0f,
		    0.0f, 0.0f, 0.0f, 1.0f
	    );

        return (mOut=rot);
    }

	Matrix4& Matrix4::RotationAxis(Matrix4& mOut, const Vector3& vAxis, float fAngle)
	{
		Matrix4 m;
		Vector3 v(vAxis);

		float mag = vAxis.Length();

		if( mag < 1.0E-8) {
			return (mOut = Matrix4::Identity);
		} else {
			mag = 1.0f/mag;
			v.Scale(mag);

			float sinTheta = (float)Math::Sin(fAngle);
			float cosTheta = (float)Math::Cos(fAngle);
			float t = 1.0f - cosTheta;

			float xz = v.X * v.Z;
			float xy = v.X * v.Y;
			float yz = v.Y * v.Z;

			m.M11 = t * v.X * v.X + cosTheta;
			m.M12 = t * xy - sinTheta * v.Z;
			m.M13 = t * xz + sinTheta * v.Y;

			m.M21 = t * xy + sinTheta * v.Z;
			m.M22 = t * v.Y * v.Y + cosTheta;
			m.M23 = t * yz - sinTheta * v.X;

			m.M31 = t * xz - sinTheta * v.Y;
			m.M32 = t * yz + sinTheta * v.X;
			m.M33 = t * v.Z * v.Z + cosTheta;
		}

		m.M14 = 0.0f;
		m.M24 = 0.0f;
		m.M34 = 0.0f;

		m.M41 = 0.0f;
		m.M42 = 0.0f;
		m.M43 = 0.0f;
		m.M44 = 1.0f;

		return (mOut=m);
	}

	Matrix4& Matrix4::LookAtLH(Matrix4& mOut, const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector)
	{
		Vector3 zaxis;
		Vector3 xaxis;
		Vector3 yaxis;

		Vector3::Substract(zaxis, cameraTarget, cameraPosition);
		zaxis.Normalize();

		Vector3::Cross(xaxis, cameraUpVector, zaxis);
		xaxis.Normalize();

		Vector3::Cross(yaxis, zaxis, xaxis);


		Matrix4 m(
				xaxis.X,           yaxis.X,           zaxis.X,          0.0f,
				xaxis.Y,           yaxis.Y,           zaxis.Y,          0.0f,
				xaxis.Z,           yaxis.Z,           zaxis.Z,          0.0f,
				-Vector3::Dot(xaxis, cameraPosition),  -Vector3::Dot(yaxis, cameraPosition),  -Vector3::Dot(zaxis, cameraPosition),  1.0f
			);

		return (mOut = m);
	}

	Matrix4& Matrix4::LookAtRH(Matrix4& mOut, const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUpVector)
	{
		Vector3 zaxis;
		Vector3 xaxis;
		Vector3 yaxis;

		Vector3::Substract(zaxis, cameraPosition, cameraTarget);
		zaxis.Normalize();

		Vector3::Cross(xaxis, cameraUpVector, zaxis);
		xaxis.Normalize();

		Vector3::Cross(yaxis, zaxis, xaxis);

        // note that is transposed matrix, because we use DX convention of multiplying
		Matrix4 m(
			xaxis.X,           yaxis.X,           zaxis.X,          0.0f,
			xaxis.Y,           yaxis.Y,           zaxis.Y,          0.0f,
			xaxis.Z,           yaxis.Z,           zaxis.Z,          0.0f,
			-Vector3::Dot(xaxis, cameraPosition),  -Vector3::Dot(yaxis, cameraPosition),  -Vector3::Dot(zaxis, cameraPosition),  1.0f
			);

		return (mOut = m);
	}

	// symmetrical perspective projection
	Matrix4& Matrix4::PerspectiveFovLH(Matrix4& mOut, float fieldOfViewY, float aspectRatio, float znearPlane, float zfarPlane)
	{
		//for infinite perspective: inf/inf = 1, 2*NP

		//opengl way
		//float yScale = (float)(1.0f / Math::Tan(fieldOfViewY/2.0f));
		//float xScale = yScale / aspectRatio;
		//float range = znearPlane - zfarPlane;

		//Matrix4 m(
		//	xScale, 0.0f, 0.0f, 0.0f,
		//	0.0f, yScale, 0.0f, 0.0f,
		//	0.0f, 0.0f, (znearPlane+zfarPlane)/range, -1.0f,
		//	0.0f, 0.0f, 2*(znearPlane*zfarPlane)/range, 0.0f
		//	);

		//dx way
		float yScale = (float)(1.0f / Math::Tan(fieldOfViewY/2.0f));
		float xScale = yScale / aspectRatio;
		float range = zfarPlane - znearPlane;

		Matrix4 m(
			xScale, 0.0f, 0.0f, 0.0f,
			0.0f, yScale, 0.0f, 0.0f,
			0.0f, 0.0f, (zfarPlane)/range, 1.0f,
			0.0f, 0.0f, (znearPlane*zfarPlane)/range, 0.0f
			);

		return (mOut = m);
	}

	Matrix4& Matrix4::PerspectiveFovRH(Matrix4& mOut, float fieldOfViewY, float aspectRatio, float znearPlane, float zfarPlane)
	{
        // Perspective distortion
        // _                                             _
        // | ctg(u)    0          0             0        |
        // |   0     ctg(v)       0             0        |
        // |   0       0     (B+F)/(B-F)   (-2BF)/(B-F)  |
        // |   0       0          1             0        |
        // -                                             -
        // u - angle between a line pointing out of the camera in z direction and the plane through the camera and the right-hand edge of the screen
        // v - angle between the same line and the plane through the camera and the top edge of the screen
        // F - far, B - near
        // for infinite perspective (B + F)/(B ? F) = 1 and ?2BF/(B ? F) = ?2F

        // ======================================================================
		// ogl way
        // ======================================================================
		//float yScale = (float)(1.0f / Math::Tan(fieldOfViewY/2.0f));
		//float xScale = yScale / aspectRatio;
		//float range = znearPlane - zfarPlane;

		//Matrix4 m(
		//	xScale, 0.0f, 0.0f, 0.0f,
		//	0.0f, yScale, 0.0f, 0.0f,
		//	0.0f, 0.0f, (znearPlane+zfarPlane)/range, -1.0f,
		//	0.0f, 0.0f, 2*(znearPlane*zfarPlane)/range, 0.0f
		//	);

        // ======================================================================
        // dx way
        // ======================================================================

        float yScale = (float)(1.0f / Math::Tan(fieldOfViewY/2.0f));
		float xScale = yScale / aspectRatio;
		float range = znearPlane - zfarPlane;

		Matrix4 m(
			xScale, 0.0f, 0.0f, 0.0f,
			0.0f, yScale, 0.0f, 0.0f,
			0.0f, 0.0f, (zfarPlane)/range, -1.0f,
			0.0f, 0.0f, (znearPlane*zfarPlane)/range, 0.0f
			);

        // so after transformation of [X,Y,Z]
        // X' = X * xScale + 0
        // Y' = Y * yScale + 0
        // Z' = Z * F / (N-F) + (N*F)/(N-F) = (Z + N)*F/(N-F)
        // W' = -Z

		return (mOut = m);
	}

    Matrix4& Matrix4::OrthoLH(Matrix4& mOut, float width, float height, float znearPlane, float zfarPlane)
    {
        // dx way
        //2/w  0    0           0
        //     0    2/h  0           0
        //     0    0    1/(zf-zn)   0
        //     0    0    zn/(zn-zf)  1

        float range = znearPlane - zfarPlane;

        Matrix4 m(
        2.0f/width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/height, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f/range, 0.0f,
        0.0f, 0.0f, znearPlane/range, 1.0f
        );

        return (mOut = m);
    }

    Matrix4& Matrix4::OrthoRH(Matrix4& mOut, float width, float height, float znearPlane, float zfarPlane)
    {
        float range = znearPlane - zfarPlane;

        Matrix4 m(
            2.0f/width, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f/height, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f/range, 0.0f,
            0.0f, 0.0f, znearPlane/range, 1.0f
            );

        return (mOut = m);
    }

    Matrix4& Matrix4::OrthoOffCenterLH(Matrix4& mOut, float xMin, float xMax, float yMin, float yMax, float znearPlane, float zfarPlane)
    {
        float range = znearPlane - zfarPlane;
        float w = xMax - xMin;
        float h = yMax - yMin;

        Matrix4 m(
        2.0f/w, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/h, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f/range, 0.0f,
        -(xMin+xMax)/w, -(yMin+yMax)/h, znearPlane/range, 1.0f
        );

        return (mOut = m);
    }

    Matrix4& Matrix4::OrthoOffCenterRH(Matrix4& mOut, float xMin, float xMax, float yMin, float yMax, float znearPlane, float zfarPlane)
    {
        float range = znearPlane - zfarPlane;
        float w = xMax - xMin;
        float h = yMax - yMin;

        Matrix4 m(
        2.0f/w, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/h, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f/range, 0.0f,
        -(xMin+xMax)/w, -(yMin+yMax)/h, znearPlane/range, 1.0f
        );

        return (mOut = m);
    }

    Matrix4& Matrix4::Inverse(Matrix4& mOut, const Matrix4& src, float* pDeterminant)
    {
        float det = src.Determinant();

        if (det != 0)
        {
            mOut.M11 = src.M23*src.M34*src.M42 - src.M24*src.M33*src.M42 + src.M24*src.M32*src.M43
                     - src.M22*src.M34*src.M43 - src.M23*src.M32*src.M44 + src.M22*src.M33*src.M44;
            mOut.M12 = src.M14*src.M33*src.M42 - src.M13*src.M34*src.M42 - src.M14*src.M32*src.M43
                     + src.M12*src.M34*src.M43 + src.M13*src.M32*src.M44 - src.M12*src.M33*src.M44;
            mOut.M13 = src.M13*src.M24*src.M42 - src.M14*src.M23*src.M42 + src.M14*src.M22*src.M43
                     - src.M12*src.M24*src.M43 - src.M13*src.M22*src.M44 + src.M12*src.M23*src.M44;
            mOut.M14 = src.M14*src.M23*src.M32 - src.M13*src.M24*src.M32 - src.M14*src.M22*src.M33
                     + src.M12*src.M24*src.M33 + src.M13*src.M22*src.M34 - src.M12*src.M23*src.M34;
            mOut.M21 = src.M24*src.M33*src.M41 - src.M23*src.M34*src.M41 - src.M24*src.M31*src.M43
                     + src.M21*src.M34*src.M43 + src.M23*src.M31*src.M44 - src.M21*src.M33*src.M44;
            mOut.M22 = src.M13*src.M34*src.M41 - src.M14*src.M33*src.M41 + src.M14*src.M31*src.M43
                     - src.M11*src.M34*src.M43 - src.M13*src.M31*src.M44 + src.M11*src.M33*src.M44;
            mOut.M23 = src.M14*src.M23*src.M41 - src.M13*src.M24*src.M41 - src.M14*src.M21*src.M43
                     + src.M11*src.M24*src.M43 + src.M13*src.M21*src.M44 - src.M11*src.M23*src.M44;
            mOut.M24 = src.M13*src.M24*src.M31 - src.M14*src.M23*src.M31 + src.M14*src.M21*src.M33
                     - src.M11*src.M24*src.M33 - src.M13*src.M21*src.M34 + src.M11*src.M23*src.M34;
            mOut.M31 = src.M22*src.M34*src.M41 - src.M24*src.M32*src.M41 + src.M24*src.M31*src.M42
                     - src.M21*src.M34*src.M42 - src.M22*src.M31*src.M44 + src.M21*src.M32*src.M44;
            mOut.M32 = src.M14*src.M32*src.M41 - src.M12*src.M34*src.M41 - src.M14*src.M31*src.M42
                     + src.M11*src.M34*src.M42 + src.M12*src.M31*src.M44 - src.M11*src.M32*src.M44;
            mOut.M33 = src.M12*src.M24*src.M41 - src.M14*src.M22*src.M41 + src.M14*src.M21*src.M42
                     - src.M11*src.M24*src.M42 - src.M12*src.M21*src.M44 + src.M11*src.M22*src.M44;
            mOut.M34 = src.M14*src.M22*src.M31 - src.M12*src.M24*src.M31 - src.M14*src.M21*src.M32
                     + src.M11*src.M24*src.M32 + src.M12*src.M21*src.M34 - src.M11*src.M22*src.M34;
            mOut.M41 = src.M23*src.M32*src.M41 - src.M22*src.M33*src.M41 - src.M23*src.M31*src.M42
                     + src.M21*src.M33*src.M42 + src.M22*src.M31*src.M43 - src.M21*src.M32*src.M43;
            mOut.M42 = src.M12*src.M33*src.M41 - src.M13*src.M32*src.M41 + src.M13*src.M31*src.M42
                     - src.M11*src.M33*src.M42 - src.M12*src.M31*src.M43 + src.M11*src.M32*src.M43;
            mOut.M43 = src.M13*src.M22*src.M41 - src.M12*src.M23*src.M41 - src.M13*src.M21*src.M42
                     + src.M11*src.M23*src.M42 + src.M12*src.M21*src.M43 - src.M11*src.M22*src.M43;
            mOut.M44 = src.M12*src.M23*src.M31 - src.M13*src.M22*src.M31 + src.M13*src.M21*src.M32
                     - src.M11*src.M23*src.M32 - src.M12*src.M21*src.M33 + src.M11*src.M22*src.M33;
        }

        if (pDeterminant)
            *pDeterminant = det;

        return mOut;
    }


}}
