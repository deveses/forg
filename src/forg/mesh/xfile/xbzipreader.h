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

#ifndef XFILE_XBZIPREADER_INCLUDED
#define XFILE_XBZIPREADER_INCLUDED

#ifdef FORG_USE_ZLIB

#include "mesh/xfile/xbinreader.h"

#define CAB_BLOCKMAX (32768)
#define CAB_INPUTMAX (CAB_BLOCKMAX+6144)

namespace forg { namespace xfile { namespace reader {

	class xbzipreader : public xbinreader {

    public:
		xbzipreader(std::ifstream& input, bool doubleFloat);
        virtual ~xbzipreader();

    private:
        unsigned int m_org_size;
        void* m_zstream;

        char m_buf_in[CAB_INPUTMAX];    ///< buffer with compressed data
        char m_buf_out[CAB_BLOCKMAX];   ///< buffer with uncompressed data
        char m_dict[CAB_INPUTMAX];      ///< previous uncompressed data
        char* m_unpacked;

        unsigned int m_data_size;       ///< compressed size
        unsigned int m_block_size;      ///< uncompressed size

        unsigned int m_num_avail;       ///< number of bytes available from block of uncompressed data

    protected:
        /// returns true if read failed
        bool read_data(char* buffer, unsigned int count);

        bool read_next_block();

        bool unpack_data();
	};


}}}

#endif

#endif // XFILE_XBZIPREADER_INCLUDED

