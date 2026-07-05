// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2006 Slawomir Strumecki

#pragma once
#ifdef FORG_USE_ZLIB

#include "mesh/xfile/xbinreader.h"

#define CAB_BLOCKMAX (32768)
#define CAB_INPUTMAX (CAB_BLOCKMAX + 6144)

namespace forg::xfile::reader {

class xbzipreader : public xbinreader
{

  public:
    xbzipreader(std::ifstream& input, bool doubleFloat);
    virtual ~xbzipreader();

  private:
    unsigned int m_org_size;
    void* m_zstream;

    char m_buf_in[CAB_INPUTMAX];  ///< buffer with compressed data
    char m_buf_out[CAB_BLOCKMAX]; ///< buffer with uncompressed data
    char m_dict[CAB_INPUTMAX];    ///< previous uncompressed data
    char* m_unpacked;

    unsigned int m_data_size;  ///< compressed size
    unsigned int m_block_size; ///< uncompressed size

    unsigned int m_num_avail; ///< number of bytes available from block of
                              ///< uncompressed data

  protected:
    /// returns true if read failed
    bool read_data(char* buffer, unsigned int count);

    bool read_next_block();

    bool unpack_data();
};

} // namespace forg::xfile::reader

#endif
