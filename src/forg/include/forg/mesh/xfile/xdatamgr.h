// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "base.h"
#include "mesh/xfile/xdata.h"
#include "mesh/xfile/xreader.h"

namespace forg::xfile {

class XTemplatesMgr;
class XTemplateArray;
class XTemplatePrimitive;

class XDataMgr
{
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
    u32 GetDataObjectsCount() const;
    const XDataObject* GetDataObject(u32 nIndex) const;

    // Helpers
  private:
    void PrintInfo(const IData* data, int indent) const;
    void PrintInfo(const XDataObject* data, int indent) const;
    void PrintInfo(const XDataFloatList* data, int indent) const;
    void PrintInfo(const XDataIntegerList* data, int indent) const;
    void PrintInfo(const XDataStringList* data, int indent) const;
    void PrintInfo(const XDataReference* data, int indent) const;
};

} // namespace forg::xfile
