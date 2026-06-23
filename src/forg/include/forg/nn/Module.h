/*******************************************************************************
    This source file is part of FORG library.

    Small neural-network helpers modeled after micrograd's Module, Neuron,
    Layer, and MLP types.
*******************************************************************************/

#ifndef FORG_NN_MODULE_H
#define FORG_NN_MODULE_H

#include "forg/nn/Value.h"

#include <cstddef>
#include <random>
#include <vector>

namespace forg::nn {

class Module
{
  public:
    virtual ~Module() = default;

    void ZeroGrad();
    virtual std::vector<ValuePtr> Parameters() const;
};

class Neuron : public Module
{
  public:
    explicit Neuron(std::size_t input_count, bool nonlin = true);
    Neuron(std::size_t input_count, std::mt19937& rng, bool nonlin = true);

    std::vector<ValuePtr> Forward(const std::vector<ValuePtr>& input) const;
    std::vector<ValuePtr> Parameters() const override;

    const std::vector<ValuePtr>& Weights() const noexcept { return m_weights; }
    const ValuePtr& Bias() const noexcept { return m_bias; }
    bool Nonlinear() const noexcept { return m_nonlin; }

  private:
    std::vector<ValuePtr> m_weights;
    ValuePtr m_bias;
    bool m_nonlin = true;
};

class Layer : public Module
{
  public:
    Layer(std::size_t input_count, std::size_t output_count,
          bool nonlin = true);
    Layer(std::size_t input_count, std::size_t output_count, std::mt19937& rng,
          bool nonlin = true);

    std::vector<ValuePtr> Forward(const std::vector<ValuePtr>& input) const;
    std::vector<ValuePtr> Parameters() const override;

    const std::vector<Neuron>& Neurons() const noexcept { return m_neurons; }

  private:
    std::vector<Neuron> m_neurons;
};

class MLP : public Module
{
  public:
    MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts);
    MLP(std::size_t input_count, const std::vector<std::size_t>& output_counts,
        std::mt19937& rng);

    std::vector<ValuePtr> Forward(const std::vector<ValuePtr>& input) const;
    std::vector<ValuePtr> Parameters() const override;

    const std::vector<Layer>& Layers() const noexcept { return m_layers; }

  private:
    std::vector<Layer> m_layers;
};

} // namespace forg::nn

#endif // FORG_NN_MODULE_H
