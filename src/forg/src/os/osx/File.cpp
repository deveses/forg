#include "forg_pch.h"

#include "forg/os/File.h"
#include <optional>

#ifdef FORG_PLATFORM_OSX

static std::optional<uint> getFileSize(FILE* f)
{
    int err = fseeko(f, 0, SEEK_END);
    if (err)
        return std::nullopt;

    off_t pos = ftello(f);
    if (pos < 0)
        return std::nullopt;

    fseeko(f, 0, SEEK_SET);

    return pos;
}

namespace forg::os {

File::File() { m_handle = 0; }

File::~File() { Close(); }

bool File::Open(const char* _filename)
{
    if (FILE* f = fopen(_filename, "rb"))
    {
        m_handle = f;

        if (auto size = getFileSize(f))
            m_size = *size;

        return true;
    }

    return false;
}

void File::Close()
{
    if (m_handle)
    {
        fclose((FILE*)m_handle);
        m_handle = 0;
    }
}

uint File::Read(void* _buffer, uint _size)
{
    if (!m_handle)
        return 0;

    uint bytesRead = fread(_buffer, 1, _size, (FILE*)m_handle);

    return bytesRead;
}

bool File::GetSize(uint& _out_size)
{
    if (!m_handle)
        return false;

    _out_size = m_size;
    return true;
}

} // namespace forg::os
#endif
