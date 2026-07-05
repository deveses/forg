#include "forg/nn/Layers.h"

#include "ModuleUtils.h"

#include <cstddef>
#include <random>
#include <utility>

namespace forg::nn {
using namespace detail;

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

Values Neuron::Forward(const Values& input) const
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
    return output ? Values{output} : Values{};
}

Values Neuron::Parameters() const
{
    Values parameters = m_weights;
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

Values Layer::Forward(const Values& input) const
{
    if (m_neurons.empty())
        return {};

    Values output;
    output.reserve(m_neurons.size());
    for (const Neuron& neuron : m_neurons)
    {
        Values neuron_output = neuron.Forward(input);
        if (neuron_output.empty())
            return {};

        output.push_back(neuron_output.front());
    }
    return output;
}

Values Layer::Parameters() const
{
    Values parameters;
    for (const Neuron& neuron : m_neurons)
    {
        Values neuron_parameters = neuron.Parameters();
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

Values MLP::Forward(const Values& input) const
{
    if (m_layers.empty())
        return {};

    Values output = input;
    for (const Layer& layer : m_layers)
    {
        output = layer.Forward(output);
        if (output.empty())
            return {};
    }
    return output;
}

Values MLP::Parameters() const
{
    Values parameters;
    for (const Layer& layer : m_layers)
    {
        Values layer_parameters = layer.Parameters();
        parameters.insert(parameters.end(), layer_parameters.begin(),
                          layer_parameters.end());
    }
    return parameters;
}

Linear::Linear(std::size_t input_count, std::size_t output_count)
    : Linear(input_count, output_count, DefaultRng())
{
}

Linear::Linear(std::size_t input_count, std::size_t output_count,
               std::mt19937& rng)
    : m_layer(input_count, output_count, rng, false)
{
}

Values Linear::Forward(const Values& input) const
{
    return m_layer.Forward(input);
}

Values Linear::Parameters() const { return m_layer.Parameters(); }

RNN::RNN(std::size_t input_size, std::size_t hidden_size,
         std::size_t sequence_length)
    : RNN(input_size, hidden_size, sequence_length, DefaultRng())
{
}

RNN::RNN(std::size_t input_size, std::size_t hidden_size,
         std::size_t sequence_length, std::mt19937& rng)
    : m_input_size(input_size), m_hidden_size(hidden_size),
      m_sequence_length(sequence_length)
{
    if (m_input_size == 0 || m_hidden_size == 0 || m_sequence_length == 0)
    {
        m_input_size = 0;
        m_hidden_size = 0;
        m_sequence_length = 0;
        return;
    }

    m_input_weights = MakeWeights(m_hidden_size * m_input_size, rng);
    m_hidden_weights = MakeWeights(m_hidden_size * m_hidden_size, rng);
    m_biases.reserve(m_hidden_size);
    for (std::size_t index = 0; index < m_hidden_size; ++index)
    {
        m_biases.push_back(MakeValue(0.0));
    }
}

std::size_t RNN::InputWeightIndex(std::size_t hidden,
                                  std::size_t input) const noexcept
{
    return hidden * m_input_size + input;
}

std::size_t RNN::HiddenWeightIndex(std::size_t hidden,
                                   std::size_t previous_hidden) const noexcept
{
    return hidden * m_hidden_size + previous_hidden;
}

Values RNN::Forward(const Values& input) const
{
    Values initial_hidden;
    initial_hidden.reserve(m_hidden_size);
    for (std::size_t index = 0; index < m_hidden_size; ++index)
    {
        initial_hidden.push_back(MakeValue(0.0));
    }
    return Forward(input, initial_hidden);
}

Values RNN::Forward(const Values& input, const Values& initial_hidden) const
{
    if (m_input_weights.empty() || m_hidden_weights.empty() ||
        m_biases.empty() ||
        !IsInputValid(input, m_sequence_length * m_input_size) ||
        !IsInputValid(initial_hidden, m_hidden_size))
    {
        return {};
    }

    Values previous_hidden = initial_hidden;
    Values output;
    output.reserve(m_sequence_length * m_hidden_size);

    for (std::size_t timestep = 0; timestep < m_sequence_length; ++timestep)
    {
        Values current_hidden;
        current_hidden.reserve(m_hidden_size);
        for (std::size_t hidden = 0; hidden < m_hidden_size; ++hidden)
        {
            ValuePtr activation = m_biases[hidden];
            for (std::size_t input_index = 0; input_index < m_input_size;
                 ++input_index)
            {
                activation =
                    activation +
                    m_input_weights[InputWeightIndex(hidden, input_index)] *
                        input[timestep * m_input_size + input_index];
                if (!activation)
                    return {};
            }

            for (std::size_t previous = 0; previous < m_hidden_size;
                 ++previous)
            {
                activation =
                    activation +
                    m_hidden_weights[HiddenWeightIndex(hidden, previous)] *
                        previous_hidden[previous];
                if (!activation)
                    return {};
            }

            ValuePtr value = Tanh(activation);
            if (!value)
                return {};

            current_hidden.push_back(value);
            output.push_back(value);
        }
        previous_hidden = std::move(current_hidden);
    }

    return output;
}

Values RNN::Parameters() const
{
    Values parameters = m_input_weights;
    parameters.insert(parameters.end(), m_hidden_weights.begin(),
                      m_hidden_weights.end());
    parameters.insert(parameters.end(), m_biases.begin(), m_biases.end());
    return parameters;
}

} // namespace forg::nn
