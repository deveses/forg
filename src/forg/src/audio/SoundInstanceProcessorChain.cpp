// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/SoundInstanceProcessorChain.h"

namespace forg::audio {

bool SoundInstanceProcessorChain::AddProcessor(
    std::shared_ptr<SoundInstanceProcessor> processor)
{
    if (processor == nullptr || m_count == m_processors.size())
        return false;

    m_processors[m_count++] = std::move(processor);
    return true;
}

std::size_t SoundInstanceProcessorChain::Count() const noexcept
{
    return m_count;
}

SoundInstanceProcessor*
SoundInstanceProcessorChain::Processor(std::size_t index) noexcept
{
    if (index >= m_count)
        return nullptr;

    return m_processors[index].get();
}

const SoundInstanceProcessor*
SoundInstanceProcessorChain::Processor(std::size_t index) const noexcept
{
    if (index >= m_count)
        return nullptr;

    return m_processors[index].get();
}

std::shared_ptr<SoundInstanceProcessor>
SoundInstanceProcessorChain::ProcessorPtr(std::size_t index) const
{
    if (index >= m_count)
        return nullptr;

    return m_processors[index];
}

} // namespace forg::audio
