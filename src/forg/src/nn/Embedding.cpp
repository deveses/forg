#include "forg/nn/Embedding.h"

#include "ModuleUtils.h"

#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

namespace forg::nn {
using namespace detail;

Embedding::Embedding(std::size_t num_embeddings, std::size_t embedding_dim)
    : Embedding(num_embeddings, embedding_dim, DefaultRng())
{
}

Embedding::Embedding(std::size_t num_embeddings, std::size_t embedding_dim,
                     std::mt19937& rng)
    : m_num_embeddings(num_embeddings), m_embedding_dim(embedding_dim)
{
    if (m_num_embeddings == 0 || m_embedding_dim == 0)
    {
        m_num_embeddings = 0;
        m_embedding_dim = 0;
        return;
    }

    m_weights = MakeWeights(m_num_embeddings * m_embedding_dim, rng);
}

std::size_t Embedding::WeightIndex(std::size_t index,
                                   std::size_t dimension) const noexcept
{
    return index * m_embedding_dim + dimension;
}

Values Embedding::Forward(const Values& input) const
{
    if (m_weights.empty())
        return {};

    std::vector<std::size_t> indices;
    indices.reserve(input.size());

    // Convert ValuePtr input to integer indices, validating each one.
    for (const ValuePtr& value : input)
    {
        std::size_t index = 0;
        if (!TryValueIndex(value, m_num_embeddings, index))
            return {};
        indices.push_back(index);
    }

    return Forward(indices);
}

Values Embedding::Forward(const std::vector<std::size_t>& indices) const
{
    if (m_weights.empty())
        return {};

    Values output;
    output.reserve(indices.size() * m_embedding_dim);

    // Select embedding rows in input order, flattening each row.
    for (const std::size_t index : indices)
    {
        if (index >= m_num_embeddings)
            return {};

        for (std::size_t dimension = 0; dimension < m_embedding_dim;
             ++dimension)
        {
            output.push_back(m_weights[WeightIndex(index, dimension)]);
        }
    }
    return output;
}

Values Embedding::Parameters() const { return m_weights; }

} // namespace forg::nn
