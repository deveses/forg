#include "forg/nn/Activations.h"

#include "ModuleUtils.h"

#include <cstddef>
#include <random>
#include <vector>

namespace forg::nn {
using namespace detail;

Values ReLU::Forward(const Values& input) const
{
    Values output;
    output.reserve(input.size());
    for (const ValuePtr& value : input)
    {
        ValuePtr activated = Relu(value);
        if (!activated)
            return {};
        output.push_back(activated);
    }
    return output;
}

Values Flatten::Forward(const Values& input) const { return input; }

Values Flatten::From(const std::vector<double>& input)
{
    Values output;
    Into(input, output);
    return output;
}

bool Flatten::Into(const std::vector<double>& input, Values& output)
{
    if (output.size() != input.size())
    {
        output.clear();
        output.reserve(input.size());
        for (const double value : input)
        {
            output.push_back(MakeValue(value));
        }
        return true;
    }

    for (std::size_t index = 0; index < input.size(); ++index)
    {
        if (!output[index])
            output[index] = MakeValue(input[index]);
        else
            output[index]->SetData(input[index]);

        output[index]->SetGrad(0.0);
    }
    return true;
}

Values Flatten::FromImage(const std::vector<std::vector<double>>& image)
{
    std::size_t count = 0;
    for (const std::vector<double>& row : image)
    {
        count += row.size();
    }

    Values output;
    output.reserve(count);
    for (const std::vector<double>& row : image)
    {
        for (const double value : row)
        {
            output.push_back(MakeValue(value));
        }
    }
    return output;
}

Dropout::Dropout(double probability)
    : m_probability(ClampProbability(probability)),
      m_rng(std::random_device{}())
{
}

Dropout::Dropout(double probability, std::mt19937& rng)
    : m_probability(ClampProbability(probability)), m_rng(rng)
{
}

Values Dropout::Forward(const Values& input) const
{
    if (m_probability <= 0.0 || !Training())
        return input;

    if (m_probability >= 1.0)
    {
        Values output;
        output.reserve(input.size());
        for (const ValuePtr& value : input)
        {
            if (!value)
                return {};
            output.push_back(MakeValue(0.0));
        }
        return output;
    }

    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    const double scale = 1.0 / (1.0 - m_probability);
    Values output;
    output.reserve(input.size());
    for (const ValuePtr& value : input)
    {
        if (!value)
            return {};

        if (distribution(m_rng) < m_probability)
            output.push_back(MakeValue(0.0));
        else
            output.push_back(value * scale);
    }
    return output;
}

} // namespace forg::nn
