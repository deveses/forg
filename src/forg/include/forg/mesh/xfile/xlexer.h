/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2008  Slawomir Strumecki

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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef XFILE_XLEXER_INCLUDED
#define XFILE_XLEXER_INCLUDED

#include <base.h>
#include "mesh/xfile/xdefs.h"

#include <string>
#include <vector>
#include <list>

namespace forg { namespace xfile {

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
        int GetToken( ScannerToken& stok );

        void SetInput( std::ifstream* _input ) { m_state.yyinput = _input; };

        private:
        void Initialize();
    };

}}

#endif
