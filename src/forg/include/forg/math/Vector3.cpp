#include "forg_pch.h"

#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4.h"
#include "math/Math.h"

namespace forg { namespace math {

	const Vector3 Vector3::Empty(0.0f, 0.0f, 0.0f);
	const Vector3 Vector3::XAxis(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::YAxis(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::ZAxis(0.0f, 0.0f, 1.0f);

	// assignment operators
	Vector3& Vector3::operator += ( const Vector3& value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;

		return *this;
	}

	Vector3& Vector3::operator -= ( const Vector3& value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;

		return *this;
	}

	Vector3& Vector3::operator *= ( float value)
	{
		X *= value;
		Y *= value;
		Z *= value;
		return *this;
	}

	Vector3& Vector3::operator /= ( float value)
	{
		X /= value;
		Y /= value;
		Z /= value;

		return *this;
	}

    Vector3& Vector3::operator = (const Vector4& value)
	{
		X = value.X;
		Y = value.Y;
		Z = value.Z;

		return *this;
	}

	// binary operators
	Vector3 Vector3::operator + ( const Vector3& value) const
	{
        Vector3 v(*this);

        v += value;

		return v;
	}

	Vector3 Vector3::operator - ( const Vector3& value) const
	{
        Vector3 v(*this);

        v -= value;

        return v;
	}

	Vector3 Vector3::operator * ( float value) const
	{
        Vector3 v(*this);

        v *= value;

        return v;
	}

	Vector3 Vector3::operator / ( float value) const
	{
        Vector3 v(*this);

        v /= value;

        return v;
	}

	float Vector3::operator * (const Vector3& value) const
	{
		return Dot(value);
	}

	Vector3 Vector3::operator % (const Vector3& value) const
	{
		Vector3 v;

		Vector3::Cross(v, *this, value);

		return v;
	}

	bool Vector3::operator == ( const Vector3& value) const
	{
        if (X == value.X && Y == value.Y && Z == value.Z)
            return true;

		return false;
	}

	bool Vector3::operator != ( const Vector3& value) const
	{
        if (X == value.X && Y == value.Y && Z == value.Z)
            return false;

		return true;
	}

	Vector3 operator * ( float scalar, const Vector3& value)
	{
		return Vector3(value)*scalar;
	}

	void Vector3::Add(Vector3& vOut, const Vector3& vLeft, const Vector3& vRight)
	{
		vOut.X = vLeft.X + vRight.X;
		vOut.Y = vLeft.Y + vRight.Y;
		vOut.Z = vLeft.Z + vRight.Z;
	}

	void Vector3::Substract(Vector3& vOut, const Vector3& vLeft, const Vector3& vRight)
	{
		vOut.X = vLeft.X - vRight.X;
		vOut.Y = vLeft.Y - vRight.Y;
		vOut.Z = vLeft.Z - vRight.Z;
	}

	void Vector3::Scale(float fScale)
	{
		*this *= fScale;
	}

	float Vector3::Dot( const Vector3& vLeft, const Vector3& vRight )
	{
		return (vLeft.X*vRight.X + vLeft.Y*vRight.Y + vLeft.Z*vRight.Z);
	}

	float Vector3::Dot( const Vector3& v ) const
	{
		return (X*v.X + Y*v.Y + Z*v.Z);
	}

	void Vector3::Cross(Vector3& out, const Vector3& v1, const Vector3& v2)
	{
		Vector3 v;

		v.X = v1.Y * v2.Z - v1.Z * v2.Y;
		v.Y = v1.Z * v2.X - v1.X * v2.Z;
		v.Z = v1.X * v2.Y - v1.Y * v2.X;

		out = v;
	}

	void Vector3::Cross(const Vector3& v1, const Vector3& v2)
	{
		Vector3::Cross(*this, v1, v2);
	}

	float Vector3::LengthSq() const
	{
		return Dot(*this);
	}

	float Vector3::Length() const
	{
		return (float)Math::Sqrt(LengthSq());
	}

	Vector3& Vector3::Normalize()
	{
		float l = Length();

		if (l > 0.0f)
		{
			l = 1/l;

			*this *= l;
		}

		return *this;
	}

	Vector3& Vector3::Normalize(Vector3& vOut, const Vector3& vSource)
	{
		Vector3 v(vSource);
		float l = v.Length();

		if (l > 0.0f)
		{
			l = 1/l;

			v *= l;
		}

		return (vOut = v);
	}

	Vector3& Vector3::TransformCoordinate(const Matrix4& mTransformation)
	{
		return TransformCoordinate(*this, *this, mTransformation);
	}

    Vector3& Vector3::TransformNormal(const Matrix4& mTransformation)
	{
		return TransformNormal(*this, *this, mTransformation);
	}

	Vector3& Vector3::TransformCoordinate(Vector3& vOut, const Vector3& vSource, const Matrix4& mTransformation)
	{
		Vector3 v(
				vSource.X * mTransformation.M11 + vSource.Y * mTransformation.M21 + vSource.Z * mTransformation.M31 + mTransformation.M41,
				vSource.X * mTransformation.M12 + vSource.Y * mTransformation.M22 + vSource.Z * mTransformation.M32 + mTransformation.M42,
				vSource.X * mTransformation.M13 + vSource.Y * mTransformation.M23 + vSource.Z * mTransformation.M33 + mTransformation.M43
			);

		return (vOut = v);
	}

    Vector3& Vector3::TransformCoordinate(Vector3& vOut, const Vector3& vSource, const Quaternion& qRotation)
    {
        Quaternion qRot = qRotation;
        Quaternion qv(vSource.X, vSource.Y, vSource.Z, 0.0f);
        Quaternion qinv = qRotation;

        qinv.Inverse();
        qRot.Multiply(qv);
        qRot.Multiply(qinv);

        vOut = qRot.v;

        return vOut;
    }

    Vector3& Vector3::TransformNormal(Vector3& vOut, const Vector3& vSource, const Matrix4& mTransformation)
	{
		Vector3 v(
				vSource.X * mTransformation.M11 + vSource.Y * mTransformation.M21 + vSource.Z * mTransformation.M31,
				vSource.X * mTransformation.M12 + vSource.Y * mTransformation.M22 + vSource.Z * mTransformation.M32,
				vSource.X * mTransformation.M13 + vSource.Y * mTransformation.M23 + vSource.Z * mTransformation.M33
			);

		return (vOut = v);
	}

    Vector3& Vector3::CatmullRom(Vector3& vOut, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3, float s)
    {
        // Q(s) = [(-s3 + 2s2 - s)p1 + (3s3 - 5s2 + 2)p2 + (-3s3 + 4s2 + s)p3 + (s3 - s2)p4] / 2;

        float s2 = s*s;
        float s3 = s2*s;
        float h1 = -s3 + 2*s2 - s;
        float h2 = 3*s3 - 5*s2 + 2;
        float h3 = -3*s3 + 4*s2 + s;
        float h4 = s3 - s2;

        vOut.X = v0.X*h1 + v1.X*h2 + v2.X*h3 + v3.X*h4;
        vOut.Y = v0.Y*h1 + v1.Y*h2 + v2.Y*h3 + v3.Y*h4;
        vOut.Z = v0.Z*h1 + v1.Z*h2 + v2.Z*h3 + v3.Z*h4;

        return (vOut*=0.5f);
    }

}}
