// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/SoundInstanceProcessor.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace forg::audio {

class SoundInstanceProcessorChain;

using SoundInstanceId = std::uint64_t;
inline constexpr SoundInstanceId INVALID_SOUND_INSTANCE_ID = 0;

class FORG_API ProcessedSoundInstance
{
  public:
    explicit ProcessedSoundInstance(
        std::shared_ptr<SoundInstanceProcessorChain> chain);
    virtual ~ProcessedSoundInstance();

    ProcessedSoundInstance(const ProcessedSoundInstance&) = delete;
    ProcessedSoundInstance& operator=(const ProcessedSoundInstance&) = delete;

    SoundInstanceState State() const noexcept;
    SoundInstanceId Id() const noexcept;
    bool IsActive() const noexcept;
    bool IsPending() const noexcept;
    bool IsComplete() const noexcept;
    bool IsFailed() const noexcept;
    bool IsStopped() const noexcept;

    bool HasPendingWork() const noexcept;
    SoundInstanceLifecyclePhase PendingPhase() const noexcept;
    std::size_t PendingProcessorIndex() const noexcept;

    SoundProcessingParameters& Parameters() noexcept;
    const SoundProcessingParameters& Parameters() const noexcept;

    std::shared_ptr<SoundInstanceProcessorChain> ProcessorChain() const;
    std::size_t ProcessorContextCount() const noexcept;
    SoundProcessorContext* ProcessorContext(std::size_t index) noexcept;
    const SoundProcessorContext*
    ProcessorContext(std::size_t index) const noexcept;

    void* SenderObject() const;
    void* EmitterObject() const;

    void SetSenderObject(void* sender) noexcept;
    void SetEmitterObject(void* emitter) noexcept;

    bool Create(SoundInstanceId id = INVALID_SOUND_INSTANCE_ID);
    bool Play();
    void Update();
    void Stop();
    void Pause();
    void Resume();
    void SetVolumeMultiplier(float volumeMultiplier);

    void RequestStop();
    void RequestPause();
    void RequestResume();
    void RequestVolumeMultiplier(float volumeMultiplier);

  protected:
    virtual void* ResolveSenderObject() const;
    virtual void* ResolveEmitterObject() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    SoundProcessingResult CallProcessor(SoundInstanceLifecyclePhase phase,
                                        SoundInstanceProcessor& processor,
                                        SoundProcessorContext& context);
    SoundProcessingResult RunPhase(SoundInstanceLifecyclePhase phase,
                                   std::size_t startIndex);
    bool ResumePendingWork();
    void TransitionTo(SoundInstanceState state);
    void DestroyProcessors();
};

} // namespace forg::audio
