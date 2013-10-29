#include "forg_pch.h"

#include "os/WindowsLibrary.h"

#ifdef WIN32

namespace forg { namespace os {

WindowsLibrary::WindowsLibrary()
 : m_hModule(0)
{
}

WindowsLibrary::~WindowsLibrary()
{
    Close();
}

int WindowsLibrary::Open(LPCTSTR szName, int /*nFlags*/)
{
    Close();

    m_hModule = LoadLibrary(szName);

    return (m_hModule == 0);
}

void WindowsLibrary::Close()
{
    if (m_hModule != 0)
    {
        FreeLibrary(m_hModule);
        m_hModule = 0;
    }
}

void* WindowsLibrary::Address(LPCSTR szName)
{
    if (m_hModule != 0)
        return (void*)GetProcAddress(m_hModule, szName);

    return (void*)0;
}

}}


#endif

