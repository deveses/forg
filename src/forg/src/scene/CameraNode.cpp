#include "forg_pch.h"

#include "scene/CameraNode.h"

#include "forg/io/ISerializer.h"
#include "forg/rendering/IRenderDevice.h"

#include <cstring>

namespace forg::scene {
namespace {

bool StringEquals(const core::string& lhs, const char* rhs)
{
    return std::strcmp(lhs.c_str(), rhs) == 0;
}

bool ValueOrDefault(io::ISerializer& serializer, const char* name, float& value)
{
    float loaded = value;
    if (serializer.Value(name, loaded))
        value = loaded;
    return true;
}

} // namespace

const char* CameraProjectionName(CameraProjection projection)
{
    switch (projection)
    {
    case CameraProjection::Perspective:
        return "perspective";
    case CameraProjection::Orthogonal:
        return "orthogonal";
    case CameraProjection::Screen:
        return "screen";
    }
    return "perspective";
}

bool CameraProjectionFromName(const core::string& name,
                              CameraProjection& projection)
{
    if (StringEquals(name, "perspective"))
        projection = CameraProjection::Perspective;
    else if (StringEquals(name, "orthogonal"))
        projection = CameraProjection::Orthogonal;
    else if (StringEquals(name, "screen"))
        projection = CameraProjection::Screen;
    else
        return false;

    return true;
}

CameraNode::CameraNode() = default;

const char* CameraNode::TypeName() const { return "CameraNode"; }

bool CameraNode::Save(io::ISerializer& serializer) const
{
    if (!SceneNode::Save(serializer) || !serializer.BeginObject("camera"))
        return false;

    core::string projection(CameraProjectionName(m_projection));
    int active = m_active ? 1 : 0;
    int controllable = m_controllable ? 1 : 0;

    Vector3 position = m_camera.get_Position();
    Vector3 target = m_camera.get_Target();
    float fov = m_camera.get_FOV();
    float nearRange = m_camera.get_NearRange();
    float farRange = m_camera.get_FarRange();

    if (!serializer.Value("projection", projection) ||
        !serializer.Value("active", active) ||
        !serializer.Value("controllable", controllable) ||
        !serializer.Value("position_x", position.X) ||
        !serializer.Value("position_y", position.Y) ||
        !serializer.Value("position_z", position.Z) ||
        !serializer.Value("target_x", target.X) ||
        !serializer.Value("target_y", target.Y) ||
        !serializer.Value("target_z", target.Z) ||
        !serializer.Value("fov", fov) || !serializer.Value("near", nearRange) ||
        !serializer.Value("far", farRange))
    {
        return false;
    }

    return serializer.EndObject();
}

bool CameraNode::Load(io::ISerializer& serializer)
{
    if (!SceneNode::Load(serializer) || !serializer.BeginObject("camera"))
        return false;

    core::string projectionName("perspective");
    if (!serializer.Value("projection", projectionName))
        return false;

    CameraProjection projection = CameraProjection::Perspective;
    if (!CameraProjectionFromName(projectionName, projection))
        return false;

    int active = 1;
    serializer.Value("active", active);
    int controllable = 0;
    serializer.Value("controllable", controllable);

    Vector3 position = m_camera.get_Position();
    Vector3 target = m_camera.get_Target();
    float fov = m_camera.get_FOV();

    ValueOrDefault(serializer, "position_x", position.X);
    ValueOrDefault(serializer, "position_y", position.Y);
    ValueOrDefault(serializer, "position_z", position.Z);
    ValueOrDefault(serializer, "target_x", target.X);
    ValueOrDefault(serializer, "target_y", target.Y);
    ValueOrDefault(serializer, "target_z", target.Z);
    ValueOrDefault(serializer, "fov", fov);

    float ignoredNear = m_camera.get_NearRange();
    float ignoredFar = m_camera.get_FarRange();
    ValueOrDefault(serializer, "near", ignoredNear);
    ValueOrDefault(serializer, "far", ignoredFar);

    if (!serializer.EndObject())
        return false;

    m_projection = projection;
    m_active = active != 0;
    m_controllable = controllable != 0;
    m_camera.set_Position(position);
    m_camera.set_Target(target);
    m_camera.FieldOfView(fov - m_camera.get_FOV());
    m_camera.set_View(projection == CameraProjection::Orthogonal ? Orthogonal
                                                                 : Perspective);
    return true;
}

Camera& CameraNode::GetCamera() { return m_camera; }

const Camera& CameraNode::GetCamera() const { return m_camera; }

void CameraNode::SetProjection(CameraProjection projection)
{
    m_projection = projection;
    m_camera.set_View(projection == CameraProjection::Orthogonal ? Orthogonal
                                                                 : Perspective);
}

CameraProjection CameraNode::Projection() const { return m_projection; }

void CameraNode::SetActive(bool active) { m_active = active; }

bool CameraNode::Active() const { return m_active; }

void CameraNode::SetControllable(bool controllable)
{
    m_controllable = controllable;
}

bool CameraNode::Controllable() const { return m_controllable; }

void CameraNode::Apply(IRenderDevice* device)
{
    if (device == nullptr)
        return;

    Viewport viewport;
    device->GetViewport(&viewport);

    if (m_projection == CameraProjection::Screen)
    {
        Matrix4 view = Matrix4::Identity;
        Matrix4 projection;
        Matrix4::OrthoOffCenterRH(projection, 0.0f, viewport.Width,
                                  viewport.Height, 0.0f, 0.0f, 100.0f);
        device->SetTransform(TransformType_View, view);
        device->SetTransform(TransformType_Projection, projection);
    }
    else
    {
        m_camera.set_ScreenSize(viewport.Width, viewport.Height);

        Matrix4 view;
        Matrix4 projection;
        m_camera.GetViewMatrix(view);
        m_camera.GetProjectionMatrix(projection);

        device->SetTransform(TransformType_View, view);
        device->SetTransform(TransformType_Projection, projection);
    }

    device->SetTransform(TransformType_World, Matrix4::Identity);
}

} // namespace forg::scene
