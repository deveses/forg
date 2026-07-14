// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/SoundInstanceProcessor.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

namespace forg::audio {

class IAudioSource;
class SoundInstanceProcessorChain;

using SoundInstanceId = std::uint64_t;
inline constexpr SoundInstanceId INVALID_SOUND_INSTANCE_ID = 0;

struct SoundInstanceDescription
{
    std::shared_ptr<IAudioSource> source;
    bool looping = false;
    SoundProcessingParameters parameters;
};

class FORG_API ProcessedSoundInstance
{
  public:
    static constexpr std::size_t MAX_QUEUED_COMMANDS = 16;

    explicit ProcessedSoundInstance(
        std::shared_ptr<SoundInstanceProcessorChain> chain,
        SoundInstanceDescription description = {});
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
    std::shared_ptr<IAudioSource> Source() const;
    bool Looping() const noexcept;

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

    bool RequestStop();
    bool RequestPause();
    bool RequestResume();
    bool RequestVolumeMultiplier(float volumeMultiplier);

  protected:
    virtual void* ResolveSenderObject() const;
    virtual void* ResolveEmitterObject() const;

  private:
    enum class CommandType
    {
        Stop,
        Pause,
        Resume,
        Volume
    };

    struct Command
    {
        CommandType type = CommandType::Stop;
        float volumeMultiplier = 1.0f;
    };

    std::shared_ptr<SoundInstanceProcessorChain> m_chain;
    std::shared_ptr<IAudioSource> m_source;
    bool m_looping = false;
    std::array<SoundProcessorContext, MAX_SOUND_INSTANCE_PROCESSORS> m_contexts;
    std::size_t m_contextCount = 0;
    SoundProcessingParameters m_parameters;

    SoundInstanceId m_id = INVALID_SOUND_INSTANCE_ID;
    SoundInstanceState m_state = SoundInstanceState::Pending;
    SoundInstanceLifecyclePhase m_pendingPhase =
        SoundInstanceLifecyclePhase::Create;
    std::size_t m_pendingIndex = 0;
    bool m_hasPending = false;
    bool m_creationStarted = false;
    bool m_created = false;
    bool m_destroyed = false;

    void* m_sender = nullptr;
    void* m_emitter = nullptr;

    mutable std::mutex m_commandMutex;
    std::array<Command, MAX_QUEUED_COMMANDS> m_commands;
    std::size_t m_commandHead = 0;
    std::size_t m_commandCount = 0;

    bool Queue(Command command);
    std::size_t
    DrainCommands(std::array<Command, MAX_QUEUED_COMMANDS>& commands);

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
