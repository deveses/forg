#include "forg/nn/Matrix.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
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

} // namespace forg::nn
