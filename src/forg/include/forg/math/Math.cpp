#include "forg_pch.h"

#include "math/Math.h"
#include <math.h>
#include <float.h>

namespace forg { namespace math {


	const double Math::PI = 3.1415926535897932384626433832795;
	const double Math::SQRT2_2 = 0.70710678118654752440084436210485;
    const double Math::RAD2DEG = 57.295779513082320876798154814105;
    const double Math::DEG2RAD = 0.017453292519943295769236907684886;
	const float Math::FloatMinValue = FLT_MIN;
	const float Math::FloatMaxValue = FLT_MAX;

	double Math::Sin(double d)
	{
		return sin(d);
	}

	double Math::Cos(double d)
	{
		return cos(d);
	}

	double Math::Tan(double d)
	{
		return tan(d);
	}

	double Math::Atan(double d)
	{
		return atan(d);
	}

	double Math::Atan2(double y, double x)
	{
		return atan2(y, x);
	}

	double Math::Acos(double d)
	{
		return acos(d);
	}

	bool Math::IsNaN(double d)
	{
#ifdef WIN32
		return _isnan(d)!=0;
#else
        return isnan(d)!=0;
#endif
	}

	double Math::Sqrt(double d)
	{
		return sqrt(d);
	}

	double Math::Abs(double d)
	{
		return (d > 0.0 ? d : -d);
	}

	double Math::Log(double d)
	{
		return log(d);
	}

	double Math::Floor(double d)
	{
		return floor(d);
	}

	double Math::Log10(double d)
	{
		return log10(d);
	}

	double Math::Pow(double a, double b)
	{
		return pow(a, b);
	}

	//Quaternion& Math::RotationVectors(
	//					Quaternion& out,
	//					const Vector3& source,
	//					const Vector3& target,
	//					const Vector3& axis)
	//{
	//	Vector3 Source, Target, Cross;
	//	Vector3::Normalize(Target, target);
	//	Vector3::Normalize(Source, source);
	//	Vector2 x;
	//
	//	//Quaternion::RotationAxis(out, axis, (float)PI);
	//
	//	return (Source == -Target) ?
	//		Quaternion::RotationAxis(out, axis, (float)PI) :
	//	Quaternion::RotationAxis(out, Vector3::Cross(Cross, Source, Target),
	//		(float) acos(Vector3::Dot(Source, Target)));


	//}


	//Quaternion& Math::RotationQuaternion(Quaternion& out, const Vector3& target)
	//{

	//
	//	if (target == Vector3(0,0,0))
	//			return Quaternion::Identity;
	//
	//		Vector3 Target;
	//		Vector3::Normalize(Target, target);
	//		if (Target == -Axis::Z)
	//			return Quaternion::RotationAxis(out, Axis::Y, (float) PI);
	//
	//		Vector3 axis(Target.x,  Target.y,  0);
	//		Vector3::Normalize(axis, axis);
	//

	//
	//
	//	if (axis != Vector3(0,0,0))
	//	{
	//		Quaternion q1;
	//		Quaternion::RotationAxis(q1, Axis::Y ,(float) acos(Vector3::Dot(Axis::Z, Target)));
	//		float angle = Vector3::Dot(Axis::X,  axis);
	//		if (angle < -1.0f)
	//			angle = -1.0f;
	//		if (angle > 1.0f)
	//			angle = 1.0f;
	//		float Acos = (float)acos(angle);
	//		Quaternion q2;
	//		Quaternion::RotationAxis(q2, axis.y >= 0 ? Axis::Z : (-Axis::Z), Acos);

	//		out = q1 * q2;

	//		return out;
	//	}
	//	else
	//		return
	//		Quaternion::RotationAxis(out, Axis::Y, (float) acos(Vector3::Dot(Axis::Z, Target)));
	//
	//}

	//Quaternion& Math::RotationQuaternion(Quaternion& out,Vector3 v1, Vector3 v2, bool normalize)
	//{
	//	if (normalize)
	//	{
	//		v1 = Vector3(v1.x, v1.y, v1.z);
	//		v1.Normalize();
	//		v2 = Vector3(v2.x, v2.y, v2.z);
	//		v2.Normalize();
	//	}

	//	float dot = Vector3::Dot(v1, v2);
	//	Vector3 vRotAxis;

	//	if (dot >= 1.0f - Math::FloatMinValue)
	//		return out = Quaternion::Identity;
	//	if (dot <= -1.0f + Math::FloatMinValue)
	//		return out =  Quaternion(0.0f, 1.0f, 0.0f, 0.0f);
	//	Vector3::Cross(vRotAxis, v1, v2);
	//	vRotAxis.Normalize();
	//	vRotAxis.Scale((float) Math::Sqrt((1 - dot)/2));
	//	return out = Quaternion(vRotAxis.x, vRotAxis.y, vRotAxis.z, (float) Math::Sqrt((1 + dot)/2));

	//}

	//bool Math::LineLineIntersection(
	//								const Vector3& p1,
	//								const Vector3& p2,
	//								const Vector3& p3,
	//								const Vector3& p4,
	//								float eps,
	//								Vector3& pa,
	//								Vector3& pb,
	//								float& mua,
	//								float& mub)
	//{
	//	Vector3 p13, p43, p21;
	//	float d1343, d4321, d1321, d4343, d2121;
	//	float numer, denom;
	//	mua = 0.0f;
	//	mub = 0.0f;
	//	pa = Vector3::Empty;
	//	pb = Vector3::Empty;

	//	p13 = p1 - p3;
	//	p43 = p4 - p3;

	//	if (Math::Abs(p43.x)  < eps
	//		&& Math::Abs(p43.y)  < eps
	//		&& Math::Abs(p43.z)  < eps)
	//		return false;

	//	p21 = p2 - p1;

	//	if (Math::Abs(p21.x)  < eps &&
	//		Math::Abs(p21.y)  < eps &&
	//		Math::Abs(p21.z)  < eps)
	//		return false;

	//	d1343 = Vector3::Dot(p13,p43);
	//	d4321 = Vector3::Dot(p43,p21);
	//	d1321 = Vector3::Dot(p13,p21);
	//	d4343 = p43.LengthSq();
	//	d2121 = p21.LengthSq();

	//	denom = d2121 * d4343 - d4321 * d4321;
	//	if (Math::Abs(denom) < eps)
	//		return false;

	//	numer = d1343 * d4321 - d1321 * d4343;

	//	mua = numer / denom;
	//	mub = (d1343 + d4321 * (mua)) / d4343;

	//	pa = p1 + mua*p21;
	//	pb = p3 + mub*p43;

	//	return true;

	//}


	//void Math::decompSwingTwistZ(const Quaternion& q, Vector3& swing, Vector3& twist)
	//{
	//	//kat obrotu wokol osi z (twist/skret)
	//	float tau = 0.0f;
	//	float gamma = 0.0f;
	//	float beta = 0.0f;

	//	if (q.w != 0.0f)
	//	{
	//		gamma = (float) Math::Atan(q.z/q.w);
	//		tau = gamma+gamma;
	//		beta = (float) Math::Atan(Math::Sqrt(q.x*q.x + q.y*q.y)/Math::Sqrt(q.z*q.z + q.w*q.w));
	//	}
	//	//w przciwnym razie dowolnie mozna ustawic gamma,tau,beta

	//	float cs = (float) Math::Cos(gamma);
	//	float sn = (float) Math::Sin(gamma);
	//	float snc = beta > 0.0f ? (float) Math::Sin(beta)/beta : 1.0f;
	//
	//	swing.x = ((q.x*cs - sn*q.y)*2)/snc;
	//	swing.y = ((q.x*sn + cs*q.y)*2)/snc;
	//	swing.z = 0.0f;
	//
	//	twist.x = twist.y = 0.0f;
	//	twist.z = tau;

	//}

	//float Math::constraintEllipse(float sx, float sy, float rx, float ry)
	//{
	//	float xx = rx != 0.0f ? sx/rx : 1.0f;
	//	float yy = ry != 0.0f ? sy/ry : 1.0f;
	//	return xx*xx + yy*yy; //-1.0f;
	//	//4*;2+
	//}

	//Quaternion& Math::PlaneRotation(Quaternion& out, PlaneType plane)
	//{
	//	switch (plane)
	//	{
	//	case PLANETYPE_Z:
	//	case PLANETYPE_XY:
	//		return out = Quaternion::Identity;
	//	case PLANETYPE_X:
	//	case PLANETYPE_YZ:
	//		return Quaternion::RotationYawPitchRoll(out, (float) Math::PI/2.0f, 0, (float) Math::PI/2.0f);
	//	case PLANETYPE_Y:
	//	case PLANETYPE_ZX:
	//		return Quaternion::RotationYawPitchRoll(out, -(float) Math::PI/2.0f, -(float) Math::PI/2.0f, 0);
	//	default:
	//		return out = Quaternion::Identity;
	//	}
	//}

	//Vector3 Math::TransformCoordinate(Vector3& out, const Vector3 source, const Quaternion sourceQuaternion)
	//{
	//	Quaternion tmp;
	//	tmp.Conjugate(sourceQuaternion);

	//	Quaternion taq	= tmp * Quaternion(source.x,source.y,source.z,0) * sourceQuaternion;
	//	out = Vector3(taq.x, taq.y, taq.z);
	//	return out;
	//}

}}
