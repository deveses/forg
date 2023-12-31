#include "forg_pch.h"

#include "mesh/xfile/xtemplatesmgr.h"
#include "mesh/xfile/xstdtemplates.h"
#include "debug/dbg.h"
#include "PerformanceCounter.h"

#define ADD_TO_STD_STRING_MAP(x) m_std_string_map[ XTemplate##x::NAME ] = new XTemplate##x()

namespace forg { namespace xfile {

    XTemplatesMgr::XTemplatesMgr()
    {
        ADD_TO_STD_STRING_MAP(Animation);
        ADD_TO_STD_STRING_MAP(AnimationKey);
        ADD_TO_STD_STRING_MAP(AnimationOptions);
        ADD_TO_STD_STRING_MAP(AnimationSet);
        ADD_TO_STD_STRING_MAP(AnimTicksPerSecond);
        ADD_TO_STD_STRING_MAP(Boolean);
        ADD_TO_STD_STRING_MAP(Boolean2d);
        ADD_TO_STD_STRING_MAP(ColorRGB);
        ADD_TO_STD_STRING_MAP(ColorRGBA);
        ADD_TO_STD_STRING_MAP(CompressedAnimationSet);
        ADD_TO_STD_STRING_MAP(Coords2d);
        ADD_TO_STD_STRING_MAP(DeclData);
        ADD_TO_STD_STRING_MAP(EffectDWord);
        ADD_TO_STD_STRING_MAP(EffectFloats);
        ADD_TO_STD_STRING_MAP(EffectInstance);
        ADD_TO_STD_STRING_MAP(EffectParamDWord);
        ADD_TO_STD_STRING_MAP(EffectParamFloats);
        ADD_TO_STD_STRING_MAP(EffectParamString);
        ADD_TO_STD_STRING_MAP(EffectString);
        ADD_TO_STD_STRING_MAP(FaceAdjacency);
        ADD_TO_STD_STRING_MAP(FloatKeys);
        ADD_TO_STD_STRING_MAP(Frame);
        ADD_TO_STD_STRING_MAP(FrameTransformMatrix);
        ADD_TO_STD_STRING_MAP(FVFData);
        ADD_TO_STD_STRING_MAP(Guid);
        ADD_TO_STD_STRING_MAP(IndexedColor);
        ADD_TO_STD_STRING_MAP(Material);
        ADD_TO_STD_STRING_MAP(MaterialWrap);
        ADD_TO_STD_STRING_MAP(Matrix4x4);
        ADD_TO_STD_STRING_MAP(Mesh);
        ADD_TO_STD_STRING_MAP(MeshFace);
        ADD_TO_STD_STRING_MAP(MeshFaceWraps);
        ADD_TO_STD_STRING_MAP(MeshMaterialList);
        ADD_TO_STD_STRING_MAP(MeshNormals);
        ADD_TO_STD_STRING_MAP(MeshTextureCoords);
        ADD_TO_STD_STRING_MAP(MeshVertexColors);
        ADD_TO_STD_STRING_MAP(Patch);
        ADD_TO_STD_STRING_MAP(PatchMesh);
        ADD_TO_STD_STRING_MAP(PatchMesh9);
        ADD_TO_STD_STRING_MAP(PMAttributeRange);
        ADD_TO_STD_STRING_MAP(PMInfo);
        ADD_TO_STD_STRING_MAP(PMVSplitRecord);
        ADD_TO_STD_STRING_MAP(SkinWeights);
        ADD_TO_STD_STRING_MAP(TextureFilename);
        ADD_TO_STD_STRING_MAP(TimedFloatKeys);
        ADD_TO_STD_STRING_MAP(Vector);
        ADD_TO_STD_STRING_MAP(VertexDuplicationIndices);
        ADD_TO_STD_STRING_MAP(VertexElement);
        ADD_TO_STD_STRING_MAP(XSkinMeshHeader);
    }

    XTemplatesMgr::~XTemplatesMgr()
    {

        for (XTemplateVectorI iter = m_templates.begin(); iter != m_templates.end(); ++iter)
        {
            delete *iter;
        }

        for (XStringTemplateMapI iter = m_std_string_map.begin(); iter != m_std_string_map.end(); ++iter)
        {
            delete iter->second;
        }


    }

    const XTemplate* XTemplatesMgr::Find(const xstring& name)
    {
        XStringTemplateIndexMap::iterator iter = m_string_map.find(name);

        if (iter != m_string_map.end())
        {
            return m_templates[iter->second];
        }

        XStringTemplateMapI istd = m_std_string_map.find(name);

        if (istd != m_std_string_map.end())
        {
            return istd->second;
        }

        return 0;
    }

    const XTemplate* XTemplatesMgr::Find(const xguid& guid)
    {
        XGuidTemplateIndexMap::iterator iter = m_guid_map.find(guid);

        if (iter != m_guid_map.end())
        {
            return m_templates[iter->second];
        }

        return 0;
    }

    void XTemplatesMgr::PrintTemplates()
    {
        char buff[256];

        DBG_MSG("//===================================================\n");
        sprintf(buff, "//templates (%d):\n", m_templates.size());
        DBG_MSG(buff);

        for (uint i=0; i<m_templates.size(); i++)
        {
            const XTemplate* tmpl = m_templates[i];

            if (tmpl != 0)
            {
                DBG_MSG(tmpl->ToString().c_str());
            } else
            {
                DBG_MSG("[XTemplatesMgr::PrintTemplates] tmpl == null !!!\n");
            }

        }

        DBG_MSG("//===================================================\n");
    }

    int XTemplatesMgr::ReadTemplates(xreader& treader)
    {
        int res = 0;

        forg::PerformanceCounter counter;
        counter.Start();

        do {
            XTemplate* tmpl = 0;

            res = ReadTemplate(treader, &tmpl);

            if (res == 0 && tmpl != 0)
            {
                uint tindex = (uint)m_templates.size();
                m_templates.push_back(tmpl);

                m_string_map[tmpl->GetName()] = tindex;
                m_guid_map[tmpl->GetGUID()] = tindex;
            }

        } while (res == 0);

        counter.Stop();

        char msg[256];
        uint64 dur = 0;
        counter.GetDurationInMs(dur);
        sprintf(msg, "[XTemplatesMgr::ReadTemplates] took %dms\n", dur);
        DBG_MSG(msg);

        return (res == 1);
    }

    int XTemplatesMgr::ReadTemplate(xreader& treader, XTemplate** tmpl)
    {
        WORD tok = treader.ReadToken();

        if (tok == 0) return 1;
        if (tok != EToken::TOKEN_TEMPLATE)
        {
            treader.UnreadToken();
            return -1;
        }

        //tmpl.is_open = false;
        xstring tname;
        xguid tguid;

        if (treader.ReadName(tname)) return 1;
        if (treader.ReadToken() != EToken::TOKEN_OBRACE) return 1;
        if (treader.ReadGUID(tguid)) return 1;

        std::auto_ptr<XTemplate> aptr_tmpl(new XTemplate(tname.c_str(), tguid));

        if (ReadTemplateParts(treader, aptr_tmpl.get() )) return 1;

        if (treader.ReadToken() != EToken::TOKEN_CBRACE) return 1;

        *tmpl = aptr_tmpl.release();

        return 0;
    }


    int XTemplatesMgr::ReadTemplateParts(xreader& treader, XTemplate* tmpl)
    {
        //template_parts        : template_members_part EToken::TOKEN_OBRACKET
        //							template_option_info
        //							EToken::TOKEN_CBRACKET
        //							| template_members_list
        //
        //template_members_part : /* Empty */
        //						| template_members_list
        int rval;

        if ((rval = ReadTemplateMembersList(treader, tmpl)) > 0) return 1;
        if (treader.ReadToken()!= EToken::TOKEN_OBRACKET)
        {
            treader.UnreadToken();

            return (rval > 0);	//jesli nie ma EToken::TOKEN_OBRACKET to musi byc template_members_list
        }

        if (ReadTemplateOptionInfo(treader, tmpl) > 0) return 1;
        if (treader.ReadToken()!= EToken::TOKEN_CBRACKET) return 1;

        return 0;
    }

    int XTemplatesMgr::ReadTemplateMembersList(xreader& treader, XTemplate* tmpl)
    {
        //template_members_list = template_members {template_members}

        int list_size = 0;
        int rval;

        do {
            XTemplateMember* member  = null;

            rval = ReadTemplateMembers(treader, &member);

            if (rval == 0)
            {
                tmpl->AddMember(member);
                list_size++;
            }
        } while(rval == 0);

        return rval > 0 ? rval : ((list_size == 0) ? rval : 0);
    }

    int XTemplatesMgr::ReadTemplateMembers(xreader& treader, XTemplateMember** member)
    {
        int rval;

        rval = ReadTemplatePrimitive(treader, member);

        /* OR */

        if (rval < 0)
            rval = ReadTemplateArray(treader, member);

        /* OR */

        if (rval < 0)
            rval = ReadTemplateReference(treader, member);

        return rval;
    }

    int XTemplatesMgr::ReadTemplatePrimitive(xreader& treader, XTemplateMember** member)
    {
        xstring optional_name;
        ETemplatePrimitiveType::TYPE primitive_type = ETemplatePrimitiveType::Unknown;
        int rval;

        rval = treader.ReadPrimitiveType(primitive_type);
        if (rval) return rval;

        rval = treader.ReadName(optional_name);

        if (rval > 0)
            return 1;

        if (treader.ReadToken() != EToken::TOKEN_SEMICOLON) return 1;

        *member = new XTemplatePrimitive(primitive_type, optional_name);

        return 0;
    }



    int XTemplatesMgr::ReadTemplateArray(xreader& treader, XTemplateMember** member)
    {
        //array                 : EToken::TOKEN_ARRAY array_data_type name dimension_list
        //						EToken::TOKEN_SEMICOLON
        //int rval;

        if (treader.ReadToken() != EToken::TOKEN_ARRAY)
        {
            treader.UnreadToken();
            return -1;
        }

        std::auto_ptr< XTemplateArray > aptr_array( new XTemplateArray() );
        xstring aname;

        if (ReadTemplateArrayDataType(treader, aptr_array.get())) return 1;
        if (treader.ReadName(aname)) return 1;
        if (ReadTemplateArrayDimensionList(treader, aptr_array.get())) return 1;
        if (treader.ReadToken() != EToken::TOKEN_SEMICOLON) return 1;

        aptr_array->SetName(aname);

        *member = aptr_array.release();

        return 0;
    }

    int XTemplatesMgr::ReadTemplateArrayDataType(xreader& treader, XTemplateArray* arr)
    {
        int rval;
        ETemplatePrimitiveType::TYPE ptype = ETemplatePrimitiveType::Unknown;
        xstring tname;

        rval = treader.ReadPrimitiveType(ptype);	//primitive_type
        if (rval == 0)
        {
            arr->SetArrayType(ptype);
        }

        if (rval < 0)
        {
            rval = treader.ReadName(tname);	//| name
            if (rval == 0)
            {
                arr->SetArrayType(tname);
            }
        }

        return rval;
    }

    int XTemplatesMgr::ReadTemplateArrayDimensionList(xreader& treader, XTemplateArray* arr)
    {
        //dimension_list = dimension {dimension}

        //dimension_list        : dimension
        //						| dimension_list dimension
        //
        int list_size = 0;
        int rval;

        while ((rval = ReadTemplateArrayDimension(treader, arr)) == 0)
        {
            list_size++;
        }

        return rval > 0 ? rval : ((list_size == 0) ? rval : 0);
    }

    int XTemplatesMgr::ReadTemplateArrayDimension(xreader& treader, XTemplateArray* arr)
    {
        if (treader.ReadToken() != EToken::TOKEN_OBRACKET)
        {
            treader.UnreadToken();
            return -1;
        }
        if (ReadTemplateArrayDimensionSize(treader, arr)) return 1;
        if (treader.ReadToken() != EToken::TOKEN_CBRACKET) return 1;

        return 0;
    }

    int XTemplatesMgr::ReadTemplateArrayDimensionSize(xreader& treader, XTemplateArray* arr)
    {
        int ivalue = 0;
        xstring name;
        int rval = treader.ReadInteger(ivalue);			//EToken::TOKEN_INTEGER

        if (rval == 0)
        {
            arr->AddDimension(ivalue);
        }

        if (rval < 0)
        {
            rval = treader.ReadName(name);	//| name
            if (rval == 0)
            {
                arr->AddDimension(name);
            }
        }

        return rval;
    }


    int XTemplatesMgr::ReadTemplateReference(xreader& treader, XTemplateMember** member)
    {
        xstring name;
        xstring optional_name;
        int rval;

        rval = treader.ReadName(name);
        if (rval == 0)
        {
            if (treader.ReadName(optional_name) > 0) return 1;
            if (treader.ReadToken() != EToken::TOKEN_SEMICOLON/*YT_SEMICOLON*/) return 1;

            *member = new XTemplateReference(name, optional_name);
        }

        return rval;
    }

    int XTemplatesMgr::ReadTemplateOptionInfo(xreader& treader, XTemplate* tmpl)
    {
        int rval = ReadTemplateElipsis(treader, tmpl);

        if (rval < 0)
            rval = ReadTemplateOptionList(treader, tmpl);

        return rval;
    }

    int XTemplatesMgr::ReadTemplateElipsis(xreader& treader, XTemplate* tmpl)
    {
        WORD tok = treader.ReadToken();

        if (tok == 0) return 1;
        if (tok != EToken::TOKEN_DOT)
        {
            treader.UnreadToken();
            return -1;
        }
        if (treader.ReadToken() != EToken::TOKEN_DOT) return 1;
        if (treader.ReadToken() != EToken::TOKEN_DOT) return 1;

        tmpl->SetOpenMode(true);

        return 0;
    }

    int XTemplatesMgr::ReadTemplateOptionList(xreader& treader, XTemplate* tmpl)
    {
        int rval;
        int list_size = 0;
        xstring oname;
        xguid oguid;

        while ((rval = ReadTemplateOptionPart(treader, oname, oguid)) == 0)
        {
            list_size++;

            tmpl->AddOption(oname, oguid);
        }

        return rval > 0 ? rval : ((list_size == 0) ? rval : 0);
    }

    int XTemplatesMgr::ReadTemplateOptionPart(xreader& treader, xstring& out_name, xguid& out_guid)
    {
        int rval = treader.ReadName(out_name);

        if (rval == 0)
        {
            rval = (treader.ReadGUID(out_guid) > 0);
        }

        return rval;
    }

}}
