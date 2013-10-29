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

#ifndef _XDATAMGR_H_
#define _XDATAMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "mesh/xfile/xreader.h"
#include "mesh/xfile/xdata.h"

namespace forg { namespace xfile {

    class XTemplatesMgr;
    class XTemplateArray;
    class XTemplatePrimitive;

    class XDataMgr {
        // Nested
    public:
        typedef std::vector<XDataObject*> XDataObjectVector;
        typedef XDataObjectVector::iterator XDataObjectVectorI;

        // 'structors
    public:
        XDataMgr();
        ~XDataMgr();

        // Attributes
    private:
        XDataObjectVector m_data;

        // Public Methods
    public:
        int ReadData(xreader& treader, XTemplatesMgr& tmpl_mgr);
        void PrintInfo() const;
        uint GetDataObjectsCount() const;
        const XDataObject* GetDataObject(uint nIndex) const;

        // Helpers
    private:
        void PrintInfo(const IData* data, int indent) const;
        void PrintInfo(const XDataObject *data, int indent) const;
        void PrintInfo(const XDataFloatList *data, int indent) const;
        void PrintInfo(const XDataIntegerList *data, int indent) const;
        void PrintInfo(const XDataStringList *data, int indent) const;
        void PrintInfo(const XDataReference *data, int indent) const;
    };

}}

#endif  //_XDATAMGR_H_
