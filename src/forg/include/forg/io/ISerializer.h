// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
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

    virtual bool BeginArray(std::string_view _name, u32& _count) = 0;
    virtual bool EndArray() = 0;

    virtual bool Value(std::string_view _name, int& _value) = 0;
    virtual bool Value(std::string_view _name, u32& _value) = 0;
    virtual bool Value(std::string_view _name, float& _value) = 0;
    virtual bool Value(std::string_view _name, core::string& _value) = 0;
};

} // namespace forg::io
