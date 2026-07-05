// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

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
