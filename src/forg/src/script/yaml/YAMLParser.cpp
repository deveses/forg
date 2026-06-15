#include "forg_pch.h"

#include <cctype>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "forg/script/yaml/YAMLParser.h"

namespace forg::script::yaml
{

namespace ParserError
{
enum TYPE
{
    NoErrors = 0,
    InvalidSyntax
};
}

namespace
{

std::string Trim(const std::string& text)
{
    std::string::size_type begin = 0;
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin])))
    {
        begin++;
    }

    std::string::size_type end = text.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(text[end - 1])))
    {
        end--;
    }

    return text.substr(begin, end - begin);
}

bool IsBlankOrComment(const std::string& line)
{
    std::string trimmed = Trim(line);
    return trimmed.empty() || trimmed[0] == '#';
}

std::string StripInlineComment(const std::string& line)
{
    bool in_single_quote = false;
    bool in_double_quote = false;

    for (std::string::size_type i = 0; i < line.size(); ++i)
    {
        char ch = line[i];

        if (ch == '\'' && !in_double_quote)
        {
            in_single_quote = !in_single_quote;
            continue;
        }

        if (ch == '"' && !in_single_quote)
        {
            bool escaped = (i > 0 && line[i - 1] == '\\');
            if (!escaped)
                in_double_quote = !in_double_quote;
            continue;
        }

        if (ch == '#' && !in_single_quote && !in_double_quote)
        {
            if (i == 0 || std::isspace(static_cast<unsigned char>(line[i - 1])))
                return line.substr(0, i);
        }
    }

    return line;
}

std::string::size_type FindMappingColon(const std::string& text)
{
    bool in_single_quote = false;
    bool in_double_quote = false;

    for (std::string::size_type i = 0; i < text.size(); ++i)
    {
        char ch = text[i];

        if (ch == '\'' && !in_double_quote)
        {
            in_single_quote = !in_single_quote;
            continue;
        }

        if (ch == '"' && !in_single_quote)
        {
            bool escaped = (i > 0 && text[i - 1] == '\\');
            if (!escaped)
                in_double_quote = !in_double_quote;
            continue;
        }

        if (ch == ':' && !in_single_quote && !in_double_quote)
            return i;
    }

    return std::string::npos;
}

bool HasBalancedQuotes(const std::string& text)
{
    bool in_single_quote = false;
    bool in_double_quote = false;

    for (std::string::size_type i = 0; i < text.size(); ++i)
    {
        char ch = text[i];

        if (ch == '\'' && !in_double_quote)
        {
            in_single_quote = !in_single_quote;
            continue;
        }

        if (ch == '"' && !in_single_quote)
        {
            bool escaped = (i > 0 && text[i - 1] == '\\');
            if (!escaped)
                in_double_quote = !in_double_quote;
        }
    }

    return !in_single_quote && !in_double_quote;
}

}

YAMLParser::YAMLParser()
{
    m_doc = nullptr;
    m_error_code = ParserError::NoErrors;
}

YAMLParser::~YAMLParser()
{
    delete m_doc;
}

YAMLDocument* YAMLParser::Parse()
{
    if (m_doc)
    {
        delete m_doc;
        m_doc = nullptr;
    }

    m_error_code = ParserError::NoErrors;

    std::unique_ptr<YAMLDocument> doc(new YAMLDocument());

    if (ReadDocument(doc.get()))
    {
        m_doc = doc.release();
    }

    return m_doc;
}

bool YAMLParser::ReadDocument(YAMLDocument* _doc)
{
    std::unique_ptr<YAMLNode> root_node(
        new YAMLNode(forg::script::generic::ENodeType::Root));

    std::vector<std::pair<int, YAMLNode*> > stack;
    stack.push_back(std::make_pair(-1, root_node.get()));

    std::string line;
    int line_number = 0;
    while (ReadLine(line))
    {
        line_number++;

        if (!ParseLine(line, line_number, stack))
            return false;
    }

    _doc->SetRootNode(root_node.release());

    return true;
}

bool YAMLParser::ParseLine(const std::string& _line, int /*_line_number*/,
                           std::vector<std::pair<int, YAMLNode*> >& _stack)
{
    if (IsBlankOrComment(_line))
        return true;

    int indent = 0;
    while (indent < static_cast<int>(_line.size()))
    {
        if (_line[indent] == ' ')
        {
            indent++;
            continue;
        }

        if (_line[indent] == '\t')
        {
            SetErrorCode(ParserError::InvalidSyntax);
            return false;
        }

        break;
    }

    std::string text = Trim(StripInlineComment(_line.substr(indent)));
    if (text.empty())
        return true;

    if (text[0] == '-' || text[0] == '&' || text[0] == '*' || text[0] == '!')
    {
        SetErrorCode(ParserError::InvalidSyntax);
        return false;
    }

    std::string key;
    std::string value;
    bool has_value = false;

    if (!ParseKeyValue(text, key, value, has_value))
    {
        SetErrorCode(ParserError::InvalidSyntax);
        return false;
    }

    while (!_stack.empty() && indent <= _stack.back().first)
        _stack.pop_back();

    if (_stack.empty())
    {
        SetErrorCode(ParserError::InvalidSyntax);
        return false;
    }

    YAMLNode* parent = _stack.back().second;

    if (has_value)
    {
        std::unique_ptr<YAMLNode> attr(
            new YAMLNode(forg::script::generic::ENodeType::Attribute));
        attr->SetName(key.c_str());
        attr->SetContent(value.c_str());
        parent->AddAttribute(attr.release());
        return true;
    }

    std::unique_ptr<YAMLNode> child(
        new YAMLNode(forg::script::generic::ENodeType::Element));
    child->SetName(key.c_str());

    YAMLNode* child_ptr = child.get();
    parent->AddChild(child.release());
    _stack.push_back(std::make_pair(indent, child_ptr));

    return true;
}

bool YAMLParser::ParseKeyValue(const std::string& _text, std::string& _key,
                               std::string& _value, bool& _has_value)
{
    if (!HasBalancedQuotes(_text))
        return false;

    std::string::size_type colon = FindMappingColon(_text);
    if (colon == std::string::npos)
        return false;

    _key = Trim(_text.substr(0, colon));
    _value = Trim(_text.substr(colon + 1));
    _has_value = !_value.empty();

    if (!ValidateKey(_key))
        return false;

    if (_has_value)
    {
        if (IsUnsupportedValue(_value))
            return false;

        if (!NormalizeValue(_value))
            return false;
    }

    return true;
}

bool YAMLParser::ValidateKey(const std::string& _key) const
{
    if (_key.empty())
        return false;

    for (std::string::size_type i = 0; i < _key.size(); ++i)
    {
        char ch = _key[i];
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' ||
              ch == '-' || ch == '.'))
        {
            return false;
        }
    }

    return true;
}

bool YAMLParser::NormalizeValue(std::string& _value) const
{
    if (_value.empty())
        return true;

    if (_value.size() >= 2)
    {
        char first = _value[0];
        char last = _value[_value.size() - 1];

        if ((first == '\'' && last == '\'') || (first == '"' && last == '"'))
        {
            _value = _value.substr(1, _value.size() - 2);
            return true;
        }
    }

    if (_value[0] == '\'' || _value[0] == '"')
        return false;

    return true;
}

bool YAMLParser::IsUnsupportedValue(const std::string& _value) const
{
    if (_value.empty())
        return false;

    char first = _value[0];
    if (first == '[' || first == '{' || first == '|' || first == '>' ||
        first == '&' || first == '*' || first == '!')
    {
        return true;
    }

    return false;
}

} // namespace forg::script::yaml
