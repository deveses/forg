#include "forg_pch.h"

#include "debug/dbg.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <Windows.h>
#else
    void OutputDebugString(const char* text)
    {
        printf("%s\n", text);
    }

    void DebugBreak()
    {
        __asm("int3");
    }
#endif

namespace forg { namespace debug {


    void DbgOutputString(LPCTSTR lpOutputString, ...)
    {
        static TCHAR msg[1024];
        va_list args;

        va_start(args, lpOutputString);

        int c = _vscprintf(lpOutputString, args);

        _vsnprintf(msg, sizeof(msg), lpOutputString, args);

        OutputDebugString(msg);
    }

    void DbgOutputStringA(const char* lpOutputString, va_list args)
    {
        static char msg[1024];

        int c = _vscprintf(lpOutputString, args);

        _vsnprintf(msg, sizeof(msg), lpOutputString, args);

        OutputDebugStringA(msg);
    }

    void DbgOutputStringW(const wchar_t* lpOutputString, va_list args)
    {
        static wchar_t msg[1024];

        int c = _vscwprintf(lpOutputString, args);

        _vsnwprintf(msg, sizeof(msg), lpOutputString, args);

        OutputDebugStringW(msg);
    }

    template <>
    void DbgOutputString<char>(const char* lpOutputString, ...)
    {
        va_list args;

        va_start(args, lpOutputString);
        DbgOutputStringA(lpOutputString, args);
        va_end(args);
    }

    template <>
    void DbgOutputString<wchar_t>(const wchar_t* lpOutputString, ...)
    {
        va_list args;

        va_start(args, lpOutputString);
        DbgOutputStringW(lpOutputString, args);
        va_end(args);
    }

    int DbgTrace( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg)
    {
        static TCHAR dmsg[1024];

        if (iResult != 0)
        {
            sprintf(dmsg, "%s(%d): %s result=%d\n", strFile, dwLine, strMsg, iResult);
            DbgOutputString(dmsg);
        } else
        {
            sprintf(dmsg, "%s(%d): %s\n", strFile, dwLine, strMsg);
            DbgOutputString(dmsg);
        }

        return iResult;
    }

    int DbgTraceOnlyNonZero( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg)
    {
        TCHAR dmsg[1024];

        if (iResult != 0)
        {
            sprintf(dmsg, "%s(%d): %s result=%d\n", strFile, dwLine, strMsg, iResult);
            DbgOutputString(dmsg);
            DebugBreak();
        }

        return iResult;
    }

	void DbgTrap(LPCTSTR strFile, uint dwLine, LPCTSTR strMsg)
	{
		TCHAR dmsg[1024];

		sprintf(dmsg, "%s(%d): TRAP: %s\n", strFile, dwLine, strMsg);
		DbgOutputString(dmsg);
		DebugBreak();
	}

}}
