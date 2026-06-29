#include "forg/nn/Mnist.h"

#include <fstream>
#include <string>
#include <vector>

namespace forg::nn {
namespace {

constexpr std::uint32_t kImageMagic = 2051;
constexpr std::uint32_t kLabelMagic = 2049;

bool ReadU32(std::ifstream& input, std::uint32_t& value)
{
    unsigned char bytes[4] = {};
    input.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
    if (!input)
        return false;

    value = (static_cast<std::uint32_t>(bytes[0]) << 24U) |
            (static_cast<std::uint32_t>(bytes[1]) << 16U) |
            (static_cast<std::uint32_t>(bytes[2]) << 8U) |
            static_cast<std::uint32_t>(bytes[3]);
    return true;
}

bool ReadHeader(std::ifstream& input, std::uint32_t expected_magic,
                std::uint32_t& count, std::string& error, const char* kind)
{
    std::uint32_t magic = 0;
    if (!ReadU32(input, magic) || !ReadU32(input, count))
    {
        error = std::string("Unable to read MNIST ") + kind + " header";
        return false;
    }

    if (magic != expected_magic)
    {
        error = std::string("Invalid MNIST ") + kind + " magic";
        return false;
    }
    return true;
}

} // namespace

bool MnistDataset::Load(const std::string& image_path,
                        const std::string& label_path)
{
    m_samples.clear();
    m_error.clear();
    m_rows = 0;
    m_columns = 0;

    std::ifstream images(image_path, std::ios::binary);
    if (!images)
    {
        m_error = "Unable to open MNIST image file";
        return false;
    }

    std::ifstream labels(label_path, std::ios::binary);
    if (!labels)
    {
        m_error = "Unable to open MNIST label file";
        return false;
    }

    std::uint32_t image_count = 0;
    if (!ReadHeader(images, kImageMagic, image_count, m_error, "image"))
        return false;

    std::uint32_t rows = 0;
    std::uint32_t columns = 0;
    if (!ReadU32(images, rows) || !ReadU32(images, columns))
    {
        m_error = "Unable to read MNIST image dimensions";
        return false;
    }

    if (rows == 0 || columns == 0)
    {
        m_error = "Invalid MNIST image dimensions";
        return false;
    }

    std::uint32_t label_count = 0;
    if (!ReadHeader(labels, kLabelMagic, label_count, m_error, "label"))
        return false;

    if (image_count != label_count)
    {
        m_error = "MNIST image and label counts do not match";
        return false;
    }

    const std::size_t image_size =
        static_cast<std::size_t>(rows) * static_cast<std::size_t>(columns);
    std::vector<unsigned char> pixels(image_size);

    m_samples.reserve(image_count);
    for (std::uint32_t index = 0; index < image_count; ++index)
    {
        unsigned char label = 0;
        labels.read(reinterpret_cast<char*>(&label), sizeof(label));
        if (!labels)
        {
            m_error = "Unable to read MNIST label data";
            m_samples.clear();
            return false;
        }

        images.read(reinterpret_cast<char*>(pixels.data()),
                    static_cast<std::streamsize>(pixels.size()));
        if (!images)
        {
            m_error = "Unable to read MNIST image data";
            m_samples.clear();
            return false;
        }

        MnistSample sample;
        sample.label = label;
        sample.pixels.reserve(image_size);
        for (const unsigned char pixel : pixels)
        {
            sample.pixels.push_back(static_cast<double>(pixel) / 255.0);
        }
        m_samples.push_back(std::move(sample));
    }

    m_rows = rows;
    m_columns = columns;
    return true;
}

} // namespace forg::nn
