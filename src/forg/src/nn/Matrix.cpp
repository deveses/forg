#include "forg/nn/Matrix.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <thread>
#include <utility>

namespace forg::nn {
namespace {

void SetError(std::string* error, const std::string& message)
{
    if (error)
        *error = message;
}

std::mt19937& DefaultRng()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

std::size_t DefaultThreadCount()
{
    const unsigned int thread_count = std::thread::hardware_concurrency();
    return thread_count == 0 ? 1 : static_cast<std::size_t>(thread_count);
}

std::size_t EffectiveThreadCount(std::size_t requested, std::size_t rows)
{
    if (rows <= 1)
        return 1;

    const std::size_t thread_count =
        requested == 0 ? DefaultThreadCount() : requested;
    return std::max<std::size_t>(1, std::min(thread_count, rows));
}

template <typename Fn>
void ParallelRows(std::size_t rows, std::size_t requested_threads, Fn&& fn)
{
    const std::size_t thread_count =
        EffectiveThreadCount(requested_threads, rows);
    if (thread_count <= 1)
    {
        fn(0, rows, 0);
        return;
    }

    const std::size_t rows_per_thread =
        (rows + thread_count - 1) / thread_count;
    std::vector<std::thread> threads;
    threads.reserve(thread_count - 1);
    for (std::size_t thread_index = 1; thread_index < thread_count;
         ++thread_index)
    {
        const std::size_t begin = thread_index * rows_per_thread;
        const std::size_t end = std::min(rows, begin + rows_per_thread);
        if (begin >= end)
            break;

        threads.emplace_back([&fn, begin, end, thread_index]()
                             { fn(begin, end, thread_index); });
    }

    fn(0, std::min(rows, rows_per_thread), 0);
    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

double InitLimit(std::size_t input_count, std::size_t output_count)
{
    if (input_count == 0 || output_count == 0)
        return 0.0;

    return std::sqrt(6.0 / static_cast<double>(input_count + output_count));
}

void InitWeights(Matrix& weights, std::mt19937& rng)
{
    std::uniform_real_distribution<double> distribution(
        -InitLimit(weights.Columns(), weights.Rows()),
        InitLimit(weights.Columns(), weights.Rows()));
    for (double& value : weights.Data())
    {
        value = distribution(rng);
    }
}

std::size_t ArgMaxRow(const Matrix& matrix, std::size_t row)
{
    if (matrix.Columns() == 0)
        return std::numeric_limits<std::size_t>::max();

    std::size_t best_index = 0;
    double best_value = matrix(row, 0);
    for (std::size_t column = 1; column < matrix.Columns(); ++column)
    {
        if (matrix(row, column) > best_value)
        {
            best_index = column;
            best_value = matrix(row, column);
        }
    }
    return best_index;
}

struct MatrixMLPGradients
{
    Matrix grad_w1;
    std::vector<double> grad_b1;
    Matrix grad_w2;
    std::vector<double> grad_b2;
    double loss = 0.0;
};

} // namespace

Matrix::Matrix(std::size_t rows, std::size_t columns, double value)
    : m_rows(rows), m_columns(columns), m_data(rows * columns, value)
{
}

double& Matrix::operator()(std::size_t row, std::size_t column) noexcept
{
    return m_data[row * m_columns + column];
}

double Matrix::operator()(std::size_t row, std::size_t column) const noexcept
{
    return m_data[row * m_columns + column];
}

MatrixMLP::MatrixMLP(std::size_t input_count, std::size_t hidden_count,
                     std::size_t output_count)
    : MatrixMLP(input_count, hidden_count, output_count, DefaultRng())
{
}

MatrixMLP::MatrixMLP(std::size_t input_count, std::size_t hidden_count,
                     std::size_t output_count, std::mt19937& rng)
    : m_input_count(input_count), m_hidden_count(hidden_count),
      m_output_count(output_count), m_w1(hidden_count, input_count),
      m_b1(hidden_count, 0.0), m_w2(output_count, hidden_count),
      m_b2(output_count, 0.0), m_thread_count(DefaultThreadCount())
{
    if (!Valid())
    {
        m_input_count = 0;
        m_hidden_count = 0;
        m_output_count = 0;
        m_w1 = {};
        m_b1.clear();
        m_w2 = {};
        m_b2.clear();
        return;
    }

    InitWeights(m_w1, rng);
    InitWeights(m_w2, rng);
}

bool MatrixMLP::Valid() const noexcept
{
    return m_input_count > 0 && m_hidden_count > 0 && m_output_count > 0;
}

Matrix MatrixMLP::Forward(const Matrix& input) const
{
    if (!Valid() || input.Columns() != m_input_count)
        return {};

    Matrix hidden(input.Rows(), m_hidden_count);
    ParallelRows(
        input.Rows(), m_thread_count,
        [this, &input, &hidden](std::size_t begin, std::size_t end, std::size_t)
        {
            for (std::size_t row = begin; row < end; ++row)
            {
                for (std::size_t hidden_index = 0;
                     hidden_index < m_hidden_count; ++hidden_index)
                {
                    double value = m_b1[hidden_index];
                    for (std::size_t input_index = 0;
                         input_index < m_input_count; ++input_index)
                    {
                        value += input(row, input_index) *
                                 m_w1(hidden_index, input_index);
                    }
                    hidden(row, hidden_index) = value > 0.0 ? value : 0.0;
                }
            }
        });

    Matrix logits(input.Rows(), m_output_count);
    ParallelRows(input.Rows(), m_thread_count,
                 [this, &hidden, &logits](std::size_t begin, std::size_t end,
                                          std::size_t)
                 {
                     for (std::size_t row = begin; row < end; ++row)
                     {
                         for (std::size_t output_index = 0;
                              output_index < m_output_count; ++output_index)
                         {
                             double value = m_b2[output_index];
                             for (std::size_t hidden_index = 0;
                                  hidden_index < m_hidden_count; ++hidden_index)
                             {
                                 value += hidden(row, hidden_index) *
                                          m_w2(output_index, hidden_index);
                             }
                             logits(row, output_index) = value;
                         }
                     }
                 });
    return logits;
}

void MatrixMLP::SetThreadCount(std::size_t thread_count)
{
    m_thread_count = thread_count == 0 ? DefaultThreadCount()
                                       : std::max<std::size_t>(1, thread_count);
}

double MatrixMLP::TrainBatch(const Matrix& input,
                             const std::vector<std::size_t>& labels,
                             double learning_rate)
{
    if (!Valid() || input.Rows() == 0 || input.Columns() != m_input_count ||
        labels.size() != input.Rows())
    {
        return 0.0;
    }

    for (const std::size_t label : labels)
    {
        if (label >= m_output_count)
            return 0.0;
    }

    const std::size_t worker_count =
        EffectiveThreadCount(m_thread_count, input.Rows());
    Matrix z1(input.Rows(), m_hidden_count);
    Matrix hidden(input.Rows(), m_hidden_count);
    ParallelRows(input.Rows(), m_thread_count,
                 [this, &input, &z1, &hidden](std::size_t begin,
                                              std::size_t end, std::size_t)
                 {
                     for (std::size_t row = begin; row < end; ++row)
                     {
                         for (std::size_t hidden_index = 0;
                              hidden_index < m_hidden_count; ++hidden_index)
                         {
                             double value = m_b1[hidden_index];
                             for (std::size_t input_index = 0;
                                  input_index < m_input_count; ++input_index)
                             {
                                 value += input(row, input_index) *
                                          m_w1(hidden_index, input_index);
                             }
                             z1(row, hidden_index) = value;
                             hidden(row, hidden_index) =
                                 value > 0.0 ? value : 0.0;
                         }
                     }
                 });

    Matrix logits(input.Rows(), m_output_count);
    ParallelRows(input.Rows(), m_thread_count,
                 [this, &hidden, &logits](std::size_t begin, std::size_t end,
                                          std::size_t)
                 {
                     for (std::size_t row = begin; row < end; ++row)
                     {
                         for (std::size_t output_index = 0;
                              output_index < m_output_count; ++output_index)
                         {
                             double value = m_b2[output_index];
                             for (std::size_t hidden_index = 0;
                                  hidden_index < m_hidden_count; ++hidden_index)
                             {
                                 value += hidden(row, hidden_index) *
                                          m_w2(output_index, hidden_index);
                             }
                             logits(row, output_index) = value;
                         }
                     }
                 });

    Matrix grad_logits(input.Rows(), m_output_count);
    std::vector<MatrixMLPGradients> gradients;
    gradients.reserve(worker_count);
    for (std::size_t index = 0; index < worker_count; ++index)
    {
        gradients.push_back({
            Matrix(m_hidden_count, m_input_count),
            std::vector<double>(m_hidden_count, 0.0),
            Matrix(m_output_count, m_hidden_count),
            std::vector<double>(m_output_count, 0.0),
            0.0,
        });
    }

    const double batch_scale = 1.0 / static_cast<double>(input.Rows());
    ParallelRows(
        input.Rows(), m_thread_count,
        [this, &input, &labels, &z1, &hidden, &logits, &grad_logits, &gradients,
         batch_scale](std::size_t begin, std::size_t end,
                      std::size_t worker_index)
        {
            MatrixMLPGradients& gradient = gradients[worker_index];
            std::vector<double> grad_hidden(m_hidden_count, 0.0);
            for (std::size_t row = begin; row < end; ++row)
            {
                double max_logit = logits(row, 0);
                for (std::size_t output_index = 1;
                     output_index < m_output_count; ++output_index)
                {
                    max_logit = std::max(max_logit, logits(row, output_index));
                }

                double sum = 0.0;
                for (std::size_t output_index = 0;
                     output_index < m_output_count; ++output_index)
                {
                    const double probability =
                        std::exp(logits(row, output_index) - max_logit);
                    grad_logits(row, output_index) = probability;
                    sum += probability;
                }

                for (std::size_t output_index = 0;
                     output_index < m_output_count; ++output_index)
                {
                    grad_logits(row, output_index) =
                        grad_logits(row, output_index) / sum;
                }

                gradient.loss +=
                    -std::log(std::max(grad_logits(row, labels[row]),
                                       std::numeric_limits<double>::min()));
                grad_logits(row, labels[row]) -= 1.0;
                for (std::size_t output_index = 0;
                     output_index < m_output_count; ++output_index)
                {
                    grad_logits(row, output_index) *= batch_scale;
                }

                std::fill(grad_hidden.begin(), grad_hidden.end(), 0.0);
                for (std::size_t output_index = 0;
                     output_index < m_output_count; ++output_index)
                {
                    const double output_grad = grad_logits(row, output_index);
                    gradient.grad_b2[output_index] += output_grad;
                    for (std::size_t hidden_index = 0;
                         hidden_index < m_hidden_count; ++hidden_index)
                    {
                        gradient.grad_w2(output_index, hidden_index) +=
                            output_grad * hidden(row, hidden_index);
                        grad_hidden[hidden_index] +=
                            output_grad * m_w2(output_index, hidden_index);
                    }
                }

                for (std::size_t hidden_index = 0;
                     hidden_index < m_hidden_count; ++hidden_index)
                {
                    const double hidden_grad = z1(row, hidden_index) > 0.0
                                                   ? grad_hidden[hidden_index]
                                                   : 0.0;
                    gradient.grad_b1[hidden_index] += hidden_grad;
                    for (std::size_t input_index = 0;
                         input_index < m_input_count; ++input_index)
                    {
                        gradient.grad_w1(hidden_index, input_index) +=
                            hidden_grad * input(row, input_index);
                    }
                }
            }
        });

    Matrix grad_w2(m_output_count, m_hidden_count);
    std::vector<double> grad_b2(m_output_count, 0.0);
    Matrix grad_w1(m_hidden_count, m_input_count);
    std::vector<double> grad_b1(m_hidden_count, 0.0);
    double loss = 0.0;
    for (const MatrixMLPGradients& gradient : gradients)
    {
        loss += gradient.loss;
        for (std::size_t index = 0; index < grad_w2.Size(); ++index)
        {
            grad_w2.Data()[index] += gradient.grad_w2.Data()[index];
        }
        for (std::size_t index = 0; index < grad_b2.size(); ++index)
        {
            grad_b2[index] += gradient.grad_b2[index];
        }
        for (std::size_t index = 0; index < grad_w1.Size(); ++index)
        {
            grad_w1.Data()[index] += gradient.grad_w1.Data()[index];
        }
        for (std::size_t index = 0; index < grad_b1.size(); ++index)
        {
            grad_b1[index] += gradient.grad_b1[index];
        }
    }

    for (std::size_t index = 0; index < m_w1.Size(); ++index)
    {
        m_w1.Data()[index] -= learning_rate * grad_w1.Data()[index];
    }
    for (std::size_t index = 0; index < m_b1.size(); ++index)
    {
        m_b1[index] -= learning_rate * grad_b1[index];
    }
    for (std::size_t index = 0; index < m_w2.Size(); ++index)
    {
        m_w2.Data()[index] -= learning_rate * grad_w2.Data()[index];
    }
    for (std::size_t index = 0; index < m_b2.size(); ++index)
    {
        m_b2[index] -= learning_rate * grad_b2[index];
    }

    return loss * batch_scale;
}

std::size_t MatrixMLP::Predict(const std::vector<double>& input) const
{
    if (input.size() != m_input_count)
        return std::numeric_limits<std::size_t>::max();

    Matrix batch(1, input.size());
    batch.Data() = input;
    const Matrix logits = Forward(batch);
    if (logits.Empty())
        return std::numeric_limits<std::size_t>::max();

    return ArgMaxRow(logits, 0);
}

bool MatrixMLP::SaveParameters(const std::string& filename,
                               std::string* error) const
{
    std::ofstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open matrix parameter file for writing");
        return false;
    }

    stream << "FORG_NN_MATRIX_MLP 1\n";
    stream << m_input_count << ' ' << m_hidden_count << ' ' << m_output_count
           << '\n';
    stream << std::setprecision(std::numeric_limits<double>::max_digits10);
    for (const double value : m_w1.Data())
        stream << value << '\n';
    for (const double value : m_b1)
        stream << value << '\n';
    for (const double value : m_w2.Data())
        stream << value << '\n';
    for (const double value : m_b2)
        stream << value << '\n';

    if (!stream)
    {
        SetError(error, "Unable to write matrix parameter file");
        return false;
    }

    SetError(error, {});
    return true;
}

bool MatrixMLP::LoadParameters(const std::string& filename, std::string* error)
{
    std::ifstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open matrix parameter file for reading");
        return false;
    }

    std::string magic;
    int version = 0;
    if (!(stream >> magic >> version) || magic != "FORG_NN_MATRIX_MLP" ||
        version != 1)
    {
        SetError(error, "Invalid matrix parameter file header");
        return false;
    }

    std::size_t input_count = 0;
    std::size_t hidden_count = 0;
    std::size_t output_count = 0;
    if (!(stream >> input_count >> hidden_count >> output_count) ||
        input_count != m_input_count || hidden_count != m_hidden_count ||
        output_count != m_output_count)
    {
        SetError(error, "Matrix parameter shape mismatch");
        return false;
    }

    Matrix w1(m_hidden_count, m_input_count);
    std::vector<double> b1(m_hidden_count, 0.0);
    Matrix w2(m_output_count, m_hidden_count);
    std::vector<double> b2(m_output_count, 0.0);
    for (double& value : w1.Data())
    {
        if (!(stream >> value))
        {
            SetError(error, "Invalid first-layer matrix weight");
            return false;
        }
    }
    for (double& value : b1)
    {
        if (!(stream >> value))
        {
            SetError(error, "Invalid first-layer matrix bias");
            return false;
        }
    }
    for (double& value : w2.Data())
    {
        if (!(stream >> value))
        {
            SetError(error, "Invalid second-layer matrix weight");
            return false;
        }
    }
    for (double& value : b2)
    {
        if (!(stream >> value))
        {
            SetError(error, "Invalid second-layer matrix bias");
            return false;
        }
    }

    m_w1 = std::move(w1);
    m_b1 = std::move(b1);
    m_w2 = std::move(w2);
    m_b2 = std::move(b2);

    SetError(error, {});
    return true;
}

namespace {

struct Tensor3
{
    std::size_t batch = 0;
    std::size_t time = 0;
    std::size_t channels = 0;
    std::vector<double> data;

    Tensor3() = default;
    Tensor3(std::size_t b, std::size_t t, std::size_t c, double value = 0.0)
        : batch(b), time(t), channels(c), data(b * t * c, value)
    {
    }

    bool Empty() const noexcept
    {
        return batch == 0 || time == 0 || channels == 0;
    }

    std::size_t Rows() const noexcept { return batch * time; }
    std::size_t Size() const noexcept { return data.size(); }

    double& operator()(std::size_t b, std::size_t t,
                       std::size_t c) noexcept
    {
        return data[(b * time + t) * channels + c];
    }

    double operator()(std::size_t b, std::size_t t,
                      std::size_t c) const noexcept
    {
        return data[(b * time + t) * channels + c];
    }
};

struct MatrixLinearParameters
{
    Matrix weights;
    std::vector<double> bias;
};

struct MatrixGPTBlockParameters
{
    MatrixLinearParameters query;
    MatrixLinearParameters key;
    MatrixLinearParameters value;
    MatrixLinearParameters output;
    std::vector<double> norm1_scale;
    std::vector<double> norm1_bias;
    MatrixLinearParameters ff1;
    MatrixLinearParameters ff2;
    std::vector<double> norm2_scale;
    std::vector<double> norm2_bias;
};

struct MatrixGPTParameters
{
    Matrix token_embedding;
    Matrix position_embedding;
    std::vector<MatrixGPTBlockParameters> blocks;
    std::vector<double> final_norm_scale;
    std::vector<double> final_norm_bias;
    MatrixLinearParameters head;
};

struct MatrixGPTBlockCache
{
    Tensor3 input;
    Tensor3 query;
    Tensor3 key;
    Tensor3 value;
    Tensor3 context;
    Tensor3 attention_output;
    Tensor3 residual1;
    Tensor3 norm1;
    std::vector<double> norm1_mean;
    std::vector<double> norm1_inv_std;
    Tensor3 ff1_pre;
    Tensor3 ff1;
    Tensor3 ff2;
    Tensor3 residual2;
    Tensor3 norm2;
    std::vector<double> norm2_mean;
    std::vector<double> norm2_inv_std;
    std::vector<double> probabilities;
};

struct MatrixGPTForwardCache
{
    Tensor3 embeddings;
    std::vector<MatrixGPTBlockCache> blocks;
    Tensor3 final_input;
    Tensor3 final_norm;
    std::vector<double> final_norm_mean;
    std::vector<double> final_norm_inv_std;
};

bool ValidConfig(const MatrixGPTConfig& config)
{
    return config.vocab_size > 0 && config.block_size > 0 &&
           config.model_size > 0 && config.head_count > 0 &&
           config.layer_count > 0 && config.feed_forward_size > 0 &&
           config.model_size % config.head_count == 0;
}

void InitLinear(MatrixLinearParameters& linear, std::size_t input_size,
                std::size_t output_size, std::mt19937& rng)
{
    linear.weights = Matrix(output_size, input_size);
    linear.bias.assign(output_size, 0.0);
    InitWeights(linear.weights, rng);
}

void InitVector(std::vector<double>& values, std::size_t count, double value)
{
    values.assign(count, value);
}

MatrixGPTParameters MakeGPTParameters(const MatrixGPTConfig& config,
                                      std::mt19937& rng)
{
    MatrixGPTParameters parameters;
    parameters.token_embedding = Matrix(config.vocab_size, config.model_size);
    parameters.position_embedding = Matrix(config.block_size,
                                           config.model_size);
    InitWeights(parameters.token_embedding, rng);
    InitWeights(parameters.position_embedding, rng);

    parameters.blocks.reserve(config.layer_count);
    for (std::size_t index = 0; index < config.layer_count; ++index)
    {
        MatrixGPTBlockParameters block;
        InitLinear(block.query, config.model_size, config.model_size, rng);
        InitLinear(block.key, config.model_size, config.model_size, rng);
        InitLinear(block.value, config.model_size, config.model_size, rng);
        InitLinear(block.output, config.model_size, config.model_size, rng);
        InitVector(block.norm1_scale, config.model_size, 1.0);
        InitVector(block.norm1_bias, config.model_size, 0.0);
        InitLinear(block.ff1, config.model_size, config.feed_forward_size,
                   rng);
        InitLinear(block.ff2, config.feed_forward_size, config.model_size,
                   rng);
        InitVector(block.norm2_scale, config.model_size, 1.0);
        InitVector(block.norm2_bias, config.model_size, 0.0);
        parameters.blocks.push_back(std::move(block));
    }

    InitVector(parameters.final_norm_scale, config.model_size, 1.0);
    InitVector(parameters.final_norm_bias, config.model_size, 0.0);
    InitLinear(parameters.head, config.model_size, config.vocab_size, rng);
    return parameters;
}

Tensor3 LinearForward(const Tensor3& input,
                      const MatrixLinearParameters& parameters)
{
    if (input.Empty() || parameters.weights.Columns() != input.channels ||
        parameters.weights.Rows() != parameters.bias.size())
    {
        return {};
    }

    Tensor3 output(input.batch, input.time, parameters.weights.Rows());
    for (std::size_t b = 0; b < input.batch; ++b)
    {
        for (std::size_t t = 0; t < input.time; ++t)
        {
            for (std::size_t out = 0; out < parameters.weights.Rows(); ++out)
            {
                double value = parameters.bias[out];
                for (std::size_t in = 0; in < input.channels; ++in)
                {
                    value += input(b, t, in) * parameters.weights(out, in);
                }
                output(b, t, out) = value;
            }
        }
    }
    return output;
}

void LinearBackward(const Tensor3& input,
                    const MatrixLinearParameters& parameters,
                    const Tensor3& grad_output, Tensor3& grad_input,
                    Matrix& grad_weights, std::vector<double>& grad_bias)
{
    for (std::size_t b = 0; b < input.batch; ++b)
    {
        for (std::size_t t = 0; t < input.time; ++t)
        {
            for (std::size_t out = 0; out < parameters.weights.Rows(); ++out)
            {
                const double grad = grad_output(b, t, out);
                grad_bias[out] += grad;
                for (std::size_t in = 0; in < input.channels; ++in)
                {
                    grad_weights(out, in) += grad * input(b, t, in);
                    grad_input(b, t, in) += grad * parameters.weights(out, in);
                }
            }
        }
    }
}

Tensor3 LayerNormForward(const Tensor3& input,
                         const std::vector<double>& scale,
                         const std::vector<double>& bias,
                         std::vector<double>& mean,
                         std::vector<double>& inv_std)
{
    if (input.Empty() || scale.size() != input.channels ||
        bias.size() != input.channels)
    {
        return {};
    }

    Tensor3 output(input.batch, input.time, input.channels);
    mean.assign(input.Rows(), 0.0);
    inv_std.assign(input.Rows(), 0.0);
    constexpr double epsilon = 1e-5;

    for (std::size_t b = 0; b < input.batch; ++b)
    {
        for (std::size_t t = 0; t < input.time; ++t)
        {
            const std::size_t row = b * input.time + t;
            for (std::size_t c = 0; c < input.channels; ++c)
                mean[row] += input(b, t, c);
            mean[row] /= static_cast<double>(input.channels);

            double variance = 0.0;
            for (std::size_t c = 0; c < input.channels; ++c)
            {
                const double centered = input(b, t, c) - mean[row];
                variance += centered * centered;
            }
            variance /= static_cast<double>(input.channels);
            inv_std[row] = 1.0 / std::sqrt(variance + epsilon);

            for (std::size_t c = 0; c < input.channels; ++c)
            {
                const double normalized =
                    (input(b, t, c) - mean[row]) * inv_std[row];
                output(b, t, c) = normalized * scale[c] + bias[c];
            }
        }
    }
    return output;
}

void LayerNormBackward(const Tensor3& input, const std::vector<double>& scale,
                       const std::vector<double>& mean,
                       const std::vector<double>& inv_std,
                       const Tensor3& grad_output, Tensor3& grad_input,
                       std::vector<double>& grad_scale,
                       std::vector<double>& grad_bias)
{
    const double inv_channels = 1.0 / static_cast<double>(input.channels);
    for (std::size_t b = 0; b < input.batch; ++b)
    {
        for (std::size_t t = 0; t < input.time; ++t)
        {
            const std::size_t row = b * input.time + t;
            double mean_dnorm = 0.0;
            double mean_dnorm_norm = 0.0;
            for (std::size_t c = 0; c < input.channels; ++c)
            {
                const double norm = (input(b, t, c) - mean[row]) *
                                    inv_std[row];
                const double dnorm = grad_output(b, t, c) * scale[c];
                mean_dnorm += dnorm;
                mean_dnorm_norm += dnorm * norm;
                grad_scale[c] += grad_output(b, t, c) * norm;
                grad_bias[c] += grad_output(b, t, c);
            }
            mean_dnorm *= inv_channels;
            mean_dnorm_norm *= inv_channels;

            for (std::size_t c = 0; c < input.channels; ++c)
            {
                const double norm = (input(b, t, c) - mean[row]) *
                                    inv_std[row];
                const double dnorm = grad_output(b, t, c) * scale[c];
                grad_input(b, t, c) +=
                    inv_std[row] *
                    (dnorm - mean_dnorm - norm * mean_dnorm_norm);
            }
        }
    }
}

Tensor3 AddTensors(const Tensor3& lhs, const Tensor3& rhs)
{
    if (lhs.batch != rhs.batch || lhs.time != rhs.time ||
        lhs.channels != rhs.channels)
    {
        return {};
    }

    Tensor3 output(lhs.batch, lhs.time, lhs.channels);
    for (std::size_t index = 0; index < output.Size(); ++index)
        output.data[index] = lhs.data[index] + rhs.data[index];
    return output;
}

Tensor3 ReluForward(const Tensor3& input)
{
    Tensor3 output(input.batch, input.time, input.channels);
    for (std::size_t index = 0; index < input.Size(); ++index)
        output.data[index] = input.data[index] > 0.0 ? input.data[index] : 0.0;
    return output;
}

void ReluBackward(const Tensor3& input, const Tensor3& grad_output,
                  Tensor3& grad_input)
{
    for (std::size_t index = 0; index < input.Size(); ++index)
    {
        grad_input.data[index] +=
            input.data[index] > 0.0 ? grad_output.data[index] : 0.0;
    }
}

Tensor3 CausalAttentionForward(const Tensor3& query, const Tensor3& key,
                               const Tensor3& value, std::size_t head_count,
                               std::vector<double>& probabilities)
{
    if (query.batch != key.batch || query.batch != value.batch ||
        query.time != key.time || query.time != value.time ||
        query.channels != key.channels || query.channels != value.channels ||
        head_count == 0 || query.channels % head_count != 0)
    {
        return {};
    }

    const std::size_t head_size = query.channels / head_count;
    const double scale = 1.0 / std::sqrt(static_cast<double>(head_size));
    probabilities.assign(query.batch * head_count * query.time * query.time,
                         0.0);
    Tensor3 output(query.batch, query.time, query.channels);

    for (std::size_t b = 0; b < query.batch; ++b)
    {
        for (std::size_t h = 0; h < head_count; ++h)
        {
            for (std::size_t t = 0; t < query.time; ++t)
            {
                double max_score = -std::numeric_limits<double>::infinity();
                for (std::size_t s = 0; s <= t; ++s)
                {
                    double score = 0.0;
                    for (std::size_t d = 0; d < head_size; ++d)
                    {
                        const std::size_t c = h * head_size + d;
                        score += query(b, t, c) * key(b, s, c);
                    }
                    score *= scale;
                    probabilities[((b * head_count + h) * query.time + t) *
                                      query.time +
                                  s] = score;
                    max_score = std::max(max_score, score);
                }

                double sum = 0.0;
                for (std::size_t s = 0; s <= t; ++s)
                {
                    double& probability =
                        probabilities[((b * head_count + h) * query.time + t) *
                                          query.time +
                                      s];
                    probability = std::exp(probability - max_score);
                    sum += probability;
                }
                for (std::size_t s = 0; s <= t; ++s)
                {
                    probabilities[((b * head_count + h) * query.time + t) *
                                      query.time +
                                  s] /= sum;
                }

                for (std::size_t d = 0; d < head_size; ++d)
                {
                    const std::size_t c = h * head_size + d;
                    double out = 0.0;
                    for (std::size_t s = 0; s <= t; ++s)
                    {
                        const double probability =
                            probabilities[((b * head_count + h) * query.time +
                                           t) *
                                              query.time +
                                          s];
                        out += probability * value(b, s, c);
                    }
                    output(b, t, c) = out;
                }
            }
        }
    }
    return output;
}

void CausalAttentionBackward(const Tensor3& query, const Tensor3& key,
                             const Tensor3& value, std::size_t head_count,
                             const std::vector<double>& probabilities,
                             const Tensor3& grad_output, Tensor3& grad_query,
                             Tensor3& grad_key, Tensor3& grad_value)
{
    const std::size_t head_size = query.channels / head_count;
    const double scale = 1.0 / std::sqrt(static_cast<double>(head_size));
    std::vector<double> grad_probability(query.time, 0.0);
    std::vector<double> grad_score(query.time, 0.0);

    for (std::size_t b = 0; b < query.batch; ++b)
    {
        for (std::size_t h = 0; h < head_count; ++h)
        {
            for (std::size_t t = 0; t < query.time; ++t)
            {
                std::fill(grad_probability.begin(), grad_probability.end(),
                          0.0);
                std::fill(grad_score.begin(), grad_score.end(), 0.0);

                for (std::size_t s = 0; s <= t; ++s)
                {
                    double grad = 0.0;
                    for (std::size_t d = 0; d < head_size; ++d)
                    {
                        const std::size_t c = h * head_size + d;
                        grad += grad_output(b, t, c) * value(b, s, c);
                        grad_value(b, s, c) +=
                            grad_output(b, t, c) *
                            probabilities[((b * head_count + h) * query.time +
                                           t) *
                                              query.time +
                                          s];
                    }
                    grad_probability[s] = grad;
                }

                double dot = 0.0;
                for (std::size_t s = 0; s <= t; ++s)
                {
                    const double probability =
                        probabilities[((b * head_count + h) * query.time + t) *
                                          query.time +
                                      s];
                    dot += grad_probability[s] * probability;
                }

                for (std::size_t s = 0; s <= t; ++s)
                {
                    const double probability =
                        probabilities[((b * head_count + h) * query.time + t) *
                                          query.time +
                                      s];
                    grad_score[s] = probability * (grad_probability[s] - dot);
                }

                for (std::size_t s = 0; s <= t; ++s)
                {
                    for (std::size_t d = 0; d < head_size; ++d)
                    {
                        const std::size_t c = h * head_size + d;
                        grad_query(b, t, c) +=
                            grad_score[s] * scale * key(b, s, c);
                        grad_key(b, s, c) +=
                            grad_score[s] * scale * query(b, t, c);
                    }
                }
            }
        }
    }
}

Matrix TensorToLogits(const Tensor3& input,
                      const MatrixLinearParameters& parameters)
{
    const Tensor3 logits = LinearForward(input, parameters);
    Matrix output(logits.Rows(), logits.channels);
    output.Data() = logits.data;
    return output;
}

Tensor3 LogitsGrad(const Matrix& logits, const std::vector<std::size_t>& labels,
                   std::size_t batch_size, std::size_t block_size,
                   double& loss)
{
    if (logits.Rows() != labels.size() ||
        logits.Rows() != batch_size * block_size)
        return {};

    Tensor3 grad(batch_size, block_size, logits.Columns());
    const double scale = 1.0 / static_cast<double>(logits.Rows());
    loss = 0.0;
    for (std::size_t row = 0; row < logits.Rows(); ++row)
    {
        if (labels[row] >= logits.Columns())
            return {};

        double max_logit = logits(row, 0);
        for (std::size_t column = 1; column < logits.Columns(); ++column)
            max_logit = std::max(max_logit, logits(row, column));

        double sum = 0.0;
        for (std::size_t column = 0; column < logits.Columns(); ++column)
        {
            grad(row / block_size, row % block_size, column) =
                std::exp(logits(row, column) - max_logit);
            sum += grad(row / block_size, row % block_size, column);
        }
        for (std::size_t column = 0; column < logits.Columns(); ++column)
            grad(row / block_size, row % block_size, column) /= sum;

        loss += -std::log(
            std::max(grad(row / block_size, row % block_size, labels[row]),
                     std::numeric_limits<double>::min()));
        grad(row / block_size, row % block_size, labels[row]) -= 1.0;
        for (std::size_t column = 0; column < logits.Columns(); ++column)
            grad(row / block_size, row % block_size, column) *= scale;
    }
    loss *= scale;
    return grad;
}

void AppendMatrixValues(std::vector<const double*>& values,
                        const Matrix& matrix)
{
    for (const double& value : matrix.Data())
        values.push_back(&value);
}

void AppendVectorValues(std::vector<const double*>& values,
                        const std::vector<double>& vector)
{
    for (const double& value : vector)
        values.push_back(&value);
}

void AppendMatrixPointers(std::vector<double*>& values, Matrix& matrix)
{
    for (double& value : matrix.Data())
        values.push_back(&value);
}

void AppendVectorPointers(std::vector<double*>& values,
                          std::vector<double>& vector)
{
    for (double& value : vector)
        values.push_back(&value);
}

void AppendMatrixGradients(std::vector<double>& gradients,
                           const Matrix& matrix)
{
    gradients.insert(gradients.end(), matrix.Data().begin(),
                     matrix.Data().end());
}

void AppendVectorGradients(std::vector<double>& gradients,
                           const std::vector<double>& vector)
{
    gradients.insert(gradients.end(), vector.begin(), vector.end());
}

void CollectLinearValues(std::vector<const double*>& values,
                         const MatrixLinearParameters& linear)
{
    AppendMatrixValues(values, linear.weights);
    AppendVectorValues(values, linear.bias);
}

void CollectLinearPointers(std::vector<double*>& values,
                           MatrixLinearParameters& linear)
{
    AppendMatrixPointers(values, linear.weights);
    AppendVectorPointers(values, linear.bias);
}

void CollectLinearGradients(std::vector<double>& gradients,
                            const MatrixLinearParameters& linear)
{
    AppendMatrixGradients(gradients, linear.weights);
    AppendVectorGradients(gradients, linear.bias);
}

void CollectParameterValues(std::vector<const double*>& values,
                            const MatrixGPTParameters& parameters)
{
    AppendMatrixValues(values, parameters.token_embedding);
    AppendMatrixValues(values, parameters.position_embedding);
    for (const MatrixGPTBlockParameters& block : parameters.blocks)
    {
        CollectLinearValues(values, block.query);
        CollectLinearValues(values, block.key);
        CollectLinearValues(values, block.value);
        CollectLinearValues(values, block.output);
        AppendVectorValues(values, block.norm1_scale);
        AppendVectorValues(values, block.norm1_bias);
        CollectLinearValues(values, block.ff1);
        CollectLinearValues(values, block.ff2);
        AppendVectorValues(values, block.norm2_scale);
        AppendVectorValues(values, block.norm2_bias);
    }
    AppendVectorValues(values, parameters.final_norm_scale);
    AppendVectorValues(values, parameters.final_norm_bias);
    CollectLinearValues(values, parameters.head);
}

void CollectParameterPointers(std::vector<double*>& values,
                              MatrixGPTParameters& parameters)
{
    AppendMatrixPointers(values, parameters.token_embedding);
    AppendMatrixPointers(values, parameters.position_embedding);
    for (MatrixGPTBlockParameters& block : parameters.blocks)
    {
        CollectLinearPointers(values, block.query);
        CollectLinearPointers(values, block.key);
        CollectLinearPointers(values, block.value);
        CollectLinearPointers(values, block.output);
        AppendVectorPointers(values, block.norm1_scale);
        AppendVectorPointers(values, block.norm1_bias);
        CollectLinearPointers(values, block.ff1);
        CollectLinearPointers(values, block.ff2);
        AppendVectorPointers(values, block.norm2_scale);
        AppendVectorPointers(values, block.norm2_bias);
    }
    AppendVectorPointers(values, parameters.final_norm_scale);
    AppendVectorPointers(values, parameters.final_norm_bias);
    CollectLinearPointers(values, parameters.head);
}

void CollectGradientValues(std::vector<double>& values,
                           const MatrixGPTParameters& gradients)
{
    AppendMatrixGradients(values, gradients.token_embedding);
    AppendMatrixGradients(values, gradients.position_embedding);
    for (const MatrixGPTBlockParameters& block : gradients.blocks)
    {
        CollectLinearGradients(values, block.query);
        CollectLinearGradients(values, block.key);
        CollectLinearGradients(values, block.value);
        CollectLinearGradients(values, block.output);
        AppendVectorGradients(values, block.norm1_scale);
        AppendVectorGradients(values, block.norm1_bias);
        CollectLinearGradients(values, block.ff1);
        CollectLinearGradients(values, block.ff2);
        AppendVectorGradients(values, block.norm2_scale);
        AppendVectorGradients(values, block.norm2_bias);
    }
    AppendVectorGradients(values, gradients.final_norm_scale);
    AppendVectorGradients(values, gradients.final_norm_bias);
    CollectLinearGradients(values, gradients.head);
}

MatrixGPTParameters MakeZeroGradients(const MatrixGPTConfig& config)
{
    MatrixGPTParameters gradients;
    gradients.token_embedding = Matrix(config.vocab_size, config.model_size);
    gradients.position_embedding = Matrix(config.block_size,
                                          config.model_size);
    gradients.blocks.reserve(config.layer_count);
    for (std::size_t index = 0; index < config.layer_count; ++index)
    {
        MatrixGPTBlockParameters block;
        block.query.weights = Matrix(config.model_size, config.model_size);
        block.query.bias.assign(config.model_size, 0.0);
        block.key.weights = Matrix(config.model_size, config.model_size);
        block.key.bias.assign(config.model_size, 0.0);
        block.value.weights = Matrix(config.model_size, config.model_size);
        block.value.bias.assign(config.model_size, 0.0);
        block.output.weights = Matrix(config.model_size, config.model_size);
        block.output.bias.assign(config.model_size, 0.0);
        block.norm1_scale.assign(config.model_size, 0.0);
        block.norm1_bias.assign(config.model_size, 0.0);
        block.ff1.weights = Matrix(config.feed_forward_size,
                                   config.model_size);
        block.ff1.bias.assign(config.feed_forward_size, 0.0);
        block.ff2.weights = Matrix(config.model_size,
                                   config.feed_forward_size);
        block.ff2.bias.assign(config.model_size, 0.0);
        block.norm2_scale.assign(config.model_size, 0.0);
        block.norm2_bias.assign(config.model_size, 0.0);
        gradients.blocks.push_back(std::move(block));
    }
    gradients.final_norm_scale.assign(config.model_size, 0.0);
    gradients.final_norm_bias.assign(config.model_size, 0.0);
    gradients.head.weights = Matrix(config.vocab_size, config.model_size);
    gradients.head.bias.assign(config.vocab_size, 0.0);
    return gradients;
}

} // namespace

struct MatrixGPT::Impl
{
    MatrixGPTParameters parameters;
    std::vector<double> first_moment;
    std::vector<double> second_moment;
    std::size_t step = 0;
};

MatrixGPT::MatrixGPT(const MatrixGPTConfig& config)
    : MatrixGPT(config, DefaultRng())
{
}

MatrixGPT::MatrixGPT(const MatrixGPTConfig& config, std::mt19937& rng)
    : m_config(config), m_thread_count(DefaultThreadCount())
{
    if (!ValidConfig(m_config))
    {
        m_config = {};
        return;
    }

    m_impl = new Impl;
    m_impl->parameters = MakeGPTParameters(m_config, rng);
    const std::size_t count = ParameterCount();
    m_impl->first_moment.assign(count, 0.0);
    m_impl->second_moment.assign(count, 0.0);
}

MatrixGPT::~MatrixGPT() { delete m_impl; }

MatrixGPT::MatrixGPT(MatrixGPT&& other) noexcept
    : m_config(other.m_config), m_impl(other.m_impl),
      m_thread_count(other.m_thread_count)
{
    other.m_config = {};
    other.m_impl = nullptr;
    other.m_thread_count = 1;
}

MatrixGPT& MatrixGPT::operator=(MatrixGPT&& other) noexcept
{
    if (this == &other)
        return *this;

    delete m_impl;
    m_config = other.m_config;
    m_impl = other.m_impl;
    m_thread_count = other.m_thread_count;
    other.m_config = {};
    other.m_impl = nullptr;
    other.m_thread_count = 1;
    return *this;
}

bool MatrixGPT::Valid() const noexcept
{
    return m_impl != nullptr && ValidConfig(m_config);
}

std::size_t MatrixGPT::ParameterCount() const noexcept
{
    if (!m_impl)
        return 0;

    std::vector<const double*> values;
    CollectParameterValues(values, m_impl->parameters);
    return values.size();
}

namespace {

Tensor3 MatrixGPTForwardInternal(const MatrixGPTConfig& config,
                                 const MatrixGPTParameters& parameters,
                                 const std::vector<std::size_t>& input,
                                 std::size_t batch_size,
                                 MatrixGPTForwardCache* cache)
{
    if (!ValidConfig(config) || batch_size == 0 ||
        input.size() != batch_size * config.block_size)
    {
        return {};
    }

    Tensor3 current(batch_size, config.block_size, config.model_size);
    for (std::size_t b = 0; b < batch_size; ++b)
    {
        for (std::size_t t = 0; t < config.block_size; ++t)
        {
            const std::size_t token = input[b * config.block_size + t];
            if (token >= config.vocab_size)
                return {};

            for (std::size_t c = 0; c < config.model_size; ++c)
            {
                current(b, t, c) = parameters.token_embedding(token, c) +
                                   parameters.position_embedding(t, c);
            }
        }
    }
    if (cache)
    {
        cache->embeddings = current;
        cache->blocks.clear();
        cache->blocks.reserve(config.layer_count);
    }

    for (std::size_t layer = 0; layer < config.layer_count; ++layer)
    {
        const MatrixGPTBlockParameters& block = parameters.blocks[layer];
        MatrixGPTBlockCache block_cache;
        if (cache)
            block_cache.input = current;

        Tensor3 query = LinearForward(current, block.query);
        Tensor3 key = LinearForward(current, block.key);
        Tensor3 value = LinearForward(current, block.value);
        std::vector<double> probabilities;
        Tensor3 context = CausalAttentionForward(query, key, value,
                                                 config.head_count,
                                                 probabilities);
        Tensor3 attention_output = LinearForward(context, block.output);
        Tensor3 residual1 = AddTensors(current, attention_output);
        std::vector<double> norm1_mean;
        std::vector<double> norm1_inv_std;
        Tensor3 norm1 = LayerNormForward(residual1, block.norm1_scale,
                                         block.norm1_bias, norm1_mean,
                                         norm1_inv_std);
        Tensor3 ff1_pre = LinearForward(norm1, block.ff1);
        Tensor3 ff1 = ReluForward(ff1_pre);
        Tensor3 ff2 = LinearForward(ff1, block.ff2);
        Tensor3 residual2 = AddTensors(norm1, ff2);
        std::vector<double> norm2_mean;
        std::vector<double> norm2_inv_std;
        Tensor3 norm2 = LayerNormForward(residual2, block.norm2_scale,
                                         block.norm2_bias, norm2_mean,
                                         norm2_inv_std);

        if (query.Empty() || key.Empty() || value.Empty() || context.Empty() ||
            attention_output.Empty() || residual1.Empty() || norm1.Empty() ||
            ff1_pre.Empty() || ff1.Empty() || ff2.Empty() ||
            residual2.Empty() || norm2.Empty())
        {
            return {};
        }

        if (cache)
        {
            block_cache.query = std::move(query);
            block_cache.key = std::move(key);
            block_cache.value = std::move(value);
            block_cache.context = std::move(context);
            block_cache.attention_output = std::move(attention_output);
            block_cache.residual1 = std::move(residual1);
            block_cache.norm1 = norm1;
            block_cache.norm1_mean = std::move(norm1_mean);
            block_cache.norm1_inv_std = std::move(norm1_inv_std);
            block_cache.ff1_pre = std::move(ff1_pre);
            block_cache.ff1 = std::move(ff1);
            block_cache.ff2 = std::move(ff2);
            block_cache.residual2 = std::move(residual2);
            block_cache.norm2 = norm2;
            block_cache.norm2_mean = std::move(norm2_mean);
            block_cache.norm2_inv_std = std::move(norm2_inv_std);
            block_cache.probabilities = std::move(probabilities);
            cache->blocks.push_back(std::move(block_cache));
        }
        current = std::move(norm2);
    }

    if (cache)
        cache->final_input = current;

    std::vector<double> final_mean;
    std::vector<double> final_inv_std;
    current = LayerNormForward(current, parameters.final_norm_scale,
                               parameters.final_norm_bias,
                               cache ? cache->final_norm_mean : final_mean,
                               cache ? cache->final_norm_inv_std
                                     : final_inv_std);
    if (current.Empty())
        return {};

    if (cache)
        cache->final_norm = current;
    return current;
}

} // namespace

Matrix MatrixGPT::Forward(const std::vector<std::size_t>& input,
                          std::size_t batch_size) const
{
    if (!Valid())
        return {};

    MatrixGPTForwardCache cache;
    const Tensor3 final = MatrixGPTForwardInternal(
        m_config, m_impl->parameters, input, batch_size, &cache);
    if (final.Empty())
        return {};

    return TensorToLogits(final, m_impl->parameters.head);
}

double MatrixGPT::TrainBatch(const std::vector<std::size_t>& input,
                             const std::vector<std::size_t>& targets,
                             std::size_t batch_size, double learning_rate)
{
    if (!Valid() || targets.size() != input.size() || learning_rate == 0.0)
        return 0.0;

    MatrixGPTForwardCache cache;
    const Tensor3 final = MatrixGPTForwardInternal(
        m_config, m_impl->parameters, input, batch_size, &cache);
    if (final.Empty())
        return 0.0;

    const Matrix logits = TensorToLogits(final, m_impl->parameters.head);
    double loss = 0.0;
    Tensor3 grad_logits =
        LogitsGrad(logits, targets, batch_size, m_config.block_size, loss);
    if (grad_logits.Empty())
        return 0.0;

    MatrixGPTParameters gradients = MakeZeroGradients(m_config);
    Tensor3 grad_final(final.batch, final.time, final.channels);
    LinearBackward(final, m_impl->parameters.head, grad_logits, grad_final,
                   gradients.head.weights, gradients.head.bias);

    Tensor3 grad_current(final.batch, final.time, final.channels);
    LayerNormBackward(cache.final_input, m_impl->parameters.final_norm_scale,
                      cache.final_norm_mean, cache.final_norm_inv_std,
                      grad_final, grad_current, gradients.final_norm_scale,
                      gradients.final_norm_bias);

    for (std::size_t reverse_layer = m_config.layer_count; reverse_layer > 0;
         --reverse_layer)
    {
        const std::size_t layer = reverse_layer - 1;
        const MatrixGPTBlockParameters& block = m_impl->parameters.blocks[layer];
        MatrixGPTBlockParameters& grad_block = gradients.blocks[layer];
        const MatrixGPTBlockCache& block_cache = cache.blocks[layer];

        Tensor3 grad_residual2(grad_current.batch, grad_current.time,
                               grad_current.channels);
        LayerNormBackward(block_cache.residual2, block.norm2_scale,
                          block_cache.norm2_mean,
                          block_cache.norm2_inv_std, grad_current,
                          grad_residual2, grad_block.norm2_scale,
                          grad_block.norm2_bias);

        Tensor3 grad_norm1 = grad_residual2;
        Tensor3 grad_ff2 = grad_residual2;
        Tensor3 grad_ff1(grad_ff2.batch, grad_ff2.time,
                         block_cache.ff1.channels);
        LinearBackward(block_cache.ff1, block.ff2, grad_ff2, grad_ff1,
                       grad_block.ff2.weights, grad_block.ff2.bias);

        Tensor3 grad_ff1_pre(grad_ff1.batch, grad_ff1.time,
                             grad_ff1.channels);
        ReluBackward(block_cache.ff1_pre, grad_ff1, grad_ff1_pre);
        Tensor3 grad_norm1_from_ff(grad_norm1.batch, grad_norm1.time,
                                   grad_norm1.channels);
        LinearBackward(block_cache.norm1, block.ff1, grad_ff1_pre,
                       grad_norm1_from_ff, grad_block.ff1.weights,
                       grad_block.ff1.bias);
        for (std::size_t index = 0; index < grad_norm1.Size(); ++index)
            grad_norm1.data[index] += grad_norm1_from_ff.data[index];

        Tensor3 grad_residual1(grad_norm1.batch, grad_norm1.time,
                               grad_norm1.channels);
        LayerNormBackward(block_cache.residual1, block.norm1_scale,
                          block_cache.norm1_mean,
                          block_cache.norm1_inv_std, grad_norm1,
                          grad_residual1, grad_block.norm1_scale,
                          grad_block.norm1_bias);

        Tensor3 grad_input = grad_residual1;
        Tensor3 grad_attention_output = grad_residual1;
        Tensor3 grad_context(grad_attention_output.batch,
                             grad_attention_output.time,
                             block_cache.context.channels);
        LinearBackward(block_cache.context, block.output,
                       grad_attention_output, grad_context,
                       grad_block.output.weights, grad_block.output.bias);

        Tensor3 grad_query(block_cache.query.batch, block_cache.query.time,
                           block_cache.query.channels);
        Tensor3 grad_key(block_cache.key.batch, block_cache.key.time,
                         block_cache.key.channels);
        Tensor3 grad_value(block_cache.value.batch, block_cache.value.time,
                           block_cache.value.channels);
        CausalAttentionBackward(block_cache.query, block_cache.key,
                                block_cache.value, m_config.head_count,
                                block_cache.probabilities, grad_context,
                                grad_query, grad_key, grad_value);

        Tensor3 grad_from_query(block_cache.input.batch, block_cache.input.time,
                                block_cache.input.channels);
        Tensor3 grad_from_key(block_cache.input.batch, block_cache.input.time,
                              block_cache.input.channels);
        Tensor3 grad_from_value(block_cache.input.batch,
                                block_cache.input.time,
                                block_cache.input.channels);
        LinearBackward(block_cache.input, block.query, grad_query,
                       grad_from_query, grad_block.query.weights,
                       grad_block.query.bias);
        LinearBackward(block_cache.input, block.key, grad_key, grad_from_key,
                       grad_block.key.weights, grad_block.key.bias);
        LinearBackward(block_cache.input, block.value, grad_value,
                       grad_from_value, grad_block.value.weights,
                       grad_block.value.bias);

        for (std::size_t index = 0; index < grad_input.Size(); ++index)
        {
            grad_input.data[index] += grad_from_query.data[index] +
                                      grad_from_key.data[index] +
                                      grad_from_value.data[index];
        }
        grad_current = std::move(grad_input);
    }

    for (std::size_t b = 0; b < batch_size; ++b)
    {
        for (std::size_t t = 0; t < m_config.block_size; ++t)
        {
            const std::size_t token = input[b * m_config.block_size + t];
            for (std::size_t c = 0; c < m_config.model_size; ++c)
            {
                gradients.token_embedding(token, c) += grad_current(b, t, c);
                gradients.position_embedding(t, c) += grad_current(b, t, c);
            }
        }
    }

    std::vector<double*> parameter_pointers;
    std::vector<double> gradient_values;
    CollectParameterPointers(parameter_pointers, m_impl->parameters);
    CollectGradientValues(gradient_values, gradients);
    if (parameter_pointers.size() != gradient_values.size())
        return 0.0;

    if (m_impl->first_moment.size() != parameter_pointers.size())
    {
        m_impl->first_moment.assign(parameter_pointers.size(), 0.0);
        m_impl->second_moment.assign(parameter_pointers.size(), 0.0);
        m_impl->step = 0;
    }

    ++m_impl->step;
    constexpr double beta1 = 0.9;
    constexpr double beta2 = 0.999;
    constexpr double epsilon = 1e-8;
    const double beta1_correction = 1.0 - std::pow(beta1, m_impl->step);
    const double beta2_correction = 1.0 - std::pow(beta2, m_impl->step);
    for (std::size_t index = 0; index < parameter_pointers.size(); ++index)
    {
        const double grad = gradient_values[index];
        m_impl->first_moment[index] =
            beta1 * m_impl->first_moment[index] + (1.0 - beta1) * grad;
        m_impl->second_moment[index] =
            beta2 * m_impl->second_moment[index] + (1.0 - beta2) * grad * grad;
        const double first = m_impl->first_moment[index] / beta1_correction;
        const double second = m_impl->second_moment[index] / beta2_correction;
        *parameter_pointers[index] -=
            learning_rate * first / (std::sqrt(second) + epsilon);
    }
    return loss;
}

std::size_t MatrixGPT::PredictNext(
    const std::vector<std::size_t>& context) const
{
    if (!Valid() || context.empty())
        return std::numeric_limits<std::size_t>::max();

    std::vector<std::size_t> input(m_config.block_size, context.front());
    if (context.size() >= m_config.block_size)
    {
        std::copy(context.end() -
                      static_cast<std::ptrdiff_t>(m_config.block_size),
                  context.end(), input.begin());
    }
    else
    {
        std::copy(context.begin(), context.end(),
                  input.end() - static_cast<std::ptrdiff_t>(context.size()));
    }

    const Matrix logits = Forward(input, 1);
    if (logits.Empty())
        return std::numeric_limits<std::size_t>::max();

    const std::size_t row = m_config.block_size - 1;
    std::size_t best = 0;
    for (std::size_t column = 1; column < logits.Columns(); ++column)
    {
        if (logits(row, column) > logits(row, best))
            best = column;
    }
    return best;
}

void MatrixGPT::SetThreadCount(std::size_t thread_count)
{
    m_thread_count = thread_count == 0 ? DefaultThreadCount()
                                       : std::max<std::size_t>(1, thread_count);
}

bool MatrixGPT::SaveParameters(const std::string& filename,
                               std::string* error) const
{
    if (!Valid())
    {
        SetError(error, "Cannot save invalid MatrixGPT");
        return false;
    }

    std::ofstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open MatrixGPT parameter file for writing");
        return false;
    }

    stream << "FORG_NN_MATRIX_GPT 1\n";
    stream << m_config.vocab_size << ' ' << m_config.block_size << ' '
           << m_config.model_size << ' ' << m_config.head_count << ' '
           << m_config.layer_count << ' ' << m_config.feed_forward_size
           << '\n';
    stream << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::vector<const double*> values;
    CollectParameterValues(values, m_impl->parameters);
    stream << values.size() << '\n';
    for (const double* value : values)
        stream << *value << '\n';

    if (!stream)
    {
        SetError(error, "Unable to write MatrixGPT parameter file");
        return false;
    }

    SetError(error, {});
    return true;
}

bool MatrixGPT::LoadParameters(const std::string& filename, std::string* error)
{
    if (!Valid())
    {
        SetError(error, "Cannot load into invalid MatrixGPT");
        return false;
    }

    std::ifstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open MatrixGPT parameter file for reading");
        return false;
    }

    std::string magic;
    int version = 0;
    if (!(stream >> magic >> version) || magic != "FORG_NN_MATRIX_GPT" ||
        version != 1)
    {
        SetError(error, "Invalid MatrixGPT parameter file header");
        return false;
    }

    MatrixGPTConfig file_config;
    if (!(stream >> file_config.vocab_size >> file_config.block_size >>
          file_config.model_size >> file_config.head_count >>
          file_config.layer_count >> file_config.feed_forward_size) ||
        file_config.vocab_size != m_config.vocab_size ||
        file_config.block_size != m_config.block_size ||
        file_config.model_size != m_config.model_size ||
        file_config.head_count != m_config.head_count ||
        file_config.layer_count != m_config.layer_count ||
        file_config.feed_forward_size != m_config.feed_forward_size)
    {
        SetError(error, "MatrixGPT parameter shape mismatch");
        return false;
    }

    std::size_t count = 0;
    if (!(stream >> count) || count != ParameterCount())
    {
        SetError(error, "MatrixGPT parameter count mismatch");
        return false;
    }

    std::vector<double> values(count, 0.0);
    for (double& value : values)
    {
        if (!(stream >> value))
        {
            SetError(error, "Invalid MatrixGPT parameter value");
            return false;
        }
    }

    std::vector<double*> parameters;
    CollectParameterPointers(parameters, m_impl->parameters);
    for (std::size_t index = 0; index < parameters.size(); ++index)
        *parameters[index] = values[index];
    std::fill(m_impl->first_moment.begin(), m_impl->first_moment.end(), 0.0);
    std::fill(m_impl->second_moment.begin(), m_impl->second_moment.end(), 0.0);
    m_impl->step = 0;

    SetError(error, {});
    return true;
}

} // namespace forg::nn
