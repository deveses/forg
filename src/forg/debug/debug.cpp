#include "forg_pch.h"

#include "debug/dbg.h"

#include <stdio.h>

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

        if (c < 1024)
            vstprintf(msg, lpOutputString, args);
        else
            msg[0] = 0;

        OutputDebugString(msg);
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


}}
