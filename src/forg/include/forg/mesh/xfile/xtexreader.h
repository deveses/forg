// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2008 Slawomir Strumecki

#pragma once
#include "mesh/xfile/xlexer.h"
#include "mesh/xfile/xreader.h"
#include <fstream>
#include <iostream>
#include <list>

namespace forg::xfile::reader {

class xtexreader : public xreader
{
  public:
    xtexreader(std::ifstream& input, bool doubleFloat);

  private:
    std::list<ScannerToken> m_last_tokens;
    std::list<ScannerToken> m_next_tokens;
    bool m_bDoubleFloat;
    std::ifstream& m_input;
    XLexer m_lexer;

  public:
    WORD ReadToken();
    int UnreadToken();

    int EvalToken(xstring& value);
    int EvalToken(xguid& value);
    int EvalToken(int& value);
    int EvalToken(FloatList& value);
    int EvalToken(IntegerList& value);

    int ReadInteger(int& value);
    int ReadIntegerList(IntegerList& int_list);
    int ReadFloatList(FloatList& float_list);
    int ReadStringList(StringList& string_list);
    int ReadName(xstring& name);
    int ReadString(xstring& str);
    int ReadGUID(xguid& tguid);
};

} // namespace forg::xfile::reader
