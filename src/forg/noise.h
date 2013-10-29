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

#ifndef _FORG_NOISE_H_
#define _FORG_NOISE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "base.h"

namespace forg {

// Ken Perlin noise
class FORG_API PerlinNoise
{
    public:
    PerlinNoise();

    //////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////
    private:
    bool    m_bInitialized;
    int     m_nOctavesCount;
    float   m_fPersistence;

    public:
    void set_Persistence(float p) { m_fPersistence = p; }
    void set_OctavesCount(int n) { m_nOctavesCount = (n > 0 ? n : 0); }

    //////////////////////////////////////////////////////////////////////
    // Public methods
    //////////////////////////////////////////////////////////////////////
    public:
    float int_noise2d(float x, float y);
    float grad_noise2d(float x, float y);
    float simplex_noise2d(float x, float y);

    //////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////
    private:
    // perlin / gradient method
    void init();
    void grad2d(float x, float y, float& gx, float& gy);
    float GradientNoise_2D(float x, float y);

    // other / prg method
    float int_noise(int x);

    float SmoothNoise_1D(int x);
    float InterpolatedNoise_1D(float x);
    float PerlinNoise_1D(float x);

    float Noise2D(float x, float y);
    float SmoothNoise_2D(float x, float y);
    float InterpolatedNoise_2D(float x, float y);
    float PerlinNoise_2D(float x, float y);
};


}

#endif  // _FORG_NOISE_H_

