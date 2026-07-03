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

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "forg/api.h"

#ifdef _MSC_VER
#define FORG_MSVC 1
#else
#define FORG_GNUC 1
#endif

// #include <stdlib.h>
// #include <tchar.h>

// #include <string>
// #include "core/string.hpp"

namespace forg {
////////////////////////////////////////////////////////////////////////////////
// Base functions (maybe should go to base_func.h?)
////////////////////////////////////////////////////////////////////////////////

template <typename T> constexpr T max(T l, T r) { return std::max(l, r); }

template <typename T> constexpr T min(T a, T b) { return std::min(a, b); }

template <typename T> constexpr T clamp(T v, T min, T max)
{
    return std::clamp(v, min, max);
}

// little-endian
constexpr int first_bit(unsigned int x)
{
    return x == 0 ? 31 : std::countr_zero(x);
}

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;

using ushort [[deprecated("Use forg::u16 instead.")]] = u16;
using uint [[deprecated("Use forg::u32 instead.")]] = u32;
using ulong [[deprecated("Use a fixed-width type such as forg::u32 or forg::u64 instead.")]] =
    unsigned long;
using byte = u8;
using uint32 = u32;
using uint64 = unsigned long long;
using int64 = signed long long;

static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(uint64) == 8);
static_assert(sizeof(int64) == 8);

// typedef forg::core::string string;

#define RESULT_NO_ERRORS 0

#define OUT
#define IN

#define _clear(target, size, type) std::memset(target, 0, (size) * sizeof(type))
#define null 0

#define _fget(state, arg) (((arg)) == ((state) & ((arg))))
#define _fadd(state, arg) ((int&)(state) = ((state) | ((arg))))
#define _frem(state, arg) ((int&)(state) = ((state) & ~((arg))))

#ifdef __T
#undef __T
#endif

// =============================================================================
// Unicode handling
// =============================================================================

#ifdef _UNICODE

#define __T(x) L##x
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

#define _T(x) __T(x)
#define _TEXT(x) __T(x)

typedef const TCHAR* LPCTSTR;
typedef const char* LPCSTR;

// =============================================================================

typedef void* HWIN;
typedef void* HANDLE;

#define FORG_OK 0x0000
#define FORG_INVALID_CALL 0x0400

} // namespace forg

#endif //_FORG_BASE_H_
