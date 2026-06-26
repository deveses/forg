#ifndef FORG_YAMLSERIALIZER_H
#define FORG_YAMLSERIALIZER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/io/ISerializer.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace forg::io {

class FORG_API YAMLSerializer : public ISerializer
{
  public:
    explicit YAMLSerializer(Mode mode = Mode::Write);
    ~YAMLSerializer() override;

    bool OpenRead(std::string_view filename);
    bool OpenWrite(std::string_view filename);
    bool LoadFromString(std::string_view text);
    bool SaveToString(std::string& text) const;
    bool Flush();
    void Close();

    Mode GetMode() const override;
    void Reset(Mode mode);
    bool ResetReading();

    bool BeginObject(std::string_view name) override;
    bool EndObject() override;

    bool BeginArray(std::string_view name, uint& count) override;
    bool EndArray() override;

    bool Value(std::string_view name, int& value) override;
    bool Value(std::string_view name, uint& value) override;
    bool Value(std::string_view name, float& value) override;
    bool Value(std::string_view name, core::string& value) override;

  private:
    struct ValueEntry
    {
        std::string name;
        std::string value;
    };

    struct Node
    {
        explicit Node(std::string_view nodeName);

        std::string name;
        bool isArray = false;
        std::vector<ValueEntry> values;
        std::vector<std::unique_ptr<Node>> children;
    };

    struct Frame
    {
        Node* node = nullptr;
        uint nextChild = 0;
    };

    Node* FindChild(Node& node, std::string_view name) const;
    ValueEntry* FindValue(Node& node, std::string_view name);
    const ValueEntry* FindValue(const Node& node, std::string_view name) const;
    bool WriteValue(std::string_view name, const std::string& value);
    bool ReadValue(std::string_view name, std::string& value) const;
    bool Parse(std::string_view text);
    void WriteNode(std::string& out, const Node& node, int indent,
                   int arrayIndex = -1) const;

    Mode m_mode;
    std::unique_ptr<Node> m_root;
    std::vector<Frame> m_stack;
    std::string m_filename;
};

} // namespace forg::io

#endif // FORG_YAMLSERIALIZER_H
