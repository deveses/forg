// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#include "base.h"
#include "mesh/xfile/xreader.h"
#include "mesh/xfile/xtemplate.h"

#include <map>

namespace forg::xfile {

using namespace xfile::reader;

class XTemplatesMgr
{
    // Nested
  public:
    typedef std::vector<XTemplate*> XTemplateVec;
    typedef XTemplateVec::iterator XTemplateVectorI;
    typedef std::map<xstring, u32> XStringTemplateIndexMap;
    typedef std::map<xguid, u32> XGuidTemplateIndexMap;

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

    // Public methods
  public:
    int ReadTemplates(xreader& treader);
    const XTemplate* Find(const xstring& name);
    const XTemplate* Find(const xguid& guid);

    void PrintTemplates();

    // Helpers
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
    int ReadTemplateOptionPart(xreader& treader, xstring& out_name,
                               xguid& out_guid);
    int ReadTemplateElipsis(xreader& treader, XTemplate* tmpl);
};

} // namespace forg::xfile
