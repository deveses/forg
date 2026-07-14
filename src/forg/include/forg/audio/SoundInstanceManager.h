// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/core/ObjectPool.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace forg::audio {

class SoundInstanceProcessorChain;

class FORG_API SoundInstanceManager
{
  public:
    explicit SoundInstanceManager(std::size_t capacity);
    ~SoundInstanceManager();

    SoundInstanceManager(const SoundInstanceManager&) = delete;
    SoundInstanceManager& operator=(const SoundInstanceManager&) = delete;

    ProcessedSoundInstance*
    Create(std::shared_ptr<SoundInstanceProcessorChain> processorChain,
           SoundInstanceDescription description = {});
    bool Destroy(ProcessedSoundInstance* instance);

    ProcessedSoundInstance* Find(SoundInstanceId id) noexcept;
    const ProcessedSoundInstance* Find(SoundInstanceId id) const noexcept;

    void Update();
    bool Stop(SoundInstanceId id);
    bool StopAll();
    void Clear();

    std::size_t Capacity() const noexcept;
    std::size_t Size() const noexcept;

  private:
    SoundInstanceId NextInstanceId() noexcept;

    core::ObjectPool<ProcessedSoundInstance> m_pool;
    std::vector<ProcessedSoundInstance*> m_instances;
    SoundInstanceId m_nextInstanceId = 1;
};

} // namespace forg::audio
