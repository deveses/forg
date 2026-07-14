// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/SoundInstanceProcessor.h"

#include <array>
#include <memory>
#include <utility>

namespace forg::audio {

class FORG_API SoundInstanceProcessorChain
{
  public:
    static constexpr std::size_t MAX_PROCESSORS = MAX_SOUND_INSTANCE_PROCESSORS;

    bool AddProcessor(std::shared_ptr<SoundInstanceProcessor> processor);
    [[nodiscard]] std::size_t Count() const noexcept;

    [[nodiscard]] SoundInstanceProcessor* Processor(std::size_t index) noexcept;
    [[nodiscard]] const SoundInstanceProcessor*
    Processor(std::size_t index) const noexcept;

    [[nodiscard]] std::shared_ptr<SoundInstanceProcessor>
    ProcessorPtr(std::size_t index) const;

  private:
    std::array<std::shared_ptr<SoundInstanceProcessor>, MAX_PROCESSORS>
        m_processors;
    std::size_t m_count = 0;
};

} // namespace forg::audio
