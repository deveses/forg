#include "forg/nn/Normalization.h"

#include "ModuleUtils.h"

#include <cstddef>

namespace forg::nn {
using namespace detail;

BatchNorm::BatchNorm(std::size_t feature_count, double epsilon)
    : m_feature_count(feature_count), m_epsilon(PositiveEpsilon(epsilon))
{
    if (m_feature_count == 0)
        return;

    m_scale.reserve(m_feature_count);
    m_bias.reserve(m_feature_count);
    for (std::size_t index = 0; index < m_feature_count; ++index)
    {
        m_scale.push_back(MakeValue(1.0));
        m_bias.push_back(MakeValue(0.0));
    }
}

Values BatchNorm::Forward(const Values& input) const
{
    if (m_scale.empty() || m_bias.empty() ||
        !IsInputValid(input, m_feature_count))
    {
        return {};
    }

    ValuePtr mean = MakeValue(0.0);
    for (const ValuePtr& value : input)
    {
        mean = mean + value;
        if (!mean)
            return {};
    }
    mean = mean / static_cast<double>(m_feature_count);

    ValuePtr variance = MakeValue(0.0);
    for (const ValuePtr& value : input)
    {
        const ValuePtr centered = value - mean;
        variance = variance + centered * centered;
        if (!variance)
            return {};
    }
    variance = variance / static_cast<double>(m_feature_count);

    const ValuePtr inv_std = Pow(variance + m_epsilon, -0.5);
    if (!inv_std)
        return {};

    Values output;
    output.reserve(m_feature_count);
    for (std::size_t index = 0; index < m_feature_count; ++index)
    {
        const ValuePtr normalized = (input[index] - mean) * inv_std;
        ValuePtr value = m_scale[index] * normalized + m_bias[index];
        if (!value)
            return {};
        output.push_back(value);
    }
    return output;
}

Values BatchNorm::Parameters() const
{
    Values parameters = m_scale;
    parameters.insert(parameters.end(), m_bias.begin(), m_bias.end());
    return parameters;
}

} // namespace forg::nn
