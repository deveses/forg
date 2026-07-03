/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2026  Slawomir Strumecki

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

#ifndef _FORG_PPM_H_
#define _FORG_PPM_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "image/Image.h"
#include "rendering/Color.h"

#include <string_view>

namespace forg {

FORG_API Color4b* LoadPpm(const char* filename, ImageDescription* ppm_info);
FORG_API bool SavePpm(std::string_view filename, const Color4b* pixels,
                      u32 width, u32 height, u32 rowPitchBytes);

} // namespace forg

#endif //_FORG_PPM_H_
