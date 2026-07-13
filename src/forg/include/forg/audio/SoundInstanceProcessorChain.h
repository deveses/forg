// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/SoundInstanceProcessor.h"

#include <memory>
#include <utility>
#include <vector>

namespace forg::audio {

class FORG_API SoundInstanceProcessorChain
{
  public:
    bool AddProcessor(std::shared_ptr<SoundInstanceProcessor> processor);
    [[nodiscard]] std::size_t Count() const noexcept;

    [[nodiscard]] SoundInstanceProcessor* Processor(std::size_t index) noexcept;
    [[nodiscard]] const SoundInstanceProcessor*
    Processor(std::size_t index) const noexcept;

    [[nodiscard]] std::shared_ptr<SoundInstanceProcessor>
    ProcessorPtr(std::size_t index) const;

  private:
    std::vector<std::shared_ptr<SoundInstanceProcessor>> m_processors;
};

} // namespace forg::audio
