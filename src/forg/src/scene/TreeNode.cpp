#include "forg_pch.h"

#include "scene/TreeNode.h"

#include <algorithm>

namespace forg::scene {

TreeNode::TreeNode() : m_parent(nullptr) {}

TreeNode::~TreeNode()
{
    RemoveFromParent();
    ClearChildren();
}

TreeNode* TreeNode::Parent() { return m_parent; }

const TreeNode* TreeNode::Parent() const { return m_parent; }

bool TreeNode::IsRoot() const { return m_parent == nullptr; }

uint TreeNode::ChildCount() const
{
    return static_cast<uint>(m_children.size());
}

TreeNode* TreeNode::Child(uint index)
{
    return index < m_children.size() ? m_children[index] : nullptr;
}

const TreeNode* TreeNode::Child(uint index) const
{
    return index < m_children.size() ? m_children[index] : nullptr;
}

const std::vector<TreeNode*>& TreeNode::Children() const { return m_children; }

bool TreeNode::AddChild(TreeNode& child)
{
    if (&child == this || HasAncestor(child))
        return false;

    if (std::find(m_children.begin(), m_children.end(), &child) !=
        m_children.end())
    {
        return false;
    }

    child.RemoveFromParent();
    child.m_parent = this;
    m_children.push_back(&child);
    return true;
}

bool TreeNode::RemoveChild(TreeNode& child)
{
    const auto it = std::find(m_children.begin(), m_children.end(), &child);
    if (it == m_children.end())
        return false;

    child.m_parent = nullptr;
    m_children.erase(it);
    return true;
}

void TreeNode::RemoveFromParent()
{
    if (m_parent != nullptr)
        m_parent->RemoveChild(*this);
}

void TreeNode::ClearChildren()
{
    for (TreeNode* child : m_children)
    {
        if (child != nullptr)
            child->m_parent = nullptr;
    }
    m_children.clear();
}

bool TreeNode::HasAncestor(const TreeNode& node) const
{
    const TreeNode* parent = this;
    while (parent != nullptr)
    {
        if (parent == &node)
            return true;
        parent = parent->m_parent;
    }

    return false;
}

} // namespace forg::scene
