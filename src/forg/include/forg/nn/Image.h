/*******************************************************************************
    This source file is part of FORG library.

    Scalar image-shaped neural-network modules.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <cstddef>
#include <random>

namespace forg::nn {

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

} // namespace forg::nn
