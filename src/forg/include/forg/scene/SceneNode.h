#ifndef FORG_SCENE_SCENENODE_H
#define FORG_SCENE_SCENENODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scene/TreeNode.h"

namespace forg {
class IRenderDevice;

namespace io {
class ISerializer;
}
} // namespace forg

namespace forg::scene {

class FORG_API SceneNode : public TreeNode
{
  public:
    virtual const char* TypeName() const;
    virtual bool Save(io::ISerializer& serializer) const;
    virtual bool Load(io::ISerializer& serializer);

    virtual void Update(double deltaSeconds);
    virtual void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_SCENENODE_H
