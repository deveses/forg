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

#pragma once
#include <cstdio>
#include <memory>
#include <vector>

#include "forg/base.h"

namespace forg::audio {

struct RIFFHDR
{
    unsigned int nID;
    unsigned int nSize;
    unsigned int nFileType;
};

struct LISTHDR
{
    unsigned int nID;
    unsigned int nSize;
    unsigned int nFileType;
};

struct CHUNKHDR
{
    unsigned int nID;
    unsigned int nSize;
};

struct SWaveChunk
{
    CHUNKHDR header;
    unsigned int offset;
};

#define RiffID 0x46464952u   // 'FFIR'
#define ListID 0x5453494Cu   // 'TSIL'
#define WaveID 0x45564157u   // 'EVAW'
#define FormatID 0x20746D66u // ' tmf'
#define DataID 0x61746164u   // 'atad'

// xaudio2/xma2
#define XWmaID 0x414D5758u // 'AMWX'
#define DpdsID 0x73647064u // 'sdpd'
#define XMA2ID 0x32414D58u // '2AMX'
#define SeekID 0x6B656573u // 'kees'

class FORG_API WaveFile
{
  private:
    struct FileCloser
    {
        void operator()(FILE* file) const;
    };

    typedef std::unique_ptr<FILE, FileCloser> FilePtr;
    typedef std::vector<SWaveChunk> WaveChunkVec;

  private:
    FilePtr m_file;
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
    unsigned int ReadChunkData(const SWaveChunk& chunk, char* buf,
                               unsigned int size);
    unsigned int Read(unsigned int offset, char* buf, unsigned int size);
};

} // namespace forg::audio
