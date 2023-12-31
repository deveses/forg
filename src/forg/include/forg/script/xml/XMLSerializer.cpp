#include "forg_pch.h"
#include "XMLSerializer.h"

namespace forg { namespace io {

XMLSerializer::XMLSerializer()
{}

XMLSerializer::~XMLSerializer()
{
    Close();
}

bool XMLSerializer::Open(const char* _filename)
{
    return m_parser.Open(_filename);
}

void XMLSerializer::Close()
{
    m_parser.Close();
}

bool XMLSerializer::Begin(const char* _name)
{
    return false;
}

void XMLSerializer::End()
{
}

bool XMLSerializer::Read(void* _buffer, uint32 _size)
{
    return false;

}

bool XMLSerializer::ReadFloat32(float& _out, const char* _name)
{
    return false;
}

bool XMLSerializer::ReadInt32(int& _out, const char* _name)
{
    return false;
}

bool XMLSerializer::ReadString(core::string& _out, const char* _name)
{
    return false;
}

}}