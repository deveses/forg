// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "math/Vector3.h"

namespace forg {

using namespace math;

struct Ray
{
    Vector3 Origin;
    Vector3 Direction;

    bool TriangleIntersection(const Vector3& vert0, const Vector3& vert1,
                              const Vector3& vert2, float epsilon, bool culling,
                              float& t, float& u, float& v) const;
};

} // namespace forg
