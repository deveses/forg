/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef _FORG_BASE_H_
#define _FORG_BASE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "api.h"

#ifdef _MSC_VER
#   define FORG_MSVC 1
#else
#   define FORG_GNUC 1
#endif

#include <stdlib.h>
//#include <tchar.h>

//#include <string>
//#include "core/string.hpp"

namespace forg {
////////////////////////////////////////////////////////////////////////////////
// Base functions (maybe should go to base_func.h?)
////////////////////////////////////////////////////////////////////////////////

template <typename T>
T max(T l, T r) 
{
    return (l>r ? l : r);
}

template <typename T> 
T clamp(T v, T min, T max) 
{
    if (v > max)
        return max;

    if (v < min)
        return min;

    return v;
}

// little-endian
inline int first_bit(unsigned int x)
{
    int ret;

    ret  = (x & 0x0000ffffUL) ? 0 : 16;
    ret += (x & 0x00ff00ffUL) ? 0 : 8;  // 0xff = 11111111
    ret += (x & 0x0f0f0f0fUL) ? 0 : 4;  // 0x0f = 00001111
    ret += (x & 0x33333333UL) ? 0 : 2;  // 0x33 = 00110011
    ret += (x & 0x55555555UL) ? 0 : 1;  // 0x55 = 01010101

    return ret;
}

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char byte;
typedef unsigned int uint32;

#ifdef FORG_MSVC
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
#else
typedef unsigned long long uint64;
typedef signed long long int64;
#endif

//typedef forg::core::string string;

#define RESULT_NO_ERRORS 0

#define OUT
#define IN

#define _clear(target, size, type) 	memset(target, 0, (size) * sizeof(type))
#define null 0

template<typename T> 
T min(T a, T b) {
    return (((a) < (b)) ? (a) : (b));
}

#define _fget(state, arg)			((arg) == (state & (arg)))
#define _fadd(state, arg) 			((int&)state = (state | (arg)))
#define _frem(state, arg) 			((int&)state = (state & ~(arg)))

#ifdef __T
#undef __T
#endif

// =============================================================================
// Unicode handling
// =============================================================================

#ifdef _UNICODE

#define __T(x) L ## x
typedef wchar_t TCHAR;

#define vstprintf vswprintf

#else

#define __T(x) x
typedef char TCHAR;

#define vstprintf vsprintf

#endif //_UNICODE

#ifdef _T
#undef _T
#endif

#ifdef _TEXT
#undef _TEXT
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)

typedef const TCHAR*	LPCTSTR;
typedef const char*		LPCSTR;

// =============================================================================


typedef void* HWIN;
typedef void* HANDLE;

#define FORG_OK				0x0000
#define FORG_INVALID_CALL	0x0400

} // namespace forg

#endif //_FORG_BASE_H_


//#define GL_NO_ERROR                       0
//#define GL_INVALID_ENUM                   0x0500
//#define GL_INVALID_VALUE                  0x0501
//#define GL_INVALID_OPERATION              0x0502
//#define GL_STACK_OVERFLOW                 0x0503
//#define GL_STACK_UNDERFLOW                0x0504
//#define GL_OUT_OF_MEMORY                  0x0505
//
//#define D3DERR_WRONGTEXTUREFORMAT               MAKE_D3DHRESULT(2072)
//#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_D3DHRESULT(2073)
//#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_D3DHRESULT(2074)
//#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_D3DHRESULT(2075)
//#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_D3DHRESULT(2076)
//#define D3DERR_TOOMANYOPERATIONS                MAKE_D3DHRESULT(2077)
//#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_D3DHRESULT(2078)
//#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_D3DHRESULT(2079)
//#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_D3DHRESULT(2081)
//#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_D3DHRESULT(2082)
//#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_D3DHRESULT(2086)
//#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)
//#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
//#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
//#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
//#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
//#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
//#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
//#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
//#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)
//#define D3DERR_DRIVERINVALIDCALL                MAKE_D3DHRESULT(2157)
//#define D3DERR_WASSTILLDRAWING                  MAKE_D3DHRESULT(540)
//#define D3DOK_NOAUTOGEN                         MAKE_D3DSTATUS(2159)

