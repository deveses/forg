#include "forg_pch.h"

#include "math/Quaternion.h"
#include "math/Math.h"

namespace forg { namespace math {

	const Quaternion Quaternion::Empty = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);

	// assignment operators
	Quaternion& Quaternion::operator += ( const Quaternion& value)
	{
        v += value.v;
        s += value.s;

		return *this;
	}

	Quaternion& Quaternion::operator -= ( const Quaternion& value)
	{
        v -= value.v;
        s -= value.s;

		return *this;
	}

    Quaternion& Quaternion::operator *= (const Quaternion& value)
    {
        Vector3 out_v;
        float out_s = Vector3::Dot(v, value.v);

        Vector3::Cross(out_v, v, value.v);
        out_v += s * value.v;
        out_v += v * value.s;

        s = out_s;
        v = out_v;

        return *this;
    }

    Quaternion& Quaternion::operator *= (float value)
    {
        v *= value;
        s *= value;

        return *this;
    }

	// binary operators
	Quaternion Quaternion::operator + ( const Quaternion& value) const
	{
        Quaternion v(*this);

        v += value;

		return v;
	}

	Quaternion Quaternion::operator - ( const Quaternion& value) const
	{
        Quaternion v(*this);

        v -= value;

        return v;
	}

	bool Quaternion::operator == ( const Quaternion& value) const
	{
        if (v == value.v && s == value.s)
            return true;

		return false;
	}

	bool Quaternion::operator != ( const Quaternion& value) const
	{
        if (s != value.s || v != value.v)
            return false;

		return false;
	}

    Quaternion& Quaternion::Multiply(float scalar)
    {
        v *= scalar;
        s *= scalar;

        return *this;
    }

    /************************************************************************/
    /* Statics                                                              */
    /************************************************************************/

	void Quaternion::Add(Quaternion& out, const Quaternion& left, const Quaternion& right)
	{
        Vector3::Add(out.v, left.v, right.v);
        out.s = left.s + right.s;
	}

	void Quaternion::Substract(Quaternion& out, const Quaternion& left, const Quaternion& right)
	{
        Vector3::Substract(out.v, left.v, right.v);
        out.s = left.s - right.s;
	}

    Quaternion& Quaternion::Multiply(Quaternion& out, const Quaternion& left, const Quaternion& right)
    {
        Quaternion q;

        q.s = left.s * right.s - Vector3::Dot(left.v, right.v);

        Vector3::Cross(q.v, left.v, right.v);
        q.v += left.s * right.v;
        q.v += left.v * right.s;

        return (out = q);
    }

    Quaternion& Quaternion::Conjugate(Quaternion& out, const Quaternion& source)
    {
        out.s = source.s;
        out.v = -out.v;

        return out;
    }

    float Quaternion::Length(const Quaternion& source)
    {
        return (float)Math::Sqrt( Quaternion::Dot(source, source) );
    }

    float Quaternion::LengthSq(const Quaternion& source)
    {
        return Quaternion::Dot(source, source);
    }

    float Quaternion::Dot(const Quaternion& left, const Quaternion& right)
    {
        return (left.s * right.s + Vector3::Dot(left.v, right.v));
    }

    Quaternion& Quaternion::Normalize(Quaternion& out, const Quaternion& source)
    {
        float l = Length(source);

        if (l >= 0)
        {
            out = source;
            out *= 1.0f/l;
        } else
        {
            out.Zero();
            out.s = 1.0f;
        }

        return out;
    }

    Quaternion& Quaternion::Inverse(Quaternion& out, const Quaternion& src)
    {
        Conjugate(out, src);
        Normalize(out, out);

        return out;
    }

    Quaternion& Quaternion::Ln(Quaternion& out, const Quaternion& source)
    {
        // Q == (cos(theta), sin(theta) * v) where |v| = 1
        // The natural logarithm of Q is, ln(Q) = (0, theta * v)

        float theta = (float)Math::Acos(source.s);
        float sn = (float)Math::Sin(theta);

        out.s = 0;
        out.v = source.v;

        out.v *= theta/sn;

        return out;
    }

    Quaternion& Quaternion::Exp(Quaternion& out, const Quaternion& source)
    {
        //Given a pure quaternion defined by:
        //  q = (0, theta * v);
        //This method calculates the exponential result.
        //  exp(Q) = (cos(theta), sin(theta) * v)

        float theta = source.v.Length();
        float sn = (float)Math::Sin(theta);

        out.s = (float)Math::Cos(theta);
        out.v = source.v;

        out.v *= sn/theta;

        return out;
    }

    Quaternion& Quaternion::RotationAxis(Quaternion& out, const Vector3& axis, float angle)
    {
        angle *= 0.5f;
        out.v = axis;
        out.v.Normalize();
        out.v *= (float)Math::Sin(angle);
        out.s = (float)Math::Cos(angle);

        return out;
    }

}}
