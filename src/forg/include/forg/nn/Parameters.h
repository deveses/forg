/*******************************************************************************
    This source file is part of FORG library.

    Scalar module parameter serialization helpers.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <string>

namespace forg::nn {

/// Saves only the ordered parameter values for a module.
///
/// Recreate the same architecture before loading these parameters.
bool SaveParameters(const Module& module, const std::string& filename,
                    std::string* error = nullptr);

/// Loads ordered parameter values into an existing module.
bool LoadParameters(Module& module, const std::string& filename,
                    std::string* error = nullptr);

} // namespace forg::nn
