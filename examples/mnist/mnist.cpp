#include <forg/nn.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace {

using Clock = std::chrono::steady_clock;

struct EpochProfile
{
    std::uint64_t input_us = 0;
    std::uint64_t forward_us = 0;
    std::uint64_t loss_us = 0;
    std::uint64_t zero_grad_us = 0;
    std::uint64_t backward_us = 0;
    std::uint64_t update_us = 0;
    std::uint64_t eval_us = 0;
    std::uint64_t epoch_us = 0;
};

std::uint64_t ElapsedUs(Clock::time_point start)
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() -
                                                              start)
            .count());
}

double AverageUs(std::uint64_t total_us, std::size_t count)
{
    if (count == 0)
        return 0.0;

    return static_cast<double>(total_us) / static_cast<double>(count);
}

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

double ParseDouble(const char* text, double fallback)
{
    if (!text)
        return fallback;

    char* end = nullptr;
    const double value = std::strtod(text, &end);
    if (end == text || (end && *end != '\0'))
        return fallback;
    return value;
}

void PrintUsage(const char* program)
{
    std::cout << "Usage: " << program
              << " <train-images> <train-labels> <test-images> <test-labels>"
                 " [epochs] [train-limit] [test-limit] [learning-rate]\n";
}

double Evaluate(forg::nn::Sequential& model,
                const std::vector<forg::nn::MnistSample>& samples,
                std::size_t limit)
{
    const std::size_t count = std::min(limit, samples.size());
    if (count == 0)
        return 0.0;

    std::size_t correct = 0;
    forg::nn::Values input;
    for (std::size_t index = 0; index < count; ++index)
    {
        forg::nn::Flatten::Into(samples[index].pixels, input);
        const forg::nn::Values output = model.Forward(input);
        if (!output.empty() && forg::nn::ArgMax(output) == samples[index].label)
            ++correct;
    }
    return static_cast<double>(correct) / static_cast<double>(count);
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    const std::size_t epochs = argc > 5 ? ParseSize(argv[5], 1) : 1;
    const std::size_t train_limit = argc > 6 ? ParseSize(argv[6], 1000) : 1000;
    const std::size_t test_limit = argc > 7 ? ParseSize(argv[7], 200) : 200;
    const double learning_rate = argc > 8 ? ParseDouble(argv[8], 0.01) : 0.01;

    forg::nn::MnistDataset train;
    if (!train.Load(argv[1], argv[2]))
    {
        std::cerr << train.Error() << '\n';
        return 1;
    }

    forg::nn::MnistDataset test;
    if (!test.Load(argv[3], argv[4]))
    {
        std::cerr << test.Error() << '\n';
        return 1;
    }

    if (train.ImageSize() != 784)
    {
        std::cerr << "Expected 28x28 MNIST images\n";
        return 1;
    }

    forg::nn::Sequential model({
        std::make_shared<forg::nn::Linear>(784, 64),
        std::make_shared<forg::nn::ReLU>(),
        std::make_shared<forg::nn::Linear>(64, 10),
    });
    forg::nn::SGD optimizer(model.Parameters(), learning_rate);

    const std::vector<forg::nn::MnistSample>& train_samples = train.Samples();
    const std::vector<forg::nn::MnistSample>& test_samples = test.Samples();
    const std::size_t samples_per_epoch =
        std::min(train_limit, train_samples.size());
    forg::nn::Values input;
    forg::nn::BackwardScratch backward_scratch;

    for (std::size_t epoch = 0; epoch < epochs; ++epoch)
    {
        const Clock::time_point epoch_start = Clock::now();
        EpochProfile profile;
        double total_loss = 0.0;
        std::size_t trained = 0;
        for (std::size_t index = 0; index < samples_per_epoch; ++index)
        {
            Clock::time_point start = Clock::now();
            forg::nn::Flatten::Into(train_samples[index].pixels, input);
            profile.input_us += ElapsedUs(start);

            start = Clock::now();
            const forg::nn::Values output = model.Forward(input);
            profile.forward_us += ElapsedUs(start);

            start = Clock::now();
            const forg::nn::ValuePtr loss =
                forg::nn::CrossEntropyLoss(output, train_samples[index].label);
            profile.loss_us += ElapsedUs(start);
            if (!loss)
                continue;

            start = Clock::now();
            optimizer.ZeroGrad();
            profile.zero_grad_us += ElapsedUs(start);

            start = Clock::now();
            forg::nn::Backward(loss, backward_scratch);
            profile.backward_us += ElapsedUs(start);

            start = Clock::now();
            optimizer.Step();
            profile.update_us += ElapsedUs(start);

            total_loss += loss->GetData();
            ++trained;
        }

        const double mean_loss =
            trained == 0 ? 0.0 : total_loss / static_cast<double>(trained);
        const Clock::time_point eval_start = Clock::now();
        const double accuracy = Evaluate(model, test_samples, test_limit);
        profile.eval_us = ElapsedUs(eval_start);
        profile.epoch_us = ElapsedUs(epoch_start);

        std::cout << "epoch " << (epoch + 1) << "/" << epochs
                  << " loss=" << mean_loss << " accuracy=" << accuracy << '\n';
        std::cout << "profile_us"
                  << " epoch=" << profile.epoch_us
                  << " input=" << profile.input_us
                  << " forward=" << profile.forward_us
                  << " loss=" << profile.loss_us
                  << " zero_grad=" << profile.zero_grad_us
                  << " backward=" << profile.backward_us
                  << " update=" << profile.update_us
                  << " eval=" << profile.eval_us << '\n';
        std::cout << "profile_avg_us_per_sample"
                  << " input=" << AverageUs(profile.input_us, trained)
                  << " forward=" << AverageUs(profile.forward_us, trained)
                  << " loss=" << AverageUs(profile.loss_us, trained)
                  << " zero_grad=" << AverageUs(profile.zero_grad_us, trained)
                  << " backward=" << AverageUs(profile.backward_us, trained)
                  << " update=" << AverageUs(profile.update_us, trained)
                  << '\n';
    }

    return 0;
}
