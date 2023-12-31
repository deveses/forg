#include "forg_pch.h"

#include "math/Vector4.h"
#include "math/Matrix4.h"
#include "math/Math.h"

namespace forg { namespace math {

	const Vector4 Vector4::Empty(0.0f, 0.0f, 0.0f, 0.0f);

	// assignment operators
	Vector4& Vector4::operator += ( const Vector4& value)
	{
		X += value.X;
		Y += value.Y;
		Z += value.Z;
        W += value.W;

		return *this;
	}

	Vector4& Vector4::operator -= ( const Vector4& value)
	{
		X -= value.X;
		Y -= value.Y;
		Z -= value.Z;
		W -= value.W;

		return *this;
	}

	Vector4& Vector4::operator *= ( float value)
	{
		X *= value;
		Y *= value;
		Z *= value;
		W *= value;

		return *this;
	}

	Vector4& Vector4::operator /= ( float value)
	{
		X /= value;
		Y /= value;
		Z /= value;
		W /= value;

		return *this;
	}

	// binary operators
	Vector4 Vector4::operator + ( const Vector4& value) const
	{
        Vector4 v(*this);

        v += value;

		return v;
	}

	Vector4 Vector4::operator - ( const Vector4& value) const
	{
        Vector4 v(*this);

        v -= value;

        return v;
	}

	Vector4 Vector4::operator * ( float value) const
	{
        Vector4 v(*this);

        v *= value;

        return v;
	}

	Vector4 Vector4::operator / ( float value) const
	{
        Vector4 v(*this);

        v /= value;

        return v;
	}

	float Vector4::operator * (const Vector4& value) const
	{
		return Dot(value);
	}

	bool Vector4::operator == ( const Vector4& value) const
	{
        if (X == value.X && Y == value.Y && Z == value.Z && W == value.W)
            return true;

		return false;
	}

	bool Vector4::operator != ( const Vector4& value) const
	{
        if (X == value.X && Y == value.Y && Z == value.Z && W == value.W)
            return false;

		return true;
	}

	Vector4 operator * ( float scalar, const Vector4& value)
	{
		return Vector4(value)*scalar;
	}

	void Vector4::Add(Vector4& vOut, const Vector4& vLeft, const Vector4& vRight)
	{
		vOut.X = vLeft.X + vRight.X;
		vOut.Y = vLeft.Y + vRight.Y;
		vOut.Z = vLeft.Z + vRight.Z;
		vOut.Z = vLeft.W + vRight.W;
	}

	void Vector4::Substract(Vector4& vOut, const Vector4& vLeft, const Vector4& vRight)
	{
		vOut.X = vLeft.X - vRight.X;
		vOut.Y = vLeft.Y - vRight.Y;
		vOut.Z = vLeft.Z - vRight.Z;
		vOut.W = vLeft.W - vRight.W;
	}

	void Vector4::Scale(float fScale)
	{
		*this *= fScale;
	}

	float Vector4::Dot( const Vector4& vLeft, const Vector4& vRight )
	{
		return (vLeft.X*vRight.X + vLeft.Y*vRight.Y + vLeft.Z*vRight.Z + vLeft.W*vRight.W);
	}

	float Vector4::Dot( const Vector4& v ) const
	{
		return (X*v.X + Y*v.Y + Z*v.Z + W*v.W);
	}

	float Vector4::LengthSq() const
	{
		return Dot(*this);
	}

	float Vector4::Length() const
	{
		return (float)Math::Sqrt(LengthSq());
	}

	Vector4& Vector4::Normalize()
	{
		float l = Length();

		if (l > 0.0f)
		{
			l = 1/l;

			*this *= l;
		}

		return *this;
	}

	Vector4& Vector4::Normalize(Vector4& vOut, const Vector4& vSource)
	{
		Vector4 v(vSource);
		float l = v.Length();

		if (l > 0.0f)
		{
			l = 1/l;

			v *= l;
		}

		return (vOut = v);
	}

	Vector4& Vector4::TransformCoordinate(const Matrix4& mTransformation)
	{
		return TransformCoordinate(*this, *this, mTransformation);
	}

	Vector4& Vector4::TransformCoordinate(Vector4& vOut, const Vector4& vSource, const Matrix4& mTransformation)
	{
		Vector4 v(
				vSource.X * mTransformation.M11 + vSource.Y * mTransformation.M21 + vSource.Z * mTransformation.M31 + vSource.W * mTransformation.M41,
				vSource.X * mTransformation.M12 + vSource.Y * mTransformation.M22 + vSource.Z * mTransformation.M32 + vSource.W * mTransformation.M42,
				vSource.X * mTransformation.M13 + vSource.Y * mTransformation.M23 + vSource.Z * mTransformation.M33 + vSource.W * mTransformation.M43,
				vSource.X * mTransformation.M14 + vSource.Y * mTransformation.M24 + vSource.Z * mTransformation.M34 + vSource.W * mTransformation.M44
			);

		return (vOut = v);
	}

    Vector4& Vector4::CatmullRom(Vector4& vOut, const Vector4& v0, const Vector4& v1, const Vector4& v2, const Vector4& v3, float s)
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
        vOut.W = v0.W*h1 + v1.W*h2 + v2.W*h3 + v3.W*h4;

        return (vOut*=0.5f);
    }

}}
