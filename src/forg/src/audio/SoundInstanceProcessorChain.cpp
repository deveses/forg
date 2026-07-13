// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/SoundInstanceProcessorChain.h"

namespace forg::audio {

bool SoundInstanceProcessorChain::AddProcessor(
    std::shared_ptr<SoundInstanceProcessor> processor)
{
    if (processor == nullptr)
        return false;

    m_processors.push_back(std::move(processor));
    return true;
}

std::size_t SoundInstanceProcessorChain::Count() const noexcept
{
    return m_processors.size();
}

SoundInstanceProcessor*
SoundInstanceProcessorChain::Processor(std::size_t index) noexcept
{
    if (index >= m_processors.size())
        return nullptr;

    return m_processors[index].get();
}

const SoundInstanceProcessor*
SoundInstanceProcessorChain::Processor(std::size_t index) const noexcept
{
    if (index >= m_processors.size())
        return nullptr;

    return m_processors[index].get();
}

std::shared_ptr<SoundInstanceProcessor>
SoundInstanceProcessorChain::ProcessorPtr(std::size_t index) const
{
    if (index >= m_processors.size())
        return nullptr;

    return m_processors[index];
}

} // namespace forg::audio
