#include "forg_pch.h"

#include "forg/io/MemorySerializer.h"

#include <cerrno>
#include <cstdlib>

namespace forg::io {
namespace {

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

} // namespace

MemorySerializer::Node::Node(std::string_view nodeName) : name(nodeName) {}

MemorySerializer::MemorySerializer(Mode mode) : m_mode(mode) {}

MemorySerializer::~MemorySerializer() = default;

ISerializer::Mode MemorySerializer::GetMode() const { return m_mode; }

void MemorySerializer::Reset(Mode mode)
{
    m_mode = mode;
    m_root.reset();
    m_stack.clear();
}

bool MemorySerializer::ResetReading()
{
    if (!m_root)
        return false;

    m_mode = Mode::Read;
    m_stack.clear();
    return true;
}

bool MemorySerializer::BeginObject(std::string_view name)
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
            if (node->name != name)
                return false;
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

bool MemorySerializer::EndObject()
{
    if (m_stack.empty())
        return false;

    m_stack.pop_back();
    return true;
}

bool MemorySerializer::BeginArray(std::string_view name, uint& count)
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

bool MemorySerializer::EndArray()
{
    if (m_stack.empty())
        return false;

    m_stack.pop_back();
    return true;
}

bool MemorySerializer::Value(std::string_view name, int& value)
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

bool MemorySerializer::Value(std::string_view name, uint& value)
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

bool MemorySerializer::Value(std::string_view name, float& value)
{
    if (IsWriting())
        return WriteValue(name, std::to_string(value));

    std::string text;
    return ReadValue(name, text) && parseFloat(text, value);
}

bool MemorySerializer::Value(std::string_view name, core::string& value)
{
    if (IsWriting())
        return WriteValue(name, value.c_str());

    std::string text;
    if (!ReadValue(name, text))
        return false;

    value = text.c_str();
    return true;
}

MemorySerializer::Node* MemorySerializer::FindChild(Node& node,
                                                    std::string_view name) const
{
    for (const std::unique_ptr<Node>& child : node.children)
    {
        if (child && child->name == name)
            return child.get();
    }
    return nullptr;
}

MemorySerializer::ValueEntry* MemorySerializer::FindValue(Node& node,
                                                          std::string_view name)
{
    for (ValueEntry& value : node.values)
    {
        if (value.name == name)
            return &value;
    }
    return nullptr;
}

const MemorySerializer::ValueEntry*
MemorySerializer::FindValue(const Node& node, std::string_view name) const
{
    for (const ValueEntry& value : node.values)
    {
        if (value.name == name)
            return &value;
    }
    return nullptr;
}

bool MemorySerializer::WriteValue(std::string_view name,
                                  const std::string& value)
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

bool MemorySerializer::ReadValue(std::string_view name,
                                 std::string& value) const
{
    if (m_stack.empty())
        return false;

    const ValueEntry* entry = FindValue(*m_stack.back().node, name);
    if (entry == nullptr)
        return false;

    value = entry->value;
    return true;
}

} // namespace forg::io
