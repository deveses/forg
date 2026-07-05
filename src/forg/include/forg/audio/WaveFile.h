// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

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
