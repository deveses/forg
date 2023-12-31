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

#ifndef XFILE_XTEXREADER_INCLUDED
#define XFILE_XTEXREADER_INCLUDED

#include "mesh/xfile/xreader.h"
#include "mesh/xfile/xlexer.h"
#include <list>
#include <iostream>
#include <fstream>

namespace forg { namespace xfile { namespace reader {

	class xtexreader : public xreader {
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
		int	ReadFloatList(FloatList& float_list);
		int ReadStringList(StringList& string_list);
		int ReadName(xstring& name);
		int ReadString(xstring& str);
		int ReadGUID(xguid& tguid);
	};


}}}

#endif // XFILE_XTEXREADER_INCLUDED
