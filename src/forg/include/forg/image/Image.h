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

#ifndef _FORG_IMAGE_H_
#define _FORG_IMAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

namespace forg {

struct ImageDescription
{
    uint Width;
    uint Height;
    uint Bpp;
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
    char** m_data;  ///< array of mipmaps
    uint m_width;
    uint m_height;
    uint m_num_mipmaps;

    // Public Methods
public:
    bool Load(const char* _filename);

    const char* GetData(uint _level = 0) const;

    uint GetSize(uint _level = 0) const;

    uint GetWidth() const { return m_width; };

    uint GetHeight() const { return m_height; };

    uint GetWidth(uint _level) const;

    uint GetHeight(uint _level) const;

    /// Change image size
    /**
    * @param _width new width, 0 - no change
    * @param _height new height, 0 - no change
    */
    void Resize(uint _width, uint _height);

    /**
    * Generates mipmaps chain
    * @return Number of mipmaps
    */
    uint GenerateMipmaps();
private:
    void Clean();

};


}

#endif  //_FORG_IMAGE_H_
