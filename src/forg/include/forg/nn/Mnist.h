/*******************************************************************************
    This source file is part of FORG library.

    Minimal MNIST IDX dataset reader for neural-network examples and tests.
*******************************************************************************/

#ifndef FORG_NN_MNIST_H
#define FORG_NN_MNIST_H

#include "forg/base.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace forg::nn {

struct MnistSample
{
    std::vector<double> pixels;
    std::uint8_t label = 0;
};

class MnistDataset
{
  public:
    bool Load(const std::string& image_path, const std::string& label_path);

    const std::vector<MnistSample>& Samples() const noexcept
    {
        return m_samples;
    }
    const std::string& Error() const noexcept { return m_error; }
    std::size_t Rows() const noexcept { return m_rows; }
    std::size_t Columns() const noexcept { return m_columns; }
    std::size_t ImageSize() const noexcept { return m_rows * m_columns; }

  private:
    std::vector<MnistSample> m_samples;
    std::string m_error;
    std::size_t m_rows = 0;
    std::size_t m_columns = 0;
};

} // namespace forg::nn

#endif // FORG_NN_MNIST_H
