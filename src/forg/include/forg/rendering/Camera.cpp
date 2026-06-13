#include "forg_pch.h"

#include "rendering/Camera.h"
#include "math/Math.h"
#include "math/Matrix4.h"
//#include <d3dx9math.h>

namespace forg {

const float Camera::minimal_distance = 0.05f;

Camera::Camera(CameraView view_type)
: m_aspect(1.0f)
, m_fovy(0.78539816339744830961566084581988f)    // 70 degrees
, m_near(1.5f)
, m_far(1000.0f)
, m_target(0.0f, 0.0f, 0.0f)
, m_position(0.0f, 0.0f, 5.0f)
, m_up(0.0f, 1.0f, 0.0f)
, m_camera_view(view_type)
{
    m_dir = m_target - m_position;
    m_dir.Normalize();
}

Camera::~Camera(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Camera properties
//////////////////////////////////////////////////////////////////////////


const Vector3& Camera::get_Target() const
{
	return m_target;
}

const Vector3& Camera::get_Position() const
{
    return m_position;
}

float Camera::get_ScreenWidth() const
{
    return m_screen_width;
}

float Camera::get_ScreenHeight() const
{
    return m_screen_height;
}

float Camera::get_FOV() const
{
    return m_fovy;
}

float Camera::get_Aspect() const
{
    return m_aspect;
}

float Camera::get_NearRange() const
{
    return m_near;
}

float Camera::get_FarRange() const
{
    return m_far;
}

/*

void Camera::set_Target(const Vector3& value)
{
	m_target = value;
}

void Camera::set_Position(const Vector3& value)
{
	m_position = value;
}

const Vector3& Camera::get_Up() const
{
	return m_up;
}
void Camera::set_Up(const Vector3& value)
{
	m_up = value;
}

void Camera::set_View(CameraView value)
{
	m_camera_view = value;
}


void Camera::set_FOV(float value)
{
	m_fov = value;
}


void Camera::set_Aspect(float value)
{
	m_aspect = value;
}


void Camera::set_NearRange(float value)
{
	m_near = value;
}


void Camera::set_FarRange(float value)
{
	m_far = value;
}*/

CameraView Camera::get_View() const
{
    return m_camera_view;
}

void Camera::set_ScreenSize(float width, float height)
{
	m_screen_width = width;
	m_screen_height = height;

	SetAspect(m_screen_width / m_screen_height);
}

void Camera::GetViewMatrix(Matrix4& view)
{
    view = m_matView;
}

void Camera::GetProjectionMatrix(Matrix4& projection)
{
    projection = m_matProjection;
}

void Camera::UpdateViewMatrix()
{
    // TODO: LH/RH
    Matrix4::LookAtRH(m_matView, m_position, m_target, m_up);
}

void Camera::UpdateProjectionMatrix()
{
    // TODO: LH/RH, Ortho
    Matrix4::PerspectiveFovRH(m_matProjection, m_fovy, m_aspect, m_near, m_far);
}

void Camera::SetFOVY(float value)
{
    m_fovy = value;

    UpdateProjectionMatrix();
}

void Camera::SetAspect(float value)
{
    m_aspect = value;

    UpdateProjectionMatrix();
}

void Camera::SetPosition(const Vector3& value)
{
    m_position = value;

    Vector3::Substract(m_dir, m_target, m_position);
    m_dir.Normalize();

    UpdateViewMatrix();
}

void Camera::SetTarget(const Vector3& value)
{
    m_target = value;

    Vector3::Substract(m_dir, m_target, m_position);
    m_dir.Normalize();

    UpdateViewMatrix();
}

void Camera::SetUp(const Vector3& value)
{
    m_up = value;

    UpdateViewMatrix();
}

//////////////////////////////////////////////////////////////////////////
//	Camera operations
//////////////////////////////////////////////////////////////////////////


void Camera::Dolly(float camera, float target)
{
	Vector3 new_position = m_position;
	Vector3 new_target = m_target;
	Vector3 c = m_dir * camera;
	Vector3 t = m_dir * target;

	//if ((m_dir - c).Length() > minimal_distance)
	new_position += c;

	//if ((m_dir - t).Length() > minimal_distance)
	new_target += t;

	SetPosition(new_position);
	SetTarget(new_target);

}

void Camera::Roll(float angle)
{
	Matrix4 rot;
	Vector3 up = m_up;

	up.TransformCoordinate(Matrix4::RotationAxis(
		rot,
		m_position - m_target,
		angle));

	SetUp(up);
}

void Camera::FieldOfView(float fov_change)
{
	SetFOVY(m_fovy + fov_change);
}

void Camera::Truck(float x, float y)
{
    Vector3 right;
    Vector3 up(m_up);

    Vector3::Cross(right, m_dir, m_up);
    right.Normalize();

    right *= x;
    up *= y;

    Vector3 truck;
    Vector3::Add(truck, right, up);

	SetPosition(m_position + truck);
	SetTarget(m_target + truck);
}

void Camera::Orbit(float x, float y)
{
	Vector3 normal(0.0f, 1.0f, 0.0f);

	Vector3 direction = m_position - m_target;
	Vector3 n_dir;
	Vector3 horizon;
	Matrix4 rot;
	Vector3 dir;
	Vector3 up;

	Vector3::Cross(horizon, m_up, Vector3::Normalize(n_dir, direction));
	Matrix4::RotationAxis(rot, horizon, y);
	Vector3::TransformCoordinate(dir, direction, rot);
	SetPosition(dir + m_target);
	up = m_up;
	up.TransformCoordinate(rot);
	SetUp(up);


	direction = m_position - m_target;
	Vector3::Cross(horizon, m_up, Vector3::Normalize(n_dir, direction));
	Matrix4::RotationAxis(rot, normal, - x);
	Vector3::TransformCoordinate(dir, direction, rot);

	SetPosition(dir + m_target);
	up = m_up;
	up.TransformCoordinate(rot);
	SetUp(up);
}

void Camera::Pan(float x, float y)
{
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 direction = m_target - m_position;
    Vector3 horizon;
    Vector3 dir;
    Vector3 up;

    Vector3::Cross(horizon, m_up, m_dir);

    Quaternion qRotX, qRotY, rot;

    Quaternion::RotationAxis(qRotX, horizon, y);
    Quaternion::RotationAxis(qRotY, normal, x);

    Quaternion::Multiply(rot, qRotX, qRotY);

    Vector3::TransformCoordinate(dir, direction, rot);
    Vector3::TransformCoordinate(up, m_up, rot);

    SetTarget(m_position + dir);
    SetUp(up);
}

/*
void Camera::Pan(float x, float y)
{
	Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 direction = m_target - m_position;
	Vector3 n_dir;
	Vector3 horizon;
	Vector3 dir;
	Vector3 up;
	Matrix4 rot;
    Matrix4 rotx;
    Matrix4 roty;

    Vector3::Cross(horizon, m_up, m_dir);

    // vertical rotation
	Matrix4::RotationAxis(rotx, horizon, -y);
    // horizontal rotation
    Matrix4::RotationAxis(roty, normal, -x);

    Matrix4::Multiply(rot, rotx, roty);

    // move view cone axis up/down
	Vector3::TransformCoordinate(dir, direction, rot);

    up = m_up;
    up.TransformCoordinate(rot);

    SetTarget(m_position + dir);
	SetUp(up);
}
*/

}
