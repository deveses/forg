// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/base.h"

#include <cmath>

namespace forg::audio::dsp {

struct float4
{
    float x;
    float y;
    float z;
    float w;

    const float& operator[](uint32 _index) const
    {
        return ((float*)this)[_index];
    }
    float& operator[](uint32 _index) { return ((float*)this)[_index]; }

    void set(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }
};

float dot(const float4& lhs, const float4& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

__inline float RadiansToCutoffFrequency(float Radians, float SampleRate)
{
    return SampleRate * std::asin(Radians / 2.0f) /
           (float)3.1415926535897932384626433832795;
}

void CalcLowpassCoeffs(float Fc, float Q, float4& coeffsA, float4& coeffsB);
} // namespace forg::audio::dsp
