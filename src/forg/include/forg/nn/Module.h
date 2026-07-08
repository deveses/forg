/*******************************************************************************
    This source file is part of FORG library.

    Base scalar neural-network module type.
*******************************************************************************/

#pragma once
#include "forg/nn/Value.h"

namespace forg::nn {

/// Base class for scalar neural-network modules.
///
/// A module maps a vector of scalar autograd values to another vector and may
/// expose trainable parameters. The default implementation is an identity
/// transform with no parameters.
class Module
{
  public:
    virtual ~Module() = default;

    virtual Values Forward(const Values& input) const;

    /// Switches this module into training or inference behavior.
    virtual void Train(bool training = true);

    /// Convenience wrapper for Train(false).
    void Eval();
    bool Training() const noexcept { return m_training; }

    /// Sets all trainable parameter gradients to zero.
    void ZeroGrad();

    /// Returns trainable parameters in a stable module-defined order.
    virtual Values Parameters() const;

  private:
    bool m_training = true;
};

} // namespace forg::nn
