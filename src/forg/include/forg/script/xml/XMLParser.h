/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef _FORG_XMLPARSER_H_
#define _FORG_XMLPARSER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/vector.hpp"
#include "core/string.hpp"
#include "os/File.h"
#include "script/lexer.h"

namespace forg { namespace script { namespace xml {

    class FORG_API ParserBase
    {
    protected:
        typedef core::vector<SToken> TokenVec;

        Lexer m_lexer;
        TokenVec m_tokens;
        SParserState m_state;
        uint m_current_token;

    public:
        virtual ~ParserBase() {};

        bool ReadTokens();

        bool HasMoreTokens();
        SToken* GetNextToken();
        SToken* PeekNextToken();

        uint GetNextTokenIndex() const { return m_current_token; }
        void SetNextTokenIndex(uint _index) { m_current_token = _index; }

        virtual bool GetChar(int& _ch) = 0;
        virtual int GetSymbol(int _ch) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace EXMLNodeType
    {
        enum TYPE
        {
            Unknown,
            Root,
            Element,
            Attribute
        };
    }

    class FORG_API XMLNode
    {
        core::string m_name;
        core::string m_content;

        XMLNode* m_parent;
        XMLNode* m_next;
        XMLNode* m_children;
        XMLNode* m_attributes;
        int m_type;

    public:
        XMLNode(int _type);
        ~XMLNode();

        void AddChild(XMLNode* _child)
        {
            _child->m_parent = this;
            _child->m_next = m_children;
            m_children = _child;
        }

        XMLNode* GetChildren()
        {
            return m_children;
        }

        XMLNode* GetNext()
        {
            return m_next;
        }

        void AddAttribute(XMLNode* _attr)
        {
            _attr->m_parent = this;
            _attr->m_next = m_attributes;
            m_attributes = _attr;
        }

        XMLNode* FindAttribute(const core::string& _name);

        void SetParent(XMLNode* _parent)
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

        bool IsRoot() const { return m_type == EXMLNodeType::Root; }
    };

    class FORG_API XMLDocument
    {
        XMLNode* m_root;

    public:
        XMLDocument();
        ~XMLDocument();

        void SetRootNode(XMLNode* _root) { m_root = _root; }

        XMLNode* FindNode(const core::string& _name);
    };

    ///////////////////////////////////////////////////////////////////////////
    class FORG_API XMLParser : public ParserBase
    {
    private:
        forg::os::File m_file;
        int m_error_code;
        XMLDocument* m_doc;

    public:
        XMLParser();
        ~XMLParser();

        bool Open(const char* _filename);
        XMLDocument* Parse();
        void Close();

    private:
        int GetToken();
        int GetSymbol(int _ch);
        bool GetChar(int& _ch);
        void EatWhitespace();
        void InitTokens();

        bool ReadDocument(XMLDocument* _doc);
        XMLNode* ReadElement(XMLNode* _parent);
        bool ReadAttribute(XMLNode* _node);
        bool ReadContent(XMLNode* _node);
        bool ReadEndTag(XMLNode* _node);
        void SetErrorCode(int _code) { m_error_code = _code; }

    };

}}}

#endif
