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

#ifndef _FORG_IO_ISERIALIZER_H_
#define _FORG_IO_ISERIALIZER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/core/string.hpp"

#include <string_view>

namespace forg::io {

class ISerializer
{
  public:
    enum class Mode
    {
        Read,
        Write
    };

    virtual ~ISerializer() = default;

    virtual Mode GetMode() const = 0;

    bool IsReading() const { return GetMode() == Mode::Read; }
    bool IsWriting() const { return GetMode() == Mode::Write; }

    virtual bool BeginObject(std::string_view _name) = 0;
    virtual bool EndObject() = 0;

    virtual bool BeginArray(std::string_view _name, uint& _count) = 0;
    virtual bool EndArray() = 0;

    virtual bool Value(std::string_view _name, int& _value) = 0;
    virtual bool Value(std::string_view _name, uint& _value) = 0;
    virtual bool Value(std::string_view _name, float& _value) = 0;
    virtual bool Value(std::string_view _name, core::string& _value) = 0;
};

} // namespace forg::io

#endif
