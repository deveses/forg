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

#ifndef _FORG_CPU_VECTOR_SSE_H_
#define _FORG_CPU_VECTOR_SSE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <xmmintrin.h>
#include <emmintrin.h>

namespace forg { namespace cpu { namespace simd {

class vec
{
    __m128 value;

public:
    typedef const vec&  param;
    typedef const vec&  param3;
    typedef vec&        param_out;
    typedef const vec   returned;

    vec() {}
    vec(__m128 x) : value(x) {}
    vec(float x, float y, float z, float w) { value = _mm_set_ps(w, z, y, x); }
    vec(float x, float y, float z) { value = _mm_set_ps(0, z, y, x); }
    explicit vec(float x) { value = _mm_set_ps1(x); }
    explicit vec(bool zero) { value = _mm_setzero_ps(); zero; }

    operator __m128() const { return value; }

    float x() const { float v; _mm_store_ss(&v, value); return v; }
    float y() const { float v; _mm_store_ss(&v, _mm_shuffle_ps(value, value, _MM_SHUFFLE(1,1,1,1))); return v; }
    float z() const { float v; _mm_store_ss(&v, _mm_shuffle_ps(value, value, _MM_SHUFFLE(2,2,2,2))); return v; }
    float w() const { float v; _mm_store_ss(&v, _mm_shuffle_ps(value, value, _MM_SHUFFLE(3,3,3,3))); return v; }

    returned xxxx() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(0,0,0,0)); }
    returned yyyy() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(1,1,1,1)); }
    returned zzzz() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(2,2,2,2)); }
    returned wwww() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(3,3,3,3)); }
    returned wzyx() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(0,1,2,3)); }
    returned yzxw() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(3,0,2,1)); }
    returned zxyw() const { return _mm_shuffle_ps(value, value, _MM_SHUFFLE(3,1,0,2)); }

    void zero() { value = _mm_setzero_ps(); }
    void set(float x, float y, float z, float w) { value = _mm_set_ps(w, z, y, x); }
    void set_vector(float x, float y, float z) { value = _mm_set_ps(0, z, y, x); }
    void set_point(float x, float y, float z) { value = _mm_set_ps(1.0f, z, y, x); }
    void set(float x) { value = _mm_set_ps1(x); }

    void load_u(const float* v) { value = _mm_loadu_ps(v); }
    void load_a(const float* v) { value = _mm_load_ps(v); }
};

}}} // namespace forg::cpu::simd

#endif
