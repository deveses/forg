#include "forg/script/xml/XMLSerializer.h"
#include "forg_pch.h"

namespace forg::io {

XMLSerializer::XMLSerializer() {}

XMLSerializer::~XMLSerializer() { Close(); }

bool XMLSerializer::Open(const char* _filename)
{
    return m_parser.Open(_filename);
}

void XMLSerializer::Close() { m_parser.Close(); }

ISerializer::Mode XMLSerializer::GetMode() const { return Mode::Read; }

bool XMLSerializer::BeginObject(std::string_view) { return false; }

bool XMLSerializer::EndObject() { return false; }

bool XMLSerializer::BeginArray(std::string_view, uint&) { return false; }

bool XMLSerializer::EndArray() { return false; }

bool XMLSerializer::Value(std::string_view, int&) { return false; }

bool XMLSerializer::Value(std::string_view, uint&) { return false; }

bool XMLSerializer::Value(std::string_view, float&) { return false; }

bool XMLSerializer::Value(std::string_view, core::string&) { return false; }

} // namespace forg::io
