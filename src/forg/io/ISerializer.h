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

#ifndef _FORG_IO_ISERIALIZER_H_
#define _FORG_IO_ISERIALIZER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "core/string.hpp"

namespace forg { namespace io {

    class ISerializer
    {
    public:
        virtual ~ISerializer(){}

        virtual bool Begin(const char* _name) = 0;
        virtual void End() = 0;

        virtual bool Read(void* _buffer, uint32 _size) = 0;
        virtual bool ReadInt32(int& _out, const char* _name) = 0;
        virtual bool ReadFloat32(float& _out, const char* _name) = 0;
        virtual bool ReadString(core::string& _out, const char* _name) = 0;
    };

}}

#endif
