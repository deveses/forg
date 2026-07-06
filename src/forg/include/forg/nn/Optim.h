/*******************************************************************************
    This source file is part of FORG library.

    Scalar optimizers.
*******************************************************************************/

#pragma once
#include "forg/nn/Value.h"

#include <cstddef>
#include <vector>

namespace forg::nn {

/// Stochastic gradient descent optimizer over Value parameters.
///
/// Gradients are expected to have been populated by Backward(). Step(scale) is
/// useful for averaged mini-batch gradient accumulation.
class SGD
{
  public:
    SGD(Values parameters, double learning_rate);

    void ZeroGrad();
    void Step();
    void Step(double gradient_scale);

    double LearningRate() const noexcept { return m_learning_rate; }
    const Values& Parameters() const noexcept { return m_parameters; }

  private:
    Values m_parameters;
    double m_learning_rate = 0.0;
};

/// SGD with momentum over Value parameters.
///
/// Step(scale) applies the scale to current gradients before updating the
/// momentum buffer, which supports averaged mini-batch accumulation.
class MomentumSGD
{
  public:
    MomentumSGD(Values parameters, double learning_rate, double momentum = 0.9);

    void ZeroGrad();
    void Step();
    void Step(double gradient_scale);

    double LearningRate() const noexcept { return m_learning_rate; }
    double Momentum() const noexcept { return m_momentum; }
    const Values& Parameters() const noexcept { return m_parameters; }
    const std::vector<double>& Velocity() const noexcept { return m_velocity; }

  private:
    Values m_parameters;
    std::vector<double> m_velocity;
    double m_learning_rate = 0.0;
    double m_momentum = 0.0;
};

/// Adam optimizer over Value parameters.
///
/// Maintains first and second moment estimates with bias correction.
/// Step(scale) applies the scale to current gradients before updating the
/// moment buffers.
class Adam
{
  public:
    Adam(Values parameters, double learning_rate = 0.001, double beta1 = 0.9,
         double beta2 = 0.999, double epsilon = 1e-8);

    void ZeroGrad();
    void Step();
    void Step(double gradient_scale);

    double LearningRate() const noexcept { return m_learning_rate; }
    double Beta1() const noexcept { return m_beta1; }
    double Beta2() const noexcept { return m_beta2; }
    double Epsilon() const noexcept { return m_epsilon; }
    std::size_t StepCount() const noexcept { return m_step; }
    const Values& Parameters() const noexcept { return m_parameters; }
    const std::vector<double>& FirstMoment() const noexcept
    {
        return m_first_moment;
    }
    const std::vector<double>& SecondMoment() const noexcept
    {
        return m_second_moment;
    }

  private:
    Values m_parameters;
    std::vector<double> m_first_moment;
    std::vector<double> m_second_moment;
    double m_learning_rate = 0.0;
    double m_beta1 = 0.0;
    double m_beta2 = 0.0;
    double m_epsilon = 1e-8;
    std::size_t m_step = 0;
};

} // namespace forg::nn
