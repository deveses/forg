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

#ifndef _CORELIB_DEBUG_H_
#define _CORELIB_DEBUG_H_

#ifdef TRACE_MEMORY_LEAKS

#if defined(new) || defined(_MFC_VER)  // if new is already defined use current definition
#   define OVERRIDE_NEW     new
#else
#   define OVERRIDE_NEW     new(__FILE__,__LINE__)
#   define new              OVERRIDE_NEW
#endif //new

#include "dbg_new.h"

#endif

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


FORG_API void DbgOutputString(LPCTSTR lpOutputString, ...);
FORG_API int DbgTrace( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg);
FORG_API int DbgTraceOnlyNonZero( LPCTSTR strFile, uint dwLine, int iResult, LPCTSTR strMsg);
FORG_API void DbgTrap( LPCTSTR strFile, uint dwLine, LPCTSTR strMsg);

#define RMSG forg::debug::DbgOutputString
#define RTRACE_MSG(strLiteral) forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define RTRACE_ERR(strLiteral, iResult) forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
#define REXECUTE_ASSERT(bCondition) forg::debug::DbgTraceOnlyIfZero(__FILE__, __LINE__, (bCondition), "Assertion failed! assertion: "#bCondition)

#ifndef NO_DEBUG_MSG

#define DBG_MSG forg::debug::DbgOutputString

#ifdef _WIN32
#define DBG_TRACE_MSG(strLiteral) forg::debug::DbgTrace(__FILE__, __LINE__, 0, strLiteral)
#define DBG_TRACE_ERR(strLiteral, iResult) forg::debug::DbgTrace(__FILE__, __LINE__, iResult, strLiteral)
#else
#define DBG_TRACE_MSG(strLiteral) ((void)0)
#define DBG_TRACE_ERR(strLiteral, iResult) ((void)0)
#endif

#define REMIND(strLiteral) DBG_TRACE_MSG(strLiteral)
#define ASSERT(bCondition) if (bCondition == 0) { DBG_TRACE_MSG("Assertion failed!"); abort(); }
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

#endif // _CORELIB_DEBUG_H_
