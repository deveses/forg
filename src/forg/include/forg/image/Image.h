// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2008 Slawomir Strumecki

#pragma once
#include "base.h"

#include <span>
#include <string_view>
#include <vector>

namespace forg {

struct ImageDescription
{
    u32 Width;
    u32 Height;
    u32 Bpp;
};

/// Raw image with mipmaps
class Image
{
  public:
    // 'structors
    Image();
    ~Image();

    // Attributes
  private:
    std::vector<std::vector<char>> m_data; ///< array of mipmaps
    u32 m_width;
    u32 m_height;
    u32 m_num_mipmaps;

    // Public Methods
  public:
    bool Load(std::string_view filename);
    bool Save(std::string_view filename) const;

    const char* GetData(u32 _level = 0) const;

    u32 GetSize(u32 _level = 0) const;

    u32 GetWidth() const { return m_width; };

    u32 GetHeight() const { return m_height; };

    u32 GetWidth(u32 _level) const;

    u32 GetHeight(u32 _level) const;

    /// Change image size
    /**
     * @param _width new width, 0 - no change
     * @param _height new height, 0 - no change
     */
    void Resize(u32 _width, u32 _height);

    /**
     * Applies a convolution kernel to the base image level.
     *
     * Kernel dimensions must be odd and non-zero. Sampling outside the image
     * clamps to the nearest edge pixel. Generated mipmaps are discarded because
     * the base pixels have changed.
     *
     * @param _kernel row-major kernel values
     * @param _kernel_width kernel width, must be odd
     * @param _kernel_height kernel height, must be odd
     * @param _scale multiplier applied to the accumulated color
     * @param _bias value added to each filtered channel
     * @param _preserve_alpha keep the source alpha channel unchanged
     * @return true if the filter was applied
     */
    bool ApplyConvolution(std::span<const float> _kernel, u32 _kernel_width,
                          u32 _kernel_height, float _scale = 1.0f,
                          float _bias = 0.0f, bool _preserve_alpha = true);

    bool ApplyBoxBlur();
    bool ApplyGaussianBlur();
    bool ApplySharpen();
    bool ApplyEdgeDetect();

    /**
     * Generates mipmaps chain
     * @return Number of mipmaps
     */
    u32 GenerateMipmaps();

  private:
    void Clean();
};

} // namespace forg
