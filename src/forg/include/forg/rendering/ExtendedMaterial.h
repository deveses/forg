#pragma once
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
