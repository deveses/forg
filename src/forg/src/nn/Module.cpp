#include "forg/nn/Module.h"

#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <utility>

namespace forg::nn {
namespace {

std::mt19937& DefaultRng()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

Values MakeWeights(std::size_t count, std::mt19937& rng)
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

double ClampProbability(double probability)
{
    if (!(probability > 0.0))
        return 0.0;
    if (probability >= 1.0)
        return 1.0;
    return probability;
}

double PositiveEpsilon(double epsilon)
{
    if (!(epsilon > 0.0))
        return 1e-5;
    return epsilon;
}

double ValidBeta(double beta, double fallback)
{
    if (beta <= 0.0 || beta >= 1.0)
        return fallback;
    return beta;
}

std::size_t DimensionAfter(std::size_t input_size, std::size_t kernel_size,
                           std::size_t stride, std::size_t padding)
{
    if (input_size == 0 || kernel_size == 0 || stride == 0)
        return 0;

    const std::size_t padded_size = input_size + 2 * padding;
    if (padded_size < kernel_size)
        return 0;

    return (padded_size - kernel_size) / stride + 1;
}

bool IsInputValid(const Values& input, std::size_t expected)
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

void SetError(std::string* error, const std::string& message)
{
    if (error)
        *error = message;
}

} // namespace

Values Module::Forward(const Values& input) const { return input; }

void Module::Train(bool training) { m_training = training; }

void Module::Eval() { Train(false); }

void Module::ZeroGrad()
{
    for (const ValuePtr& parameter : Parameters())
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

Values Module::Parameters() const { return {}; }

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

BatchNorm::BatchNorm(std::size_t feature_count, double epsilon)
    : m_feature_count(feature_count), m_epsilon(PositiveEpsilon(epsilon))
{
    if (m_feature_count == 0)
        return;

    m_scale.reserve(m_feature_count);
    m_bias.reserve(m_feature_count);
    for (std::size_t index = 0; index < m_feature_count; ++index)
    {
        m_scale.push_back(MakeValue(1.0));
        m_bias.push_back(MakeValue(0.0));
    }
}

Values BatchNorm::Forward(const Values& input) const
{
    if (m_scale.empty() || m_bias.empty() ||
        !IsInputValid(input, m_feature_count))
    {
        return {};
    }

    ValuePtr mean = MakeValue(0.0);
    for (const ValuePtr& value : input)
    {
        mean = mean + value;
        if (!mean)
            return {};
    }
    mean = mean / static_cast<double>(m_feature_count);

    ValuePtr variance = MakeValue(0.0);
    for (const ValuePtr& value : input)
    {
        const ValuePtr centered = value - mean;
        variance = variance + centered * centered;
        if (!variance)
            return {};
    }
    variance = variance / static_cast<double>(m_feature_count);

    const ValuePtr inv_std = Pow(variance + m_epsilon, -0.5);
    if (!inv_std)
        return {};

    Values output;
    output.reserve(m_feature_count);
    for (std::size_t index = 0; index < m_feature_count; ++index)
    {
        const ValuePtr normalized = (input[index] - mean) * inv_std;
        ValuePtr value = m_scale[index] * normalized + m_bias[index];
        if (!value)
            return {};
        output.push_back(value);
    }
    return output;
}

Values BatchNorm::Parameters() const
{
    Values parameters = m_scale;
    parameters.insert(parameters.end(), m_bias.begin(), m_bias.end());
    return parameters;
}

Sequential::Sequential(std::vector<std::shared_ptr<Module>> modules)
    : m_modules(std::move(modules))
{
}

void Sequential::Add(std::shared_ptr<Module> module)
{
    if (module)
        m_modules.push_back(std::move(module));
}

Values Sequential::Forward(const Values& input) const
{
    Values output = input;
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (!module)
            return {};

        output = module->Forward(output);
        if (output.empty())
            return {};
    }
    return output;
}

void Sequential::Train(bool training)
{
    Module::Train(training);
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (module)
            module->Train(training);
    }
}

Values Sequential::Parameters() const
{
    Values parameters;
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (!module)
            continue;

        Values module_parameters = module->Parameters();
        parameters.insert(parameters.end(), module_parameters.begin(),
                          module_parameters.end());
    }
    return parameters;
}

ValuePtr MSELoss(const Values& prediction, const Values& target)
{
    if (prediction.empty() || prediction.size() != target.size())
        return nullptr;

    ValuePtr loss = MakeValue(0.0);
    for (std::size_t index = 0; index < prediction.size(); ++index)
    {
        if (!prediction[index] || !target[index])
            return nullptr;

        const ValuePtr error = prediction[index] - target[index];
        loss = loss + error * error;
        if (!loss)
            return nullptr;
    }
    return loss / static_cast<double>(prediction.size());
}

Values Softmax(const Values& logits)
{
    if (logits.empty())
        return {};

    for (const ValuePtr& logit : logits)
    {
        if (!logit)
            return {};
    }

    double max_logit = logits.front()->GetData();
    for (const ValuePtr& logit : logits)
    {
        if (logit->GetData() > max_logit)
            max_logit = logit->GetData();
    }

    Values exps;
    exps.reserve(logits.size());
    for (const ValuePtr& logit : logits)
    {
        exps.push_back(Exp(logit - max_logit));
        if (!exps.back())
            return {};
    }

    ValuePtr sum = exps.front();
    for (std::size_t index = 1; index < exps.size(); ++index)
    {
        sum = sum + exps[index];
        if (!sum)
            return {};
    }

    Values probabilities;
    probabilities.reserve(exps.size());
    for (const ValuePtr& value : exps)
    {
        probabilities.push_back(value / sum);
        if (!probabilities.back())
            return {};
    }
    return probabilities;
}

ValuePtr CrossEntropyLoss(const Values& logits, std::size_t target_index)
{
    if (target_index >= logits.size())
        return nullptr;

    const Values probabilities = Softmax(logits);
    if (probabilities.empty())
        return nullptr;

    return -Log(probabilities[target_index]);
}

ValuePtr CrossEntropyLoss(const Values& logits, const Values& target)
{
    if (logits.empty() || logits.size() != target.size())
        return nullptr;

    const Values probabilities = Softmax(logits);
    if (probabilities.empty())
        return nullptr;

    ValuePtr loss = MakeValue(0.0);
    for (std::size_t index = 0; index < target.size(); ++index)
    {
        if (!target[index])
            return nullptr;

        loss = loss - target[index] * Log(probabilities[index]);
        if (!loss)
            return nullptr;
    }
    return loss;
}

Values OneHot(std::size_t class_count, std::size_t index)
{
    Values output;
    OneHotInto(class_count, index, output);
    return output;
}

bool OneHotInto(std::size_t class_count, std::size_t index, Values& output)
{
    if (class_count == 0 || index >= class_count)
    {
        output.clear();
        return false;
    }

    if (output.size() != class_count)
    {
        output.clear();
        output.reserve(class_count);
        for (std::size_t class_index = 0; class_index < class_count;
             ++class_index)
        {
            output.push_back(MakeValue(class_index == index ? 1.0 : 0.0));
        }
        return true;
    }

    for (std::size_t class_index = 0; class_index < class_count; ++class_index)
    {
        if (!output[class_index])
            output[class_index] = MakeValue(class_index == index ? 1.0 : 0.0);
        else
            output[class_index]->SetData(class_index == index ? 1.0 : 0.0);

        output[class_index]->SetGrad(0.0);
    }
    return true;
}

std::size_t ArgMax(const Values& values)
{
    if (values.empty())
        return std::numeric_limits<std::size_t>::max();

    std::size_t best_index = 0;
    for (std::size_t index = 1; index < values.size(); ++index)
    {
        if (!values[index])
            continue;

        if (!values[best_index] ||
            values[index]->GetData() > values[best_index]->GetData())
        {
            best_index = index;
        }
    }
    return best_index;
}

bool SaveParameters(const Module& module, const std::string& filename,
                    std::string* error)
{
    const Values parameters = module.Parameters();
    for (const ValuePtr& parameter : parameters)
    {
        if (!parameter)
        {
            SetError(error, "Cannot save null model parameter");
            return false;
        }
    }

    std::ofstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open parameter file for writing");
        return false;
    }

    stream << "FORG_NN_PARAMETERS 1\n";
    stream << parameters.size() << '\n';
    stream << std::setprecision(std::numeric_limits<double>::max_digits10);
    for (const ValuePtr& parameter : parameters)
    {
        stream << parameter->GetData() << '\n';
    }

    if (!stream)
    {
        SetError(error, "Unable to write parameter file");
        return false;
    }

    SetError(error, {});
    return true;
}

bool LoadParameters(Module& module, const std::string& filename,
                    std::string* error)
{
    std::ifstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open parameter file for reading");
        return false;
    }

    std::string magic;
    int version = 0;
    if (!(stream >> magic >> version) || magic != "FORG_NN_PARAMETERS" ||
        version != 1)
    {
        SetError(error, "Invalid parameter file header");
        return false;
    }

    std::size_t count = 0;
    if (!(stream >> count))
    {
        SetError(error, "Invalid parameter count");
        return false;
    }

    const Values parameters = module.Parameters();
    if (parameters.size() != count)
    {
        std::ostringstream message;
        message << "Parameter count mismatch: file has " << count
                << ", model expects " << parameters.size();
        SetError(error, message.str());
        return false;
    }

    std::vector<double> values;
    values.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        double value = 0.0;
        if (!(stream >> value))
        {
            SetError(error, "Invalid parameter value");
            return false;
        }
        values.push_back(value);
    }

    for (const ValuePtr& parameter : parameters)
    {
        if (!parameter)
        {
            SetError(error, "Cannot load null model parameter");
            return false;
        }
    }

    for (std::size_t index = 0; index < parameters.size(); ++index)
    {
        parameters[index]->SetData(values[index]);
        parameters[index]->SetGrad(0.0);
    }

    SetError(error, {});
    return true;
}

SGD::SGD(Values parameters, double learning_rate)
    : m_parameters(std::move(parameters)), m_learning_rate(learning_rate)
{
}

void SGD::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void SGD::Step() { Step(1.0); }

void SGD::Step(double gradient_scale)
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (!parameter)
            continue;

        parameter->SetData(parameter->GetData() - m_learning_rate *
                                                      gradient_scale *
                                                      parameter->GetGrad());
    }
}

MomentumSGD::MomentumSGD(Values parameters, double learning_rate,
                         double momentum)
    : m_parameters(std::move(parameters)), m_velocity(m_parameters.size(), 0.0),
      m_learning_rate(learning_rate),
      m_momentum(momentum < 0.0 ? 0.0 : momentum)
{
}

void MomentumSGD::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void MomentumSGD::Step() { Step(1.0); }

void MomentumSGD::Step(double gradient_scale)
{
    for (std::size_t index = 0; index < m_parameters.size(); ++index)
    {
        const ValuePtr& parameter = m_parameters[index];
        if (!parameter)
            continue;

        m_velocity[index] = m_momentum * m_velocity[index] +
                            gradient_scale * parameter->GetGrad();
        parameter->SetData(parameter->GetData() -
                           m_learning_rate * m_velocity[index]);
    }
}

Adam::Adam(Values parameters, double learning_rate, double beta1, double beta2,
           double epsilon)
    : m_parameters(std::move(parameters)),
      m_first_moment(m_parameters.size(), 0.0),
      m_second_moment(m_parameters.size(), 0.0), m_learning_rate(learning_rate),
      m_beta1(ValidBeta(beta1, 0.9)), m_beta2(ValidBeta(beta2, 0.999)),
      m_epsilon(PositiveEpsilon(epsilon))
{
}

void Adam::ZeroGrad()
{
    for (const ValuePtr& parameter : m_parameters)
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

void Adam::Step() { Step(1.0); }

void Adam::Step(double gradient_scale)
{
    ++m_step;
    const double beta1_correction = 1.0 - std::pow(m_beta1, m_step);
    const double beta2_correction = 1.0 - std::pow(m_beta2, m_step);

    for (std::size_t index = 0; index < m_parameters.size(); ++index)
    {
        const ValuePtr& parameter = m_parameters[index];
        if (!parameter)
            continue;

        const double grad = gradient_scale * parameter->GetGrad();
        m_first_moment[index] =
            m_beta1 * m_first_moment[index] + (1.0 - m_beta1) * grad;
        m_second_moment[index] =
            m_beta2 * m_second_moment[index] + (1.0 - m_beta2) * grad * grad;

        const double first_unbiased = m_first_moment[index] / beta1_correction;
        const double second_unbiased =
            m_second_moment[index] / beta2_correction;
        parameter->SetData(parameter->GetData() -
                           m_learning_rate * first_unbiased /
                               (std::sqrt(second_unbiased) + m_epsilon));
    }
}

} // namespace forg::nn
