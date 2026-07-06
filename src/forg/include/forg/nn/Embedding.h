/*******************************************************************************
    This source file is part of FORG library.

    Trainable scalar embedding tables.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <cstddef>
#include <random>
#include <vector>

namespace forg::nn {

/// Trainable lookup table for integer token IDs.
///
/// Forward(indices) returns the selected embedding rows flattened in input
/// order. The Module override reads integer IDs from Value::GetData(); token
/// indices themselves are not differentiable, but gradients flow to the
/// selected embedding weights.
class Embedding : public Module
{
  public:
    Embedding(std::size_t num_embeddings, std::size_t embedding_dim);
    Embedding(std::size_t num_embeddings, std::size_t embedding_dim,
              std::mt19937& rng);

    Values Forward(const Values& input) const override;
    Values Forward(const std::vector<std::size_t>& indices) const;
    Values Parameters() const override;

    std::size_t NumEmbeddings() const noexcept { return m_num_embeddings; }
    std::size_t EmbeddingDim() const noexcept { return m_embedding_dim; }
    const Values& Weights() const noexcept { return m_weights; }

  private:
    std::size_t WeightIndex(std::size_t index,
                            std::size_t dimension) const noexcept;

    std::size_t m_num_embeddings = 0;
    std::size_t m_embedding_dim = 0;
    Values m_weights;
};

} // namespace forg::nn
