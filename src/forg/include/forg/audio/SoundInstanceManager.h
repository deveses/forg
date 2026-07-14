// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/core/ObjectPool.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace forg::audio {

class SoundInstanceProcessorChain;

class FORG_API SoundInstanceManager
{
  public:
    static constexpr std::size_t MAX_QUEUED_COMMANDS = 64;

    SoundInstanceManager(
        std::size_t capacity,
        std::shared_ptr<SoundInstanceProcessorChain> processorChain);
    ~SoundInstanceManager();

    SoundInstanceManager(const SoundInstanceManager&) = delete;
    SoundInstanceManager& operator=(const SoundInstanceManager&) = delete;

    // Play, Update, and Clear are owner-thread operations.
    SoundInstanceId Play(SoundInstanceDescription description = {});
    void Update();
    // These methods do not access pooled instance pointers and may be called
    // by other threads while this manager remains alive.
    bool Stop(SoundInstanceId id);
    bool StopAll();
    bool SetGainPan(SoundInstanceId id, float gain, float pan);
    bool IsPlaying(SoundInstanceId id) const noexcept;
    // Immediately destroys all instances; used by owner-thread shutdown.
    void Clear();

    std::size_t Capacity() const noexcept;
    std::size_t Size() const noexcept;

  private:
    enum class CommandType
    {
        Stop,
        StopAll,
        GainPan
    };

    struct Command
    {
        CommandType type = CommandType::Stop;
        SoundInstanceId id = INVALID_SOUND_INSTANCE_ID;
        float gain = 1.0f;
        float pan = 0.0f;
    };

    struct InstanceEntry
    {
        ProcessedSoundInstance* instance = nullptr;
        std::size_t snapshotIndex = 0;
    };

    struct InstanceSnapshot
    {
        std::atomic<SoundInstanceId> id{INVALID_SOUND_INSTANCE_ID};
        std::atomic<SoundInstanceState> state{SoundInstanceState::Stopped};
    };

    ProcessedSoundInstance* Create(SoundInstanceDescription description);
    ProcessedSoundInstance* Find(SoundInstanceId id) noexcept;

    bool Queue(Command command);
    std::size_t
    DrainCommands(std::array<Command, MAX_QUEUED_COMMANDS>& commands);
    void ProcessCommands();

    std::size_t AcquireSnapshotIndex() const noexcept;
    void PublishSnapshot(std::size_t index, SoundInstanceId id,
                         SoundInstanceState state) noexcept;
    void UnpublishSnapshot(std::size_t index) noexcept;

    SoundInstanceId NextInstanceId() noexcept;

    core::ObjectPool<ProcessedSoundInstance> m_pool;
    std::shared_ptr<SoundInstanceProcessorChain> m_processorChain;
    std::vector<InstanceEntry> m_instances;
    std::unique_ptr<InstanceSnapshot[]> m_snapshots;
    std::size_t m_capacity = 0;

    mutable std::mutex m_commandMutex;
    std::array<Command, MAX_QUEUED_COMMANDS> m_commands;
    std::size_t m_commandHead = 0;
    std::size_t m_commandCount = 0;

    SoundInstanceId m_nextInstanceId = 1;
};

} // namespace forg::audio
