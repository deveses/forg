/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2008  Slawomir Strumecki

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

#ifndef FORG_IMAGE_IMAGE_H_
#define FORG_IMAGE_IMAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

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
     * Generates mipmaps chain
     * @return Number of mipmaps
     */
    u32 GenerateMipmaps();

  private:
    void Clean();
};

} // namespace forg

#endif // FORG_IMAGE_IMAGE_H_
