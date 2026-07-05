#pragma once
#include "forg/nn/Value.h"

#include <cmath>
#include <cstddef>
#include <random>
#include <string>

namespace forg::nn::detail {

inline std::mt19937& DefaultRng()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

inline Values MakeWeights(std::size_t count, std::mt19937& rng)
{
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);
    Values weights;
    weights.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        weights.push_back(MakeValue(distribution(rng)));
    }
    return weights;
}

inline double ClampProbability(double probability)
{
    if (!(probability > 0.0))
        return 0.0;
    if (probability >= 1.0)
        return 1.0;
    return probability;
}

inline double PositiveEpsilon(double epsilon)
{
    if (!(epsilon > 0.0))
        return 1e-5;
    return epsilon;
}

inline double ValidBeta(double beta, double fallback)
{
    if (beta <= 0.0 || beta >= 1.0)
        return fallback;
    return beta;
}

inline std::size_t DimensionAfter(std::size_t input_size,
                                  std::size_t kernel_size, std::size_t stride,
                                  std::size_t padding)
{
    if (input_size == 0 || kernel_size == 0 || stride == 0)
        return 0;

    const std::size_t padded_size = input_size + 2 * padding;
    if (padded_size < kernel_size)
        return 0;

    return (padded_size - kernel_size) / stride + 1;
}

inline bool IsInputValid(const Values& input, std::size_t expected)
{
    if (input.size() != expected)
        return false;

    for (const ValuePtr& value : input)
    {
        if (!value)
            return false;
    }
    return true;
}

inline bool TryValueIndex(const ValuePtr& value, std::size_t upper_bound,
                          std::size_t& index)
{
    if (!value)
        return false;

    const double data = value->GetData();
    if (!std::isfinite(data) || data < 0.0 || data != std::floor(data) ||
        data >= static_cast<double>(upper_bound))
    {
        return false;
    }

    index = static_cast<std::size_t>(data);
    return true;
}

inline void SetError(std::string* error, const std::string& message)
{
    if (error)
        *error = message;
}

} // namespace forg::nn::detail
