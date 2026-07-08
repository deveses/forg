#include "forg/nn/Attention.h"

#include "ModuleUtils.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <random>
#include <utility>

namespace forg::nn {
using namespace detail;

namespace {

void AppendParameters(Values& parameters, const Module* module)
{
    if (!module)
        return;

    Values module_parameters = module->Parameters();
    parameters.insert(parameters.end(), module_parameters.begin(),
                      module_parameters.end());
}

Values AddResidual(const Values& lhs, const Values& rhs)
{
    if (lhs.size() != rhs.size())
        return {};

    Values output;
    output.reserve(lhs.size());
    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
        ValuePtr value = lhs[index] + rhs[index];
        if (!value)
            return {};
        output.push_back(value);
    }
    return output;
}

Values ProjectSequence(const Values& input, std::size_t sequence_length,
                       std::size_t model_size, const Linear& projection)
{
    if (!IsInputValid(input, sequence_length * model_size))
        return {};

    Values output;
    output.reserve(input.size());
    for (std::size_t timestep = 0; timestep < sequence_length; ++timestep)
    {
        const Values token(input.begin() + timestep * model_size,
                           input.begin() + (timestep + 1) * model_size);
        Values projected = projection.Forward(token);
        if (projected.empty())
            return {};
        output.insert(output.end(), projected.begin(), projected.end());
    }
    return output;
}

Values HeadSlice(const Values& input, std::size_t sequence_length,
                 std::size_t model_size, std::size_t head_size,
                 std::size_t head)
{
    Values output;
    output.reserve(sequence_length * head_size);
    const std::size_t head_offset = head * head_size;
    for (std::size_t timestep = 0; timestep < sequence_length; ++timestep)
    {
        const std::size_t token_offset = timestep * model_size + head_offset;
        for (std::size_t dimension = 0; dimension < head_size; ++dimension)
        {
            output.push_back(input[token_offset + dimension]);
        }
    }
    return output;
}

Values ApplyFeedForward(const Values& input, std::size_t sequence_length,
                        std::size_t model_size, const Linear& first,
                        const Linear& second)
{
    if (!IsInputValid(input, sequence_length * model_size))
        return {};

    Values output;
    output.reserve(input.size());
    for (std::size_t timestep = 0; timestep < sequence_length; ++timestep)
    {
        const Values token(input.begin() + timestep * model_size,
                           input.begin() + (timestep + 1) * model_size);
        Values hidden = first.Forward(token);
        if (hidden.empty())
            return {};

        for (ValuePtr& value : hidden)
        {
            value = Relu(value);
            if (!value)
                return {};
        }

        Values projected = second.Forward(hidden);
        if (projected.empty())
            return {};
        output.insert(output.end(), projected.begin(), projected.end());
    }
    return output;
}

AttentionMask MaskFromBool(bool causal)
{
    return causal ? AttentionMask::Causal : AttentionMask::None;
}

} // namespace

ScaledDotProductAttention::ScaledDotProductAttention(
    std::size_t key_size, std::size_t value_size, std::size_t query_length,
    std::size_t key_length, AttentionMask mask)
    : m_key_size(key_size), m_value_size(value_size),
      m_query_length(query_length), m_key_length(key_length), m_mask(mask)
{
    if (m_key_size == 0 || m_value_size == 0 || m_query_length == 0 ||
        m_key_length == 0)
    {
        m_key_size = 0;
        m_value_size = 0;
        m_query_length = 0;
        m_key_length = 0;
    }
}

Values ScaledDotProductAttention::Forward(const Values& input) const
{
    const std::size_t query_count = m_query_length * m_key_size;
    const std::size_t key_count = m_key_length * m_key_size;
    const std::size_t value_count = m_key_length * m_value_size;
    if (!IsInputValid(input, query_count + key_count + value_count))
        return {};

    const Values query(input.begin(), input.begin() + query_count);
    const Values key(input.begin() + query_count,
                     input.begin() + query_count + key_count);
    const Values value(input.begin() + query_count + key_count, input.end());
    return Forward(query, key, value);
}

Values ScaledDotProductAttention::Forward(const Values& query,
                                          const Values& key,
                                          const Values& value) const
{
    if (m_key_size == 0 || m_value_size == 0 || m_query_length == 0 ||
        m_key_length == 0 ||
        !IsInputValid(query, m_query_length * m_key_size) ||
        !IsInputValid(key, m_key_length * m_key_size) ||
        !IsInputValid(value, m_key_length * m_value_size))
    {
        return {};
    }

    Values output;
    output.reserve(m_query_length * m_value_size);
    const double scale = 1.0 / std::sqrt(static_cast<double>(m_key_size));
    for (std::size_t query_index = 0; query_index < m_query_length;
         ++query_index)
    {
        Values scores;
        std::vector<std::size_t> allowed_keys;
        scores.reserve(m_key_length);
        allowed_keys.reserve(m_key_length);
        double max_score = -std::numeric_limits<double>::infinity();

        for (std::size_t key_index = 0; key_index < m_key_length; ++key_index)
        {
            if (m_mask == AttentionMask::Causal && key_index > query_index)
                continue;

            ValuePtr score = MakeValue(0.0);
            for (std::size_t dimension = 0; dimension < m_key_size;
                 ++dimension)
            {
                score =
                    score +
                    query[query_index * m_key_size + dimension] *
                        key[key_index * m_key_size + dimension];
                if (!score)
                    return {};
            }
            score = score * scale;
            if (!score)
                return {};

            max_score = std::max(max_score, score->GetData());
            scores.push_back(score);
            allowed_keys.push_back(key_index);
        }

        if (scores.empty())
            return {};

        Values weights;
        weights.reserve(scores.size());
        ValuePtr denominator = MakeValue(0.0);
        for (const ValuePtr& score : scores)
        {
            ValuePtr weight = Exp(score - max_score);
            if (!weight)
                return {};
            denominator = denominator + weight;
            if (!denominator)
                return {};
            weights.push_back(weight);
        }

        for (ValuePtr& weight : weights)
        {
            weight = weight / denominator;
            if (!weight)
                return {};
        }

        for (std::size_t value_dimension = 0; value_dimension < m_value_size;
             ++value_dimension)
        {
            ValuePtr context = MakeValue(0.0);
            for (std::size_t index = 0; index < allowed_keys.size(); ++index)
            {
                const std::size_t key_index = allowed_keys[index];
                context =
                    context +
                    weights[index] *
                        value[key_index * m_value_size + value_dimension];
                if (!context)
                    return {};
            }
            output.push_back(context);
        }
    }
    return output;
}

MultiHeadAttention::MultiHeadAttention(std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t sequence_length,
                                       bool causal)
    : MultiHeadAttention(model_size, head_count, sequence_length,
                         DefaultRng(), causal)
{
}

MultiHeadAttention::MultiHeadAttention(std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t sequence_length,
                                       std::mt19937& rng, bool causal)
    : MultiHeadAttention(model_size, head_count, sequence_length,
                         sequence_length, rng, causal)
{
}

MultiHeadAttention::MultiHeadAttention(std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t query_length,
                                       std::size_t key_length, bool causal)
    : MultiHeadAttention(model_size, head_count, query_length, key_length,
                         DefaultRng(), causal)
{
}

MultiHeadAttention::MultiHeadAttention(std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t query_length,
                                       std::size_t key_length,
                                       std::mt19937& rng, bool causal)
    : m_model_size(model_size), m_head_count(head_count),
      m_query_length(query_length), m_key_length(key_length),
      m_causal(causal)
{
    if (m_model_size == 0 || m_head_count == 0 || m_query_length == 0 ||
        m_key_length == 0 || m_model_size % m_head_count != 0)
    {
        m_model_size = 0;
        m_head_count = 0;
        m_query_length = 0;
        m_key_length = 0;
        return;
    }

    m_head_size = m_model_size / m_head_count;
    m_query_projection = std::make_unique<Linear>(m_model_size, m_model_size,
                                                  rng);
    m_key_projection =
        std::make_unique<Linear>(m_model_size, m_model_size, rng);
    m_value_projection =
        std::make_unique<Linear>(m_model_size, m_model_size, rng);
    m_output_projection =
        std::make_unique<Linear>(m_model_size, m_model_size, rng);
}

Values MultiHeadAttention::Forward(const Values& input) const
{
    return Forward(input, input, input);
}

Values MultiHeadAttention::Forward(const Values& query, const Values& key,
                                   const Values& value) const
{
    if (!m_query_projection || !m_key_projection || !m_value_projection ||
        !m_output_projection ||
        !IsInputValid(query, m_query_length * m_model_size) ||
        !IsInputValid(key, m_key_length * m_model_size) ||
        !IsInputValid(value, m_key_length * m_model_size))
    {
        return {};
    }

    const Values projected_query = ProjectSequence(
        query, m_query_length, m_model_size, *m_query_projection);
    const Values projected_key =
        ProjectSequence(key, m_key_length, m_model_size, *m_key_projection);
    const Values projected_value =
        ProjectSequence(value, m_key_length, m_model_size, *m_value_projection);
    if (projected_query.empty() || projected_key.empty() ||
        projected_value.empty())
    {
        return {};
    }

    Values combined(m_query_length * m_model_size);
    for (std::size_t head = 0; head < m_head_count; ++head)
    {
        const Values query_head = HeadSlice(projected_query, m_query_length,
                                           m_model_size, m_head_size, head);
        const Values key_head = HeadSlice(projected_key, m_key_length,
                                         m_model_size, m_head_size, head);
        const Values value_head = HeadSlice(projected_value, m_key_length,
                                           m_model_size, m_head_size, head);
        const ScaledDotProductAttention attention(
            m_head_size, m_head_size, m_query_length, m_key_length,
            MaskFromBool(m_causal));
        const Values head_output =
            attention.Forward(query_head, key_head, value_head);
        if (head_output.empty())
            return {};

        const std::size_t head_offset = head * m_head_size;
        for (std::size_t timestep = 0; timestep < m_query_length; ++timestep)
        {
            for (std::size_t dimension = 0; dimension < m_head_size;
                 ++dimension)
            {
                combined[timestep * m_model_size + head_offset + dimension] =
                    head_output[timestep * m_head_size + dimension];
            }
        }
    }

    return ProjectSequence(combined, m_query_length, m_model_size,
                           *m_output_projection);
}

Values MultiHeadAttention::Parameters() const
{
    Values parameters;
    AppendParameters(parameters, m_query_projection.get());
    AppendParameters(parameters, m_key_projection.get());
    AppendParameters(parameters, m_value_projection.get());
    AppendParameters(parameters, m_output_projection.get());
    return parameters;
}

void MultiHeadAttention::Train(bool training)
{
    Module::Train(training);
    if (m_query_projection)
        m_query_projection->Train(training);
    if (m_key_projection)
        m_key_projection->Train(training);
    if (m_value_projection)
        m_value_projection->Train(training);
    if (m_output_projection)
        m_output_projection->Train(training);
}

TransformerEncoderBlock::TransformerEncoderBlock(
    std::size_t model_size, std::size_t head_count,
    std::size_t sequence_length, std::size_t feed_forward_size)
    : TransformerEncoderBlock(model_size, head_count, sequence_length,
                              feed_forward_size, DefaultRng())
{
}

TransformerEncoderBlock::TransformerEncoderBlock(
    std::size_t model_size, std::size_t head_count,
    std::size_t sequence_length, std::size_t feed_forward_size,
    std::mt19937& rng)
    : m_model_size(model_size), m_sequence_length(sequence_length),
      m_feed_forward_size(feed_forward_size)
{
    if (m_model_size == 0 || m_sequence_length == 0 ||
        m_feed_forward_size == 0 || head_count == 0 ||
        m_model_size % head_count != 0)
    {
        m_model_size = 0;
        m_sequence_length = 0;
        m_feed_forward_size = 0;
        return;
    }

    m_attention = std::make_unique<MultiHeadAttention>(
        m_model_size, head_count, m_sequence_length, rng);
    m_norm1 = std::make_unique<LayerNorm>(m_model_size, m_sequence_length);
    m_norm2 = std::make_unique<LayerNorm>(m_model_size, m_sequence_length);
    m_feed_forward1 =
        std::make_unique<Linear>(m_model_size, m_feed_forward_size, rng);
    m_feed_forward2 =
        std::make_unique<Linear>(m_feed_forward_size, m_model_size, rng);
}

Values TransformerEncoderBlock::Forward(const Values& input) const
{
    if (!m_attention || !m_norm1 || !m_norm2 || !m_feed_forward1 ||
        !m_feed_forward2 ||
        !IsInputValid(input, m_sequence_length * m_model_size))
    {
        return {};
    }

    const Values attention = m_attention->Forward(input);
    const Values attention_residual = AddResidual(input, attention);
    const Values normalized_attention = m_norm1->Forward(attention_residual);
    const Values feed_forward =
        ApplyFeedForward(normalized_attention, m_sequence_length, m_model_size,
                         *m_feed_forward1, *m_feed_forward2);
    const Values feed_forward_residual =
        AddResidual(normalized_attention, feed_forward);
    return m_norm2->Forward(feed_forward_residual);
}

Values TransformerEncoderBlock::Parameters() const
{
    Values parameters;
    AppendParameters(parameters, m_attention.get());
    AppendParameters(parameters, m_norm1.get());
    AppendParameters(parameters, m_norm2.get());
    AppendParameters(parameters, m_feed_forward1.get());
    AppendParameters(parameters, m_feed_forward2.get());
    return parameters;
}

void TransformerEncoderBlock::Train(bool training)
{
    Module::Train(training);
    if (m_attention)
        m_attention->Train(training);
    if (m_norm1)
        m_norm1->Train(training);
    if (m_norm2)
        m_norm2->Train(training);
    if (m_feed_forward1)
        m_feed_forward1->Train(training);
    if (m_feed_forward2)
        m_feed_forward2->Train(training);
}

TransformerDecoderBlock::TransformerDecoderBlock(
    std::size_t model_size, std::size_t head_count,
    std::size_t decoder_length, std::size_t encoder_length,
    std::size_t feed_forward_size)
    : TransformerDecoderBlock(model_size, head_count, decoder_length,
                              encoder_length, feed_forward_size, DefaultRng())
{
}

TransformerDecoderBlock::TransformerDecoderBlock(
    std::size_t model_size, std::size_t head_count,
    std::size_t decoder_length, std::size_t encoder_length,
    std::size_t feed_forward_size, std::mt19937& rng)
    : m_model_size(model_size), m_decoder_length(decoder_length),
      m_encoder_length(encoder_length), m_feed_forward_size(feed_forward_size)
{
    if (m_model_size == 0 || m_decoder_length == 0 ||
        m_encoder_length == 0 || m_feed_forward_size == 0 ||
        head_count == 0 || m_model_size % head_count != 0)
    {
        m_model_size = 0;
        m_decoder_length = 0;
        m_encoder_length = 0;
        m_feed_forward_size = 0;
        return;
    }

    m_self_attention = std::make_unique<MultiHeadAttention>(
        m_model_size, head_count, m_decoder_length, rng, true);
    m_cross_attention = std::make_unique<MultiHeadAttention>(
        m_model_size, head_count, m_decoder_length, m_encoder_length, rng);
    m_norm1 = std::make_unique<LayerNorm>(m_model_size, m_decoder_length);
    m_norm2 = std::make_unique<LayerNorm>(m_model_size, m_decoder_length);
    m_norm3 = std::make_unique<LayerNorm>(m_model_size, m_decoder_length);
    m_feed_forward1 =
        std::make_unique<Linear>(m_model_size, m_feed_forward_size, rng);
    m_feed_forward2 =
        std::make_unique<Linear>(m_feed_forward_size, m_model_size, rng);
}

Values TransformerDecoderBlock::Forward(const Values& input) const
{
    const std::size_t decoder_count = m_decoder_length * m_model_size;
    const std::size_t encoder_count = m_encoder_length * m_model_size;
    if (!IsInputValid(input, decoder_count + encoder_count))
        return {};

    const Values decoder_input(input.begin(), input.begin() + decoder_count);
    const Values encoder_memory(input.begin() + decoder_count, input.end());
    return Forward(decoder_input, encoder_memory);
}

Values TransformerDecoderBlock::Forward(const Values& decoder_input,
                                        const Values& encoder_memory) const
{
    if (!m_self_attention || !m_cross_attention || !m_norm1 || !m_norm2 ||
        !m_norm3 || !m_feed_forward1 || !m_feed_forward2 ||
        !IsInputValid(decoder_input, m_decoder_length * m_model_size) ||
        !IsInputValid(encoder_memory, m_encoder_length * m_model_size))
    {
        return {};
    }

    const Values self_attention = m_self_attention->Forward(decoder_input);
    const Values self_residual = AddResidual(decoder_input, self_attention);
    const Values normalized_self = m_norm1->Forward(self_residual);
    const Values cross_attention = m_cross_attention->Forward(
        normalized_self, encoder_memory, encoder_memory);
    const Values cross_residual = AddResidual(normalized_self, cross_attention);
    const Values normalized_cross = m_norm2->Forward(cross_residual);
    const Values feed_forward =
        ApplyFeedForward(normalized_cross, m_decoder_length, m_model_size,
                         *m_feed_forward1, *m_feed_forward2);
    const Values feed_forward_residual =
        AddResidual(normalized_cross, feed_forward);
    return m_norm3->Forward(feed_forward_residual);
}

Values TransformerDecoderBlock::Parameters() const
{
    Values parameters;
    AppendParameters(parameters, m_self_attention.get());
    AppendParameters(parameters, m_cross_attention.get());
    AppendParameters(parameters, m_norm1.get());
    AppendParameters(parameters, m_norm2.get());
    AppendParameters(parameters, m_norm3.get());
    AppendParameters(parameters, m_feed_forward1.get());
    AppendParameters(parameters, m_feed_forward2.get());
    return parameters;
}

void TransformerDecoderBlock::Train(bool training)
{
    Module::Train(training);
    if (m_self_attention)
        m_self_attention->Train(training);
    if (m_cross_attention)
        m_cross_attention->Train(training);
    if (m_norm1)
        m_norm1->Train(training);
    if (m_norm2)
        m_norm2->Train(training);
    if (m_norm3)
        m_norm3->Train(training);
    if (m_feed_forward1)
        m_feed_forward1->Train(training);
    if (m_feed_forward2)
        m_feed_forward2->Train(training);
}

TransformerEncoder::TransformerEncoder(std::size_t layer_count,
                                       std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t sequence_length,
                                       std::size_t feed_forward_size)
    : TransformerEncoder(layer_count, model_size, head_count, sequence_length,
                         feed_forward_size, DefaultRng())
{
}

TransformerEncoder::TransformerEncoder(std::size_t layer_count,
                                       std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t sequence_length,
                                       std::size_t feed_forward_size,
                                       std::mt19937& rng)
{
    if (layer_count == 0)
        return;

    m_layers.reserve(layer_count);
    for (std::size_t index = 0; index < layer_count; ++index)
    {
        auto layer = std::make_unique<TransformerEncoderBlock>(
            model_size, head_count, sequence_length, feed_forward_size, rng);
        if (layer->Parameters().empty())
        {
            m_layers.clear();
            return;
        }
        m_layers.push_back(std::move(layer));
    }
}

Values TransformerEncoder::Forward(const Values& input) const
{
    if (m_layers.empty())
        return {};

    Values output = input;
    for (const std::unique_ptr<TransformerEncoderBlock>& layer : m_layers)
    {
        output = layer->Forward(output);
        if (output.empty())
            return {};
    }
    return output;
}

Values TransformerEncoder::Parameters() const
{
    Values parameters;
    for (const std::unique_ptr<TransformerEncoderBlock>& layer : m_layers)
    {
        AppendParameters(parameters, layer.get());
    }
    return parameters;
}

void TransformerEncoder::Train(bool training)
{
    Module::Train(training);
    for (const std::unique_ptr<TransformerEncoderBlock>& layer : m_layers)
    {
        if (layer)
            layer->Train(training);
    }
}

TransformerDecoder::TransformerDecoder(std::size_t layer_count,
                                       std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t decoder_length,
                                       std::size_t encoder_length,
                                       std::size_t feed_forward_size)
    : TransformerDecoder(layer_count, model_size, head_count, decoder_length,
                         encoder_length, feed_forward_size, DefaultRng())
{
}

TransformerDecoder::TransformerDecoder(std::size_t layer_count,
                                       std::size_t model_size,
                                       std::size_t head_count,
                                       std::size_t decoder_length,
                                       std::size_t encoder_length,
                                       std::size_t feed_forward_size,
                                       std::mt19937& rng)
    : m_model_size(model_size), m_decoder_length(decoder_length),
      m_encoder_length(encoder_length)
{
    if (layer_count == 0)
        return;

    m_layers.reserve(layer_count);
    for (std::size_t index = 0; index < layer_count; ++index)
    {
        auto layer = std::make_unique<TransformerDecoderBlock>(
            model_size, head_count, decoder_length, encoder_length,
            feed_forward_size, rng);
        if (layer->Parameters().empty())
        {
            m_layers.clear();
            m_model_size = 0;
            m_decoder_length = 0;
            m_encoder_length = 0;
            return;
        }
        m_layers.push_back(std::move(layer));
    }
}

Values TransformerDecoder::Forward(const Values& input) const
{
    const std::size_t decoder_count = m_decoder_length * m_model_size;
    const std::size_t encoder_count = m_encoder_length * m_model_size;
    if (!IsInputValid(input, decoder_count + encoder_count))
        return {};

    const Values decoder_input(input.begin(), input.begin() + decoder_count);
    const Values encoder_memory(input.begin() + decoder_count, input.end());
    return Forward(decoder_input, encoder_memory);
}

Values TransformerDecoder::Forward(const Values& decoder_input,
                                   const Values& encoder_memory) const
{
    if (m_layers.empty() ||
        !IsInputValid(decoder_input, m_decoder_length * m_model_size) ||
        !IsInputValid(encoder_memory, m_encoder_length * m_model_size))
    {
        return {};
    }

    Values output = decoder_input;
    for (const std::unique_ptr<TransformerDecoderBlock>& layer : m_layers)
    {
        output = layer->Forward(output, encoder_memory);
        if (output.empty())
            return {};
    }
    return output;
}

Values TransformerDecoder::Parameters() const
{
    Values parameters;
    for (const std::unique_ptr<TransformerDecoderBlock>& layer : m_layers)
    {
        AppendParameters(parameters, layer.get());
    }
    return parameters;
}

void TransformerDecoder::Train(bool training)
{
    Module::Train(training);
    for (const std::unique_ptr<TransformerDecoderBlock>& layer : m_layers)
    {
        if (layer)
            layer->Train(training);
    }
}

} // namespace forg::nn
