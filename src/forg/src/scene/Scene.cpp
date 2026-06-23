#include "forg_pch.h"

#include "scene/Scene.h"

#include <algorithm>
#include <utility>

namespace forg::scene {

SceneNode& Scene::CreateNode()
{
    std::unique_ptr<SceneNode> node(new SceneNode());
    SceneNode& ref = *node;
    m_nodes.push_back(std::move(node));
    AddChild(ref);
    return ref;
}

MeshNode& Scene::CreateMeshNode()
{
    std::unique_ptr<MeshNode> node(new MeshNode());
    MeshNode& ref = *node;
    m_nodes.push_back(std::move(node));
    AddChild(ref);
    return ref;
}

uint Scene::NodeCount() const { return static_cast<uint>(m_nodes.size()); }

SceneNode* Scene::Node(uint index)
{
    return index < m_nodes.size() ? m_nodes[index].get() : nullptr;
}

const SceneNode* Scene::Node(uint index) const
{
    return index < m_nodes.size() ? m_nodes[index].get() : nullptr;
}

bool Scene::DestroyNode(SceneNode& node)
{
    const auto it =
        std::find_if(m_nodes.begin(), m_nodes.end(),
                     [&node](const std::unique_ptr<SceneNode>& candidate)
                     { return candidate.get() == &node; });

    if (it == m_nodes.end())
        return false;

    std::vector<TreeNode*> children = node.Children();
    for (TreeNode* child : children)
    {
        if (child != nullptr)
            AddChild(*child);
    }

    node.RemoveFromParent();
    m_nodes.erase(it);
    return true;
}

void Scene::ClearNodes()
{
    ClearChildren();
    m_nodes.clear();
}

void Scene::Render(IRenderDevice* device)
{
    for (TreeNode* child : Children())
    {
        SceneNode* sceneNode = dynamic_cast<SceneNode*>(child);
        if (sceneNode != nullptr)
            sceneNode->Render(device);
    }
}

} // namespace forg::scene
