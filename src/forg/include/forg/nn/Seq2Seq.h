/*******************************************************************************
    This source file is part of FORG library.

    Recurrent encoder-decoder and sequence-to-sequence scalar modules.
*******************************************************************************/

#pragma once
#include "forg/nn/Layers.h"

#include <cstddef>
#include <memory>
#include <random>

namespace forg::nn {

class RecurrentModule;

enum class RecurrentCellType
{
    RNN,
    LSTM,
    GRU,
};

/// Teacher-forced recurrent encoder-decoder over flat time-major input.
class EncoderDecoder : public Module
{
  public:
    EncoderDecoder(std::size_t encoder_input_size,
                   std::size_t decoder_input_size, std::size_t hidden_size,
                   std::size_t encoder_length, std::size_t decoder_length,
                   RecurrentCellType cell_type);
    EncoderDecoder(std::size_t encoder_input_size,
                   std::size_t decoder_input_size, std::size_t hidden_size,
                   std::size_t encoder_length, std::size_t decoder_length,
                   RecurrentCellType cell_type, std::mt19937& rng);
    ~EncoderDecoder() override;

    Values Forward(const Values& input) const override;
    Values Forward(const Values& encoder_input,
                   const Values& decoder_input) const;
    RecurrentState Encode(const Values& encoder_input) const;
    RecurrentState Decode(const Values& decoder_input,
                          const RecurrentState& encoder_state) const;
    Values Parameters() const override;

    std::size_t EncoderInputSize() const noexcept
    {
        return m_encoder_input_size;
    }
    std::size_t DecoderInputSize() const noexcept
    {
        return m_decoder_input_size;
    }
    std::size_t HiddenSize() const noexcept { return m_hidden_size; }
    std::size_t EncoderLength() const noexcept { return m_encoder_length; }
    std::size_t DecoderLength() const noexcept { return m_decoder_length; }
    RecurrentCellType CellType() const noexcept { return m_cell_type; }

  private:
    std::size_t EncoderValueCount() const noexcept;
    std::size_t DecoderValueCount() const noexcept;

    std::size_t m_encoder_input_size = 0;
    std::size_t m_decoder_input_size = 0;
    std::size_t m_hidden_size = 0;
    std::size_t m_encoder_length = 0;
    std::size_t m_decoder_length = 0;
    RecurrentCellType m_cell_type = RecurrentCellType::RNN;
    std::unique_ptr<RecurrentModule> m_encoder;
    std::unique_ptr<RecurrentModule> m_decoder;
};

/// Encoder-decoder with a shared per-timestep output projection.
class Seq2Seq : public Module
{
  public:
    Seq2Seq(std::size_t encoder_input_size, std::size_t decoder_input_size,
            std::size_t hidden_size, std::size_t output_size,
            std::size_t encoder_length, std::size_t decoder_length,
            RecurrentCellType cell_type);
    Seq2Seq(std::size_t encoder_input_size, std::size_t decoder_input_size,
            std::size_t hidden_size, std::size_t output_size,
            std::size_t encoder_length, std::size_t decoder_length,
            RecurrentCellType cell_type, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& encoder_input,
                   const Values& decoder_input) const;
    Values Parameters() const override;

    const EncoderDecoder* InnerEncoderDecoder() const noexcept
    {
        return m_encoder_decoder.get();
    }
    const Linear* Projection() const noexcept { return m_projection.get(); }
    std::size_t OutputSize() const noexcept { return m_output_size; }

  private:
    std::size_t m_output_size = 0;
    std::unique_ptr<EncoderDecoder> m_encoder_decoder;
    std::unique_ptr<Linear> m_projection;
};

} // namespace forg::nn
