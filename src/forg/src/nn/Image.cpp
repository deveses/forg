#include "forg/nn/Image.h"

#include "ModuleUtils.h"

#include <cstddef>
#include <random>

namespace forg::nn {
using namespace detail;

Conv2d::Conv2d(std::size_t input_channels, std::size_t output_channels,
               std::size_t input_height, std::size_t input_width,
               std::size_t kernel_height, std::size_t kernel_width,
               std::size_t stride, std::size_t padding)
    : Conv2d(input_channels, output_channels, input_height, input_width,
             kernel_height, kernel_width, stride, padding, DefaultRng())
{
}

Conv2d::Conv2d(std::size_t input_channels, std::size_t output_channels,
               std::size_t input_height, std::size_t input_width,
               std::size_t kernel_height, std::size_t kernel_width,
               std::size_t stride, std::size_t padding, std::mt19937& rng)
    : m_input_channels(input_channels), m_output_channels(output_channels),
      m_input_height(input_height), m_input_width(input_width),
      m_output_height(
          DimensionAfter(input_height, kernel_height, stride, padding)),
      m_output_width(
          DimensionAfter(input_width, kernel_width, stride, padding)),
      m_kernel_height(kernel_height), m_kernel_width(kernel_width),
      m_stride(stride), m_padding(padding)
{
    if (m_input_channels == 0 || m_output_channels == 0 ||
        m_output_height == 0 || m_output_width == 0)
    {
        m_input_channels = 0;
        m_output_channels = 0;
        m_output_height = 0;
        m_output_width = 0;
        return;
    }

    m_weights = MakeWeights(m_output_channels * m_input_channels *
                                m_kernel_height * m_kernel_width,
                            rng);
    m_biases.reserve(m_output_channels);
    for (std::size_t index = 0; index < m_output_channels; ++index)
    {
        m_biases.push_back(MakeValue(0.0));
    }
}

std::size_t Conv2d::WeightIndex(std::size_t output_channel,
                                std::size_t input_channel, std::size_t kernel_y,
                                std::size_t kernel_x) const noexcept
{
    return (((output_channel * m_input_channels + input_channel) *
                 m_kernel_height +
             kernel_y) *
            m_kernel_width) +
           kernel_x;
}

std::size_t Conv2d::InputIndex(std::size_t input_channel, std::size_t input_y,
                               std::size_t input_x) const noexcept
{
    return (input_channel * m_input_height + input_y) * m_input_width + input_x;
}

Values Conv2d::Forward(const Values& input) const
{
    if (m_weights.empty() || m_biases.empty() ||
        !IsInputValid(input, m_input_channels * m_input_height * m_input_width))
    {
        return {};
    }

    Values output;
    output.reserve(m_output_channels * m_output_height * m_output_width);
    for (std::size_t output_channel = 0; output_channel < m_output_channels;
         ++output_channel)
    {
        for (std::size_t output_y = 0; output_y < m_output_height; ++output_y)
        {
            for (std::size_t output_x = 0; output_x < m_output_width;
                 ++output_x)
            {
                ValuePtr activation = m_biases[output_channel];
                for (std::size_t input_channel = 0;
                     input_channel < m_input_channels; ++input_channel)
                {
                    for (std::size_t kernel_y = 0; kernel_y < m_kernel_height;
                         ++kernel_y)
                    {
                        const std::ptrdiff_t input_y =
                            static_cast<std::ptrdiff_t>(output_y * m_stride +
                                                        kernel_y) -
                            static_cast<std::ptrdiff_t>(m_padding);
                        if (input_y < 0 ||
                            input_y >=
                                static_cast<std::ptrdiff_t>(m_input_height))
                        {
                            continue;
                        }

                        for (std::size_t kernel_x = 0;
                             kernel_x < m_kernel_width; ++kernel_x)
                        {
                            const std::ptrdiff_t input_x =
                                static_cast<std::ptrdiff_t>(
                                    output_x * m_stride + kernel_x) -
                                static_cast<std::ptrdiff_t>(m_padding);
                            if (input_x < 0 ||
                                input_x >=
                                    static_cast<std::ptrdiff_t>(m_input_width))
                            {
                                continue;
                            }

                            activation =
                                activation +
                                m_weights[WeightIndex(output_channel,
                                                      input_channel, kernel_y,
                                                      kernel_x)] *
                                    input[InputIndex(
                                        input_channel,
                                        static_cast<std::size_t>(input_y),
                                        static_cast<std::size_t>(input_x))];
                            if (!activation)
                                return {};
                        }
                    }
                }
                output.push_back(activation);
            }
        }
    }
    return output;
}

Values Conv2d::Parameters() const
{
    Values parameters = m_weights;
    parameters.insert(parameters.end(), m_biases.begin(), m_biases.end());
    return parameters;
}

MaxPool2d::MaxPool2d(std::size_t channels, std::size_t input_height,
                     std::size_t input_width, std::size_t kernel_height,
                     std::size_t kernel_width, std::size_t stride)
    : m_channels(channels), m_input_height(input_height),
      m_input_width(input_width),
      m_output_height(DimensionAfter(input_height, kernel_height,
                                     stride == 0 ? kernel_height : stride, 0)),
      m_output_width(DimensionAfter(input_width, kernel_width,
                                    stride == 0 ? kernel_height : stride, 0)),
      m_kernel_height(kernel_height), m_kernel_width(kernel_width),
      m_stride(stride == 0 ? kernel_height : stride)
{
    if (m_channels == 0 || m_output_height == 0 || m_output_width == 0)
    {
        m_channels = 0;
        m_output_height = 0;
        m_output_width = 0;
    }
}

std::size_t MaxPool2d::InputIndex(std::size_t channel, std::size_t input_y,
                                  std::size_t input_x) const noexcept
{
    return (channel * m_input_height + input_y) * m_input_width + input_x;
}

Values MaxPool2d::Forward(const Values& input) const
{
    if (m_channels == 0 ||
        !IsInputValid(input, m_channels * m_input_height * m_input_width))
    {
        return {};
    }

    Values output;
    output.reserve(m_channels * m_output_height * m_output_width);
    for (std::size_t channel = 0; channel < m_channels; ++channel)
    {
        for (std::size_t output_y = 0; output_y < m_output_height; ++output_y)
        {
            for (std::size_t output_x = 0; output_x < m_output_width;
                 ++output_x)
            {
                ValuePtr best;
                for (std::size_t kernel_y = 0; kernel_y < m_kernel_height;
                     ++kernel_y)
                {
                    const std::size_t input_y = output_y * m_stride + kernel_y;
                    for (std::size_t kernel_x = 0; kernel_x < m_kernel_width;
                         ++kernel_x)
                    {
                        const std::size_t input_x =
                            output_x * m_stride + kernel_x;
                        const ValuePtr& candidate =
                            input[InputIndex(channel, input_y, input_x)];
                        if (!best || candidate->GetData() > best->GetData())
                        {
                            best = candidate;
                        }
                    }
                }
                output.push_back(best);
            }
        }
    }
    return output;
}

} // namespace forg::nn
