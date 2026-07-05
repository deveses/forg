// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "rendering/Mesh.h"

namespace forg::xfile {

using namespace forg;
using namespace forg::geometry;

/// Mesh loader from DirectX files
class XLoader
{
  public:
    static Mesh::MeshPtr Load(const char* filename, u32 options,
                              IRenderDevice* device,
                              Mesh::ExtendedMaterialVec& materials);
};
} // namespace forg::xfile
