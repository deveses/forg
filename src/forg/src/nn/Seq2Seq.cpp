#include "forg/nn/Seq2Seq.h"

#include "forg/nn/Attention.h"
#include "ModuleUtils.h"

#include <cstddef>
#include <memory>
#include <random>

namespace forg::nn {
using namespace detail;

class RecurrentModule : public Module
{
  public:
    RecurrentModule(std::size_t input_size, std::size_t hidden_size,
                    std::size_t sequence_length)
        : m_input_size(input_size), m_hidden_size(hidden_size),
          m_sequence_length(sequence_length)
    {
    }

    RecurrentState ForwardState(const Values& input) const
    {
        return ForwardState(input, {});
    }

    virtual RecurrentState ForwardState(
        const Values& input, const RecurrentState& initial_state) const = 0;

    std::size_t InputSize() const noexcept { return m_input_size; }
    std::size_t HiddenSize() const noexcept { return m_hidden_size; }
    std::size_t SequenceLength() const noexcept { return m_sequence_length; }

  private:
    std::size_t m_input_size = 0;
    std::size_t m_hidden_size = 0;
    std::size_t m_sequence_length = 0;
};

namespace {

class RNNModule final : public RecurrentModule
{
  public:
    RNNModule(std::size_t input_size, std::size_t hidden_size,
              std::size_t sequence_length, std::mt19937& rng)
        : RecurrentModule(input_size, hidden_size, sequence_length),
          m_layer(input_size, hidden_size, sequence_length, rng)
    {
    }

    Values Forward(const Values& input) const override
    {
        return m_layer.Forward(input);
    }

    RecurrentState ForwardState(
        const Values& input,
        const RecurrentState& initial_state) const override
    {
        if (initial_state.hidden.empty())
            return m_layer.ForwardState(input);

        return m_layer.ForwardState(input, initial_state.hidden);
    }

    Values Parameters() const override { return m_layer.Parameters(); }

  private:
    RNN m_layer;
};

class LSTMModule final : public RecurrentModule
{
  public:
    LSTMModule(std::size_t input_size, std::size_t hidden_size,
               std::size_t sequence_length, std::mt19937& rng)
        : RecurrentModule(input_size, hidden_size, sequence_length),
          m_layer(input_size, hidden_size, sequence_length, rng)
    {
    }

    Values Forward(const Values& input) const override
    {
        return m_layer.Forward(input);
    }

    RecurrentState ForwardState(
        const Values& input,
        const RecurrentState& initial_state) const override
    {
        if (initial_state.hidden.empty() && initial_state.cell.empty())
            return m_layer.ForwardState(input);

        return m_layer.ForwardState(input, initial_state.hidden,
                                    initial_state.cell);
    }

    Values Parameters() const override { return m_layer.Parameters(); }

  private:
    LSTM m_layer;
};

class GRUModule final : public RecurrentModule
{
  public:
    GRUModule(std::size_t input_size, std::size_t hidden_size,
              std::size_t sequence_length, std::mt19937& rng)
        : RecurrentModule(input_size, hidden_size, sequence_length),
          m_layer(input_size, hidden_size, sequence_length, rng)
    {
    }

    Values Forward(const Values& input) const override
    {
        return m_layer.Forward(input);
    }

    RecurrentState ForwardState(
        const Values& input,
        const RecurrentState& initial_state) const override
    {
        if (initial_state.hidden.empty())
            return m_layer.ForwardState(input);

        return m_layer.ForwardState(input, initial_state.hidden);
    }

    Values Parameters() const override { return m_layer.Parameters(); }

  private:
    GRU m_layer;
};

std::unique_ptr<RecurrentModule> MakeRecurrentModule(
    RecurrentCellType cell_type, std::size_t input_size,
    std::size_t hidden_size, std::size_t sequence_length, std::mt19937& rng)
{
    switch (cell_type)
    {
    case RecurrentCellType::RNN:
        return std::make_unique<RNNModule>(input_size, hidden_size,
                                           sequence_length, rng);
    case RecurrentCellType::LSTM:
        return std::make_unique<LSTMModule>(input_size, hidden_size,
                                            sequence_length, rng);
    case RecurrentCellType::GRU:
        return std::make_unique<GRUModule>(input_size, hidden_size,
                                           sequence_length, rng);
    }

    return nullptr;
}

Values ProjectDecoderHidden(const Values& decoder_hidden,
                            const EncoderDecoder& encoder_decoder,
                            const Linear& projection,
                            std::size_t output_size)
{
    const std::size_t hidden_size = encoder_decoder.HiddenSize();
    const std::size_t decoder_length = encoder_decoder.DecoderLength();
    if (!IsInputValid(decoder_hidden, hidden_size * decoder_length))
        return {};

    Values output;
    output.reserve(decoder_length * output_size);
    for (std::size_t timestep = 0; timestep < decoder_length; ++timestep)
    {
        const Values hidden(decoder_hidden.begin() + timestep * hidden_size,
                            decoder_hidden.begin() +
                                (timestep + 1) * hidden_size);
        Values projected = projection.Forward(hidden);
        if (projected.empty())
            return {};

        output.insert(output.end(), projected.begin(), projected.end());
    }
    return output;
}

} // namespace

EncoderDecoder::EncoderDecoder(std::size_t encoder_input_size,
                               std::size_t decoder_input_size,
                               std::size_t hidden_size,
                               std::size_t encoder_length,
                               std::size_t decoder_length,
                               RecurrentCellType cell_type)
    : EncoderDecoder(encoder_input_size, decoder_input_size, hidden_size,
                     encoder_length, decoder_length, cell_type, DefaultRng())
{
}

EncoderDecoder::EncoderDecoder(std::size_t encoder_input_size,
                               std::size_t decoder_input_size,
                               std::size_t hidden_size,
                               std::size_t encoder_length,
                               std::size_t decoder_length,
                               RecurrentCellType cell_type, std::mt19937& rng)
    : m_encoder_input_size(encoder_input_size),
      m_decoder_input_size(decoder_input_size), m_hidden_size(hidden_size),
      m_encoder_length(encoder_length), m_decoder_length(decoder_length),
      m_cell_type(cell_type)
{
    if (m_encoder_input_size == 0 || m_decoder_input_size == 0 ||
        m_hidden_size == 0 || m_encoder_length == 0 || m_decoder_length == 0)
    {
        m_encoder_input_size = 0;
        m_decoder_input_size = 0;
        m_hidden_size = 0;
        m_encoder_length = 0;
        m_decoder_length = 0;
        return;
    }

    m_encoder = MakeRecurrentModule(m_cell_type, m_encoder_input_size,
                                    m_hidden_size, m_encoder_length, rng);
    m_decoder = MakeRecurrentModule(m_cell_type, m_decoder_input_size,
                                    m_hidden_size, m_decoder_length, rng);
}

EncoderDecoder::~EncoderDecoder() = default;

std::size_t EncoderDecoder::EncoderValueCount() const noexcept
{
    return m_encoder_input_size * m_encoder_length;
}

std::size_t EncoderDecoder::DecoderValueCount() const noexcept
{
    return m_decoder_input_size * m_decoder_length;
}

Values EncoderDecoder::Forward(const Values& input) const
{
    const std::size_t encoder_count = EncoderValueCount();
    const std::size_t decoder_count = DecoderValueCount();
    if (!IsInputValid(input, encoder_count + decoder_count))
        return {};

    const Values encoder_input(input.begin(), input.begin() + encoder_count);
    const Values decoder_input(input.begin() + encoder_count, input.end());
    return Forward(encoder_input, decoder_input);
}

Values EncoderDecoder::Forward(const Values& encoder_input,
                               const Values& decoder_input) const
{
    RecurrentState encoder_state = Encode(encoder_input);
    if (encoder_state.sequence.empty() || encoder_state.hidden.empty())
        return {};

    return Decode(decoder_input, encoder_state).sequence;
}

RecurrentState EncoderDecoder::Encode(const Values& encoder_input) const
{
    return m_encoder ? m_encoder->ForwardState(encoder_input)
                     : RecurrentState{};
}

RecurrentState EncoderDecoder::Decode(
    const Values& decoder_input, const RecurrentState& encoder_state) const
{
    return m_decoder ? m_decoder->ForwardState(decoder_input, encoder_state)
                     : RecurrentState{};
}

Values EncoderDecoder::Parameters() const
{
    Values parameters;
    if (m_encoder)
    {
        Values encoder_parameters = m_encoder->Parameters();
        parameters.insert(parameters.end(), encoder_parameters.begin(),
                          encoder_parameters.end());
    }
    if (m_decoder)
    {
        Values decoder_parameters = m_decoder->Parameters();
        parameters.insert(parameters.end(), decoder_parameters.begin(),
                          decoder_parameters.end());
    }

    return parameters;
}

Seq2Seq::Seq2Seq(std::size_t encoder_input_size,
                 std::size_t decoder_input_size, std::size_t hidden_size,
                 std::size_t output_size, std::size_t encoder_length,
                 std::size_t decoder_length, RecurrentCellType cell_type)
    : Seq2Seq(encoder_input_size, decoder_input_size, hidden_size, output_size,
              encoder_length, decoder_length, cell_type, DefaultRng())
{
}

Seq2Seq::Seq2Seq(std::size_t encoder_input_size,
                 std::size_t decoder_input_size, std::size_t hidden_size,
                 std::size_t output_size, std::size_t encoder_length,
                 std::size_t decoder_length, RecurrentCellType cell_type,
                 std::mt19937& rng)
    : m_output_size(output_size)
{
    if (encoder_input_size == 0 || decoder_input_size == 0 ||
        hidden_size == 0 || m_output_size == 0 || encoder_length == 0 ||
        decoder_length == 0)
    {
        m_output_size = 0;
        return;
    }

    m_encoder_decoder = std::make_unique<EncoderDecoder>(
        encoder_input_size, decoder_input_size, hidden_size, encoder_length,
        decoder_length, cell_type, rng);
    m_projection = std::make_unique<Linear>(hidden_size, m_output_size, rng);
}

Values Seq2Seq::Forward(const Values& input) const
{
    if (!m_encoder_decoder || !m_projection)
        return {};

    const Values decoder_hidden = m_encoder_decoder->Forward(input);
    return ProjectDecoderHidden(decoder_hidden, *m_encoder_decoder,
                                *m_projection, m_output_size);
}

Values Seq2Seq::Forward(const Values& encoder_input,
                        const Values& decoder_input) const
{
    if (!m_encoder_decoder || !m_projection)
        return {};

    const Values decoder_hidden =
        m_encoder_decoder->Forward(encoder_input, decoder_input);
    return ProjectDecoderHidden(decoder_hidden, *m_encoder_decoder,
                                *m_projection, m_output_size);
}

Values Seq2Seq::Parameters() const
{
    Values parameters;
    if (m_encoder_decoder)
    {
        Values encoder_decoder_parameters = m_encoder_decoder->Parameters();
        parameters.insert(parameters.end(), encoder_decoder_parameters.begin(),
                          encoder_decoder_parameters.end());
    }
    if (m_projection)
    {
        Values projection_parameters = m_projection->Parameters();
        parameters.insert(parameters.end(), projection_parameters.begin(),
                          projection_parameters.end());
    }
    return parameters;
}

AttentionSeq2Seq::AttentionSeq2Seq(
    std::size_t encoder_input_size, std::size_t decoder_input_size,
    std::size_t hidden_size, std::size_t output_size,
    std::size_t encoder_length, std::size_t decoder_length,
    RecurrentCellType cell_type, std::size_t head_count)
    : AttentionSeq2Seq(encoder_input_size, decoder_input_size, hidden_size,
                       output_size, encoder_length, decoder_length, cell_type,
                       DefaultRng(), head_count)
{
}

AttentionSeq2Seq::AttentionSeq2Seq(
    std::size_t encoder_input_size, std::size_t decoder_input_size,
    std::size_t hidden_size, std::size_t output_size,
    std::size_t encoder_length, std::size_t decoder_length,
    RecurrentCellType cell_type, std::mt19937& rng, std::size_t head_count)
    : m_encoder_input_size(encoder_input_size),
      m_decoder_input_size(decoder_input_size),
      m_encoder_length(encoder_length), m_decoder_length(decoder_length),
      m_output_size(output_size), m_head_count(head_count)
{
    if (m_encoder_input_size == 0 || m_decoder_input_size == 0 ||
        hidden_size == 0 || m_output_size == 0 || m_encoder_length == 0 ||
        m_decoder_length == 0 || m_head_count == 0 ||
        hidden_size % m_head_count != 0)
    {
        m_encoder_input_size = 0;
        m_decoder_input_size = 0;
        m_encoder_length = 0;
        m_decoder_length = 0;
        m_output_size = 0;
        m_head_count = 0;
        return;
    }

    m_encoder_decoder = std::make_unique<EncoderDecoder>(
        m_encoder_input_size, m_decoder_input_size, hidden_size,
        m_encoder_length, m_decoder_length, cell_type, rng);
    m_attention = std::make_unique<MultiHeadAttention>(
        hidden_size, m_head_count, m_decoder_length, m_encoder_length, rng);
    m_projection = std::make_unique<Linear>(hidden_size, m_output_size, rng);
}

AttentionSeq2Seq::~AttentionSeq2Seq() = default;

std::size_t AttentionSeq2Seq::EncoderValueCount() const noexcept
{
    return m_encoder_input_size * m_encoder_length;
}

std::size_t AttentionSeq2Seq::DecoderValueCount() const noexcept
{
    return m_decoder_input_size * m_decoder_length;
}

Values AttentionSeq2Seq::Forward(const Values& input) const
{
    const std::size_t encoder_count = EncoderValueCount();
    const std::size_t decoder_count = DecoderValueCount();
    if (!IsInputValid(input, encoder_count + decoder_count))
        return {};

    const Values encoder_input(input.begin(), input.begin() + encoder_count);
    const Values decoder_input(input.begin() + encoder_count, input.end());
    return Forward(encoder_input, decoder_input);
}

Values AttentionSeq2Seq::Forward(const Values& encoder_input,
                                 const Values& decoder_input) const
{
    if (!m_encoder_decoder || !m_attention || !m_projection)
        return {};

    RecurrentState encoder_state = m_encoder_decoder->Encode(encoder_input);
    if (encoder_state.sequence.empty() || encoder_state.hidden.empty())
        return {};

    RecurrentState decoder_state =
        m_encoder_decoder->Decode(decoder_input, encoder_state);
    if (decoder_state.sequence.empty() || decoder_state.hidden.empty())
        return {};

    const Values context = m_attention->Forward(
        decoder_state.sequence, encoder_state.sequence, encoder_state.sequence);
    return ProjectDecoderHidden(context, *m_encoder_decoder, *m_projection,
                                m_output_size);
}

Values AttentionSeq2Seq::Parameters() const
{
    Values parameters;
    if (m_encoder_decoder)
    {
        Values encoder_decoder_parameters = m_encoder_decoder->Parameters();
        parameters.insert(parameters.end(), encoder_decoder_parameters.begin(),
                          encoder_decoder_parameters.end());
    }
    if (m_attention)
    {
        Values attention_parameters = m_attention->Parameters();
        parameters.insert(parameters.end(), attention_parameters.begin(),
                          attention_parameters.end());
    }
    if (m_projection)
    {
        Values projection_parameters = m_projection->Parameters();
        parameters.insert(parameters.end(), projection_parameters.begin(),
                          projection_parameters.end());
    }
    return parameters;
}

void AttentionSeq2Seq::Train(bool training)
{
    Module::Train(training);
    if (m_encoder_decoder)
        m_encoder_decoder->Train(training);
    if (m_attention)
        m_attention->Train(training);
    if (m_projection)
        m_projection->Train(training);
}

} // namespace forg::nn
