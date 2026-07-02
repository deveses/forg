#include "forg_pch.h"

#include "forg/os/File.h"

#include <cstdio>
#include <optional>
#include <sys/types.h>

#ifdef FORG_PLATFORM_LINUX

static std::optional<forg::u32> getFileSize(FILE* f)
{
    int err = fseeko(f, 0, SEEK_END);
    if (err)
        return std::nullopt;

    off_t pos = ftello(f);
    if (pos < 0)
        return std::nullopt;

    fseeko(f, 0, SEEK_SET);

    return static_cast<forg::u32>(pos);
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

u32 File::Read(void* _buffer, u32 _size)
{
    if (!m_handle)
        return 0;

    return static_cast<u32>(fread(_buffer, 1, _size, (FILE*)m_handle));
}

bool File::GetSize(u32& _out_size)
{
    if (!m_handle)
        return false;

    _out_size = m_size;
    return true;
}

} // namespace forg::os
#endif
