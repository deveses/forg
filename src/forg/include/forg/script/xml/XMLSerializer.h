/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef _FORG_XMLSERIALIZER_H_
#define _FORG_XMLSERIALIZER_H_

#if _MSC_VER > 1000
#pragma once
#endif

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

    bool BeginArray(std::string_view _name, uint& _count) override;
    bool EndArray() override;

    bool Value(std::string_view _name, int& _value) override;
    bool Value(std::string_view _name, uint& _value) override;
    bool Value(std::string_view _name, float& _value) override;
    bool Value(std::string_view _name, core::string& _value) override;
};

} // namespace forg::io

#endif
