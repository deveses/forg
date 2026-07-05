// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/io/ISerializer.h"
#include "forg/script/xml/XMLParser.h"

namespace forg::io {

class XMLSerializer : public ISerializer
{
    forg::script::xml::XMLParser m_parser;

  public:
    XMLSerializer();
    virtual ~XMLSerializer();

    bool Open(const char* _filename);
    void Close();

    // ISerializer
    Mode GetMode() const override;

    bool BeginObject(std::string_view _name) override;
    bool EndObject() override;

    bool BeginArray(std::string_view _name, u32& _count) override;
    bool EndArray() override;

    bool Value(std::string_view _name, int& _value) override;
    bool Value(std::string_view _name, u32& _value) override;
    bool Value(std::string_view _name, float& _value) override;
    bool Value(std::string_view _name, core::string& _value) override;
};

} // namespace forg::io
