#include "forg_pch.h"

#include "debug/dbg.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <wchar.h>

void OutputDebugString(const char* text) { printf("%s\n", text); }

void OutputDebugStringA(const char* text) { printf("%s\n", text); }

void OutputDebugStringW(const wchar_t* text) { wprintf(L"%ls\n", text); }

void DebugBreak() { __builtin_debugtrap(); }

#define _vsnprintf vsnprintf
#define _vsnwprintf vswprintf
#endif

// These functions intentionally forward a runtime format string to the C
// formatting API. Call sites are type-checked through the declarations in
// dbg.h.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

namespace forg::debug {

void DbgOutputString(LPCTSTR lpOutputString, ...)
{
    static TCHAR msg[1024];
    va_list args;

    va_start(args, lpOutputString);

    _vsnprintf(msg, sizeof(msg), lpOutputString, args);
    msg[(sizeof(msg) / sizeof(msg[0])) - 1] = 0;

    OutputDebugString(msg);
    va_end(args);
}

void DbgOutputStringA(const char* lpOutputString, va_list args)
{
    static char msg[1024];

    _vsnprintf(msg, sizeof(msg), lpOutputString, args);
    msg[(sizeof(msg) / sizeof(msg[0])) - 1] = 0;

    OutputDebugStringA(msg);
}

void DbgOutputStringW(const wchar_t* lpOutputString, va_list args)
{
    static wchar_t msg[1024];

    _vsnwprintf(msg, sizeof(msg) / sizeof(msg[0]), lpOutputString, args);
    msg[(sizeof(msg) / sizeof(msg[0])) - 1] = 0;

    OutputDebugStringW(msg);
}

template <> void DbgOutputString<char>(const char* lpOutputString, ...)
{
    va_list args;

    va_start(args, lpOutputString);
    DbgOutputStringA(lpOutputString, args);
    va_end(args);
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

template <> void DbgOutputString<wchar_t>(const wchar_t* lpOutputString, ...)
{
    va_list args;

    va_start(args, lpOutputString);
    DbgOutputStringW(lpOutputString, args);
    va_end(args);
}

int DbgTrace(LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg)
{
    static TCHAR dmsg[1024];

    if (iResult != 0)
    {
        snprintf(dmsg, sizeof(dmsg), "%s(%d): %s result=%d\n", strFile, dwLine,
                 strMsg, iResult);
        DbgOutputString(dmsg);
    }
    else
    {
        snprintf(dmsg, sizeof(dmsg), "%s(%d): %s\n", strFile, dwLine, strMsg);
        DbgOutputString(dmsg);
    }

    return iResult;
}

int DbgTraceOnlyNonZero(LPCTSTR strFile, uint dwLine, int iResult,
                        LPCTSTR strMsg)
{
    TCHAR dmsg[1024];

    if (iResult != 0)
    {
        snprintf(dmsg, sizeof(dmsg), "%s(%d): %s result=%d\n", strFile, dwLine,
                 strMsg, iResult);
        DbgOutputString(dmsg);
        DebugBreak();
    }

    return iResult;
}

void DbgTrap(LPCTSTR strFile, uint dwLine, LPCTSTR strMsg)
{
    TCHAR dmsg[1024];

    snprintf(dmsg, sizeof(dmsg), "%s(%d): TRAP: %s\n", strFile, dwLine, strMsg);
    DbgOutputString(dmsg);
    DebugBreak();
}

} // namespace forg::debug
