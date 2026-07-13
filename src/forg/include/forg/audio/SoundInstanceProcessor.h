// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/core/ObjectBuffer.h"
#include "forg/core/string.hpp"
#include "forg/math/Vector3.h"

#include <cstdint>
#include <utility>

namespace forg::audio {

class ProcessedSoundInstance;

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
    core::ObjectBuffer storage;
    void* userData = nullptr;
    std::uintptr_t handle = 0;
    bool bypassed = false;
    core::string message;

    SoundProcessorContext() = default;
    ~SoundProcessorContext() = default;

    SoundProcessorContext(const SoundProcessorContext&) = delete;
    SoundProcessorContext& operator=(const SoundProcessorContext&) = delete;

    SoundProcessorContext(SoundProcessorContext&&) noexcept = default;
    SoundProcessorContext&
    operator=(SoundProcessorContext&&) noexcept = default;

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
