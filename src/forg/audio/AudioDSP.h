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

#ifndef _FORG_AUDIO_AUDIODSP_H_
#define _FORG_AUDIO_AUDIODSP_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

namespace forg { namespace audio { namespace dsp {

    struct float4
    {
        float x;
        float y;
        float z;
        float w;

        const float& operator [] (uint32 _index) const { return ((float*)this)[_index]; }
        float& operator [] (uint32 _index) { return ((float*)this)[_index]; }

        void set(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
    };

    float dot(const float4& lhs, const float4& rhs) { return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z + lhs.w*rhs.w; }

    __inline float RadiansToCutoffFrequency(float Radians, float SampleRate)
    {
        return SampleRate * asinf(Radians / 2.0f) / (float)3.1415926535897932384626433832795;
    }

    void CalcLowpassCoeffs( float Fc, float Q, float4& coeffsA, float4& coeffsB );
}}}

#endif