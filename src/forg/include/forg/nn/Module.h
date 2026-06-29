/*******************************************************************************
    This source file is part of FORG library.

    Small neural-network helpers modeled after micrograd's Module, Neuron,
    Layer, and MLP types.
*******************************************************************************/

#ifndef FORG_NN_MODULE_H
#define FORG_NN_MODULE_H

#include "forg/nn/Value.h"

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

namespace forg::nn {

class Module
{
  public:
    virtual ~Module() = default;

    virtual Values Forward(const Values& input) const;
    void ZeroGrad();
    virtual Values Parameters() const;
};

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

class ReLU : public Module
{
  public:
    Values Forward(const Values& input) const override;
};

class Flatten : public Module
{
  public:
    Values Forward(const Values& input) const override;

    static Values From(const std::vector<double>& input);
    static bool Into(const std::vector<double>& input, Values& output);
    static Values FromImage(const std::vector<std::vector<double>>& image);
};

class Sequential : public Module
{
  public:
    Sequential() = default;
    explicit Sequential(std::vector<std::shared_ptr<Module>> modules);

    void Add(std::shared_ptr<Module> module);
    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    const std::vector<std::shared_ptr<Module>>& Modules() const noexcept
    {
        return m_modules;
    }

  private:
    std::vector<std::shared_ptr<Module>> m_modules;
};

ValuePtr MSELoss(const Values& prediction, const Values& target);
Values OneHot(std::size_t class_count, std::size_t index);
bool OneHotInto(std::size_t class_count, std::size_t index, Values& output);
std::size_t ArgMax(const Values& values);

class SGD
{
  public:
    SGD(Values parameters, double learning_rate);

    void ZeroGrad();
    void Step();

    double LearningRate() const noexcept { return m_learning_rate; }
    const Values& Parameters() const noexcept { return m_parameters; }

  private:
    Values m_parameters;
    double m_learning_rate = 0.0;
};

} // namespace forg::nn

#endif // FORG_NN_MODULE_H
