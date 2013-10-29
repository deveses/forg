/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

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

#ifndef _FORG_AUDIO_WAVEFILE_H_
#define _FORG_AUDIO_WAVEFILE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/vector.hpp"

namespace forg { namespace audio {


struct RIFFHDR {
    unsigned int nID;
    unsigned int nSize;
    unsigned int nFileType;
};

struct LISTHDR {
    unsigned int nID;
    unsigned int nSize;
    unsigned int nFileType;
};

struct CHUNKHDR {
    unsigned int nID;
    unsigned int nSize;
};

    struct SWaveChunk
    {
        CHUNKHDR header;
        unsigned int offset;
    };


#define RiffID      'FFIR'
#define ListID      'TSIL'
#define WaveID      'EVAW'
#define FormatID    ' tmf'
#define DataID      'atad'

// xaudio2/xma2
#define XWmaID      'AMWX'
#define DpdsID      'sdpd'
#define XMA2ID      '2AMX'
#define SeekID      'kees'

class FORG_API WaveFile
{
private:
    typedef forg::core::vector<SWaveChunk> WaveChunkVec;
    typedef WaveChunkVec::iterator WaveChunkVecI;
    typedef WaveChunkVec::const_iterator WaveChunkVecCI;

private:
    FILE* m_file;
    unsigned int m_riff_type;
    WaveChunkVec m_chunks;

public:
    WaveFile();
    ~WaveFile();

public:
    bool Open(const char* _filename);
    void Close();

    bool GetFormat(char* format, unsigned int size);
    bool GetChunk(unsigned int id, SWaveChunk& chunk);
    unsigned int ReadChunkData(const SWaveChunk& chunk, char* buf, unsigned int size);
    unsigned int Read(unsigned int offset, char* buf, unsigned int size);
};

}}

#endif //_FORG_AUDIO_WAVEFILE_H_
