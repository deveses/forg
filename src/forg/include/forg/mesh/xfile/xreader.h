// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#include "mesh/xfile/xdefs.h"

namespace forg::xfile::reader {

class xreader
{
  public:
    virtual ~xreader() {}

  private:
    IntegerList m_Integers;
    IntegerListI m_IntBegin;

    FloatList m_Floats;
    FloatListI m_FloatBegin;

  public:
    virtual WORD ReadToken() = 0;
    virtual int UnreadToken() = 0;

    virtual int ReadInteger(int& value) = 0;
    virtual int ReadIntegerList(IntegerList& int_list) = 0;

    virtual int ReadFloatList(FloatList& float_list) = 0;
    virtual int ReadStringList(StringList& string_list) = 0;
    virtual int ReadName(xstring& name) = 0;
    virtual int ReadString(xstring& str) = 0;
    virtual int ReadGUID(xguid& tguid) = 0;

    int ReadPrimitiveType(ETemplatePrimitiveType::TYPE& primitive_type);
    int ReadIntegers(IntegerList& int_list, DWORD count);
    int ReadFloats(FloatList& float_list, DWORD count);
};

} // namespace forg::xfile::reader
