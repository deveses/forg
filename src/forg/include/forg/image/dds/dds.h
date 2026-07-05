// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2008 Slawomir Strumecki

#pragma once
#include "image/Image.h"
#include "rendering/Color.h"

namespace forg {

FORG_API Color4b* LoadDds(const char* filename, ImageDescription* bmp_info);

}
