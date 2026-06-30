/*******************************************************************************
    This source file is part of FORG library.

    Lightweight matrix backend for small neural-network experiments.
*******************************************************************************/

#ifndef FORG_NN_MATRIX_H
#define FORG_NN_MATRIX_H

#include <cstddef>
#include <random>
#include <string>
#include <vector>

namespace forg::nn {

/// Row-major dense matrix used by the fast NN backend.
///
/// This type stores plain doubles and does not participate in the scalar
/// ValuePtr autograd graph.
class Matrix
{
  public:
    Matrix() = default;
    Matrix(std::size_t rows, std::size_t columns, double value = 0.0);

    bool Empty() const noexcept { return m_rows == 0 || m_columns == 0; }
    std::size_t Rows() const noexcept { return m_rows; }
    std::size_t Columns() const noexcept { return m_columns; }
    std::size_t Size() const noexcept { return m_data.size(); }

    double& operator()(std::size_t row, std::size_t column) noexcept;
    double operator()(std::size_t row, std::size_t column) const noexcept;

    std::vector<double>& Data() noexcept { return m_data; }
    const std::vector<double>& Data() const noexcept { return m_data; }

  private:
    std::size_t m_rows = 0;
    std::size_t m_columns = 0;
    std::vector<double> m_data;
};

/// Batched dense MNIST-style classifier implemented with matrix operations.
///
/// Architecture: Linear(input, hidden), ReLU, Linear(hidden, output). Training
/// uses batched softmax cross-entropy and manual backpropagation, avoiding
/// scalar graph allocation for each sample.
class MatrixMLP
{
  public:
    MatrixMLP(std::size_t input_count, std::size_t hidden_count,
              std::size_t output_count);
    MatrixMLP(std::size_t input_count, std::size_t hidden_count,
              std::size_t output_count, std::mt19937& rng);

    Matrix Forward(const Matrix& input) const;
    double TrainBatch(const Matrix& input,
                      const std::vector<std::size_t>& labels,
                      double learning_rate);
    std::size_t Predict(const std::vector<double>& input) const;

    void SetThreadCount(std::size_t thread_count);
    std::size_t ThreadCount() const noexcept { return m_thread_count; }

    bool SaveParameters(const std::string& filename,
                        std::string* error = nullptr) const;
    bool LoadParameters(const std::string& filename,
                        std::string* error = nullptr);

    std::size_t InputCount() const noexcept { return m_input_count; }
    std::size_t HiddenCount() const noexcept { return m_hidden_count; }
    std::size_t OutputCount() const noexcept { return m_output_count; }

    const Matrix& FirstWeights() const noexcept { return m_w1; }
    const Matrix& SecondWeights() const noexcept { return m_w2; }
    const std::vector<double>& FirstBias() const noexcept { return m_b1; }
    const std::vector<double>& SecondBias() const noexcept { return m_b2; }

  private:
    bool Valid() const noexcept;

    std::size_t m_input_count = 0;
    std::size_t m_hidden_count = 0;
    std::size_t m_output_count = 0;
    Matrix m_w1;
    std::vector<double> m_b1;
    Matrix m_w2;
    std::vector<double> m_b2;
    std::size_t m_thread_count = 1;
};

} // namespace forg::nn

#endif // FORG_NN_MATRIX_H
