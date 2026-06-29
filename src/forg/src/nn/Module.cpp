#include "forg/nn/Module.h"

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

} // namespace forg::nn
