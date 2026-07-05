#include "forg_pch.h"

#include "scene/SoundEmitterNode.h"

#include "forg/io/ISerializer.h"

namespace forg::scene {
namespace {

bool ValueOrDefault(io::ISerializer& serializer, const char* name, float& value)
{
    float loaded = value;
    if (serializer.Value(name, loaded))
        value = loaded;
    return true;
}

} // namespace

const char* SoundEmitterNode::TypeName() const { return "SoundEmitterNode"; }

bool SoundEmitterNode::Save(io::ISerializer& serializer) const
{
    if (!SceneNode::Save(serializer) || !serializer.BeginObject("emitter"))
        return false;

    math::Vector3 position = m_position;
    float referenceDistance = m_referenceDistance;

    if (!serializer.Value("position_x", position.X) ||
        !serializer.Value("position_y", position.Y) ||
        !serializer.Value("position_z", position.Z) ||
        !serializer.Value("ref_distance", referenceDistance))
    {
        return false;
    }

    return serializer.EndObject();
}

bool SoundEmitterNode::Load(io::ISerializer& serializer)
{
    if (!SceneNode::Load(serializer) || !serializer.BeginObject("emitter"))
        return false;

    math::Vector3 position = m_position;
    float referenceDistance = m_referenceDistance;

    ValueOrDefault(serializer, "position_x", position.X);
    ValueOrDefault(serializer, "position_y", position.Y);
    ValueOrDefault(serializer, "position_z", position.Z);
    ValueOrDefault(serializer, "ref_distance", referenceDistance);

    if (!serializer.EndObject())
        return false;

    m_position = position;
    SetReferenceDistance(referenceDistance);
    return true;
}

void SoundEmitterNode::SetPosition(const math::Vector3& position)
{
    m_position = position;
}

const math::Vector3& SoundEmitterNode::Position() const { return m_position; }

void SoundEmitterNode::SetReferenceDistance(float distance)
{
    if (distance > 0.0f)
        m_referenceDistance = distance;
}

float SoundEmitterNode::ReferenceDistance() const
{
    return m_referenceDistance;
}

} // namespace forg::scene
