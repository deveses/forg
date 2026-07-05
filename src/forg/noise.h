// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
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
    bool m_bInitialized;
    int m_nOctavesCount;
    float m_fPersistence;

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

} // namespace forg
