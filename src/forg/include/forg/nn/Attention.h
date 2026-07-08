/*******************************************************************************
    This source file is part of FORG library.

    Scalar attention and transformer modules.
*******************************************************************************/

#pragma once
#include "forg/nn/Layers.h"
#include "forg/nn/Normalization.h"

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

namespace forg::nn {

enum class AttentionMask
{
    None,
    Causal,
};

/// Scaled dot-product attention over flat time-major scalar sequences.
///
/// Query and key feature sizes are both key_size. Forward(query, key, value)
/// returns query_length * value_size Values.
class ScaledDotProductAttention : public Module
{
  public:
    ScaledDotProductAttention(std::size_t key_size, std::size_t value_size,
                              std::size_t query_length,
                              std::size_t key_length,
                              AttentionMask mask = AttentionMask::None);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& query, const Values& key,
                   const Values& value) const;

    std::size_t KeySize() const noexcept { return m_key_size; }
    std::size_t ValueSize() const noexcept { return m_value_size; }
    std::size_t QueryLength() const noexcept { return m_query_length; }
    std::size_t KeyLength() const noexcept { return m_key_length; }
    AttentionMask Mask() const noexcept { return m_mask; }

  private:
    std::size_t m_key_size = 0;
    std::size_t m_value_size = 0;
    std::size_t m_query_length = 0;
    std::size_t m_key_length = 0;
    AttentionMask m_mask = AttentionMask::None;
};

/// Trainable multi-head attention using Linear Q/K/V/O projections.
class MultiHeadAttention : public Module
{
  public:
    MultiHeadAttention(std::size_t model_size, std::size_t head_count,
                       std::size_t sequence_length,
                       bool causal = false);
    MultiHeadAttention(std::size_t model_size, std::size_t head_count,
                       std::size_t sequence_length, std::mt19937& rng,
                       bool causal = false);
    MultiHeadAttention(std::size_t model_size, std::size_t head_count,
                       std::size_t query_length, std::size_t key_length,
                       bool causal = false);
    MultiHeadAttention(std::size_t model_size, std::size_t head_count,
                       std::size_t query_length, std::size_t key_length,
                       std::mt19937& rng, bool causal = false);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& query, const Values& key,
                   const Values& value) const;
    Values Parameters() const override;
    void Train(bool training = true) override;

    std::size_t ModelSize() const noexcept { return m_model_size; }
    std::size_t HeadCount() const noexcept { return m_head_count; }
    std::size_t HeadSize() const noexcept { return m_head_size; }
    std::size_t QueryLength() const noexcept { return m_query_length; }
    std::size_t KeyLength() const noexcept { return m_key_length; }
    bool Causal() const noexcept { return m_causal; }

  private:
    std::size_t m_model_size = 0;
    std::size_t m_head_count = 0;
    std::size_t m_head_size = 0;
    std::size_t m_query_length = 0;
    std::size_t m_key_length = 0;
    bool m_causal = false;
    std::unique_ptr<Linear> m_query_projection;
    std::unique_ptr<Linear> m_key_projection;
    std::unique_ptr<Linear> m_value_projection;
    std::unique_ptr<Linear> m_output_projection;
};

class TransformerEncoderBlock : public Module
{
  public:
    TransformerEncoderBlock(std::size_t model_size, std::size_t head_count,
                            std::size_t sequence_length,
                            std::size_t feed_forward_size);
    TransformerEncoderBlock(std::size_t model_size, std::size_t head_count,
                            std::size_t sequence_length,
                            std::size_t feed_forward_size,
                            std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;
    void Train(bool training = true) override;

    std::size_t ModelSize() const noexcept { return m_model_size; }
    std::size_t SequenceLength() const noexcept { return m_sequence_length; }
    std::size_t FeedForwardSize() const noexcept
    {
        return m_feed_forward_size;
    }

  private:
    std::size_t m_model_size = 0;
    std::size_t m_sequence_length = 0;
    std::size_t m_feed_forward_size = 0;
    std::unique_ptr<MultiHeadAttention> m_attention;
    std::unique_ptr<LayerNorm> m_norm1;
    std::unique_ptr<LayerNorm> m_norm2;
    std::unique_ptr<Linear> m_feed_forward1;
    std::unique_ptr<Linear> m_feed_forward2;
};

class TransformerDecoderBlock : public Module
{
  public:
    TransformerDecoderBlock(std::size_t model_size, std::size_t head_count,
                            std::size_t decoder_length,
                            std::size_t encoder_length,
                            std::size_t feed_forward_size);
    TransformerDecoderBlock(std::size_t model_size, std::size_t head_count,
                            std::size_t decoder_length,
                            std::size_t encoder_length,
                            std::size_t feed_forward_size,
                            std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& decoder_input,
                   const Values& encoder_memory) const;
    Values Parameters() const override;
    void Train(bool training = true) override;

    std::size_t ModelSize() const noexcept { return m_model_size; }
    std::size_t DecoderLength() const noexcept { return m_decoder_length; }
    std::size_t EncoderLength() const noexcept { return m_encoder_length; }

  private:
    std::size_t m_model_size = 0;
    std::size_t m_decoder_length = 0;
    std::size_t m_encoder_length = 0;
    std::size_t m_feed_forward_size = 0;
    std::unique_ptr<MultiHeadAttention> m_self_attention;
    std::unique_ptr<MultiHeadAttention> m_cross_attention;
    std::unique_ptr<LayerNorm> m_norm1;
    std::unique_ptr<LayerNorm> m_norm2;
    std::unique_ptr<LayerNorm> m_norm3;
    std::unique_ptr<Linear> m_feed_forward1;
    std::unique_ptr<Linear> m_feed_forward2;
};

class TransformerEncoder : public Module
{
  public:
    TransformerEncoder(std::size_t layer_count, std::size_t model_size,
                       std::size_t head_count, std::size_t sequence_length,
                       std::size_t feed_forward_size);
    TransformerEncoder(std::size_t layer_count, std::size_t model_size,
                       std::size_t head_count, std::size_t sequence_length,
                       std::size_t feed_forward_size, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Parameters() const override;
    void Train(bool training = true) override;

    std::size_t LayerCount() const noexcept { return m_layers.size(); }

  private:
    std::vector<std::unique_ptr<TransformerEncoderBlock>> m_layers;
};

class TransformerDecoder : public Module
{
  public:
    TransformerDecoder(std::size_t layer_count, std::size_t model_size,
                       std::size_t head_count, std::size_t decoder_length,
                       std::size_t encoder_length,
                       std::size_t feed_forward_size);
    TransformerDecoder(std::size_t layer_count, std::size_t model_size,
                       std::size_t head_count, std::size_t decoder_length,
                       std::size_t encoder_length,
                       std::size_t feed_forward_size, std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const Values& decoder_input,
                   const Values& encoder_memory) const;
    Values Parameters() const override;
    void Train(bool training = true) override;

    std::size_t LayerCount() const noexcept { return m_layers.size(); }

  private:
    std::size_t m_model_size = 0;
    std::size_t m_decoder_length = 0;
    std::size_t m_encoder_length = 0;
    std::vector<std::unique_ptr<TransformerDecoderBlock>> m_layers;
};

} // namespace forg::nn
