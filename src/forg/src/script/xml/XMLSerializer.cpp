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

bool XMLSerializer::Begin(const char*) { return false; }

void XMLSerializer::End() {}

bool XMLSerializer::Read(void*, uint32) { return false; }

bool XMLSerializer::ReadFloat32(float&, const char*) { return false; }

bool XMLSerializer::ReadInt32(int&, const char*) { return false; }

bool XMLSerializer::ReadString(core::string&, const char*) { return false; }

} // namespace forg::io
