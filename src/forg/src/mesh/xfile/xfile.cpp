#include "forg_pch.h"

#include "mesh/xfile/xfile.h"
#include "mesh/xfile/xbzipreader.h"
#include "mesh/xfile/xbinreader.h"
#include "mesh/xfile/xtexreader.h"
#include "mesh/xfile/xtemplate.h"

#include "debug/dbg.h"

namespace forg { namespace xfile {

#define XOFFILE_FORMAT_MAGIC \
	((long)'x' + ((long)'o' << 8) + ((long)'f' << 16) + ((long)' ' << 24))

#define XOFFILE_FORMAT_VERSION \
    ((long)'0' + ((long)'3' << 8) + ((long)'0' << 16) + ((long)'2' << 24))

#define XOFFILE_FORMAT_BINARY \
    ((long)'b' + ((long)'i' << 8) + ((long)'n' << 16) + ((long)' ' << 24))

#define XOFFILE_FORMAT_TEXT   \
    ((long)'t' + ((long)'x' << 8) + ((long)'t' << 16) + ((long)' ' << 24))

#define XOFFILE_FORMAT_COMPRESSED \
    ((long)'c' + ((long)'m' << 8) + ((long)'p' << 16) + ((long)' ' << 24))

#define XOFFILE_FORMAT_COMPRESSED_TEXT \
    ((long)'t' + ((long)'z' << 8) + ((long)'i' << 16) + ((long)'p' << 24))

#define XOFFILE_FORMAT_COMPRESSED_BINARY \
    ((long)'b' + ((long)'z' << 8) + ((long)'i' << 16) + ((long)'p' << 24))


#define XOFFILE_FORMAT_FLOAT_BITS_32 \
    ((long)'0' + ((long)'0' << 8) + ((long)'3' << 16) + ((long)'2' << 24))

#define XOFFILE_FORMAT_FLOAT_BITS_64 \
    ((long)'0' + ((long)'0' << 8) + ((long)'6' << 16) + ((long)'4' << 24))

XFile::XFile()
{
}

XFile::~XFile()
{
}

bool XFile::Open(const char* filename)
{
    m_input.open(filename, std::ios_base::in|std::ios_base::binary);

    if (! m_input.is_open())
        return false;

    m_filename = filename;

    m_bDoubleFloat = (m_header.float_size == XOFFILE_FORMAT_FLOAT_BITS_64);

    int rval = 0;

    if (ReadHeader())
        return false;

    switch(m_header.type) {
        case XOFFILE_FORMAT_BINARY:
        {
            reader::xbinreader reader(m_input, m_bDoubleFloat);
            rval = ReadData( reader );
            break;
        }
        case XOFFILE_FORMAT_TEXT:
        {
            reader::xtexreader reader(m_input, m_bDoubleFloat);
            rval = ReadData( reader );
            break;
        }
#ifdef FORG_USE_ZLIB
        case XOFFILE_FORMAT_COMPRESSED_BINARY:
        {
            reader::xbzipreader reader(m_input, m_bDoubleFloat);
            rval = ReadData( reader );
            break;
        }
#endif
        case XOFFILE_FORMAT_COMPRESSED_TEXT:
        {
            DBG_MSG("[XFILE] Compressed formats are not supported!\n");
            break;
        }
        default:
        {
            DBG_MSG("[XFILE] Unsupported format!\n");
        }
    }

    return (rval == 0);

}

int XFile::ReadHeader()
{
    xfile_header hdr;

    //int hs = sizeof(hdr);
    m_input.read((char*)&hdr, sizeof(hdr));

    if (m_input.fail())
        return 1;

    if (hdr.magic_number != XOFFILE_FORMAT_MAGIC)
        return 1;

    m_header = hdr;

    m_bDoubleFloat = (m_header.float_size == XOFFILE_FORMAT_FLOAT_BITS_64);

    return 0;
}

void XFile::GetDataObjects(XDataPtrVec& data_vec)
{
    uint c = m_data_mgr.GetDataObjectsCount();

    for (uint i=0; i<c; i++)
    {
        data_vec.push_back( m_data_mgr.GetDataObject(i) );
    }
}

int XFile::ReadData(reader::xreader& reader)
{
    int rval = m_tmpls_mgr.ReadTemplates(reader);

    if (rval > 0)
    {
        DBG_MSG("[XFILE] Error occured during templates reading!\n");
    }
    else
    {
        //m_tmpls_mgr.PrintTemplates();

        rval = m_data_mgr.ReadData(reader, m_tmpls_mgr);
        if (rval > 0)
        {
            DBG_MSG("[XFILE] Error occured during data reading!\n");
        } else
        {
            //m_data_mgr.PrintInfo();
            if (rval < 0 && m_input.eof())
            {
                rval = 0;
            }
        }
    }

    return rval;
}

}}
