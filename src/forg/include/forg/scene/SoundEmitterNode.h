#pragma once
#include "math/Vector3.h"
#include "scene/SceneNode.h"

namespace forg::scene {

// Provides a 3D position for descendant SoundNodes; the distance to the
// listener drives attenuation and stereo panning.
class FORG_API SoundEmitterNode : public SceneNode
{
    math::Vector3 m_position = math::Vector3(0.0f, 0.0f, 0.0f);
    float m_referenceDistance = 1.0f;

  public:
    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;

    void SetPosition(const math::Vector3& position);
    const math::Vector3& Position() const;

    // Distance at which attenuation starts; full volume within it.
    void SetReferenceDistance(float distance);
    float ReferenceDistance() const;
};

} // namespace forg::scene
