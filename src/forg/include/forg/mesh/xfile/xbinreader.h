/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2006  Slawomir Strumecki

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

#ifndef XFILE_XBINREADER_INCLUDED
#define XFILE_XBINREADER_INCLUDED

#include "mesh/xfile/xreader.h"

namespace forg { namespace xfile { namespace reader {

	class xbinreader : public xreader {
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
		int	ReadFloatList(FloatList& float_list);
		int ReadStringList(StringList& string_list);
		int ReadName(xstring& name);
		int ReadString(xstring& str);
		int ReadGUID(xguid& tguid);

    protected:
        /// returns true if read failed
        virtual bool read_data(char* buffer, unsigned int count);

        template <typename T>
        int read_value(T& var)
        {
            return read_data((char*)&var, sizeof(T));
        }

        template <typename T>
        int read_value(T* buffer, size_t count)
        {
            return read_data((char*)buffer, sizeof(T)*count);
        }
	};


}}}

#endif // XFILE_XBINREADER_INCLUDED
