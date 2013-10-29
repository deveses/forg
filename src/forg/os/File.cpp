#include "forg_pch.h"

#include "File.h"

#ifdef WIN32
#include <windows.h>

namespace forg { namespace os {

File::File()
{
    m_handle = 0;
}

File::~File()
{
    Close();
}

bool File::Open(const char* _filename)
{
    HANDLE h = CreateFile(_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (h != INVALID_HANDLE_VALUE)
    {
        m_handle = h;

        return true;
    }

    return false;
}

void File::Close()
{
    if (m_handle)
    {
        CloseHandle(m_handle);
        m_handle = 0;
    }
}

uint File::Read(void* _buffer, uint _size)
{
    uint num_read = 0;

    if (m_handle)
    {
        DWORD nr = 0;
        if ( ReadFile(m_handle, _buffer, _size, &nr, 0) )
        {
            num_read = nr;
        }
    }

    return num_read;
}

#endif

}}