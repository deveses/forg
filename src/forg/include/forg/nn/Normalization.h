/*******************************************************************************
    This source file is part of FORG library.

    Scalar normalization modules.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <cstddef>

namespace forg::nn {

/// Feature-wise affine normalization for a single Value vector.
///
/// Normalizes the current input vector, then applies trainable scale and bias
/// parameters. This v1 helper does not track running statistics.
class BatchNorm : public Module
{
  public:
    explicit BatchNorm(std::size_t feature_count, double epsilon = 1e-5);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    std::size_t FeatureCount() const noexcept { return m_feature_count; }
    double Epsilon() const noexcept { return m_epsilon; }
    const Values& Scale() const noexcept { return m_scale; }
    const Values& Bias() const noexcept { return m_bias; }

  private:
    std::size_t m_feature_count = 0;
    double m_epsilon = 1e-5;
    Values m_scale;
    Values m_bias;
};

} // namespace forg::nn
