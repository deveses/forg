#include "forg_pch.h"

#include <vector>

#include "forg/script/generic/Document.h"

namespace forg::script::generic {

Node::Node(int _type)
{
    m_parent = nullptr;
    m_next = nullptr;
    m_children = nullptr;
    m_attributes = nullptr;
    m_type = _type;
}

Node::~Node()
{
    // A node owns its children and attributes, each a singly-linked list
    // chained through the next node's m_next. Free them iteratively (a child's
    // own destructor recurses into its subtree); m_next itself belongs to the
    // parent's list, so it is not freed here.
    for (Node* child = m_children; child != nullptr;)
    {
        Node* next = child->m_next;
        delete child;
        child = next;
    }

    for (Node* attr = m_attributes; attr != nullptr;)
    {
        Node* next = attr->m_next;
        delete attr;
        attr = next;
    }
}

Node* Node::FindAttribute(const core::string& _name)
{
    Node* n = m_attributes;

    while (n)
    {
        if (n->GetName() == _name)
        {
            return n;
        }

        n = n->GetNext();
    }

    return nullptr;
}

Document::Document() : m_root(nullptr) {}

Document::~Document() { delete m_root; }

Node* Document::FindNode(const core::string& _name)
{
    std::vector<Node*> stack;

    if (m_root)
        stack.push_back(m_root);

    while (!stack.empty())
    {
        Node* n = stack.back();
        stack.pop_back();

        if (n->GetName() == _name)
        {
            return n;
        }

        n = n->GetChildren();
        while (n)
        {
            stack.push_back(n);
            n = n->GetNext();
        }
    }

    return nullptr;
}

} // namespace forg::script::generic
