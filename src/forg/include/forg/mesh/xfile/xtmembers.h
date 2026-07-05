// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#include "base.h"
#include "mesh/xfile/xdefs.h"

namespace forg::xfile {

enum xtemplate_member_type
{
    XTEMPLATE_MEMBER_TYPE_UNKNOWN = 0,
    XTEMPLATE_MEMBER_TYPE_PRIMITIVE = 1,
    XTEMPLATE_MEMBER_TYPE_ARRAY = 2,
    XTEMPLATE_MEMBER_TYPE_REFERENCE = 3,
};

class XTemplateMember
{
  public:
    virtual ~XTemplateMember() {};

    virtual bool IsReference() const
    {
        return GetType() == XTEMPLATE_MEMBER_TYPE_REFERENCE;
    };

    virtual bool IsPrimitive() const
    {
        return GetType() == XTEMPLATE_MEMBER_TYPE_PRIMITIVE;
    };

    virtual bool IsArray() const
    {
        return GetType() == XTEMPLATE_MEMBER_TYPE_ARRAY;
    };

    virtual int GetType() const = 0;

    virtual const xstring& GetName() const = 0;

    virtual xstring ToString() const = 0;
};

class XTemplatePrimitive : public XTemplateMember
{
  private:
    int m_iType;
    xstring m_sOptionalName;

  public:
    XTemplatePrimitive(int ptype) : m_iType(ptype) {}

    XTemplatePrimitive(int ptype, const xstring& name)
        : m_iType(ptype), m_sOptionalName(name)
    {
    }

    int GetType() const { return XTEMPLATE_MEMBER_TYPE_PRIMITIVE; };

    xstring ToString() const;

    int GetPrimitiveType() const { return m_iType; }

    const xstring& GetName() const { return m_sOptionalName; }
};

class XTemplateReference : public XTemplateMember
{
  private:
    xstring m_sTemplateName;
    xstring m_sOptionalName;

  public:
    XTemplateReference(const xstring& tmpl_name) : m_sTemplateName(tmpl_name) {}

    XTemplateReference(const xstring& tmpl_name, const xstring& optional_name)
        : m_sTemplateName(tmpl_name), m_sOptionalName(optional_name)
    {
    }

    int GetType() const { return XTEMPLATE_MEMBER_TYPE_REFERENCE; };

    xstring ToString() const;

    const xstring& GetTemplateName() const { return m_sTemplateName; }

    const xstring& GetName() const { return m_sOptionalName; }
};

class XArrayDimension
{
  private:
    bool m_bInteger;
    int m_iValue;
    xstring m_sVariableName;

  public:
    XArrayDimension() : m_bInteger(false) {}

    XArrayDimension(int dim) : m_bInteger(true), m_iValue(dim) {}

    XArrayDimension(const xstring& dim)
        : m_bInteger(false), m_sVariableName(dim)
    {
    }

    void Set(int value)
    {
        m_bInteger = true;
        m_iValue = value;
    }

    void Set(const xstring& value)
    {
        m_bInteger = false;
        m_sVariableName = value;
    }

    bool IsConstant() const { return m_bInteger; }

    int GetValue() const { return m_iValue; }

    xstring GetVariableName() const { return m_sVariableName; }
};

class XTemplateArray : public XTemplateMember
{
    // Nested
  public:
    typedef std::vector<XArrayDimension> XArrayDimensionVector;

  private:
    XArrayDimensionVector m_dimension_list;

    bool m_bPrimitive;
    ETemplatePrimitiveType::TYPE m_iPrimitiveType;
    xstring m_sTypeName;
    xstring m_sName;

  public:
    XTemplateArray() : m_bPrimitive(false) {}

    XTemplateArray(const xstring& _name, const xstring& _type,
                   const xstring& _dimension)
        : m_bPrimitive(false)
    {
        m_sName = _name;
        m_sTypeName = _type;

        AddDimension(_dimension);
    }

    XTemplateArray(const xstring& _name, ETemplatePrimitiveType::TYPE _type,
                   int _dimension)
        : m_bPrimitive(true), m_iPrimitiveType(_type)
    {
        m_sName = _name;
        AddDimension(_dimension);
    }

    XTemplateArray(const xstring& _name, ETemplatePrimitiveType::TYPE _type,
                   const xstring& _dimension)
        : m_bPrimitive(true), m_iPrimitiveType(_type)
    {
        m_sName = _name;

        AddDimension(_dimension);
    }

    int GetType() const { return XTEMPLATE_MEMBER_TYPE_ARRAY; };

    void SetArrayType(ETemplatePrimitiveType::TYPE _type)
    {
        m_bPrimitive = true;
        m_iPrimitiveType = _type;
    }

    void SetArrayType(const xstring& sType)
    {
        m_bPrimitive = false;
        m_sTypeName = sType;
    }

    void SetName(const xstring& sName) { m_sName = sName; }

    const xstring& GetName() const { return m_sName; }

    void AddDimension(int iDim) { m_dimension_list.push_back(iDim); }

    void AddDimension(const xstring& sDim) { m_dimension_list.push_back(sDim); }

    xstring ToString() const;

    bool IsPrimitiveType() const { return m_bPrimitive; }

    int GetArrayType() const { return m_iPrimitiveType; };

    const xstring& GetArrayTypeName() const { return m_sTypeName; };

    u32 GetDimensionListSize() const { return (u32)m_dimension_list.size(); }

    const XArrayDimension* GetDimension(u32 index) const
    {
        return &m_dimension_list[index];
    }
};

} // namespace forg::xfile
