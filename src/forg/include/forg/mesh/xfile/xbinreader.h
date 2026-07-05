// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#include "mesh/xfile/xreader.h"

namespace forg::xfile::reader {

class xbinreader : public xreader
{
  public:
    xbinreader(std::ifstream& input, bool doubleFloat);

  protected:
    std::list<WORD> m_last_tokens;
    std::list<WORD> m_next_tokens;
    bool m_bDoubleFloat;
    std::ifstream& m_input;

  public:
    WORD ReadToken();
    int UnreadToken();
    int ReadInteger(int& value);
    int ReadIntegerList(IntegerList& int_list);
    int ReadFloatList(FloatList& float_list);
    int ReadStringList(StringList& string_list);
    int ReadName(xstring& name);
    int ReadString(xstring& str);
    int ReadGUID(xguid& tguid);

  protected:
    /// returns true if read failed
    virtual bool read_data(char* buffer, unsigned int count);

    template <typename T> int read_value(T& var)
    {
        return read_data((char*)&var, sizeof(T));
    }

    template <typename T> int read_value(T* buffer, size_t count)
    {
        return read_data((char*)buffer, sizeof(T) * count);
    }
};

} // namespace forg::xfile::reader
