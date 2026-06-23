#include "forg/nn/Module.h"

#include <random>

namespace forg::nn {
namespace {

std::mt19937& DefaultRng()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

std::vector<ValuePtr> MakeWeights(std::size_t count, std::mt19937& rng)
{
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);
    std::vector<ValuePtr> weights;
    weights.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        weights.push_back(MakeValue(distribution(rng)));
    }
    return weights;
}

bool IsInputValid(const std::vector<ValuePtr>& input, std::size_t expected)
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

} // namespace

void Module::ZeroGrad()
{
    for (const ValuePtr& parameter : Parameters())
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

std::vector<ValuePtr> Module::Parameters() const { return {}; }

Neuron::Neuron(std::size_t input_count, bool nonlin)
    : Neuron(input_count, DefaultRng(), nonlin)
{
}

Neuron::Neuron(std::size_t input_count, std::mt19937& rng, bool nonlin)
    : m_nonlin(nonlin)
{
    if (input_count == 0)
        return;

    m_weights = MakeWeights(input_count, rng);
    m_bias = MakeValue(0.0);
}

std::vector<ValuePtr> Neuron::Forward(const std::vector<ValuePtr>& input) const
{
    if (!m_bias || m_weights.empty() || !IsInputValid(input, m_weights.size()))
        return {};

    ValuePtr activation = m_bias;
    for (std::size_t index = 0; index < m_weights.size(); ++index)
    {
        activation = activation + m_weights[index] * input[index];
        if (!activation)
            return {};
    }

    ValuePtr output = m_nonlin ? Relu(activation) : activation;
    return output ? std::vector<ValuePtr>{output} : std::vector<ValuePtr>{};
}

std::vector<ValuePtr> Neuron::Parameters() const
{
    std::vector<ValuePtr> parameters = m_weights;
    if (m_bias)
        parameters.push_back(m_bias);
    return parameters;
}

Layer::Layer(std::size_t input_count, std::size_t output_count, bool nonlin)
    : Layer(input_count, output_count, DefaultRng(), nonlin)
{
}

Layer::Layer(std::size_t input_count, std::size_t output_count,
             std::mt19937& rng, bool nonlin)
{
    if (input_count == 0 || output_count == 0)
        return;

    m_neurons.reserve(output_count);
    for (std::size_t index = 0; index < output_count; ++index)
    {
        m_neurons.emplace_back(input_count, rng, nonlin);
    }
}

std::vector<ValuePtr> Layer::Forward(const std::vector<ValuePtr>& input) const
{
    if (m_neurons.empty())
        return {};

    std::vector<ValuePtr> output;
    output.reserve(m_neurons.size());
    for (const Neuron& neuron : m_neurons)
    {
        std::vector<ValuePtr> neuron_output = neuron.Forward(input);
        if (neuron_output.empty())
            return {};

        output.push_back(neuron_output.front());
    }
    return output;
}

std::vector<ValuePtr> Layer::Parameters() const
{
    std::vector<ValuePtr> parameters;
    for (const Neuron& neuron : m_neurons)
    {
        std::vector<ValuePtr> neuron_parameters = neuron.Parameters();
        parameters.insert(parameters.end(), neuron_parameters.begin(),
                          neuron_parameters.end());
    }
    return parameters;
}

MLP::MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts)
    : MLP(input_count, output_counts, DefaultRng())
{
}

MLP::MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts,
         std::mt19937& rng)
{
    if (input_count == 0 || output_counts.empty())
        return;

    for (const std::size_t output_count : output_counts)
    {
        if (output_count == 0)
            return;
    }

    std::size_t previous_count = input_count;
    m_layers.reserve(output_counts.size());
    for (std::size_t index = 0; index < output_counts.size(); ++index)
    {
        const std::size_t output_count = output_counts[index];
        const bool nonlin = index + 1 != output_counts.size();
        m_layers.emplace_back(previous_count, output_count, rng, nonlin);
        previous_count = output_count;
    }
}

std::vector<ValuePtr> MLP::Forward(const std::vector<ValuePtr>& input) const
{
    if (m_layers.empty())
        return {};

    std::vector<ValuePtr> output = input;
    for (const Layer& layer : m_layers)
    {
        output = layer.Forward(output);
        if (output.empty())
            return {};
    }
    return output;
}

std::vector<ValuePtr> MLP::Parameters() const
{
    std::vector<ValuePtr> parameters;
    for (const Layer& layer : m_layers)
    {
        std::vector<ValuePtr> layer_parameters = layer.Parameters();
        parameters.insert(parameters.end(), layer_parameters.begin(),
                          layer_parameters.end());
    }
    return parameters;
}

} // namespace forg::nn
