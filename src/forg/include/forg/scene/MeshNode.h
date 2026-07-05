#pragma once
#include "scene/Model.h"
#include "scene/SceneNode.h"

namespace forg::scene {

class FORG_API MeshNode : public SceneNode
{
    Model m_model;

  public:
    Model& GetModel();
    const Model& GetModel() const;
    ModelMeshType MeshType() const;
    void SetPrimitive(ModelMeshType type);
    void SetBox(float width, float height, float depth);
    void SetSphere(float radius, int slices, int stacks);
    void SetCylinder(float radius1, float radius2, float length, int slices,
                     int stacks);
    void SetPyramid(u32 numAngles, float radius, float height);
    void SetGrid(float sizeX, float sizeY, int color, u32 subgrid);

    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;

    void Render(IRenderDevice* device) override;
};

} // namespace forg::scene
