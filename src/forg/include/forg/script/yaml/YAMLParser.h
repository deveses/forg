/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
*******************************************************************************/

#ifndef _FORG_YAMLPARSER_H_
#define _FORG_YAMLPARSER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <utility>

#include "forg/base.h"
#include "forg/script/ParserBase.h"
#include "forg/script/xml/XMLParser.h"

namespace forg { namespace script { namespace yaml {

    using YAMLDocument = forg::script::xml::XMLDocument;
    using YAMLNode = forg::script::xml::XMLNode;

    class FORG_API YAMLParser : public forg::script::FileParserBase
    {
        YAMLDocument* m_doc;
        int m_error_code;

    public:
        YAMLParser();
        ~YAMLParser();

        YAMLDocument* Parse();

    private:
        bool ReadDocument(YAMLDocument* _doc);
        bool ParseLine(const std::string& _line, int _line_number,
                       std::vector<std::pair<int, YAMLNode*> >& _stack);
        bool ParseKeyValue(const std::string& _text, std::string& _key,
                           std::string& _value, bool& _has_value);
        bool ValidateKey(const std::string& _key) const;
        bool NormalizeValue(std::string& _value) const;
        bool IsUnsupportedValue(const std::string& _value) const;
        void SetErrorCode(int _code) { m_error_code = _code; }
    };

}}}

#endif
