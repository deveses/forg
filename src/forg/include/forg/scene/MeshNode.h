#ifndef FORG_SCENE_MESHNODE_H
#define FORG_SCENE_MESHNODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scene/Model.h"
#include "scene/SceneNode.h"

namespace forg::scene {

class FORG_API MeshNode : public SceneNode
{
    Model m_model;

  public:
    Model& GetModel();
    const Model& GetModel() const;

    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;

    void Render(IRenderDevice* device) override;
};

} // namespace forg::scene

#endif // FORG_SCENE_MESHNODE_H
