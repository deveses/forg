#include <forg/nn.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

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

std::uint64_t ElapsedMs(Clock::time_point start)
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                              start)
            .count());
}

void PrintUsage(const char* program)
{
    std::cout
        << "Usage: " << program
        << " [dataset-path] [steps] [block-size] [model-size] [heads]"
           " [layers] [feed-forward-size] [learning-rate] [generate-count]"
           " [seed-text] [backend] [batch-size] [thread-count]"
           " [checkpoint-path] [target-loss]\n"
        << "Default dataset: data/gpt_dataset/tiny_shakespeare_dataset.txt\n"
        << "Example: " << program
        << " data/gpt_dataset/tiny_shakespeare_dataset.txt 20 8 16 2 1 32"
           " 0.001 200 \"First Citizen:\" matrix 16\n"
        << "Backends: matrix, scalar\n";
}

bool IsBackendName(const std::string& text)
{
    return text == "matrix" || text == "scalar";
}

std::string ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return {};

    return std::string(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());
}

void AppendParameters(forg::nn::Values& parameters,
                      const forg::nn::Module& module)
{
    forg::nn::Values module_parameters = module.Parameters();
    parameters.insert(parameters.end(), module_parameters.begin(),
                      module_parameters.end());
}

forg::nn::Values Add(const forg::nn::Values& lhs, const forg::nn::Values& rhs)
{
    if (lhs.size() != rhs.size())
        return {};

    forg::nn::Values output;
    output.reserve(lhs.size());
    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
        forg::nn::ValuePtr value = lhs[index] + rhs[index];
        if (!value)
            return {};
        output.push_back(value);
    }
    return output;
}

forg::nn::Values TokenSlice(const forg::nn::Values& input,
                            std::size_t token_index,
                            std::size_t feature_count)
{
    const std::size_t begin = token_index * feature_count;
    return forg::nn::Values(input.begin() + begin,
                            input.begin() + begin + feature_count);
}

forg::nn::Values ApplyPerToken(const forg::nn::Values& input,
                               std::size_t sequence_length,
                               std::size_t input_size,
                               const forg::nn::Linear& first,
                               const forg::nn::Linear& second)
{
    if (input.size() != sequence_length * input_size)
        return {};

    forg::nn::Values output;
    output.reserve(input.size());
    for (std::size_t timestep = 0; timestep < sequence_length; ++timestep)
    {
        forg::nn::Values hidden =
            first.Forward(TokenSlice(input, timestep, input_size));
        if (hidden.empty())
            return {};

        for (forg::nn::ValuePtr& value : hidden)
        {
            value = forg::nn::Relu(value);
            if (!value)
                return {};
        }

        forg::nn::Values projected = second.Forward(hidden);
        if (projected.empty())
            return {};
        output.insert(output.end(), projected.begin(), projected.end());
    }
    return output;
}

class Vocabulary
{
  public:
    bool Build(const std::string& text)
    {
        m_chars.clear();
        m_to_index.clear();

        std::vector<bool> seen(256, false);
        for (const unsigned char ch : text)
            seen[ch] = true;

        for (std::size_t index = 0; index < seen.size(); ++index)
        {
            if (!seen[index])
                continue;

            const char ch = static_cast<char>(index);
            m_to_index[ch] = m_chars.size();
            m_chars.push_back(ch);
        }
        return !m_chars.empty();
    }

    std::vector<std::size_t> Encode(const std::string& text) const
    {
        std::vector<std::size_t> output;
        output.reserve(text.size());
        for (const char ch : text)
        {
            const auto found = m_to_index.find(ch);
            if (found != m_to_index.end())
                output.push_back(found->second);
        }
        return output;
    }

    std::string Decode(const std::vector<std::size_t>& indices) const
    {
        std::string output;
        output.reserve(indices.size());
        for (const std::size_t index : indices)
        {
            if (index < m_chars.size())
                output.push_back(m_chars[index]);
        }
        return output;
    }

    std::size_t Size() const noexcept { return m_chars.size(); }
    std::size_t IndexOf(char ch) const
    {
        const auto found = m_to_index.find(ch);
        return found == m_to_index.end() ? 0 : found->second;
    }

  private:
    std::vector<char> m_chars;
    std::unordered_map<char, std::size_t> m_to_index;
};

class GptBlock
{
  public:
    GptBlock(std::size_t block_size, std::size_t model_size,
             std::size_t head_count, std::size_t feed_forward_size,
             std::mt19937& rng)
        : m_block_size(block_size), m_model_size(model_size),
          m_attention(model_size, head_count, block_size, rng, true),
          m_norm1(model_size, block_size), m_norm2(model_size, block_size),
          m_feed_forward1(model_size, feed_forward_size, rng),
          m_feed_forward2(feed_forward_size, model_size, rng)
    {
    }

    forg::nn::Values Forward(const forg::nn::Values& input) const
    {
        const forg::nn::Values attention = m_attention.Forward(input);
        const forg::nn::Values attention_residual = Add(input, attention);
        const forg::nn::Values normalized_attention =
            m_norm1.Forward(attention_residual);
        const forg::nn::Values feed_forward =
            ApplyPerToken(normalized_attention, m_block_size, m_model_size,
                          m_feed_forward1, m_feed_forward2);
        const forg::nn::Values feed_forward_residual =
            Add(normalized_attention, feed_forward);
        return m_norm2.Forward(feed_forward_residual);
    }

    forg::nn::Values Parameters() const
    {
        forg::nn::Values parameters;
        AppendParameters(parameters, m_attention);
        AppendParameters(parameters, m_norm1);
        AppendParameters(parameters, m_norm2);
        AppendParameters(parameters, m_feed_forward1);
        AppendParameters(parameters, m_feed_forward2);
        return parameters;
    }

  private:
    std::size_t m_block_size = 0;
    std::size_t m_model_size = 0;
    forg::nn::MultiHeadAttention m_attention;
    forg::nn::LayerNorm m_norm1;
    forg::nn::LayerNorm m_norm2;
    forg::nn::Linear m_feed_forward1;
    forg::nn::Linear m_feed_forward2;
};

class TinyGpt
{
  public:
    TinyGpt(std::size_t vocab_size, std::size_t block_size,
            std::size_t model_size, std::size_t head_count,
            std::size_t layer_count, std::size_t feed_forward_size,
            std::mt19937& rng)
        : m_vocab_size(vocab_size), m_block_size(block_size),
          m_model_size(model_size), m_token_embedding(vocab_size, model_size,
                                                     rng),
          m_position_embedding(block_size, model_size, rng),
          m_final_norm(model_size, block_size),
          m_head(model_size, vocab_size, rng)
    {
        m_blocks.reserve(layer_count);
        for (std::size_t index = 0; index < layer_count; ++index)
        {
            m_blocks.push_back(std::make_unique<GptBlock>(
                block_size, model_size, head_count, feed_forward_size, rng));
        }
    }

    forg::nn::Values Forward(const std::vector<std::size_t>& indices) const
    {
        if (indices.size() != m_block_size || m_blocks.empty())
            return {};

        const forg::nn::Values token = m_token_embedding.Forward(indices);
        std::vector<std::size_t> positions;
        positions.reserve(m_block_size);
        for (std::size_t index = 0; index < m_block_size; ++index)
            positions.push_back(index);

        const forg::nn::Values position = m_position_embedding.Forward(
            positions);
        forg::nn::Values output = Add(token, position);
        if (output.empty())
            return {};

        for (const std::unique_ptr<GptBlock>& block : m_blocks)
        {
            output = block->Forward(output);
            if (output.empty())
                return {};
        }

        output = m_final_norm.Forward(output);
        if (output.empty())
            return {};

        forg::nn::Values logits;
        logits.reserve(m_block_size * m_vocab_size);
        for (std::size_t timestep = 0; timestep < m_block_size; ++timestep)
        {
            forg::nn::Values token_logits =
                m_head.Forward(TokenSlice(output, timestep, m_model_size));
            if (token_logits.empty())
                return {};
            logits.insert(logits.end(), token_logits.begin(),
                          token_logits.end());
        }
        return logits;
    }

    forg::nn::Values Parameters() const
    {
        forg::nn::Values parameters;
        AppendParameters(parameters, m_token_embedding);
        AppendParameters(parameters, m_position_embedding);
        for (const std::unique_ptr<GptBlock>& block : m_blocks)
        {
            forg::nn::Values block_parameters = block->Parameters();
            parameters.insert(parameters.end(), block_parameters.begin(),
                              block_parameters.end());
        }
        AppendParameters(parameters, m_final_norm);
        AppendParameters(parameters, m_head);
        return parameters;
    }

    std::size_t BlockSize() const noexcept { return m_block_size; }
    std::size_t VocabSize() const noexcept { return m_vocab_size; }

  private:
    std::size_t m_vocab_size = 0;
    std::size_t m_block_size = 0;
    std::size_t m_model_size = 0;
    forg::nn::Embedding m_token_embedding;
    forg::nn::Embedding m_position_embedding;
    std::vector<std::unique_ptr<GptBlock>> m_blocks;
    forg::nn::LayerNorm m_final_norm;
    forg::nn::Linear m_head;
};

forg::nn::ValuePtr SequenceLoss(const forg::nn::Values& logits,
                                const std::vector<std::size_t>& targets,
                                std::size_t vocab_size)
{
    if (targets.empty() || logits.size() != targets.size() * vocab_size)
        return nullptr;

    forg::nn::ValuePtr loss = forg::nn::MakeValue(0.0);
    for (std::size_t timestep = 0; timestep < targets.size(); ++timestep)
    {
        const std::size_t begin = timestep * vocab_size;
        const forg::nn::Values token_logits(logits.begin() + begin,
                                            logits.begin() + begin +
                                                vocab_size);
        const forg::nn::ValuePtr token_loss = forg::nn::CrossEntropyLoss(
            token_logits, targets[timestep]);
        if (!token_loss)
            return nullptr;

        loss = loss + token_loss;
        if (!loss)
            return nullptr;
    }
    return loss / static_cast<double>(targets.size());
}

std::size_t SampleFromLogits(const forg::nn::Values& logits, std::mt19937& rng)
{
    const forg::nn::Values probabilities = forg::nn::Softmax(logits);
    if (probabilities.empty())
        return 0;

    std::vector<double> weights;
    weights.reserve(probabilities.size());
    for (const forg::nn::ValuePtr& probability : probabilities)
        weights.push_back(std::max(0.0, probability->GetData()));

    std::discrete_distribution<std::size_t> distribution(weights.begin(),
                                                         weights.end());
    return distribution(rng);
}

std::size_t SampleFromLogits(const forg::nn::Matrix& logits, std::size_t row,
                             std::mt19937& rng)
{
    if (logits.Empty() || row >= logits.Rows())
        return 0;

    double max_logit = logits(row, 0);
    for (std::size_t column = 1; column < logits.Columns(); ++column)
        max_logit = std::max(max_logit, logits(row, column));

    std::vector<double> weights(logits.Columns(), 0.0);
    for (std::size_t column = 0; column < logits.Columns(); ++column)
        weights[column] = std::exp(logits(row, column) - max_logit);

    std::discrete_distribution<std::size_t> distribution(weights.begin(),
                                                         weights.end());
    return distribution(rng);
}

std::string Generate(const TinyGpt& model, const Vocabulary& vocabulary,
                     std::vector<std::size_t> context,
                     std::size_t token_count, std::mt19937& rng)
{
    if (context.empty())
        context.push_back(0);

    std::vector<std::size_t> generated = context;
    std::vector<std::size_t> window(model.BlockSize(), context.front());
    for (std::size_t step = 0; step < token_count; ++step)
    {
        if (generated.size() >= model.BlockSize())
        {
            std::copy(generated.end() - static_cast<std::ptrdiff_t>(
                                           model.BlockSize()),
                      generated.end(), window.begin());
        }
        else
        {
            std::fill(window.begin(), window.end(), generated.front());
            std::copy(generated.begin(), generated.end(),
                      window.end() -
                          static_cast<std::ptrdiff_t>(generated.size()));
        }

        const forg::nn::Values logits = model.Forward(window);
        if (logits.empty())
            break;

        const std::size_t begin = (model.BlockSize() - 1) * model.VocabSize();
        const forg::nn::Values next_logits(logits.begin() + begin,
                                           logits.begin() + begin +
                                               model.VocabSize());
        generated.push_back(SampleFromLogits(next_logits, rng));
    }
    return vocabulary.Decode(generated);
}

std::string Generate(forg::nn::MatrixGPT& model, const Vocabulary& vocabulary,
                     std::vector<std::size_t> context,
                     std::size_t token_count, std::mt19937& rng)
{
    if (context.empty())
        context.push_back(0);

    const std::size_t block_size = model.Config().block_size;
    std::vector<std::size_t> generated = context;
    std::vector<std::size_t> window(block_size, context.front());
    for (std::size_t step = 0; step < token_count; ++step)
    {
        if (generated.size() >= block_size)
        {
            std::copy(generated.end() -
                          static_cast<std::ptrdiff_t>(block_size),
                      generated.end(), window.begin());
        }
        else
        {
            std::fill(window.begin(), window.end(), generated.front());
            std::copy(generated.begin(), generated.end(),
                      window.end() -
                          static_cast<std::ptrdiff_t>(generated.size()));
        }

        const forg::nn::Matrix logits = model.Forward(window, 1);
        if (logits.Empty())
            break;

        generated.push_back(SampleFromLogits(logits, block_size - 1, rng));
    }
    return vocabulary.Decode(generated);
}

} // namespace

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--help")
    {
        PrintUsage(argv[0]);
        return 0;
    }

    const std::string dataset_path =
        argc > 1 ? argv[1] : "data/gpt_dataset/tiny_shakespeare_dataset.txt";
    const std::size_t steps = argc > 2 ? ParseSize(argv[2], 20) : 20;
    const std::size_t block_size = argc > 3 ? ParseSize(argv[3], 8) : 8;
    const std::size_t model_size = argc > 4 ? ParseSize(argv[4], 16) : 16;
    const std::size_t head_count = argc > 5 ? ParseSize(argv[5], 2) : 2;
    const std::size_t layer_count = argc > 6 ? ParseSize(argv[6], 1) : 1;
    const std::size_t feed_forward_size =
        argc > 7 ? ParseSize(argv[7], 32) : 32;
    const double learning_rate =
        argc > 8 ? ParseDouble(argv[8], 0.001) : 0.001;
    const std::size_t generate_count =
        argc > 9 ? ParseSize(argv[9], 200) : 200;
    const std::string seed_text = argc > 10 ? argv[10] : "First Citizen:";
    const std::string backend =
        argc > 11 && IsBackendName(argv[11]) ? argv[11] : "matrix";
    const std::size_t batch_size =
        std::max<std::size_t>(1, argc > 12 ? ParseSize(argv[12], 16) : 16);
    const std::size_t thread_count =
        argc > 13 ? ParseSize(argv[13], 0) : 0;
    const std::string checkpoint_path = argc > 14 ? argv[14] : "";
    const double target_loss =
        argc > 15 ? ParseDouble(argv[15], 0.0) : 0.0;

    if (block_size == 0 || model_size == 0 || head_count == 0 ||
        layer_count == 0 || feed_forward_size == 0 ||
        model_size % head_count != 0 || !IsBackendName(backend))
    {
        std::cerr << "Invalid model dimensions or backend\n";
        PrintUsage(argv[0]);
        return 1;
    }

    const std::string text = ReadFile(dataset_path);
    if (text.size() <= block_size)
    {
        std::cerr << "Unable to read enough training text from "
                  << dataset_path << '\n';
        return 1;
    }

    Vocabulary vocabulary;
    if (!vocabulary.Build(text))
    {
        std::cerr << "Unable to build vocabulary\n";
        return 1;
    }

    const std::vector<std::size_t> encoded = vocabulary.Encode(text);
    if (encoded.size() <= block_size)
    {
        std::cerr << "Encoded dataset is too small for block size\n";
        return 1;
    }

    std::mt19937 rng(1337);
    if (backend == "matrix")
    {
        forg::nn::MatrixGPTConfig config;
        config.vocab_size = vocabulary.Size();
        config.block_size = block_size;
        config.model_size = model_size;
        config.head_count = head_count;
        config.layer_count = layer_count;
        config.feed_forward_size = feed_forward_size;

        forg::nn::MatrixGPT model(config, rng);
        model.SetThreadCount(thread_count);
        if (!model.Valid())
        {
            std::cerr << "Unable to create MatrixGPT\n";
            return 1;
        }

        if (!checkpoint_path.empty() &&
            std::filesystem::exists(checkpoint_path))
        {
            std::string error;
            if (!model.LoadParameters(checkpoint_path, &error))
            {
                std::cerr << "Unable to load checkpoint: " << error << '\n';
                return 1;
            }
            std::cout << "loaded_checkpoint=" << checkpoint_path << '\n';
        }

        std::uniform_int_distribution<std::size_t> offset_distribution(
            0, encoded.size() - block_size - 1);

        std::cout << "dataset_bytes=" << text.size()
                  << " vocab_size=" << vocabulary.Size()
                  << " block_size=" << block_size
                  << " model_size=" << model_size
                  << " heads=" << head_count
                  << " layers=" << layer_count
                  << " backend=matrix"
                  << " batch_size=" << batch_size
                  << " threads=" << model.ThreadCount()
                  << " parameters=" << model.ParameterCount();
        if (target_loss > 0.0)
            std::cout << " target_loss=" << target_loss;
        std::cout << '\n';

        std::vector<std::size_t> input(batch_size * block_size, 0);
        std::vector<std::size_t> target(batch_size * block_size, 0);
        for (std::size_t step = 0; step < steps; ++step)
        {
            for (std::size_t row = 0; row < batch_size; ++row)
            {
                const std::size_t offset = offset_distribution(rng);
                for (std::size_t token = 0; token < block_size; ++token)
                {
                    input[row * block_size + token] = encoded[offset + token];
                    target[row * block_size + token] =
                        encoded[offset + token + 1];
                }
            }

            const Clock::time_point start = Clock::now();
            const double loss =
                model.TrainBatch(input, target, batch_size, learning_rate);
            if (!(loss > 0.0))
            {
                std::cerr << "Training failed at step " << (step + 1)
                          << '\n';
                return 1;
            }

            std::cout << "step " << (step + 1) << "/" << steps
                      << " loss=" << loss
                      << " ms=" << ElapsedMs(start) << '\n';
            if (target_loss > 0.0 && loss <= target_loss)
            {
                std::cout << "target_loss_reached step=" << (step + 1)
                          << " loss=" << loss << '\n';
                break;
            }
        }

        if (!checkpoint_path.empty())
        {
            std::string error;
            if (!model.SaveParameters(checkpoint_path, &error))
            {
                std::cerr << "Unable to save checkpoint: " << error << '\n';
                return 1;
            }
            std::cout << "saved_checkpoint=" << checkpoint_path << '\n';
        }

        std::vector<std::size_t> seed = vocabulary.Encode(seed_text);
        if (seed.empty())
            seed.push_back(vocabulary.IndexOf('\n'));

        std::cout << "\n--- sample ---\n"
                  << Generate(model, vocabulary, std::move(seed),
                              generate_count, rng)
                  << "\n--- end ---\n";
        return 0;
    }

    TinyGpt model(vocabulary.Size(), block_size, model_size, head_count,
                  layer_count, feed_forward_size, rng);
    forg::nn::Adam optimizer(model.Parameters(), learning_rate);
    forg::nn::BackwardScratch scratch;
    std::uniform_int_distribution<std::size_t> offset_distribution(
        0, encoded.size() - block_size - 1);

    std::cout << "dataset_bytes=" << text.size()
              << " vocab_size=" << vocabulary.Size()
              << " block_size=" << block_size
              << " model_size=" << model_size
              << " heads=" << head_count
              << " layers=" << layer_count
              << " parameters=" << optimizer.Parameters().size();
    if (target_loss > 0.0)
        std::cout << " target_loss=" << target_loss;
    std::cout << '\n';

    for (std::size_t step = 0; step < steps; ++step)
    {
        const std::size_t offset = offset_distribution(rng);
        const std::vector<std::size_t> input(encoded.begin() + offset,
                                             encoded.begin() + offset +
                                                 block_size);
        const std::vector<std::size_t> target(encoded.begin() + offset + 1,
                                              encoded.begin() + offset + 1 +
                                                  block_size);

        const Clock::time_point start = Clock::now();
        optimizer.ZeroGrad();
        const forg::nn::Values logits = model.Forward(input);
        const forg::nn::ValuePtr loss = SequenceLoss(logits, target,
                                                     vocabulary.Size());
        if (!loss)
        {
            std::cerr << "Forward/loss failed at step " << (step + 1)
                      << '\n';
            return 1;
        }

        forg::nn::Backward(loss, scratch);
        optimizer.Step();

        std::cout << "step " << (step + 1) << "/" << steps
                  << " loss=" << loss->GetData()
                  << " ms=" << ElapsedMs(start) << '\n';
        if (target_loss > 0.0 && loss->GetData() <= target_loss)
        {
            std::cout << "target_loss_reached step=" << (step + 1)
                      << " loss=" << loss->GetData() << '\n';
            break;
        }
    }

    std::vector<std::size_t> seed = vocabulary.Encode(seed_text);
    if (seed.empty())
        seed.push_back(vocabulary.IndexOf('\n'));

    std::cout << "\n--- sample ---\n"
              << Generate(model, vocabulary, std::move(seed), generate_count,
                          rng)
              << "\n--- end ---\n";
    return 0;
}
