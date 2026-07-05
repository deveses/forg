// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once

#include "forg/base.h"

#include <cstdarg>

namespace forg::debug {

#if defined(__GNUC__)
#define FORG_PRINTF_FORMAT(format_index, first_argument)                       \
    __attribute__((format(printf, format_index, first_argument)))
#else
#define FORG_PRINTF_FORMAT(format_index, first_argument)
#endif

// #ifdef assert
// #undef assert
// #endif

// #ifdef _DEBUG
// #define assert(value) (void) ((value) || (abort(),0) )
// #define verify(value) assert(value)
// #else
// #define assert(value) ((void)0)
// #define verify(value) ((void)value)
// #endif

FORG_API void DbgOutputStringW(const wchar_t* lpOutputString, va_list args);
FORG_API void DbgOutputStringA(LPCTSTR lpOutputString, va_list args)
    FORG_PRINTF_FORMAT(1, 0);
FORG_API int DbgTrace(LPCTSTR strFile, u32 dwLine, int iResult, LPCTSTR strMsg);
FORG_API int DbgTraceOnlyNonZero(LPCTSTR strFile, u32 dwLine, int iResult,
                                 LPCTSTR strMsg);
FORG_API void DbgTrap(LPCTSTR strFile, u32 dwLine, LPCTSTR strMsg);

template <typename T> void DbgOutputString(const T* lpOutputString, ...);

template <>
FORG_API void DbgOutputString<wchar_t>(const wchar_t* lpOutputString, ...);
template <>
FORG_API void DbgOutputString<char>(const char* lpOutputString, ...)
    FORG_PRINTF_FORMAT(1, 2);

#define RMSG forg::debug::DbgOutputString
#define RTRACE_MSG(strLiteral)                                                 \
    forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define RTRACE_ERR(strLiteral, iResult)                                        \
    forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
#define REXECUTE_ASSERT(bCondition)                                            \
    forg::debug::DbgTraceOnlyIfZero(                                           \
        __FILE__, __LINE__, (bCondition),                                      \
        "Assertion failed! assertion: " #bCondition)

#ifndef NO_DEBUG_MSG

#define DBG_MSG(fmt, ...) forg::debug::DbgOutputString(fmt, ##__VA_ARGS__)

#ifdef _WIN32
#define DBG_TRACE_MSG(strLiteral)                                              \
    forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define DBG_TRACE_ERR(strLiteral, iResult)                                     \
    forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
#else
#define DBG_TRACE_MSG(strLiteral) ((void)0)
#define DBG_TRACE_ERR(strLiteral, iResult) ((void)0)
#endif

#if defined(_MSC_VER)
#define DBG_BREAK() __debugbreak()
#else
#define DBG_BREAK() abort()
#endif

#define REMIND(strLiteral) DBG_TRACE_MSG(strLiteral)
#define ASSERT(bCondition)                                                     \
    if ((bCondition) == 0)                                                     \
    {                                                                          \
        DBG_TRACE_MSG("Assertion failed!");                                    \
        DBG_BREAK();                                                           \
    }
#define EXECUTE_ASSERT(bCondition)                                             \
    forg::debug::DbgTraceOnlyNonZero(                                          \
        __FILE__, __LINE__, (bCondition),                                      \
        "Assertion failed! assertion: " #bCondition)
#define TRAP_NOT_IMPLEMENTED()                                                 \
    forg::debug::DbgTrap(__FILE__, __LINE__, "Not implemented!")

#else

#define DBG_MSG
#define DBG_TRACE_MSG(strLiteral) ((void)0)
#define DBG_TRACE_ERR(strLiteral, iResult) ((void)0)
#define REMIND(strLiteral) ((void)0)
#define ASSERT(bCondition) ((void)0)
#define EXECUTE_ASSERT(bCondition) (bCondition)
#define TRAP_NOT_IMPLEMENTED() ((void)0)

#endif //_DEBUG

} // namespace forg::debug

#undef FORG_PRINTF_FORMAT
