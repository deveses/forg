// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/core/ObjectBuffer.h"
#include "forg/math/Vector3.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace forg::audio {

class ProcessedSoundInstance;

inline constexpr std::size_t MAX_SOUND_INSTANCE_PROCESSORS = 16;
inline constexpr std::size_t SOUND_PROCESSOR_CONTEXT_STORAGE_SIZE = 64;
inline constexpr std::size_t SOUND_PROCESSOR_CONTEXT_STORAGE_ALIGNMENT =
    alignof(std::max_align_t);
inline constexpr std::size_t SOUND_PROCESSOR_CONTEXT_MESSAGE_SIZE = 128;

enum class SoundInstanceState
{
    Pending,
    Playing,
    Paused,
    Complete,
    Failed,
    Stopped
};

enum class SoundProcessingResult
{
    Continue,
    Stop,
    Fail,
    Wait,
    Complete,
    Bypass
};

enum class SoundInstanceLifecyclePhase
{
    Create,
    Play,
    Update,
    Stop,
    Pause,
    Resume,
    Volume,
    Destroy
};

struct SoundProcessingParameters
{
    float volumeMultiplier = 1.0f;
    float pan = 0.0f;
    bool volumeChangesEnabled = true;
    bool positioned = false;
    bool spatialized = false;
    bool hasListener = false;
    math::Vector3 listenerPosition;
    math::Vector3 listenerRight = math::Vector3(1.0f, 0.0f, 0.0f);
};

struct SoundProcessorContext
{
    core::StaticObjectBuffer<SOUND_PROCESSOR_CONTEXT_STORAGE_SIZE,
                             SOUND_PROCESSOR_CONTEXT_STORAGE_ALIGNMENT>
        storage;
    void* userData = nullptr;
    std::uintptr_t handle = 0;
    bool bypassed = false;
    std::array<char, SOUND_PROCESSOR_CONTEXT_MESSAGE_SIZE> message{};

    SoundProcessorContext() = default;
    ~SoundProcessorContext() = default;

    SoundProcessorContext(const SoundProcessorContext&) = delete;
    SoundProcessorContext& operator=(const SoundProcessorContext&) = delete;

    SoundProcessorContext(SoundProcessorContext&&) = delete;
    SoundProcessorContext& operator=(SoundProcessorContext&&) = delete;

    template <typename T, typename... Args> T* Emplace(Args&&... args)
    {
        return storage.Emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T> T* Get() noexcept { return storage.Get<T>(); }

    template <typename T> const T* Get() const noexcept
    {
        return storage.Get<T>();
    }
};

class FORG_API SoundInstanceProcessor
{
  public:
    virtual ~SoundInstanceProcessor() = 0;

    virtual SoundProcessingResult OnCreate(ProcessedSoundInstance& instance,
                                           SoundProcessorContext& context);
    virtual SoundProcessingResult OnPlay(ProcessedSoundInstance& instance,
                                         SoundProcessorContext& context);
    virtual SoundProcessingResult OnUpdate(ProcessedSoundInstance& instance,
                                           SoundProcessorContext& context);
    virtual SoundProcessingResult OnStop(ProcessedSoundInstance& instance,
                                         SoundProcessorContext& context);
    virtual SoundProcessingResult OnPause(ProcessedSoundInstance& instance,
                                          SoundProcessorContext& context);
    virtual SoundProcessingResult OnResume(ProcessedSoundInstance& instance,
                                           SoundProcessorContext& context);
    virtual SoundProcessingResult
    OnVolumeChanged(ProcessedSoundInstance& instance,
                    SoundProcessorContext& context);
    virtual void OnDestroy(ProcessedSoundInstance& instance,
                           SoundProcessorContext& context);
};

} // namespace forg::audio
