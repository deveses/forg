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

#ifndef XFILE_XTEMPLATE_INCLUDED
#define XFILE_XTEMPLATE_INCLUDED

#include "base.h"
#include "mesh/xfile/xreader.h"
#include "mesh/xfile/xtmembers.h"

namespace forg { namespace xfile {

using namespace xfile::reader;

enum ETemplateStandardType {
	XTEMPLATE_TYPE_UNKNOWN = 0,
	XTEMPLATE_TYPE_ANIMATION,
	XTEMPLATE_TYPE_ANIMATIONKEY,
	XTEMPLATE_TYPE_ANIMATIONOPTIONS,
	XTEMPLATE_TYPE_ANIMATIONSET,
	XTEMPLATE_TYPE_ANIMTICKSPERSECOND,
	XTEMPLATE_TYPE_BOOLEAN,
	XTEMPLATE_TYPE_BOOLEAN2D,
	XTEMPLATE_TYPE_COLORRGB,
	XTEMPLATE_TYPE_COLORRGBA,
	XTEMPLATE_TYPE_COMPRESSEDANIMATIONSET,
	XTEMPLATE_TYPE_COORDS2D,
	XTEMPLATE_TYPE_DECLDATA,
	XTEMPLATE_TYPE_EFFECTDWORD,
	XTEMPLATE_TYPE_EFFECTFLOATS,
	XTEMPLATE_TYPE_EFFECTINSTANCE,
	XTEMPLATE_TYPE_EFFECTPARAMDWORD,
	XTEMPLATE_TYPE_EFFECTPARAMFLOATS,
	XTEMPLATE_TYPE_EFFECTPARAMSTRING,
	XTEMPLATE_TYPE_EFFECTSTRING,
	XTEMPLATE_TYPE_FACEADJACENCY,
	XTEMPLATE_TYPE_FLOATKEYS,
	XTEMPLATE_TYPE_FRAME,
	XTEMPLATE_TYPE_FRAMETRANSFORMMATRIX,
	XTEMPLATE_TYPE_FVFDATA,
	XTEMPLATE_TYPE_GUID,
	XTEMPLATE_TYPE_INDEXEDCOLOR,
	XTEMPLATE_TYPE_MATERIAL,
	XTEMPLATE_TYPE_MATERIALWRAP,
	XTEMPLATE_TYPE_MATRIX4X4,
	XTEMPLATE_TYPE_MESH,
	XTEMPLATE_TYPE_MESHFACE,
	XTEMPLATE_TYPE_MESHFACEWRAPS,
	XTEMPLATE_TYPE_MESHMATERIALLIST,
	XTEMPLATE_TYPE_MESHNORMALS,
	XTEMPLATE_TYPE_MESHTEXTURECOORDS,
	XTEMPLATE_TYPE_MESHVERTEXCOLORS,
	XTEMPLATE_TYPE_PATCH,
	XTEMPLATE_TYPE_PATCHMESH,
	XTEMPLATE_TYPE_PATCHMESH9,
	XTEMPLATE_TYPE_PMATTRIBUTERANGE,
	XTEMPLATE_TYPE_PMINFO,
	XTEMPLATE_TYPE_PMVSPLITRECORD,
	XTEMPLATE_TYPE_SKINWEIGHTS,
	XTEMPLATE_TYPE_TEXTUREFILENAME,
	XTEMPLATE_TYPE_TIMEDFLOATKEYS,
	XTEMPLATE_TYPE_VECTOR,
	XTEMPLATE_TYPE_VERTEXDUPLICATIONINDICES,
	XTEMPLATE_TYPE_VERTEXELEMENT,
	XTEMPLATE_TYPE_XSKINMESHHEADER,
	XTEMPLATE_TYPES_COUNT
};

class XTemplateOption
{
    // 'structors
public:
    XTemplateOption () {};

    XTemplateOption(const xstring& name)
        : m_sName(name)
    {
    }

    XTemplateOption(const xstring& name, const xguid& guid)
        : m_sName(name)
    {
        m_optional_guid = guid;
    }

    // Attributes
private:
    xstring m_sName;
    xguid m_optional_guid;

    // Public Methods
public:
    const xstring& GetName() const { return m_sName; }

    const xguid& GetGuid() const { return m_optional_guid; }

};



class XTemplate
{
    // Nested
public:
    typedef std::vector<XTemplateMember*> XTemplateMemberVector;
    typedef XTemplateMemberVector::iterator XTemplateMemberVectorI;
    typedef XTemplateMemberVector::const_iterator XTemplateMemberVectorCI;

    typedef std::vector<XTemplateOption> XTemplateOptionVector;
    typedef XTemplateOptionVector::iterator XTemplateOptionVectorI;
    typedef XTemplateOptionVector::const_iterator XTemplateOptionVectorCI;

    // 'structors
public:
    XTemplate();
    XTemplate(const char* name, xguid guid, bool _open = false);
    virtual ~XTemplate();

    // Attributes
protected:
    xstring m_sName;    //always valid
    xguid m_guid;       //always valid
    bool m_bOpen;

    XTemplateMemberVector m_members;
    XTemplateOptionVector m_options;

    // Public Methods
public:
    int IsStandardType() const;

    bool IsOpen() const { return m_bOpen; }

    bool IsRestricted() const { return (m_options.size() > 0); }

    uint GetMembersCount() const { return (uint)m_members.size(); }

    uint GetOptionsCount() const { return (uint)m_options.size(); }


    int Load(xreader& treader);

    void AddMember(XTemplateMember* member);

    void AddOption(const xstring& name, const xguid& guid);

    const XTemplateMember* GetMember(uint idx) const;

    const XTemplateOption& GetOption(uint idx) const;

    uint GetMemberIndex(const xstring& name) const;

    const XTemplateMember* FindMemberByName(const xstring& name) const;

    virtual const xstring& GetName() const { return m_sName; }

    virtual const xguid& GetGUID() const { return m_guid; }

    void SetOpenMode(bool mode);

    xstring ToString() const;
};


}}

#endif

