#include "forg_pch.h"

#include "forg/io/ISerializer.h"
#include "forg/ui/gui.h"
#include "scene/Scene.h"

#include <algorithm>
#include <string>
#include <utility>

namespace forg::scene {
namespace {

int NodeIndex(const std::vector<std::unique_ptr<SceneNode>>& nodes,
              const SceneNode* node)
{
    for (uint i = 0; i < nodes.size(); ++i)
    {
        if (nodes[i].get() == node)
            return static_cast<int>(i);
    }
    return -1;
}

bool HasUnownedSceneNodeChild(
    const std::vector<std::unique_ptr<SceneNode>>& nodes,
    const TreeNode& parent)
{
    for (const TreeNode* child : parent.Children())
    {
        const SceneNode* sceneNode = dynamic_cast<const SceneNode*>(child);
        if (sceneNode != nullptr && NodeIndex(nodes, sceneNode) < 0)
            return true;
    }
    return false;
}

bool ValidateOwnedGraph(const std::vector<std::unique_ptr<SceneNode>>& nodes,
                        const TreeNode& sceneRoot)
{
    if (HasUnownedSceneNodeChild(nodes, sceneRoot))
        return false;

    for (const std::unique_ptr<SceneNode>& node : nodes)
    {
        if (node && HasUnownedSceneNodeChild(nodes, *node))
            return false;
    }

    return true;
}

std::unique_ptr<SceneNode> CreateSerializedNode(const core::string& type)
{
    const std::string typeText = type.c_str();
    if (typeText == "SceneNode")
        return std::unique_ptr<SceneNode>(new SceneNode());
    if (typeText == "CameraNode")
        return std::unique_ptr<SceneNode>(new CameraNode());
    if (typeText == "MeshNode")
        return std::unique_ptr<SceneNode>(new MeshNode());
    if (typeText == "GuiNode")
        return std::unique_ptr<SceneNode>(new ui::GuiNode());
    return nullptr;
}

} // namespace

SceneNode& Scene::CreateNode()
{
    std::unique_ptr<SceneNode> node(new SceneNode());
    SceneNode& ref = *node;
    m_nodes.push_back(std::move(node));
    AddChild(ref);
    return ref;
}

CameraNode& Scene::CreateCameraNode()
{
    std::unique_ptr<CameraNode> node(new CameraNode());
    CameraNode& ref = *node;
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

ui::GuiNode& Scene::CreateGuiNode()
{
    std::unique_ptr<ui::GuiNode> node(new ui::GuiNode());
    ui::GuiNode& ref = *node;
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

bool Scene::Save(io::ISerializer& serializer) const
{
    if (!ValidateOwnedGraph(m_nodes, *this))
        return false;

    if (!serializer.BeginObject("scene"))
        return false;

    int version = 1;
    if (!serializer.Value("version", version))
        return false;

    uint count = static_cast<uint>(m_nodes.size());
    if (!serializer.BeginArray("nodes", count))
        return false;

    for (const std::unique_ptr<SceneNode>& node : m_nodes)
    {
        if (!node || !serializer.BeginObject("node"))
            return false;

        core::string type(node->TypeName());
        TreeNode* parentNode = node->Parent();
        int parent = -1;
        if (parentNode != this)
        {
            parent = NodeIndex(m_nodes, dynamic_cast<SceneNode*>(parentNode));
            if (parent < 0)
                return false;
        }

        if (!serializer.Value("type", type) ||
            !serializer.Value("parent", parent) || !node->Save(serializer) ||
            !serializer.EndObject())
        {
            return false;
        }
    }

    return serializer.EndArray() && serializer.EndObject();
}

bool Scene::Load(io::ISerializer& serializer)
{
    if (!serializer.BeginObject("scene"))
        return false;

    int version = 0;
    if (!serializer.Value("version", version) || version != 1)
        return false;

    uint count = 0;
    if (!serializer.BeginArray("nodes", count))
        return false;

    Scene temp;
    std::vector<int> parents;
    parents.reserve(count);
    temp.m_nodes.reserve(count);

    for (uint i = 0; i < count; ++i)
    {
        if (!serializer.BeginObject("node"))
            return false;

        core::string type;
        int parent = -1;
        if (!serializer.Value("type", type) ||
            !serializer.Value("parent", parent))
        {
            return false;
        }

        std::unique_ptr<SceneNode> node = CreateSerializedNode(type);
        if (!node || !node->Load(serializer) || !serializer.EndObject())
            return false;

        parents.push_back(parent);
        temp.m_nodes.push_back(std::move(node));
    }

    if (!serializer.EndArray() || !serializer.EndObject())
        return false;

    for (uint i = 0; i < count; ++i)
    {
        const int parent = parents[i];
        bool attached = false;
        if (parent == -1)
        {
            attached = temp.AddChild(*temp.m_nodes[i]);
        }
        else if (parent >= 0 && static_cast<uint>(parent) < count &&
                 static_cast<uint>(parent) != i)
        {
            attached = temp.m_nodes[parent]->AddChild(*temp.m_nodes[i]);
        }

        if (!attached)
            return false;
    }

    ClearNodes();
    std::vector<TreeNode*> rootChildren = temp.Children();
    for (TreeNode* child : rootChildren)
    {
        if (child != nullptr)
        {
            child->RemoveFromParent();
            AddChild(*child);
        }
    }
    m_nodes = std::move(temp.m_nodes);
    return true;
}

bool Scene::LoadResources(IRenderDevice* device)
{
    bool loaded = true;
    for (std::unique_ptr<SceneNode>& node : m_nodes)
    {
        MeshNode* meshNode = dynamic_cast<MeshNode*>(node.get());
        if (meshNode != nullptr &&
            (meshNode->GetModel().MeshType() != ModelMeshType::None ||
             meshNode->GetModel().SourcePath().length() != 0))
        {
            loaded = meshNode->GetModel().LoadResources(device) && loaded;
        }

        ui::GuiNode* guiNode = dynamic_cast<ui::GuiNode*>(node.get());
        if (guiNode != nullptr)
            loaded = guiNode->LoadResources(device) && loaded;
    }
    return loaded;
}

CameraNode* Scene::ActiveCameraNode()
{
    CameraNode* firstCamera = nullptr;
    for (std::unique_ptr<SceneNode>& node : m_nodes)
    {
        CameraNode* cameraNode = dynamic_cast<CameraNode*>(node.get());
        if (cameraNode == nullptr)
            continue;

        if (firstCamera == nullptr)
            firstCamera = cameraNode;
        if (cameraNode->Active())
            return cameraNode;
    }
    return firstCamera;
}

const CameraNode* Scene::ActiveCameraNode() const
{
    return const_cast<Scene*>(this)->ActiveCameraNode();
}

void Scene::Update(double deltaSeconds)
{
    for (TreeNode* child : Children())
    {
        SceneNode* sceneNode = dynamic_cast<SceneNode*>(child);
        if (sceneNode != nullptr)
            sceneNode->Update(deltaSeconds);
    }
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
