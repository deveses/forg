#include "forg_pch.h"

#include "mesh/xfile/xtemplate.h"

namespace forg { namespace xfile {

//////////////////////////////////////////////////////////////////////////
// TEMPLATE
//////////////////////////////////////////////////////////////////////////

//int xtemplate::IsStandardType()
//{
//    for (int i=0; i<XTEMPLATE_TYPES_COUNT; i++)
//    {
//        if (guid.Data1 == xtemplate_standard_guids[i].Data1 &&
//            guid.Data2 == xtemplate_standard_guids[i].Data2 &&
//            guid.Data3 == xtemplate_standard_guids[i].Data3)
//        {
//            bool data4eq = true;
//            for (int j=0; j<8 && data4eq; j++)
//            {
//                data4eq = (guid.Data4[j] == xtemplate_standard_guids[i].Data4[j]);
//            }
//
//            if (data4eq)
//                return i;
//        }
//    }
//
//    return XTEMPLATE_TYPE_UNKNOWN;
//}


XTemplate::XTemplate()
: m_bOpen(false)
{
}

XTemplate::XTemplate(const char* name, xguid guid, bool _open)
: m_bOpen(_open)
{
    m_sName = name;
    m_guid = guid;
}

XTemplate::~XTemplate()
{

    for (XTemplateMemberVectorI iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        delete *iter;
    }

}

void XTemplate::AddMember(XTemplateMember* member)
{
    if (member != 0)
        m_members.push_back(member);
}

void XTemplate::AddOption(const xstring& name, const xguid& guid)
{
    XTemplateOption opt(name, guid);

    m_options.push_back(opt);
}

int XTemplate::Load(xreader& /*treader*/)
{
    return 0;
}

const XTemplateMember* XTemplate::GetMember(uint idx) const
{
    return m_members[idx];
}

const XTemplateOption& XTemplate::GetOption(uint idx) const
{
    return m_options[idx];
}

uint XTemplate::GetMemberIndex(const xstring& name) const
{
    for (XTemplateMemberVectorCI iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        if ( (*iter)->GetName() == name )
        {
            return (uint)(iter - m_members.begin());
        }
    }

    return (uint)-1;
}

const XTemplateMember* XTemplate::FindMemberByName(const xstring& name) const
{
    for (XTemplateMemberVectorCI iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        if ( (*iter)->GetName() == name )
        {
            return *iter;
        }
    }

    return NULL;
}


void XTemplate::SetOpenMode(bool mode)
{
    m_bOpen = mode;
}


xstring XTemplate::ToString() const
{
    std::stringstream sstream;

    sstream << "template " << m_sName << " {\n\t<" << m_guid.ToString() << ">\n";
    sstream << "\t// members count: " << m_members.size() << "\n";

    for (uint j=0; j<m_members.size(); j++)
    {
        const XTemplateMember* mem = m_members[j];

        sstream << "\t" << mem->ToString() << ";" << std::endl;
    }

    //    for (size_t j=0; j<options.size(); j++)
    //    {
    //    }

    if (m_bOpen)
        sstream << "\t[...]\n";

    sstream << "}\n";

    return sstream.str();
}


}}
