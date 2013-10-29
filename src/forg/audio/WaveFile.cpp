#include "forg_pch.h"
#include "audio/WaveFile.h"

namespace forg { namespace audio {

WaveFile::WaveFile()
    : m_file(0)
{
}

WaveFile::~WaveFile()
{
    Close();
}

void WaveFile::Close()
{
    if (m_file)
    {
        fclose(m_file);
        m_file = NULL;
    }

    m_chunks.clear();
}

bool WaveFile::GetFormat(char* format, unsigned int size)
{
    SWaveChunk chunk;

    if (GetChunk(FormatID, chunk))
    {
        return (0 < ReadChunkData(chunk, format, size));
    }

    return false;
}

bool WaveFile::GetChunk(unsigned int id, SWaveChunk& chunk)
{
    for (WaveChunkVecI it = m_chunks.begin(); it != m_chunks.end(); ++it)
    {
        if (it->header.nID == id)
        {
            chunk = *it;

            return true;
        }
    }

    return false;
}

unsigned int WaveFile::Read(unsigned int offset, char* buf, unsigned int size)
{
    fseek(m_file, offset, SEEK_SET);

    return (unsigned int)fread(buf, 1, size, m_file);
}

unsigned int WaveFile::ReadChunkData(const SWaveChunk& chunk, char* buf, unsigned int size)
{
    fseek(m_file, chunk.offset + 8, SEEK_SET);

    if (chunk.header.nSize < size)
        size = chunk.header.nSize;

    return Read(chunk.offset + 8, buf, size);
}

bool WaveFile::Open(const char* _filename)
{
    Close();

    m_file = fopen(_filename, "rb");

    if (m_file)
    {
        SWaveChunk chunk;

        chunk.offset = 0;

        size_t file_size = 0;

        if (fseek(m_file, 0, SEEK_END) != 0) return false;
        file_size = ftell(m_file);
        if (fseek(m_file, 0, SEEK_SET) != 0) return false;

        bool riff_found = false;
        size_t cbread = 0;

        while (0 < (cbread = fread(&chunk.header, sizeof(chunk.header), 1, m_file)))
        {
            if (riff_found)
            {
                if (chunk.offset + chunk.header.nSize + sizeof(chunk.header) > file_size)
                {
                    // broken chunk, skip it
                    break;
                }

                if (fseek(m_file, chunk.header.nSize, SEEK_CUR)!=0)
                {
                    // if we can't skip chunk, break reading
                    break;
                }

                m_chunks.push_back(chunk);

                chunk.offset = ftell(m_file);
            }
            else
            {
                if (chunk.header.nID == RiffID)
                {
                    riff_found = true;

                    fread(&m_riff_type, sizeof(m_riff_type), 1, m_file);

                    chunk.offset = ftell(m_file);
                } else
                {
                    fseek(m_file, chunk.header.nSize, SEEK_CUR);
                }
            }


        }

        return riff_found;
    }

    return false;
}

}}
