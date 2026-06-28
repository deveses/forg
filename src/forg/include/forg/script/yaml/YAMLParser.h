/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
*******************************************************************************/

#ifndef _FORG_YAMLPARSER_H_
#define _FORG_YAMLPARSER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <memory>
#include <utility>
#include <vector>

#include "forg/base.h"
#include "forg/script/ParserBase.h"
#include "forg/script/generic/Document.h"

namespace forg::script::yaml {

using YAMLDocument = forg::script::generic::Document;
using YAMLNode = forg::script::generic::Node;

template <typename Visitor>
void ForEachAttribute(YAMLNode* _node, Visitor&& _visitor)
{
    for (YAMLNode* attr = _node != nullptr ? _node->GetAttributes() : nullptr;
         attr != nullptr; attr = attr->GetNext())
    {
        _visitor(*attr);
    }
}

template <typename Visitor>
void ForEachAttribute(const YAMLNode* _node, Visitor&& _visitor)
{
    for (const YAMLNode* attr = _node != nullptr ? _node->GetAttributes()
                                                 : nullptr;
         attr != nullptr; attr = attr->GetNext())
    {
        _visitor(*attr);
    }
}

template <typename Visitor>
void ForEachNode(YAMLDocument* _document, Visitor&& _visitor)
{
    std::vector<YAMLNode*> stack;

    if (_document != nullptr)
    {
        if (YAMLNode* root = _document->GetRootNode())
        {
            for (YAMLNode* child = root->GetChildren(); child != nullptr;
                 child = child->GetNext())
            {
                stack.push_back(child);
            }
        }
    }

    while (!stack.empty())
    {
        YAMLNode* node = stack.back();
        stack.pop_back();

        _visitor(*node);

        for (YAMLNode* child = node->GetChildren(); child != nullptr;
             child = child->GetNext())
        {
            stack.push_back(child);
        }
    }
}

template <typename Visitor>
void ForEachNode(const YAMLDocument* _document, Visitor&& _visitor)
{
    std::vector<const YAMLNode*> stack;

    if (_document != nullptr)
    {
        if (const YAMLNode* root = _document->GetRootNode())
        {
            for (const YAMLNode* child = root->GetChildren(); child != nullptr;
                 child = child->GetNext())
            {
                stack.push_back(child);
            }
        }
    }

    while (!stack.empty())
    {
        const YAMLNode* node = stack.back();
        stack.pop_back();

        _visitor(*node);

        for (const YAMLNode* child = node->GetChildren(); child != nullptr;
             child = child->GetNext())
        {
            stack.push_back(child);
        }
    }
}

FORG_API const char* FindAttributeValue(const YAMLNode* _node,
                                        const char* _name);
FORG_API const char* FindNodeAttributeValue(const YAMLDocument* _document,
                                            const char* _node_name,
                                            const char* _attribute_name);

class FORG_API YAMLParser : public forg::script::FileParserBase
{
    std::unique_ptr<YAMLDocument> m_doc;
    int m_error_code;

  public:
    YAMLParser();
    ~YAMLParser();

    YAMLDocument* Parse();

  private:
    bool ReadDocument(YAMLDocument* _doc);
    bool ParseLine(const std::string& _line, int _line_number,
                   std::vector<std::pair<int, YAMLNode*>>& _stack);
    bool ParseKeyValue(const std::string& _text, std::string& _key,
                       std::string& _value, bool& _has_value);
    bool ValidateKey(const std::string& _key) const;
    bool NormalizeValue(std::string& _value) const;
    bool IsUnsupportedValue(const std::string& _value) const;
    void SetErrorCode(int _code) { m_error_code = _code; }
};

} // namespace forg::script::yaml

#endif
