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

LayerNorm::LayerNorm(std::size_t feature_count)
    : LayerNorm(feature_count, 1)
{
}

LayerNorm::LayerNorm(std::size_t feature_count, std::size_t sequence_length,
                     double epsilon)
    : m_feature_count(feature_count), m_sequence_length(sequence_length),
      m_epsilon(PositiveEpsilon(epsilon))
{
    if (m_feature_count == 0 || m_sequence_length == 0)
    {
        m_feature_count = 0;
        m_sequence_length = 0;
        return;
    }

    m_scale.reserve(m_feature_count);
    m_bias.reserve(m_feature_count);
    for (std::size_t index = 0; index < m_feature_count; ++index)
    {
        m_scale.push_back(MakeValue(1.0));
        m_bias.push_back(MakeValue(0.0));
    }
}

Values LayerNorm::Forward(const Values& input) const
{
    if (m_scale.empty() || m_bias.empty() ||
        !IsInputValid(input, m_feature_count * m_sequence_length))
    {
        return {};
    }

    Values output;
    output.reserve(input.size());
    for (std::size_t timestep = 0; timestep < m_sequence_length; ++timestep)
    {
        const std::size_t offset = timestep * m_feature_count;

        ValuePtr mean = MakeValue(0.0);
        for (std::size_t feature = 0; feature < m_feature_count; ++feature)
        {
            mean = mean + input[offset + feature];
            if (!mean)
                return {};
        }
        mean = mean / static_cast<double>(m_feature_count);

        ValuePtr variance = MakeValue(0.0);
        for (std::size_t feature = 0; feature < m_feature_count; ++feature)
        {
            const ValuePtr centered = input[offset + feature] - mean;
            variance = variance + centered * centered;
            if (!variance)
                return {};
        }
        variance = variance / static_cast<double>(m_feature_count);

        const ValuePtr inv_std = Pow(variance + m_epsilon, -0.5);
        if (!inv_std)
            return {};

        for (std::size_t feature = 0; feature < m_feature_count; ++feature)
        {
            const ValuePtr normalized = (input[offset + feature] - mean) *
                                       inv_std;
            ValuePtr value = m_scale[feature] * normalized + m_bias[feature];
            if (!value)
                return {};
            output.push_back(value);
        }
    }
    return output;
}

Values LayerNorm::Parameters() const
{
    Values parameters = m_scale;
    parameters.insert(parameters.end(), m_bias.begin(), m_bias.end());
    return parameters;
}

} // namespace forg::nn
