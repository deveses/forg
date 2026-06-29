#include <forg/nn.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

namespace {

std::size_t ParseSize(const char* text, std::size_t fallback)
{
    if (!text)
        return fallback;

    char* end = nullptr;
    const unsigned long value = std::strtoul(text, &end, 10);
    if (end == text || (end && *end != '\0'))
        return fallback;
    return static_cast<std::size_t>(value);
}

void PrintUsage(const char* program)
{
    std::cout << "Usage: " << program
              << " <checkpoint> <images> <labels> [index]\n";
}

void PrintImage(const forg::nn::MnistSample& sample, std::size_t rows,
                std::size_t columns)
{
    static const char* shades = " .:-=+*#%@";
    for (std::size_t y = 0; y < rows; ++y)
    {
        for (std::size_t x = 0; x < columns; ++x)
        {
            const double pixel = sample.pixels[y * columns + x];
            const std::size_t shade_index = std::min<std::size_t>(
                9, static_cast<std::size_t>(pixel * 9.0 + 0.5));
            std::cout << shades[shade_index];
        }
        std::cout << '\n';
    }
}

bool LoadModel(const std::string& checkpoint_path, forg::nn::MatrixMLP& model)
{
    std::string error;
    if (!model.LoadParameters(checkpoint_path, &error))
    {
        std::cerr << "Unable to load checkpoint: " << error << '\n';
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    forg::nn::MnistDataset dataset;
    if (!dataset.Load(argv[2], argv[3]))
    {
        std::cerr << dataset.Error() << '\n';
        return 1;
    }

    if (dataset.ImageSize() != 784)
    {
        std::cerr << "Expected 28x28 MNIST images\n";
        return 1;
    }

    forg::nn::MatrixMLP model(784, 64, 10);
    if (!LoadModel(argv[1], model))
        return 1;

    const std::vector<forg::nn::MnistSample>& samples = dataset.Samples();
    if (samples.empty())
    {
        std::cerr << "Dataset is empty\n";
        return 1;
    }

    if (argc > 4)
    {
        const std::size_t index =
            ParseSize(argv[4], std::numeric_limits<std::size_t>::max());
        if (index >= samples.size())
        {
            std::cerr << "Index out of range\n";
            return 1;
        }

        const std::size_t prediction = model.Predict(samples[index].pixels);
        std::cout << "index=" << index << " prediction=" << prediction
                  << " expected=" << static_cast<int>(samples[index].label)
                  << " match="
                  << (prediction == samples[index].label ? "true" : "false")
                  << '\n';
        PrintImage(samples[index], dataset.Rows(), dataset.Columns());
        return prediction == samples[index].label ? 0 : 2;
    }

    std::size_t correct = 0;
    for (const forg::nn::MnistSample& sample : samples)
    {
        if (model.Predict(sample.pixels) == sample.label)
            ++correct;
    }

    const double accuracy =
        static_cast<double>(correct) / static_cast<double>(samples.size());
    std::cout << "samples=" << samples.size() << " correct=" << correct
              << " accuracy=" << accuracy << '\n';
    return 0;
}
