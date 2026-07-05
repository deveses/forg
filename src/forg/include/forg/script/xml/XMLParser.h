// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/base.h"
#include "forg/script/ParserBase.h"
#include "forg/script/generic/Document.h"

#include <memory>

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
    std::unique_ptr<XMLDocument> m_doc;

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
