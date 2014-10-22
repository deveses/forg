/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#pragma once

#include "base.h"

namespace forg { namespace debug {

//#ifdef assert
//#undef assert
//#endif

//#ifdef _DEBUG
//#define assert(value) (void) ((value) || (abort(),0) )
//#define verify(value) assert(value)
//#else
//#define assert(value) ((void)0)
//#define verify(value) ((void)value)
//#endif

FORG_API void DbgOutputStringW(const wchar_t* lpOutputString, va_list args);
FORG_API void DbgOutputStringA(LPCTSTR lpOutputString, va_list args);
FORG_API int DbgTrace( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg);
FORG_API int DbgTraceOnlyNonZero( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg);
FORG_API void DbgTrap( LPCTSTR strFile, uint dwLine, LPCTSTR strMsg);

template <typename T>
void DbgOutputString(const T* lpOutputString, ...);

template <>
FORG_API void DbgOutputString<wchar_t>(const wchar_t* lpOutputString, ...);
template <>
FORG_API void DbgOutputString<char>(const char* lpOutputString, ...);

#define RMSG forg::debug::DbgOutputString
#define RTRACE_MSG(strLiteral) forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define RTRACE_ERR(strLiteral, iResult) forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
#define REXECUTE_ASSERT(bCondition) forg::debug::DbgTraceOnlyIfZero(__FILE__, __LINE__, (bCondition), "Assertion failed! assertion: "#bCondition)

#ifndef NO_DEBUG_MSG

#define DBG_MSG(fmt, ...) forg::debug::DbgOutputString(fmt, ##__VA_ARGS__)

#ifdef _WIN32
#define DBG_TRACE_MSG(strLiteral) forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define DBG_TRACE_ERR(strLiteral, iResult) forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
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
#define ASSERT(bCondition) if (bCondition == 0) { DBG_TRACE_MSG("Assertion failed!"); DBG_BREAK(); }
#define EXECUTE_ASSERT(bCondition) forg::debug::DbgTraceOnlyNonZero(__FILE__, __LINE__, (bCondition), "Assertion failed! assertion: "#bCondition)
#define TRAP_NOT_IMPLEMENTED() forg::debug::DbgTrap(__FILE__, __LINE__, "Not implemented!")

#else

#define DBG_MSG
#define DBG_TRACE_MSG(strLiteral) ((void)0)
#define DBG_TRACE_ERR(strLiteral, iResult) ((void)0)
#define REMIND(strLiteral) ((void)0)
#define ASSERT(bCondition) ((void)0)
#define EXECUTE_ASSERT(bCondition) (bCondition)
#define TRAP_NOT_IMPLEMENTED() ((void)0)

#endif //_DEBUG


}}
