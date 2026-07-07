/*******************************************************************************
    This source file is part of FORG library.

    Dense and recurrent scalar neural-network layers.
*******************************************************************************/

#pragma once
#define FORG_NN_INCLUDING_LAYERS_H
#include "forg/nn/Module.h"
#undef FORG_NN_INCLUDING_LAYERS_H

#include <cstddef>
#include <random>
#include <vector>

namespace forg::nn {

/// Full recurrent output plus the final state needed by encoder-decoder models.
struct RecurrentState
{
    Values sequence;
    Values hidden;
    Values cell;
};

/// One fully connected scalar neuron.
///
/// Computes dot(input, weights) + bias and optionally applies ReLU. This is the
/// building block used by Layer and MLP.
class Neuron : public Module
{
  public:
    explicit Neuron(std::size_t input_count, bool nonlin = true);
    Neuron(std::size_t input_count, std::mt19937& rng, bool nonlin = true);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    const Values& Weights() const noexcept { return m_weights; }
    const ValuePtr& Bias() const noexcept { return m_bias; }
    bool Nonlinear() const noexcept { return m_nonlin; }

  private:
    Values m_weights;
    ValuePtr m_bias;
    bool m_nonlin = true;
};

/// A dense layer made of independent Neuron instances.
///
/// For input size I and output size O, Forward() expects I values and returns O
/// values.
class Layer : public Module
{
  public:
    Layer(std::size_t input_count, std::size_t output_count,
          bool nonlin = true);
    Layer(std::size_t input_count, std::size_t output_count, std::mt19937& rng,
          bool nonlin = true);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    const std::vector<Neuron>& Neurons() const noexcept { return m_neurons; }

  private:
    std::vector<Neuron> m_neurons;
};

/// A simple multi-layer perceptron.
///
/// Creates one Layer per requested output size. Hidden layers use ReLU, while
/// the final layer is linear.
class MLP : public Module
{
  public:
    MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts);
    MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts,
        std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    const std::vector<Layer>& Layers() const noexcept { return m_layers; }

  private:
    std::vector<Layer> m_layers;
};

/// A PyTorch-style affine dense layer.
///
/// This is a thin wrapper around Layer with no built-in activation.
class Linear : public Module
{
  public:
    Linear(std::size_t input_count, std::size_t output_count);
    Linear(std::size_t input_count, std::size_t output_count,
           std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    const Layer& InnerLayer() const noexcept { return m_layer; }

  private:
    Layer m_layer;
};

/// Simple Elman recurrent neural-network layer over flat time-major input.
///
/// Forward(input) expects sequence_length * input_size Values and returns
/// sequence_length * hidden_size hidden activations flattened in time-major
/// order.
class RNN : public Module
{
  public:
    RNN(std::size_t input_size, std::size_t hidden_size,
        std::size_t sequence_length);
    RNN(std::size_t input_size, std::size_t hidden_size,
        std::size_t sequence_length, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& input, const Values& initial_hidden) const;
    RecurrentState ForwardState(const Values& input) const;
    RecurrentState ForwardState(const Values& input,
                                const Values& initial_hidden) const;
    Values Parameters() const override;

    std::size_t InputSize() const noexcept { return m_input_size; }
    std::size_t HiddenSize() const noexcept { return m_hidden_size; }
    std::size_t SequenceLength() const noexcept { return m_sequence_length; }
    const Values& InputWeights() const noexcept { return m_input_weights; }
    const Values& HiddenWeights() const noexcept { return m_hidden_weights; }
    const Values& Biases() const noexcept { return m_biases; }

  private:
    std::size_t InputWeightIndex(std::size_t hidden,
                                 std::size_t input) const noexcept;
    std::size_t HiddenWeightIndex(std::size_t hidden,
                                  std::size_t previous_hidden) const noexcept;

    std::size_t m_input_size = 0;
    std::size_t m_hidden_size = 0;
    std::size_t m_sequence_length = 0;
    Values m_input_weights;
    Values m_hidden_weights;
    Values m_biases;
};

/// Long short-term memory layer over flat time-major input.
///
/// Parameters are stored in gate-major order: input gate, forget gate, cell
/// candidate, then output gate. Forward(input) returns hidden activations for
/// every timestep flattened in time-major order.
class LSTM : public Module
{
  public:
    LSTM(std::size_t input_size, std::size_t hidden_size,
         std::size_t sequence_length);
    LSTM(std::size_t input_size, std::size_t hidden_size,
         std::size_t sequence_length, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& input, const Values& initial_hidden,
                   const Values& initial_cell) const;
    RecurrentState ForwardState(const Values& input) const;
    RecurrentState ForwardState(const Values& input,
                                const Values& initial_hidden,
                                const Values& initial_cell) const;
    Values Parameters() const override;

    std::size_t InputSize() const noexcept { return m_input_size; }
    std::size_t HiddenSize() const noexcept { return m_hidden_size; }
    std::size_t SequenceLength() const noexcept { return m_sequence_length; }
    const Values& InputWeights() const noexcept { return m_input_weights; }
    const Values& HiddenWeights() const noexcept { return m_hidden_weights; }
    const Values& Biases() const noexcept { return m_biases; }

  private:
    std::size_t InputWeightIndex(std::size_t gate, std::size_t hidden,
                                 std::size_t input) const noexcept;
    std::size_t HiddenWeightIndex(std::size_t gate, std::size_t hidden,
                                  std::size_t previous_hidden) const noexcept;
    std::size_t BiasIndex(std::size_t gate, std::size_t hidden) const noexcept;

    std::size_t m_input_size = 0;
    std::size_t m_hidden_size = 0;
    std::size_t m_sequence_length = 0;
    Values m_input_weights;
    Values m_hidden_weights;
    Values m_biases;
};

/// Gated recurrent unit layer over flat time-major input.
///
/// Parameters are stored in gate-major order: reset gate, update gate, then
/// candidate hidden state. Forward(input) returns hidden activations for every
/// timestep flattened in time-major order.
class GRU : public Module
{
  public:
    GRU(std::size_t input_size, std::size_t hidden_size,
        std::size_t sequence_length);
    GRU(std::size_t input_size, std::size_t hidden_size,
        std::size_t sequence_length, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& input, const Values& initial_hidden) const;
    RecurrentState ForwardState(const Values& input) const;
    RecurrentState ForwardState(const Values& input,
                                const Values& initial_hidden) const;
    Values Parameters() const override;

    std::size_t InputSize() const noexcept { return m_input_size; }
    std::size_t HiddenSize() const noexcept { return m_hidden_size; }
    std::size_t SequenceLength() const noexcept { return m_sequence_length; }
    const Values& InputWeights() const noexcept { return m_input_weights; }
    const Values& HiddenWeights() const noexcept { return m_hidden_weights; }
    const Values& Biases() const noexcept { return m_biases; }

  private:
    std::size_t InputWeightIndex(std::size_t gate, std::size_t hidden,
                                 std::size_t input) const noexcept;
    std::size_t HiddenWeightIndex(std::size_t gate, std::size_t hidden,
                                  std::size_t previous_hidden) const noexcept;
    std::size_t BiasIndex(std::size_t gate, std::size_t hidden) const noexcept;

    std::size_t m_input_size = 0;
    std::size_t m_hidden_size = 0;
    std::size_t m_sequence_length = 0;
    Values m_input_weights;
    Values m_hidden_weights;
    Values m_biases;
};

} // namespace forg::nn
