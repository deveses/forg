// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2008 Slawomir Strumecki

#pragma once
#include "mesh/xfile/xdefs.h"
#include <base.h>

#include <list>
#include <string>
#include <vector>

namespace forg::xfile {

struct ScannerToken
{
    // change EToken::TYPE to int?
    EToken::TYPE token;

    std::string lexem;

    int line;
    int col;
};

class XLexer
{
    // Nested
  private:
    struct LexerState
    {
        std::ifstream* yyinput;
        int yyline;
        int yycol;
        char yychar;
        std::string yylexem;

        void Reset()
        {
            yyline = yycol = 0;
            yychar = 0;
            yyinput = 0;
        }
    };

  public:
    XLexer();

    // Attributes
  private:
    LexerState m_state;
    bool m_initialized;

    // Public methods
  public:
    int GetToken(ScannerToken& stok);

    void SetInput(std::ifstream* _input) { m_state.yyinput = _input; };

  private:
    void Initialize();
};

} // namespace forg::xfile
