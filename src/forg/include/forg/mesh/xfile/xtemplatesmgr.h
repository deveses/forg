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

#include "base.h"
#include "mesh/xfile/xreader.h"
#include "mesh/xfile/xtemplate.h"

#include <map>

namespace forg { namespace xfile {

using namespace xfile::reader;

class XTemplatesMgr
{
    // Nested
public:
    typedef std::vector<XTemplate*> XTemplateVec;
    typedef XTemplateVec::iterator XTemplateVectorI;
    typedef std::map<xstring, uint> XStringTemplateIndexMap;
    typedef std::map<xguid, uint> XGuidTemplateIndexMap;

    typedef std::map<xstring, XTemplate*> XStringTemplateMap;
    typedef XStringTemplateMap::iterator XStringTemplateMapI;

    // 'structors
public:
    XTemplatesMgr();
    ~XTemplatesMgr();


    // Attributes
    XTemplateVec m_templates;
    XStringTemplateIndexMap m_string_map;
    XGuidTemplateIndexMap m_guid_map;

    XStringTemplateMap m_std_string_map;

    //Public methods
public:
    int ReadTemplates(xreader& treader);
    const XTemplate* Find(const xstring& name);
    const XTemplate* Find(const xguid& guid);

    void PrintTemplates();

    //Helpers
private:
    int CheckStandardType(const xguid& guid);
    int ReadTemplate(xreader& treader, XTemplate** tmpl);

    int ReadTemplateParts(xreader& treader, XTemplate* tmpl);
    int ReadTemplateMembersList(xreader& treader, XTemplate* tmpl);
    int ReadTemplateMembers(xreader& treader, XTemplateMember** member);
    int ReadTemplatePrimitive(xreader& treader, XTemplateMember** member);
    int ReadTemplateArray(xreader& treader, XTemplateMember** member);
    int ReadTemplateArrayDataType(xreader& treader, XTemplateArray* arr);
    int ReadTemplateArrayDimensionList(xreader& treader, XTemplateArray* arr);
    int ReadTemplateArrayDimensionSize(xreader& treader, XTemplateArray* arr);
    int ReadTemplateArrayDimension(xreader& treader, XTemplateArray* arr);
    int ReadTemplateReference(xreader& treader, XTemplateMember** member);
    int ReadTemplateOptionInfo(xreader& treader, XTemplate* tmpl);
    int ReadTemplateOptionList(xreader& treader, XTemplate* tmpl);
    int ReadTemplateOptionPart(xreader& treader, xstring& out_name, xguid& out_guid);
    int ReadTemplateElipsis(xreader& treader, XTemplate* tmpl);
};

}}
