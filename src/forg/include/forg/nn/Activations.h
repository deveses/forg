/*******************************************************************************
    This source file is part of FORG library.

    Scalar activation and utility modules.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <random>
#include <vector>

namespace forg::nn {

/// Applies ReLU element-wise to a vector of Values.
class ReLU : public Module
{
  public:
    Values Forward(const Values& input) const override;
};

/// Pass-through module plus helpers for making flat Value vectors.
///
/// Useful for converting numeric samples, such as normalized image pixels, into
/// scalar autograd inputs.
class Flatten : public Module
{
  public:
    Values Forward(const Values& input) const override;

    static Values From(const std::vector<double>& input);
    static bool Into(const std::vector<double>& input, Values& output);
    static Values FromImage(const std::vector<std::vector<double>>& image);
};

/// Randomly drops input activations during training.
///
/// Uses inverted dropout: kept values are scaled by 1 / (1 - probability), so
/// Eval() mode can return inputs unchanged.
class Dropout : public Module
{
  public:
    explicit Dropout(double probability);
    Dropout(double probability, std::mt19937& rng);

    Values Forward(const Values& input) const override;

    double Probability() const noexcept { return m_probability; }

  private:
    double m_probability = 0.0;
    mutable std::mt19937 m_rng;
};

} // namespace forg::nn
