#include "forg_pch.h"
#include "mesh/xfile/xbzipreader.h"
#include "debug/dbg.h"

#ifdef FORG_USE_ZLIB
#include <zlib.h>
#endif

#define MSZIP_MAGIC 0x4B43

namespace forg { namespace xfile { namespace reader {

#ifdef FORG_USE_ZLIB
 voidpf MSZipAlloc(voidpf opaque, uInt items, uInt size)
{
    return new char[items * size];
}

void MSZipFree (voidpf opaque, voidpf address)
{
    delete [] (char*)address;
}

xbzipreader::xbzipreader(std::ifstream& input, bool doubleFloat)
: xbinreader(input, doubleFloat)
, m_org_size(0)
, m_zstream(0)
{
    m_input.read((char*)&m_org_size, sizeof(m_org_size));

    z_stream* strm = new z_stream;

    strm->zalloc = MSZipAlloc;
    strm->zfree = MSZipFree;
    strm->opaque = Z_NULL;

    m_zstream = strm;

    read_next_block();

    int ret = inflateInit2(strm, -MAX_WBITS);

    if (ret != Z_OK || unpack_data())
        m_org_size = 0;
}

xbzipreader::~xbzipreader()
{
    z_stream* strm = (z_stream*)m_zstream;

    inflateEnd(strm);

    delete strm;
}

bool xbzipreader::read_next_block()
{
    unsigned short block_size = 0;
    unsigned short data_size = 0;
    unsigned short magic_num = 0;
    
    m_input.read((char*)&block_size, sizeof(block_size));
    m_input.read((char*)&data_size, sizeof(data_size));
    m_input.read((char*)&magic_num, sizeof(magic_num));

    m_block_size = block_size;
    m_data_size = data_size - 2;
    
    m_input.read(m_buf_in, m_data_size);

    z_stream* strm = (z_stream*)m_zstream;

    strm->avail_in = m_data_size;
    strm->next_in = (Bytef*)m_buf_in;
    strm->avail_out = 0;

    m_num_avail = 0;

    return (magic_num != MSZIP_MAGIC) || (m_input.gcount() != m_data_size);
}

bool xbzipreader::unpack_data()
{
    z_stream* strm = (z_stream*)m_zstream;

    strm->avail_out = m_block_size;
    strm->next_out = (Bytef*)m_buf_out;

    int ret = inflate(strm, Z_NO_FLUSH);

    m_num_avail = m_block_size;
    m_unpacked = m_buf_out;

    return (ret == Z_OK);
}

bool xbzipreader::read_data(char* buffer, unsigned int count)
{
    if (m_org_size == 0)
        return true;

    unsigned int to_read = count;
    int ret = Z_OK;

    do
    {
        if (m_num_avail == 0)
        {
            z_stream* strm = (z_stream*)m_zstream;

            memcpy(m_dict, m_buf_out, m_block_size);

            ret = inflateReset(strm);
            ret = inflateSetDictionary(strm, (Bytef*)m_dict, m_block_size);

            if ( read_next_block() || unpack_data())
                return true;
        }

        unsigned int c = (m_num_avail > to_read) ? to_read : m_num_avail;

        memcpy(buffer, m_unpacked, c);

        m_unpacked += c;
        m_num_avail -= c;

        to_read -= c;
        buffer += c;

    } while (to_read > 0 && ret == Z_OK);


    return false;

/*
    z_stream* strm = (z_stream*)m_zstream;

    if (strm->avail_out == 0)
    {
        strm->avail_out = count;
        strm->next_out = (Bytef*)buffer;
    }

    int ret = 0;
    int total_out = strm->total_out;
    
    do 
    {
        int before = strm->total_out;

        ret = inflate(strm, Z_NO_FLUSH);

        memcpy(m_dict + total_out, buffer, strm->total_out - before);

        if (strm->avail_in == 0 && ! m_input.eof() && ! m_input.fail())
        {
            ret = inflateReset(strm);
            ret = inflateSetDictionary(strm, (Bytef*)m_dict, m_block_size);

            DBG_MSG("[xbzipreader::read_data] dictionary size: %d\n", m_block_size);
            
            total_out = 0;

            unsigned short block_size = 0;
            unsigned short data_size = 0;
            unsigned short magic_num = 0;
            m_input.read((char*)&block_size, sizeof(block_size));
            m_input.read((char*)&data_size, sizeof(data_size));
            m_input.read((char*)&magic_num, sizeof(magic_num));

            if (magic_num != MSZIP_MAGIC)
                return 1;

            m_block_size = block_size;
            m_data_size = data_size - 2;
            m_input.read(m_buf_in, m_data_size);

            strm->avail_in = m_input.gcount();
            strm->next_in = (Bytef*)m_buf_in;

            DBG_MSG("[xbzipreader::read_data] block size: %d, data size: %d\n", block_size, data_size);
        }

    } while (strm->avail_out > 0 && ret == Z_OK);

    return (ret != Z_OK);
*/
}

#else
#endif

}}}
