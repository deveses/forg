#ifndef FORG_SCENE_TREENODE_H
#define FORG_SCENE_TREENODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

#include <vector>

namespace forg::scene {

class FORG_API TreeNode
{
    TreeNode* m_parent;
    std::vector<TreeNode*> m_children;

  public:
    TreeNode();
    virtual ~TreeNode();

    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
    TreeNode(TreeNode&&) = delete;
    TreeNode& operator=(TreeNode&&) = delete;

    TreeNode* Parent();
    const TreeNode* Parent() const;
    bool IsRoot() const;

    uint ChildCount() const;
    TreeNode* Child(uint index);
    const TreeNode* Child(uint index) const;
    const std::vector<TreeNode*>& Children() const;

    bool AddChild(TreeNode& child);
    bool RemoveChild(TreeNode& child);
    void RemoveFromParent();
    void ClearChildren();

  private:
    bool HasAncestor(const TreeNode& node) const;
};

} // namespace forg::scene

#endif // FORG_SCENE_TREENODE_H
