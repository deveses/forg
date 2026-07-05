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

#pragma once
#include "image/Image.h"
#include "rendering/Color.h"

#include <string_view>

namespace forg {

/**
 * Bitmap loading
 * @author eses
 * @version 1.0
 * @date 10-2005
 * @todo compressed bitmaps, 1 and 4 bpp loading, error checks
 * @bug
 * @warning
 */

FORG_API Color4b* LoadBmp(const char* filename, ImageDescription* bmp_info);
FORG_API bool SaveBmp(std::string_view filename, const Color4b* pixels,
                      u32 width, u32 height, u32 rowPitchBytes);

} // namespace forg
