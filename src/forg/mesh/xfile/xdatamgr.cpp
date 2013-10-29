#include "forg_pch.h"

#include "mesh/xfile/xdatamgr.h"
#include "mesh/xfile//xtemplatesmgr.h"
#include "debug/dbg.h"
#include "PerformanceCounter.h"

namespace forg { namespace xfile {

    XDataMgr::XDataMgr()
    {
    }

    XDataMgr::~XDataMgr()
    {

        for (XDataObjectVectorI iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            delete *iter;
        }

    }

    void PrintIndent(int ident)
    {
        char ind_buff[] = {
            '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
                '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
                '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
                '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
                '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t',
                '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t'
        };

        ind_buff[ident] = 0;
        DBG_MSG(ind_buff);
    }

    void XDataMgr::PrintInfo(const XDataObject *data, int indent) const
    {
        char buff[256];

        PrintIndent(indent);
        sprintf(buff, "%s %s { // %d sub-object(s)\n", data->GetIdentifier().ToString().c_str(), data->GetName().c_str(), data->GetSubdataSize());
        DBG_MSG(buff);

        for (uint i=0; i<data->GetSubdataSize(); i++) {
            const IData* subd = data->GetSubdata(i);
            PrintInfo(subd, indent + 1);
        }

        PrintIndent(indent);
        DBG_MSG("}\n");
    }

    void XDataMgr::PrintInfo(const XDataFloatList *data, int indent) const
    {
        char buff[256];

        PrintIndent(indent);
        sprintf(buff, "// Float list of size %d\n", data->GetFloats().size());
        DBG_MSG(buff);
    }

    void XDataMgr::PrintInfo(const XDataIntegerList* /*data*/, int indent) const
    {
        char buff[256];

        PrintIndent(indent);
        sprintf(buff, "// Integer list\n");
        DBG_MSG(buff);
    }

    void XDataMgr::PrintInfo(const XDataStringList* /*data*/, int indent) const
    {
        char buff[256];

        PrintIndent(indent);
        sprintf(buff, "// String list\n");
        DBG_MSG(buff);
    }

    void XDataMgr::PrintInfo(const XDataReference* /*data*/, int indent) const
    {
        char buff[256];

        PrintIndent(indent);
        sprintf(buff, "// Reference\n");
        DBG_MSG(buff);
    }

    void XDataMgr::PrintInfo(const IData* data, int indent) const
    {
        int type = data->GetDataType();

        switch(type) {
        case EDataObjectType_Object:
            PrintInfo((const XDataObject *)data, indent);
            break;
        case EDataObjectType_FloatList:
            //PrintInfo((const XDataFloatList *)data, indent);
            break;
        case EDataObjectType_IntegerList:
            //PrintInfo((const XDataIntegerList *)data, indent);
            break;
        case EDataObjectType_StringList:
            PrintInfo((const XDataStringList *)data, indent);
            break;
        case EDataObjectType_DataReference:
            PrintInfo((const XDataReference *)data, indent);
            break;
        }
    }

    void XDataMgr::PrintInfo() const
    {
        char buff[256];

        DBG_MSG("//===================================================\n");
        sprintf(buff, "//data (%d):\n", m_data.size());
        DBG_MSG(buff);

        for (uint i=0; i<m_data.size(); i++)
        {
            const XDataObject* dobj = m_data[i];

            PrintInfo(dobj, 0);
        }

        DBG_MSG("//===================================================\n");
    }

    uint XDataMgr::GetDataObjectsCount() const
    {
        return (uint)m_data.size();
    }

    const XDataObject* XDataMgr::GetDataObject(uint nIndex) const
    {
        if (nIndex < m_data.size())
            return m_data[nIndex];

        return 0;
    }

    int XDataMgr::ReadData(xreader& treader, XTemplatesMgr& tmpl_mgr)
    {
        forg::PerformanceCounter perf_counter;
        perf_counter.Start();

        int rval = 0;
        do
        {
            std::auto_ptr< XDataObject > aptr_object( new XDataObject() );

            rval = aptr_object->Load(treader, tmpl_mgr);

            if (rval == 0)
            {
                m_data.push_back(aptr_object.release());
            }
        } while (rval == 0);

        perf_counter.Stop();

        char msg[256];
        uint64 dur = 0;
        perf_counter.GetDurationInMs(dur);
        sprintf(msg, "[XDataMgr::ReadData] took %dms\n", dur);
        DBG_MSG(msg);

        return rval;
    }

}}
