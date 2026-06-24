#ifndef FORG_SCENE_SCENENODE_H
#define FORG_SCENE_SCENENODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scene/TreeNode.h"

namespace forg {
class IRenderDevice;
}

namespace forg::scene {

class FORG_API SceneNode : public TreeNode
{
  public:
    virtual void Update(double deltaSeconds);
    virtual void Render(IRenderDevice* device);
};

} // namespace forg::scene

#endif // FORG_SCENE_SCENENODE_H
