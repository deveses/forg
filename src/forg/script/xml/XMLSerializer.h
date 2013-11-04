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

#ifndef _FORG_XMLSERIALIZER_H_
#define _FORG_XMLSERIALIZER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "io/ISerializer.h"
#include "script/xml/XMLParser.h"

namespace forg { namespace io {

    class XMLSerializer : public ISerializer
    {
        forg::script::xml::XMLParser m_parser;

    public:
        XMLSerializer();
        virtual ~XMLSerializer();

        bool Open(const char* _filename);
        void Close();

        // ISerializer
        virtual bool Begin(const char* _name);
        virtual void End();

        virtual bool Read(void* _buffer, uint32 _size);
        virtual bool ReadInt32(int& _out, const char* _name);
        virtual bool ReadFloat32(float& _out, const char* _name);
        virtual bool ReadString(core::string& _out, const char* _name);

    };

}}

#endif
