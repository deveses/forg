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

#include "forg/base.h"
#include "forg/script/ParserBase.h"
#include "forg/script/generic/Document.h"

namespace forg::script::xml {

///////////////////////////////////////////////////////////////////////////
namespace EXMLNodeType {
enum TYPE
{
    Unknown = forg::script::generic::ENodeType::Unknown,
    Root = forg::script::generic::ENodeType::Root,
    Element = forg::script::generic::ENodeType::Element,
    Attribute = forg::script::generic::ENodeType::Attribute
};
}

using XMLNode = forg::script::generic::Node;
using XMLDocument = forg::script::generic::Document;

///////////////////////////////////////////////////////////////////////////
class FORG_API XMLParser : public forg::script::TokenParserBase
{
  private:
    int m_error_code;
    XMLDocument* m_doc;

  public:
    XMLParser();
    ~XMLParser();

    XMLDocument* Parse();

  private:
    int GetToken();
    int GetSymbol(int _ch);
    void EatWhitespace();
    void InitTokens();

    bool ReadDocument(XMLDocument* _doc);
    XMLNode* ReadElement(XMLNode* _parent);
    bool ReadAttribute(XMLNode* _node);
    bool ReadContent(XMLNode* _node);
    bool ReadEndTag(XMLNode* _node);
    void SetErrorCode(int _code) { m_error_code = _code; }
};

} // namespace forg::script::xml

#endif
