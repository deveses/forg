#include "forg/audio/WaveFile.h"
#include "forg_pch.h"

namespace forg::audio {

void WaveFile::FileCloser::operator()(FILE* file) const
{
    if (file)
    {
        fclose(file);
    }
}

WaveFile::WaveFile() : m_file(nullptr), m_riff_type(0) {}

WaveFile::~WaveFile() { Close(); }

void WaveFile::Close()
{
    m_file.reset();
    m_riff_type = 0;
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
    for (const SWaveChunk& wave_chunk : m_chunks)
    {
        if (wave_chunk.header.nID == id)
        {
            chunk = wave_chunk;

            return true;
        }
    }

    return false;
}

unsigned int WaveFile::Read(unsigned int offset, char* buf, unsigned int size)
{
    if (!m_file || fseek(m_file.get(), offset, SEEK_SET) != 0)
    {
        return 0;
    }

    return (unsigned int)fread(buf, 1, size, m_file.get());
}

unsigned int WaveFile::ReadChunkData(const SWaveChunk& chunk, char* buf,
                                     unsigned int size)
{
    if (chunk.header.nSize < size)
        size = chunk.header.nSize;

    return Read(chunk.offset + 8, buf, size);
}

bool WaveFile::Open(const char* _filename)
{
    Close();

    m_file.reset(fopen(_filename, "rb"));

    if (m_file)
    {
        SWaveChunk chunk;

        chunk.offset = 0;

        size_t file_size = 0;

        if (fseek(m_file.get(), 0, SEEK_END) != 0)
        {
            Close();
            return false;
        }

        long file_end = ftell(m_file.get());
        if (file_end < 0)
        {
            Close();
            return false;
        }

        file_size = static_cast<size_t>(file_end);
        if (fseek(m_file.get(), 0, SEEK_SET) != 0)
        {
            Close();
            return false;
        }

        bool riff_found = false;
        size_t cbread = 0;

        while (0 < (cbread = fread(&chunk.header, sizeof(chunk.header), 1,
                                   m_file.get())))
        {
            if (riff_found)
            {
                if (chunk.offset + chunk.header.nSize + sizeof(chunk.header) >
                    file_size)
                {
                    // broken chunk, skip it
                    break;
                }

                if (fseek(m_file.get(), chunk.header.nSize, SEEK_CUR) != 0)
                {
                    // if we can't skip chunk, break reading
                    break;
                }

                m_chunks.push_back(chunk);

                long chunk_offset = ftell(m_file.get());
                if (chunk_offset < 0)
                {
                    break;
                }

                chunk.offset = static_cast<unsigned int>(chunk_offset);
            }
            else
            {
                if (chunk.header.nID == RiffID)
                {
                    riff_found = true;

                    if (fread(&m_riff_type, sizeof(m_riff_type), 1,
                              m_file.get()) != 1)
                    {
                        break;
                    }

                    long chunk_offset = ftell(m_file.get());
                    if (chunk_offset < 0)
                    {
                        break;
                    }

                    chunk.offset = static_cast<unsigned int>(chunk_offset);
                }
                else
                {
                    fseek(m_file.get(), chunk.header.nSize, SEEK_CUR);
                }
            }
        }

        return riff_found;
    }

    return false;
}

} // namespace forg::audio
