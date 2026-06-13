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

#ifndef XFILE_XREADER_INCLUDED
#define XFILE_XREADER_INCLUDED

#include "mesh/xfile/xdefs.h"

namespace forg { namespace xfile { namespace reader {

	class xreader {
    public:
        virtual ~xreader(){}


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

		virtual int	ReadFloatList(FloatList& float_list) = 0;
		virtual int ReadStringList(StringList& string_list) = 0;
		virtual int ReadName(xstring& name) = 0;
		virtual int ReadString(xstring& str) = 0;
		virtual int ReadGUID(xguid& tguid) = 0;

        int ReadPrimitiveType(ETemplatePrimitiveType::TYPE& primitive_type);
        int ReadIntegers(IntegerList& int_list, DWORD count);
        int ReadFloats(FloatList& float_list, DWORD count);
	};


}}}

#endif
