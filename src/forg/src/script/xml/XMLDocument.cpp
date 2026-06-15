#include "forg_pch.h"

#include <vector>

#include "forg/script/xml/XMLParser.h"

namespace forg::script::xml
{

XMLNode::XMLNode(int _type)
{
    m_parent = nullptr;
    m_next = nullptr;
    m_children = nullptr;
    m_attributes = nullptr;
    m_type = _type;
}

XMLNode::~XMLNode()
{
    // A node owns its children and attributes, each a singly-linked list chained
    // through the next node's m_next. Free them iteratively (a child's own
    // destructor recurses into its subtree); m_next itself belongs to the
    // parent's list, so it is not freed here.
    for (XMLNode* child = m_children; child != nullptr;)
    {
        XMLNode* next = child->m_next;
        delete child;
        child = next;
    }

    for (XMLNode* attr = m_attributes; attr != nullptr;)
    {
        XMLNode* next = attr->m_next;
        delete attr;
        attr = next;
    }
}

XMLNode* XMLNode::FindAttribute(const core::string& _name)
{
    XMLNode* n = m_attributes;

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

XMLDocument::XMLDocument() : m_root(nullptr) {}

XMLDocument::~XMLDocument()
{
    delete m_root;
}

XMLNode* XMLDocument::FindNode(const core::string& _name)
{
    std::vector<XMLNode*> stack;

    if (m_root)
        stack.push_back(m_root);

    while (!stack.empty())
    {
        XMLNode* n = stack.back();
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

} // namespace forg::script::xml
