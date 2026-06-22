#ifndef FORG_RENDERING_EXTENDED_MATERIAL_H
#define FORG_RENDERING_EXTENDED_MATERIAL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/string.hpp"
#include "rendering/Material.h"

namespace forg {

using namespace forg::math;
using namespace forg::core;

/**
 *
 */
struct ExtendedMaterial
{
    ////////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////////

    /// Material structure that describes the material properties.
    Material Material3D;

    /// String that specifies the file name of the texture.
    string TextureFilename;
};
} // namespace forg

#endif // FORG_RENDERING_EXTENDED_MATERIAL_H
