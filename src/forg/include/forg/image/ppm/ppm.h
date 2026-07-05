// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Slawomir Strumecki

#pragma once
#include "image/Image.h"
#include "rendering/Color.h"

#include <string_view>

namespace forg {

FORG_API Color4b* LoadPpm(const char* filename, ImageDescription* ppm_info);
FORG_API bool SavePpm(std::string_view filename, const Color4b* pixels,
                      u32 width, u32 height, u32 rowPitchBytes);

} // namespace forg
