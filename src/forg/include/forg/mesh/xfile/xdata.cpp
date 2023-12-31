#include "forg_pch.h"

#include "mesh/xfile/xdata.h"
#include "mesh/xfile/xtemplatesmgr.h"
#include "debug/dbg.h"

namespace forg { namespace xfile {

    //////////////////////////////////////////////////////////////////////////
    // XDataObject
    //////////////////////////////////////////////////////////////////////////

    XDataObject::XDataObject()
    {
        m_optional_guid = xguid::Empty;
        m_template = NULL;
        m_size = 0;
    }

    XDataObject::~XDataObject()
    {
        for (XDataVectorI iter = m_subdata.begin(); iter != m_subdata.end(); ++iter)
        {
            delete *iter;
        }
    }

    int XDataObject::Load(xreader& reader, XTemplatesMgr& tmpl_mgr)
    {
        //	//object                : identifier optional_name EToken::TOKEN_OBRACE
        //	//							optional_class_id
        //	//							data_parts_list
        //	//							EToken::TOKEN_CBRACE

        xstring opt_name;
        xguid opt_guid = xguid::Empty;

        int rval =  m_identifier.Load(reader);
        if (rval > 0)
            DBG_TRACE_ERR("[XDataMgr::ReadDataObject] ReadIdentifier", rval);

        if (rval) return rval;

        if (EXECUTE_ASSERT( reader.ReadName(m_optional_name) > 0) ) rval = 1;

        if (! m_optional_name.empty())
            DBG_MSG("[XDataObject::Load] loading object: %s (%s)\n", m_optional_name.c_str(), m_identifier.GetName().c_str());
        else 
            DBG_MSG("[XDataObject::Load] loading object: %s (%s)\n", "unnamed", m_identifier.GetName().c_str());

        if (EXECUTE_ASSERT( reader.ReadToken() != EToken::TOKEN_OBRACE) ) rval = 1;
        if (EXECUTE_ASSERT( reader.ReadGUID(m_optional_guid) > 0) ) rval = 1;
        if (EXECUTE_ASSERT( ReadMembers( reader, tmpl_mgr) )) rval = 1;
        if (EXECUTE_ASSERT( reader.ReadToken() != EToken::TOKEN_CBRACE) ) rval = 1;

        if (rval > 0)
            DBG_TRACE_ERR("[XDataObject::Load]", rval);


        if (! m_optional_name.empty())
            DBG_MSG("[XDataObject::Load] loaded object: %s (%s) size: %d\n", m_optional_name.c_str(), m_identifier.GetName().c_str(), GetSize());
        else
            DBG_MSG("[XDataObject::Load] loaded object: %s (%s) size: %d\n", "unnamed", m_identifier.GetName().c_str(), GetSize());

        return rval;
    }

    int XDataObject::ReadMembers(xreader& reader, XTemplatesMgr& tmpl_mgr)
    {
        //data_parts_list       : data_part
        //							| data_parts_list data_part
        //

        int list_size = 0;
        int rval = 0;

        // find template (description) for this object
        m_template = tmpl_mgr.Find(m_identifier.GetName());

        if (m_template == NULL)
        {
            DBG_MSG("[XDataObject::ReadMembers] Undefined template \"%s\"!\n", m_identifier.GetName().c_str());

            return 1;
        }

        for (uint i=0; i<m_template->GetMembersCount(); i++)
        {
            const XTemplateMember* field = m_template->GetMember(i);

            // primitive type member
            if (field->IsPrimitive())
            {
                EXECUTE_ASSERT( ReadPrimitive(reader, static_cast<const XTemplatePrimitive*>(field)->GetPrimitiveType() ) != 0 );
            } else
            // reference, object aggregation
            if (field->IsReference())
            {
                const xstring& type = static_cast<const XTemplateReference*>(field)->GetTemplateName();
                const xstring name = static_cast<const XTemplateReference*>(field)->GetName();

                rval = ReadSubObject(reader, tmpl_mgr, type, name, xguid::Empty);
            } else
            // array of primitives or objects
            if (field->IsArray())
            {
                EXECUTE_ASSERT( ReadArray(reader, tmpl_mgr, static_cast<const XTemplateArray*>(field)) > 0);
            } else
            {
                DBG_TRACE_ERR("Unknown member type!", 1);
            }

            if (reader.ReadToken() != EToken::TOKEN_SEMICOLON)
            {
                reader.UnreadToken();
            }
        }

        // if restricted, read specified objects
        if (m_template->IsRestricted())
        {
            // read additional members (specified in template)
            // TODO: check if object are valid (they must be no option list)
            do
            {
                rval = ReadDataPart(reader, tmpl_mgr);
            } while (rval == 0);

            if (rval > 0)
                DBG_TRACE_ERR("Reading restricted data", rval);
            else
                rval = 0;    // was set to -1

        } else
        // if open, read additional data
        if (m_template->IsOpen())
        {
            do
            {
                //std::auto_ptr<IData> data_part;

                rval = ReadDataPart(reader, tmpl_mgr);

                if (rval == 0)
                {
                    list_size++;

                    //data_list.push_back(data_part.release());
                }

            } while (rval == 0);

            // if there is no optional part
            if (rval < 0)
            {
                rval = 0;
            }
        }

        return rval > 0 ? rval : ((list_size == 0) ? rval : 0);
    }

    int XDataObject::ReadSubObject(xreader& reader, XTemplatesMgr& tmpl_mgr, const xstring& type, const xstring& name, const xguid& guid)
    {
        int rval = 0;

        std::auto_ptr< XDataObject > aptr_object( new XDataObject() );

        aptr_object->m_identifier.Set(type);
        aptr_object->m_optional_guid = guid;
        aptr_object->m_optional_name = name;

        rval = aptr_object->ReadMembers(reader, tmpl_mgr);

        if (rval > 0)
            DBG_TRACE_ERR("Failed to read object members", rval);

        if (rval == 0)
        {
            AddSubData(aptr_object.release());
        }

        return rval;
    }

    int XDataObject::ReadPrimitive(xreader& reader, int primitive_type, uint count)
    {
        int rval = 0;
        static StringList aStrings;

        std::auto_ptr< IData > aptr_data;

        switch(primitive_type)
        {
        case ETemplatePrimitiveType::Word:
        case ETemplatePrimitiveType::Dword:
        case ETemplatePrimitiveType::Char:
        case ETemplatePrimitiveType::UChar:
        case ETemplatePrimitiveType::SWord:
        case ETemplatePrimitiveType::SDword:
            {
                // read array of integer types
                if (count > 1)
                {
                    std::auto_ptr<XDataIntegerList> parray( new XDataIntegerList() );

                    rval = reader.ReadIntegers(parray->GetIntegers(), count);
                    //parray->Set(aIntergers);

                    if (rval == 0)
                        aptr_data = parray;
                }
                // read single int
                else
                {
                    IntegerList aIntergers;
                    rval = reader.ReadIntegers(aIntergers, 1);

                    if (rval == 0)
                    {
                        std::auto_ptr<XDataPrimitive> psingle( new XDataPrimitive() );

                        psingle->SetInteger(aIntergers.front());

                        aptr_data = psingle;
                    }
                }

                break;
            }

        case ETemplatePrimitiveType::Float:
        case ETemplatePrimitiveType::Double:
            {
                if (count > 1)
                {
                    std::auto_ptr<XDataFloatList> parray( new XDataFloatList() );

                    rval = reader.ReadFloats(parray->GetFloats(), count);

                    if (rval == 0)
                        aptr_data = parray;
                } else
                {
                    FloatList aFloats;
                    rval = reader.ReadFloats(aFloats, 1);

                    if (rval == 0)
                    {
                        std::auto_ptr<XDataPrimitive> psingle( new XDataPrimitive() );

                        psingle->SetFloat(aFloats.front());

                        aptr_data = psingle;
                    }
                }
            }

            break;

        case ETemplatePrimitiveType::Lpstr:
        case ETemplatePrimitiveType::Unicode:
        case ETemplatePrimitiveType::CString:
            if (aStrings.size() == 0)
                rval = reader.ReadStringList(aStrings);

            if (rval == 0 && aStrings.size())
            {
                if (count > 1)
                {

                } else
                {
                    std::auto_ptr<XDataPrimitive> psingle( new XDataPrimitive() );

                    psingle->SetString(aStrings.front());
                    aStrings.pop_front();

                    aptr_data = psingle;
                }
            }
            break;

        default:
            rval = 1;
        }

        if (rval == 0 && aptr_data.get())
            AddSubData(aptr_data.release());

        return rval;
    }

    int XDataObject::ReadArray(xreader& reader, XTemplatesMgr& tmpl_mgr, const XTemplateArray* xarray)
    {
        //uint dim_count = xarray->GetDimensionListSize();
        const XArrayDimension* dim = xarray->GetDimension(0);
        uint array_size = 0;
        int rval = 0;

        if (dim->IsConstant())
        {
            array_size = (uint)dim->GetValue();
        } else
        {
            xstring var = dim->GetVariableName();

            uint idx = m_template->GetMemberIndex(var);

            if (idx < m_template->GetMembersCount() && idx < m_subdata.size())
            {
                IData* var_data = m_subdata[idx];

                if (var_data->GetDataType() == EDataObjectType_Primitive)
                {
                    array_size = static_cast<XDataPrimitive*>(var_data)->iValue;
                } else
                {
                    DBG_TRACE_ERR("[XDataObject::ReadArray] Only integer array dimension is supported!", 1);

                    return 1;
                }

            } else
            {
                DBG_TRACE_ERR("[XDataObject::ReadArray] Unknown member type!", 1);

                return 1;
            }
        }

        if (xarray->IsPrimitiveType())
        {
            int atype = xarray->GetArrayType();

            rval = ReadPrimitive(reader, atype, array_size);
        } else
        {
            xstring arr_type = xarray->GetArrayTypeName();
            std::auto_ptr< XDataObjectList > aptr_data(new XDataObjectList());

            for (uint i=0; i<array_size; i++)
            {
                std::auto_ptr< XDataObject > aptr_object( new XDataObject() );

                aptr_object->m_identifier.Set(arr_type);

                rval = aptr_object->ReadMembers(reader, tmpl_mgr);

                if (rval != 0)
                {
                    DBG_TRACE_ERR("[XDataObject::ReadArray] ReadDataObject", rval);

                    break;
                }

                aptr_data->AddObject(aptr_object.release());

                // in text mode there is comma between items
                if (reader.ReadToken() != EToken::TOKEN_COMMA)
                {
                    reader.UnreadToken();
                }
            }

            if (rval == 0)
            {
                AddSubData(aptr_data.release());
            }
        }

        return rval;
    }


    int XDataObject::ReadDataPart(xreader& reader, XTemplatesMgr& tmpl_mgr)
    {
        //data_part             : data_reference
        //						| object
        //						| number_list
        //						| float_list
        //						| string_list
        //

        int rval = 0;
        IntegerList aIntergers;
        FloatList aFloats;
        StringList aStrings;


        // check if reference
        rval = ReadDataReference(reader);
        if (rval > 0)
            DBG_TRACE_ERR("[XDataMgr::ReadDataPart] ReadDataReference", rval);

        // if not, check if int-list
        if (rval < 0)
        {
            rval = reader.ReadIntegerList(aIntergers);
            if (rval > 0)
                DBG_TRACE_ERR("[XDataMgr::ReadDataPart] ReadIntegerList", rval);

            if (rval == 0)
            {
                std::auto_ptr< XDataIntegerList > aptr_data(new XDataIntegerList());
                aptr_data->Set(aIntergers);

                AddSubData(aptr_data.release());
            }
        }

        // if not, check if float-list
        if (rval < 0)
        {
            rval = reader.ReadFloatList(aFloats);
            if (rval > 0)
                DBG_TRACE_ERR("[XDataMgr::ReadDataPart] ReadFloatList", rval);

            if (rval == 0)
            {
                std::auto_ptr< XDataFloatList > aptr_data(new XDataFloatList());
                aptr_data->Set(aFloats);

                AddSubData(aptr_data.release());
            }
        }

        // if not, check if string-list
        if (rval < 0)
        {
            rval = reader.ReadStringList(aStrings);
            if (rval > 0)
                DBG_TRACE_ERR("[XDataMgr::ReadDataPart] ReadStringList", rval);

        }

        // if not, check if another object
        if (rval < 0)
        {
            std::auto_ptr< XDataObject > aptr_object( new XDataObject() );

            rval = aptr_object->Load(reader, tmpl_mgr);

            if (rval > 0)
                DBG_TRACE_ERR("[XDataMgr::ReadDataPart] ReadDataObject", rval);

            if (rval == 0)
            {
                AddSubData(aptr_object.release());
            }

        }

        return rval;
    }

    int XDataObject::ReadDataReference(xreader& treader)
    {
        //data_reference        : EToken::TOKEN_OBRACE name optional_class_id EToken::TOKEN_CBRACE

        xstring name;
        xguid guid = xguid::Empty;

        if (treader.ReadToken() != EToken::TOKEN_OBRACE)
        {
            treader.UnreadToken();
            return -1;
        }

        if (treader.ReadName(name)) return 1;
        if (treader.ReadGUID(guid) > 0) return 1;

        if (treader.ReadToken() != EToken::TOKEN_CBRACE)
            return 1;

        AddSubData(new XDataReference(name, guid));

        return 0;
    }


    void XDataObject::AddSubData(IData* pData)
    {
        m_subdata.push_back(pData);

        m_size += pData->GetSize();
    }

    xguid XDataObject::GetGUID() const
    {
        if (! m_optional_guid.IsEmpty())
            return m_optional_guid;

        return m_template->GetGUID();
    }

    const IData* XDataObject::GetSubdata(uint index) const
    {
        if (index < m_subdata.size())
            return m_subdata[index];

        return 0;
    }

    void XDataObject::ToByteArray(void* buffer, uint buffer_size) const
    {
        uint bleft = buffer_size;
        char* bytes = (char*)buffer;

        for (XDataVectorCI iter = m_subdata.begin(); (bleft > 0) && (iter != m_subdata.end()); ++iter)
        {
            (*iter)->ToByteArray(bytes, bleft);

            uint subs = (*iter)->GetSize();

            if (subs > bleft)
            {
                bleft = 0;
            } else
            {
                bleft -= subs;
                bytes = bytes + subs;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // XDataIdentifier
    //////////////////////////////////////////////////////////////////////////

    XDataIdentifier::XDataIdentifier()
    {
        m_bPrimitive = false;
    }

    xstring XDataIdentifier::ToString() const
    {
        xstring out;

        if (m_bPrimitive)
        {
            out = GetPrimitiveTypeName(m_iPrimitiveType);
        } else
        {
            out = m_sName;
        }

        return out;
    }

    int XDataIdentifier::Load(xreader& reader)
    {
        //identifier            : name
        //							| primitive_type

        m_bPrimitive = false;

        int rval = reader.ReadName(m_sName);

        // if not name check if primitive
        if (rval < 0)
        {
            rval = reader.ReadPrimitiveType(m_iPrimitiveType);

            if (rval == 0)
            {
                m_bPrimitive = true;
            }
        }

        return rval;
    }

    //////////////////////////////////////////////////////////////////////////
    // XDataIntegerList
    //////////////////////////////////////////////////////////////////////////
    void XDataIntegerList::ToByteArray(void* buffer, uint buffer_size) const
    {
        uint bsize = (buffer_size >> 2);    // divide buffer size by int(4) size, this gives int count
        //DWORD* ints = (DWORD*)buffer;

        if (bsize > m_aIntegers.size())
        {
            bsize = m_aIntegers.size();
        }

        memcpy(buffer, m_aIntegers.get(), bsize*sizeof(DWORD));
/*
        int i = 0;
        for (IntegerList::const_iterator it=m_aIntegers.begin(); (bsize>0) && (it != m_aIntegers.end()); ++it, bsize--)
        {
            ints[i] = *it;

            i++;
        }
*/
    }

    //////////////////////////////////////////////////////////////////////////
    // XDataFloatList
    //////////////////////////////////////////////////////////////////////////
    void XDataFloatList::ToByteArray(void* buffer, uint buffer_size) const
    {
        uint bsize = (buffer_size >> 2);    // divide buffer size by float(4) size

        if (bsize > m_aFloats.size())
        {
            bsize = m_aFloats.size();
        }

        memcpy(buffer, m_aFloats.get(), bsize*sizeof(float));

/*
        float* fls = (float*)buffer;

        int i = 0;
        for (FloatList::const_iterator it=m_aFloats.begin(); (bsize>0) && (it != m_aFloats.end()); ++it, bsize--)
        {
            fls[i] = *it;

            i++;
        }*/

    }

    //////////////////////////////////////////////////////////////////////////
    // XDataPrimitive
    //////////////////////////////////////////////////////////////////////////
    void XDataPrimitive::ToByteArray(void* buffer, uint buffer_size) const
    {
        if (buffer_size < m_size)
            return;

        switch(m_type)
        {
            case EPrimitiveType_Float:
                {
                    float *fl = (float*)buffer;
                    fl[0] = fValue;
                    break;
                }
            case EPrimitiveType_Double:
                {
                    double *dbl = (double*)buffer;
                    dbl[0] = dValue;
                    break;
                }
            case EPrimitiveType_String:
                {
                    char* str = (char*)buffer;
                    strncpy(str, strValue.c_str(), strValue.length());
                    str[strValue.length()] = 0;
                    break;
                }
            case EPrimitiveType_Integer:
                {
                    int *intgr = (int*)buffer;
                    intgr[0] = iValue;
                    break;
                }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // XDataObjectList
    //////////////////////////////////////////////////////////////////////////
    void XDataObjectList::ToByteArray(void* buffer, uint buffer_size) const
    {
        char* bytes = (char*)buffer;
        uint bleft = buffer_size;

        for( XDataVectorCI it = m_objects.begin(); (bleft>0) && (it != m_objects.end()); ++it)
        {
            const IData* obj = *it;

            obj->ToByteArray(bytes, bleft);

            uint subs = obj->GetSize();

            if (subs > bleft)
            {
                bleft = 0;
            } else
            {
                bleft -= subs;
                bytes = bytes + subs;
            }
        }
    }

}}
