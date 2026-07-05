// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#include "base.h"
#include "mesh/xfile/xdefs.h"
#include "mesh/xfile/xreader.h"

namespace forg::xfile {

using namespace xfile::reader;

enum EDataObjectType
{
    EDataObjectType_Unknown = 0,
    EDataObjectType_IntegerList = 1,
    EDataObjectType_FloatList = 2,
    EDataObjectType_StringList = 3,
    EDataObjectType_DataReference = 4,
    EDataObjectType_Object = 5,
    EDataObjectType_Primitive = 6,
    EDataObjectType_Array = 7,
};

class XTemplatesMgr;
class XTemplate;
class XTemplatePrimitive;
class XTemplateArray;
/************************************************************************/

class XDataIdentifier
{
  public:
    XDataIdentifier();

  private:
    xstring m_sName;
    ETemplatePrimitiveType::TYPE m_iPrimitiveType;
    bool m_bPrimitive;

  public:
    void Set(const xstring& sName)
    {
        m_sName = sName;
        m_bPrimitive = false;
    }

    void Set(ETemplatePrimitiveType::TYPE iType)
    {
        m_iPrimitiveType = iType;
        m_bPrimitive = true;
    }

    bool IsPrimitiveType() const { return m_bPrimitive; }

    const xstring& GetName() const { return m_sName; }

    int GetType() const { return m_iPrimitiveType; }

    xstring ToString() const;

    int Load(xreader& reader);
};

/************************************************************************/

class IData
{
  public:
    virtual ~IData() {};

    virtual int GetDataType() const = 0;

    virtual u32 GetSize() const = 0;

    virtual xguid GetGUID() const = 0;

    virtual u32 GetSubdataSize() const = 0;

    virtual const IData* GetSubdata(u32 index) const = 0;

    virtual void ToByteArray(void* /*buffer*/, u32 /*buffer_size*/) const {};

    bool IsObject() const { return GetDataType() == EDataObjectType_Object; }

    bool IsArray() const { return GetDataType() == EDataObjectType_Array; }

    const IData* FindObject(const xguid& guid) const
    {
        u32 c = GetSubdataSize();
        for (u32 i = 0; i < c; i++)
        {
            const IData* sub = GetSubdata(i);

            if (sub->GetGUID() == guid)
            {
                return sub;
            }
        }

        return NULL;
    }
};

/************************************************************************/

class XDataPrimitive : public IData
{
  public:
    enum EPrimitiveType
    {
        EPrimitiveType_Integer,
        EPrimitiveType_Float,
        EPrimitiveType_Double,
        EPrimitiveType_String
    };

    union
    {
        int iValue;
        float fValue;
        double dValue;
    };
    xstring strValue;
    u32 m_size;
    EPrimitiveType m_type;

  public:
    void SetInteger(int v)
    {
        iValue = v;
        m_size = 4;
        m_type = EPrimitiveType_Integer;
    }
    void SetFloat(float v)
    {
        fValue = v;
        m_size = 4;
        m_type = EPrimitiveType_Float;
    }
    void SetDouble(double v)
    {
        dValue = v;
        m_size = 8;
        m_type = EPrimitiveType_Double;
    }
    void SetString(const xstring& v)
    {
        strValue = v;
        m_size = (u32)v.size() + 1;
        m_type = EPrimitiveType_String;
    }

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_Primitive; }

    u32 GetSize() const { return m_size; };

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return 0; };

    virtual const IData* GetSubdata(u32 /*index*/) const { return 0; };

    virtual void ToByteArray(void* buffer, u32 buffer_size) const;
};

/************************************************************************/

class XDataIntegerList : public IData
{
  private:
    IntegerList m_aIntegers;

  public:
    void Set(IntegerList& aIntegers) { m_aIntegers = aIntegers; }

    const IntegerList& GetIntegers() const { return m_aIntegers; }

    IntegerList& GetIntegers() { return m_aIntegers; }

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_IntegerList; };

    u32 GetSize() const { return (u32)m_aIntegers.size() * 4; }

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return 0; };

    virtual const IData* GetSubdata(u32 /*index*/) const { return 0; };

    virtual void ToByteArray(void* buffer, u32 buffer_size) const;
};

/************************************************************************/

class XDataFloatList : public IData
{
  private:
    FloatList m_aFloats;

  public:
    void Set(FloatList& aFloats) { m_aFloats = aFloats; }

    FloatList& GetFloats() { return m_aFloats; }

    const FloatList& GetFloats() const { return m_aFloats; }

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_FloatList; };

    u32 GetSize() const { return (u32)m_aFloats.size() * 4; }

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return 0; };

    virtual const IData* GetSubdata(u32 /*index*/) const { return 0; };

    virtual void ToByteArray(void* buffer, u32 buffer_size) const;
};

/************************************************************************/

class XDataStringList : public IData
{

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_StringList; };

    u32 GetSize() const { return 0; }

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return 0; };

    virtual const IData* GetSubdata(u32 /*index*/) const { return 0; };
};

/************************************************************************/

class XDataObjectList : public IData
{
    // Nested
  public:
    typedef std::vector<IData*> XDataVector;
    typedef XDataVector::iterator XDataVectorI;
    typedef XDataVector::const_iterator XDataVectorCI;

  private:
    XDataVector m_objects;
    u32 m_size;

  public:
    XDataObjectList() : m_size(0) {}

    virtual ~XDataObjectList()
    {
        for (XDataVectorI iter = m_objects.begin(); iter != m_objects.end();
             ++iter)
        {
            delete *iter;
        }
    }

    void AddObject(IData* object)
    {
        m_objects.push_back(object);

        m_size += object->GetSize();
    }

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_Array; }

    u32 GetSize() const { return m_size; };

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return (u32)m_objects.size(); };

    virtual const IData* GetSubdata(u32 index) const
    {
        return m_objects[index];
    };

    virtual void ToByteArray(void* buffer, u32 buffer_size) const;
};

/************************************************************************/

class XDataReference : public IData
{
  public:
    XDataReference(const xstring& name)
    {
        m_sName = name;
        m_optional_guid = xguid::Empty;
    }

    XDataReference(const xstring& name, const xguid& guid)
    {
        m_sName = name;
        m_optional_guid = guid;
    }

  private:
    xstring m_sName;
    xguid m_optional_guid;

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_DataReference; };

    u32 GetSize() const { return 0; }

    xguid GetGUID() const { return xguid::Empty; }

    virtual u32 GetSubdataSize() const { return 0; };

    virtual const IData* GetSubdata(u32 /*index*/) const { return 0; };
};

/************************************************************************/

class XDataObject : public IData
{
    // Nested
  public:
    typedef std::vector<IData*> XDataVector;
    typedef XDataVector::iterator XDataVectorI;
    typedef XDataVector::const_iterator XDataVectorCI;

    // 'structors
  public:
    XDataObject();
    virtual ~XDataObject();

    // Attributes
  private:
    XDataIdentifier m_identifier; ///< template / data type
    xstring m_optional_name;      ///< object name
    xguid m_optional_guid;

    XDataVector m_subdata;

    u32 m_size; ///< size in bytes

    // Associations
  private:
    const XTemplate* m_template;

    // Public Methods
  public:
    void AddSubData(IData* pData);

    void SetIdentifier(const xstring& sIdent) { m_identifier.Set(sIdent); }

    void SetIdentifier(ETemplatePrimitiveType::TYPE nIdent)
    {
        m_identifier.Set(nIdent);
    }

    void SetName(const xstring& sName) { m_optional_name = sName; }

    void SetGUID(const xguid& guid) { m_optional_guid = guid; }

    const xstring& GetName() const { return m_optional_name; }

    const XDataIdentifier& GetIdentifier() const { return m_identifier; }

    int Load(xreader& reader, XTemplatesMgr& tmpl_mgr);

    //////////////////////////////////////////////////////////////////////////
    // IData implementation
    //////////////////////////////////////////////////////////////////////////
  public:
    int GetDataType() const { return EDataObjectType_Object; }

    u32 GetSize() const { return m_size; }

    xguid GetGUID() const;

    u32 GetSubdataSize() const { return (u32)m_subdata.size(); };

    const IData* GetSubdata(u32 index) const;

    void ToByteArray(void* buffer, u32 buffer_size) const;
    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
  private:
    int ReadMembers(xreader& reader, XTemplatesMgr& tmpl_mgr);
    int ReadPrimitive(xreader& reader, int primitive_type, u32 count = 1);
    int ReadArray(xreader& reader, XTemplatesMgr& tmpl_mgr,
                  const XTemplateArray* xarray);
    int ReadDataPart(xreader& reader, XTemplatesMgr& tmpl_mgr);
    int ReadDataReference(xreader& treader);
    int ReadSubObject(xreader& reader, XTemplatesMgr& tmpl_mgr,
                      const xstring& type, const xstring& name,
                      const xguid& guid);
};

} // namespace forg::xfile
