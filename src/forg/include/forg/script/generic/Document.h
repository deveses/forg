/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
*******************************************************************************/

#ifndef _FORG_SCRIPT_GENERIC_DOCUMENT_H_
#define _FORG_SCRIPT_GENERIC_DOCUMENT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/base.h"
#include "forg/core/string.hpp"

namespace forg { namespace script { namespace generic {

    namespace ENodeType
    {
        enum TYPE
        {
            Unknown,
            Root,
            Element,
            Attribute
        };
    }

    class FORG_API Node
    {
        core::string m_name;
        core::string m_content;

        Node* m_parent;
        Node* m_next;
        Node* m_children;
        Node* m_attributes;
        int m_type;

    public:
        Node(int _type);
        ~Node();

        // Owns its children/attributes (freed in the destructor); non-copyable
        // so a shallow copy can't double-free the tree.
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        void AddChild(Node* _child)
        {
            _child->m_parent = this;
            _child->m_next = m_children;
            m_children = _child;
        }

        Node* GetChildren()
        {
            return m_children;
        }

        Node* GetNext()
        {
            return m_next;
        }

        void AddAttribute(Node* _attr)
        {
            _attr->m_parent = this;
            _attr->m_next = m_attributes;
            m_attributes = _attr;
        }

        Node* FindAttribute(const core::string& _name);

        void SetParent(Node* _parent)
        {
            m_parent = _parent;
        }

        void SetName(const core::string& _name)
        {
            m_name = _name;
        }

        const core::string& GetName() const { return m_name; }

        void SetContent(const core::string& _text)
        {
            m_content = _text;
        }

        const core::string& GetContent() const
        {
            return m_content;
        }

        int GetType() const { return m_type; }

        bool IsRoot() const { return m_type == ENodeType::Root; }
    };

    class FORG_API Document
    {
        Node* m_root;

    public:
        Document();
        ~Document();

        // Owns the root node (freed in the destructor); non-copyable.
        Document(const Document&) = delete;
        Document& operator=(const Document&) = delete;

        void SetRootNode(Node* _root) { m_root = _root; }

        Node* FindNode(const core::string& _name);
    };

}}}

#endif
