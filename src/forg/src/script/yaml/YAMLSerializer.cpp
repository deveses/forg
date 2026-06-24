#include "forg_pch.h"

#include "forg/script/yaml/YAMLSerializer.h"

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace forg::io {
namespace {

std::string trim(std::string_view text)
{
    std::string_view::size_type begin = 0;
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin])))
    {
        begin++;
    }

    std::string_view::size_type end = text.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(text[end - 1])))
    {
        end--;
    }

    return std::string(text.substr(begin, end - begin));
}

bool parseInt(const std::string& text, long& value)
{
    char* end = nullptr;
    errno = 0;
    value = std::strtol(text.c_str(), &end, 10);
    return errno == 0 && end != text.c_str() && *end == 0;
}

bool parseFloat(const std::string& text, float& value)
{
    char* end = nullptr;
    errno = 0;
    value = std::strtof(text.c_str(), &end);
    return errno == 0 && end != text.c_str() && *end == 0;
}

bool hasArrayItemPrefix(std::string_view name)
{
    if (name.size() < 6 || name.substr(0, 5) != "item_")
        return false;

    for (std::string_view::size_type i = 5; i < name.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(name[i])))
            return false;
    }

    return true;
}

std::string quote(std::string_view text)
{
    std::string out;
    out.reserve(text.size() + 2);
    out.push_back('"');
    for (char ch : text)
    {
        if (ch == '"' || ch == '\\')
            out.push_back('\\');
        out.push_back(ch);
    }
    out.push_back('"');
    return out;
}

std::string unquote(std::string text)
{
    if (text.size() < 2 || text.front() != '"' || text.back() != '"')
        return text;

    std::string out;
    out.reserve(text.size() - 2);
    for (std::string::size_type i = 1; i + 1 < text.size(); ++i)
    {
        if (text[i] == '\\' && i + 2 < text.size())
            ++i;
        out.push_back(text[i]);
    }
    return out;
}

} // namespace

YAMLSerializer::Node::Node(std::string_view nodeName) : name(nodeName) {}

YAMLSerializer::YAMLSerializer(Mode mode) : m_mode(mode) {}

YAMLSerializer::~YAMLSerializer() = default;

bool YAMLSerializer::OpenRead(std::string_view filename)
{
    const std::string filenameText(filename);
    std::ifstream input(filenameText);
    if (!input)
        return false;

    std::ostringstream text;
    text << input.rdbuf();
    m_filename = filenameText;
    return LoadFromString(text.str());
}

bool YAMLSerializer::OpenWrite(std::string_view filename)
{
    Reset(Mode::Write);
    m_filename = std::string(filename);
    return true;
}

bool YAMLSerializer::LoadFromString(std::string_view text)
{
    Reset(Mode::Read);
    return Parse(text);
}

bool YAMLSerializer::SaveToString(std::string& text) const
{
    text.clear();
    if (!m_root)
        return false;

    WriteNode(text, *m_root, 0);
    return true;
}

bool YAMLSerializer::Flush()
{
    if (m_filename.empty())
        return false;

    std::string text;
    if (!SaveToString(text))
        return false;

    std::ofstream output(m_filename);
    if (!output)
        return false;

    output << text;
    return static_cast<bool>(output);
}

void YAMLSerializer::Close() { m_filename.clear(); }

ISerializer::Mode YAMLSerializer::GetMode() const { return m_mode; }

void YAMLSerializer::Reset(Mode mode)
{
    m_mode = mode;
    m_root.reset();
    m_stack.clear();
}

bool YAMLSerializer::ResetReading()
{
    if (!m_root)
        return false;

    m_mode = Mode::Read;
    m_stack.clear();
    return true;
}

bool YAMLSerializer::BeginObject(std::string_view name)
{
    if (IsWriting())
    {
        std::unique_ptr<Node> node(new Node(name));
        Node* nodePtr = node.get();

        if (m_stack.empty())
        {
            m_root = std::move(node);
        }
        else
        {
            m_stack.back().node->children.push_back(std::move(node));
        }

        m_stack.push_back({nodePtr, 0});
        return true;
    }

    Node* node = nullptr;
    if (m_stack.empty())
    {
        if (!m_root || m_root->name != name)
            return false;
        node = m_root.get();
    }
    else
    {
        Frame& frame = m_stack.back();
        if (frame.node->isArray)
        {
            if (frame.nextChild >= frame.node->children.size())
                return false;
            node = frame.node->children[frame.nextChild++].get();
        }
        else
        {
            node = FindChild(*frame.node, name);
            if (node == nullptr)
                return false;
        }
    }

    m_stack.push_back({node, 0});
    return true;
}

bool YAMLSerializer::EndObject()
{
    if (m_stack.empty())
        return false;

    m_stack.pop_back();
    return true;
}

bool YAMLSerializer::BeginArray(std::string_view name, uint& count)
{
    if (m_stack.empty())
        return false;

    if (IsWriting())
    {
        std::unique_ptr<Node> node(new Node(name));
        node->isArray = true;
        Node* nodePtr = node.get();
        m_stack.back().node->children.push_back(std::move(node));
        m_stack.push_back({nodePtr, 0});
        return true;
    }

    Node* node = FindChild(*m_stack.back().node, name);
    if (node == nullptr || !node->isArray)
        return false;

    count = static_cast<uint>(node->children.size());
    m_stack.push_back({node, 0});
    return true;
}

bool YAMLSerializer::EndArray()
{
    if (m_stack.empty())
        return false;

    m_stack.pop_back();
    return true;
}

bool YAMLSerializer::Value(std::string_view name, int& value)
{
    if (IsWriting())
        return WriteValue(name, std::to_string(value));

    std::string text;
    long parsed = 0;
    if (!ReadValue(name, text) || !parseInt(text, parsed))
        return false;

    value = static_cast<int>(parsed);
    return true;
}

bool YAMLSerializer::Value(std::string_view name, uint& value)
{
    if (IsWriting())
        return WriteValue(name, std::to_string(value));

    std::string text;
    long parsed = 0;
    if (!ReadValue(name, text) || !parseInt(text, parsed) || parsed < 0)
        return false;

    value = static_cast<uint>(parsed);
    return true;
}

bool YAMLSerializer::Value(std::string_view name, float& value)
{
    if (IsWriting())
        return WriteValue(name, std::to_string(value));

    std::string text;
    return ReadValue(name, text) && parseFloat(text, value);
}

bool YAMLSerializer::Value(std::string_view name, core::string& value)
{
    if (IsWriting())
        return WriteValue(name, value.c_str());

    std::string text;
    if (!ReadValue(name, text))
        return false;

    value = text.c_str();
    return true;
}

YAMLSerializer::Node* YAMLSerializer::FindChild(Node& node,
                                                std::string_view name) const
{
    for (const std::unique_ptr<Node>& child : node.children)
    {
        if (child && child->name == name)
            return child.get();
    }
    return nullptr;
}

YAMLSerializer::ValueEntry* YAMLSerializer::FindValue(Node& node,
                                                      std::string_view name)
{
    for (ValueEntry& value : node.values)
    {
        if (value.name == name)
            return &value;
    }
    return nullptr;
}

const YAMLSerializer::ValueEntry*
YAMLSerializer::FindValue(const Node& node, std::string_view name) const
{
    for (const ValueEntry& value : node.values)
    {
        if (value.name == name)
            return &value;
    }
    return nullptr;
}

bool YAMLSerializer::WriteValue(std::string_view name, const std::string& value)
{
    if (m_stack.empty())
        return false;

    Node& node = *m_stack.back().node;
    ValueEntry* existing = FindValue(node, name);
    if (existing != nullptr)
    {
        existing->value = value;
        return true;
    }

    node.values.push_back({std::string(name), value});
    return true;
}

bool YAMLSerializer::ReadValue(std::string_view name, std::string& value) const
{
    if (m_stack.empty())
        return false;

    const ValueEntry* entry = FindValue(*m_stack.back().node, name);
    if (entry == nullptr)
        return false;

    value = entry->value;
    return true;
}

bool YAMLSerializer::Parse(std::string_view text)
{
    std::vector<std::pair<int, Node*>> stack;

    std::string textCopy(text);
    std::istringstream input(textCopy);
    std::string line;
    while (std::getline(input, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::string stripped = trim(line);
        if (stripped.empty() || stripped[0] == '#')
            continue;

        int indent = 0;
        while (indent < static_cast<int>(line.size()) && line[indent] == ' ')
            indent++;

        if (indent < static_cast<int>(line.size()) && line[indent] == '\t')
            return false;

        std::string::size_type colon = stripped.find(':');
        if (colon == std::string::npos)
            return false;

        std::string key = trim(std::string_view(stripped).substr(0, colon));
        std::string value =
            trim(std::string_view(stripped).substr(colon + 1));
        if (key.empty())
            return false;

        while (!stack.empty() && indent <= stack.back().first)
            stack.pop_back();

        Node* parent = stack.empty() ? nullptr : stack.back().second;
        if (value.empty())
        {
            std::unique_ptr<Node> node(new Node(key));
            if (parent != nullptr && hasArrayItemPrefix(key))
            {
                parent->isArray = true;
                node->name = "item";
            }
            Node* nodePtr = node.get();

            if (parent == nullptr)
            {
                if (m_root)
                    return false;
                m_root = std::move(node);
            }
            else
            {
                parent->children.push_back(std::move(node));
            }

            stack.push_back(std::make_pair(indent, nodePtr));
            continue;
        }

        if (parent == nullptr)
            return false;

        parent->values.push_back({key, unquote(value)});
    }

    return static_cast<bool>(m_root);
}

void YAMLSerializer::WriteNode(std::string& out, const Node& node, int indent,
                               int arrayIndex) const
{
    out.append(static_cast<std::string::size_type>(indent), ' ');
    if (arrayIndex >= 0)
    {
        out.append("item_");
        out.append(std::to_string(arrayIndex));
    }
    else
    {
        out.append(node.name);
    }
    out.append(":\n");

    const int childIndent = indent + 2;
    if (node.isArray)
    {
        out.append(static_cast<std::string::size_type>(childIndent), ' ');
        out.append("count: ");
        out.append(std::to_string(node.children.size()));
        out.push_back('\n');
    }

    for (const ValueEntry& value : node.values)
    {
        out.append(static_cast<std::string::size_type>(childIndent), ' ');
        out.append(value.name);
        out.append(": ");
        out.append(quote(value.value));
        out.push_back('\n');
    }

    int childIndex = 0;
    for (const std::unique_ptr<Node>& child : node.children)
    {
        if (child)
        {
            WriteNode(out, *child, childIndent,
                      node.isArray ? childIndex : -1);
            childIndex++;
        }
    }
}

} // namespace forg::io
