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

#ifndef _FORG_XMLPARSER_H_
#define _FORG_XMLPARSER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/vector.hpp"
#include "os/File.h"

namespace forg { namespace script { namespace xml {

    struct SToken
    {
        int token_id;
    };

    struct SParserState
    {
        int char_read;
        int symbol;
        int line;
        int column;

        void Reset()
        {
            char_read = 0;
            line = 0;
            column = 0;
        }
    };

    class FORG_API XMLParser
    {
        typedef core::vector<SToken> TokenVec;

    private:
        forg::os::File m_file;

        TokenVec m_tokens;
        SParserState m_state;

    public:
        XMLParser();
        ~XMLParser();

        bool Open(const char* _filename);
        bool Parse();
        void Close();

    private:
        int GetToken();
        int GetSymbol(int _ch);
        void EatWhitespace();
        bool ReadTokens();
    };

}}}

#endif
