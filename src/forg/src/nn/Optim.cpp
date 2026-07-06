#include "forg/nn/Optim.h"

#include "ModuleUtils.h"

#include <cmath>
#include <cstddef>
#include <utility>

namespace forg::nn {
using namespace detail;

SGD::SGD(Values parameters, double learning_rate)
    : m_parameters(std::move(parameters)), m_learning_rate(learning_rate)
{
}

void SGD::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void SGD::Step() { Step(1.0); }

void SGD::Step(double gradient_scale)
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (!parameter)
            continue;

        parameter->SetData(parameter->GetData() - m_learning_rate *
                                                      gradient_scale *
                                                      parameter->GetGrad());
    }
}

MomentumSGD::MomentumSGD(Values parameters, double learning_rate,
                         double momentum)
    : m_parameters(std::move(parameters)), m_velocity(m_parameters.size(), 0.0),
      m_learning_rate(learning_rate),
      m_momentum(momentum < 0.0 ? 0.0 : momentum)
{
}

void MomentumSGD::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void MomentumSGD::Step() { Step(1.0); }

void MomentumSGD::Step(double gradient_scale)
{
    for (std::size_t index = 0; index < m_parameters.size(); ++index)
    {
        const ValuePtr& parameter = m_parameters[index];
        if (!parameter)
            continue;

        m_velocity[index] = m_momentum * m_velocity[index] +
                            gradient_scale * parameter->GetGrad();
        parameter->SetData(parameter->GetData() -
                           m_learning_rate * m_velocity[index]);
    }
}

Adam::Adam(Values parameters, double learning_rate, double beta1, double beta2,
           double epsilon)
    : m_parameters(std::move(parameters)),
      m_first_moment(m_parameters.size(), 0.0),
      m_second_moment(m_parameters.size(), 0.0), m_learning_rate(learning_rate),
      m_beta1(ValidBeta(beta1, 0.9)), m_beta2(ValidBeta(beta2, 0.999)),
      m_epsilon(PositiveEpsilon(epsilon))
{
}

void Adam::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void Adam::Step() { Step(1.0); }

void Adam::Step(double gradient_scale)
{
    ++m_step;
    const double beta1_correction = 1.0 - std::pow(m_beta1, m_step);
    const double beta2_correction = 1.0 - std::pow(m_beta2, m_step);

    for (std::size_t index = 0; index < m_parameters.size(); ++index)
    {
        const ValuePtr& parameter = m_parameters[index];
        if (!parameter)
            continue;

        const double grad = gradient_scale * parameter->GetGrad();
        m_first_moment[index] =
            m_beta1 * m_first_moment[index] + (1.0 - m_beta1) * grad;
        m_second_moment[index] =
            m_beta2 * m_second_moment[index] + (1.0 - m_beta2) * grad * grad;

        const double first_unbiased = m_first_moment[index] / beta1_correction;
        const double second_unbiased =
            m_second_moment[index] / beta2_correction;
        parameter->SetData(parameter->GetData() -
                           m_learning_rate * first_unbiased /
                               (std::sqrt(second_unbiased) + m_epsilon));
    }
}

} // namespace forg::nn
