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
#include <string>
#include <vector>

namespace forg::nn {

/// Base class for scalar neural-network modules.
///
/// A module maps a vector of scalar autograd values to another vector and may
/// expose trainable parameters. The default implementation is an identity
/// transform with no parameters.
class Module
{
  public:
    virtual ~Module() = default;

    virtual Values Forward(const Values& input) const;

    /// Switches this module into training or inference behavior.
    virtual void Train(bool training = true);

    /// Convenience wrapper for Train(false).
    void Eval();
    bool Training() const noexcept { return m_training; }

    /// Sets all trainable parameter gradients to zero.
    void ZeroGrad();

    /// Returns trainable parameters in a stable module-defined order.
    virtual Values Parameters() const;

  private:
    bool m_training = true;
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

/// Applies ReLU element-wise to a vector of Values.
class ReLU : public Module
{
  public:
    Values Forward(const Values& input) const override;
};

/// Pass-through module plus helpers for making flat Value vectors.
///
/// Useful for converting numeric samples, such as normalized image pixels, into
/// scalar autograd inputs.
class Flatten : public Module
{
  public:
    Values Forward(const Values& input) const override;

    static Values From(const std::vector<double>& input);
    static bool Into(const std::vector<double>& input, Values& output);
    static Values FromImage(const std::vector<std::vector<double>>& image);
};

/// Randomly drops input activations during training.
///
/// Uses inverted dropout: kept values are scaled by 1 / (1 - probability), so
/// Eval() mode can return inputs unchanged.
class Dropout : public Module
{
  public:
    explicit Dropout(double probability);
    Dropout(double probability, std::mt19937& rng);

    Values Forward(const Values& input) const override;

    double Probability() const noexcept { return m_probability; }

  private:
    double m_probability = 0.0;
    mutable std::mt19937 m_rng;
};

/// Scalar 2D convolution over flattened channel-first image data.
///
/// Inputs are laid out as channel * height * width + y * width + x. This module
/// is correct and differentiable through the scalar autograd engine, but it is
/// not a tensor-kernel implementation.
class Conv2d : public Module
{
  public:
    Conv2d(std::size_t input_channels, std::size_t output_channels,
           std::size_t input_height, std::size_t input_width,
           std::size_t kernel_height, std::size_t kernel_width,
           std::size_t stride = 1, std::size_t padding = 0);
    Conv2d(std::size_t input_channels, std::size_t output_channels,
           std::size_t input_height, std::size_t input_width,
           std::size_t kernel_height, std::size_t kernel_width,
           std::size_t stride, std::size_t padding, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    std::size_t InputChannels() const noexcept { return m_input_channels; }
    std::size_t OutputChannels() const noexcept { return m_output_channels; }
    std::size_t InputHeight() const noexcept { return m_input_height; }
    std::size_t InputWidth() const noexcept { return m_input_width; }
    std::size_t OutputHeight() const noexcept { return m_output_height; }
    std::size_t OutputWidth() const noexcept { return m_output_width; }
    std::size_t KernelHeight() const noexcept { return m_kernel_height; }
    std::size_t KernelWidth() const noexcept { return m_kernel_width; }
    std::size_t Stride() const noexcept { return m_stride; }
    std::size_t Padding() const noexcept { return m_padding; }
    const Values& Weights() const noexcept { return m_weights; }
    const Values& Biases() const noexcept { return m_biases; }

  private:
    std::size_t WeightIndex(std::size_t output_channel,
                            std::size_t input_channel, std::size_t kernel_y,
                            std::size_t kernel_x) const noexcept;
    std::size_t InputIndex(std::size_t input_channel, std::size_t input_y,
                           std::size_t input_x) const noexcept;

    std::size_t m_input_channels = 0;
    std::size_t m_output_channels = 0;
    std::size_t m_input_height = 0;
    std::size_t m_input_width = 0;
    std::size_t m_output_height = 0;
    std::size_t m_output_width = 0;
    std::size_t m_kernel_height = 0;
    std::size_t m_kernel_width = 0;
    std::size_t m_stride = 1;
    std::size_t m_padding = 0;
    Values m_weights;
    Values m_biases;
};

/// Scalar 2D max pooling over flattened channel-first image data.
///
/// Inputs use the same channel-first layout as Conv2d. The output reuses the
/// selected input ValuePtr, so gradients flow to the maximum element.
class MaxPool2d : public Module
{
  public:
    MaxPool2d(std::size_t channels, std::size_t input_height,
              std::size_t input_width, std::size_t kernel_height,
              std::size_t kernel_width, std::size_t stride = 0);

    Values Forward(const Values& input) const override;

    std::size_t Channels() const noexcept { return m_channels; }
    std::size_t InputHeight() const noexcept { return m_input_height; }
    std::size_t InputWidth() const noexcept { return m_input_width; }
    std::size_t OutputHeight() const noexcept { return m_output_height; }
    std::size_t OutputWidth() const noexcept { return m_output_width; }
    std::size_t KernelHeight() const noexcept { return m_kernel_height; }
    std::size_t KernelWidth() const noexcept { return m_kernel_width; }
    std::size_t Stride() const noexcept { return m_stride; }

  private:
    std::size_t InputIndex(std::size_t channel, std::size_t input_y,
                           std::size_t input_x) const noexcept;

    std::size_t m_channels = 0;
    std::size_t m_input_height = 0;
    std::size_t m_input_width = 0;
    std::size_t m_output_height = 0;
    std::size_t m_output_width = 0;
    std::size_t m_kernel_height = 0;
    std::size_t m_kernel_width = 0;
    std::size_t m_stride = 1;
};

/// Feature-wise affine normalization for a single Value vector.
///
/// Normalizes the current input vector, then applies trainable scale and bias
/// parameters. This v1 helper does not track running statistics.
class BatchNorm : public Module
{
  public:
    explicit BatchNorm(std::size_t feature_count, double epsilon = 1e-5);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;

    std::size_t FeatureCount() const noexcept { return m_feature_count; }
    double Epsilon() const noexcept { return m_epsilon; }
    const Values& Scale() const noexcept { return m_scale; }
    const Values& Bias() const noexcept { return m_bias; }

  private:
    std::size_t m_feature_count = 0;
    double m_epsilon = 1e-5;
    Values m_scale;
    Values m_bias;
};

/// Runs a list of modules in order.
///
/// Parameters are returned in module order. Train() and Eval() propagate to
/// child modules.
class Sequential : public Module
{
  public:
    Sequential() = default;
    explicit Sequential(std::vector<std::shared_ptr<Module>> modules);

    void Add(std::shared_ptr<Module> module);
    Values Forward(const Values& input) const override;
    void Train(bool training = true) override;
    Values Parameters() const override;

    const std::vector<std::shared_ptr<Module>>& Modules() const noexcept
    {
        return m_modules;
    }

  private:
    std::vector<std::shared_ptr<Module>> m_modules;
};

/// Mean squared error over prediction and target vectors.
ValuePtr MSELoss(const Values& prediction, const Values& target);

/// Numerically shifted softmax over raw logits.
Values Softmax(const Values& logits);

/// Cross-entropy loss for raw logits and a class index target.
ValuePtr CrossEntropyLoss(const Values& logits, std::size_t target_index);

/// Cross-entropy loss for raw logits and a one-hot or soft target vector.
ValuePtr CrossEntropyLoss(const Values& logits, const Values& target);

/// Creates a one-hot vector with class_count entries.
Values OneHot(std::size_t class_count, std::size_t index);

/// Reuses or creates one-hot Value storage in output.
bool OneHotInto(std::size_t class_count, std::size_t index, Values& output);

/// Returns the index of the largest data value, or max size_t for empty input.
std::size_t ArgMax(const Values& values);

/// Saves only the ordered parameter values for a module.
///
/// Recreate the same architecture before loading these parameters.
bool SaveParameters(const Module& module, const std::string& filename,
                    std::string* error = nullptr);

/// Loads ordered parameter values into an existing module.
bool LoadParameters(Module& module, const std::string& filename,
                    std::string* error = nullptr);

/// Stochastic gradient descent optimizer over Value parameters.
///
/// Gradients are expected to have been populated by Backward(). Step(scale) is
/// useful for averaged mini-batch gradient accumulation.
class SGD
{
  public:
    SGD(Values parameters, double learning_rate);

    void ZeroGrad();
    void Step();
    void Step(double gradient_scale);

    double LearningRate() const noexcept { return m_learning_rate; }
    const Values& Parameters() const noexcept { return m_parameters; }

  private:
    Values m_parameters;
    double m_learning_rate = 0.0;
};

} // namespace forg::nn

#endif // FORG_NN_MODULE_H
