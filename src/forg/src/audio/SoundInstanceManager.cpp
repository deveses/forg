// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/SoundInstanceManager.h"
#include "forg_pch.h"

#include <algorithm>
#include <utility>

namespace forg::audio {
namespace {

constexpr std::size_t INVALID_SNAPSHOT_INDEX = static_cast<std::size_t>(-1);

} // namespace

SoundInstanceManager::SoundInstanceManager(
    std::size_t capacity,
    std::shared_ptr<SoundInstanceProcessorChain> processorChain)
    : m_pool(capacity), m_processorChain(std::move(processorChain)),
      m_snapshots(capacity == 0
                      ? nullptr
                      : std::make_unique<InstanceSnapshot[]>(capacity)),
      m_capacity(capacity)
{
    m_instances.reserve(capacity);
}

SoundInstanceManager::~SoundInstanceManager() { Clear(); }

SoundInstanceId SoundInstanceManager::Play(SoundInstanceDescription description)
{
    ProcessedSoundInstance* instance = Create(std::move(description));
    if (instance == nullptr)
        return INVALID_SOUND_INSTANCE_ID;

    if (!instance->Play())
    {
        m_pool.Release(instance);
        return INVALID_SOUND_INSTANCE_ID;
    }

    const std::size_t snapshotIndex = AcquireSnapshotIndex();
    if (snapshotIndex == INVALID_SNAPSHOT_INDEX)
    {
        instance->Stop();
        m_pool.Release(instance);
        return INVALID_SOUND_INSTANCE_ID;
    }

    const SoundInstanceId id = instance->Id();
    m_instances.push_back({instance, snapshotIndex});
    PublishSnapshot(snapshotIndex, id, instance->State());
    return id;
}

void SoundInstanceManager::Update()
{
    ProcessCommands();

    for (auto it = m_instances.begin(); it != m_instances.end();)
    {
        ProcessedSoundInstance* instance = it->instance;
        instance->Update();
        PublishSnapshot(it->snapshotIndex, instance->Id(), instance->State());

        if (!instance->IsActive())
        {
            UnpublishSnapshot(it->snapshotIndex);
            it = m_instances.erase(it);
            m_pool.Release(instance);
        }
        else
        {
            ++it;
        }
    }
}

bool SoundInstanceManager::Stop(SoundInstanceId id)
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return false;

    return Queue({CommandType::Stop, id});
}

bool SoundInstanceManager::StopAll() { return Queue({CommandType::StopAll}); }

bool SoundInstanceManager::SetGainPan(SoundInstanceId id, float gain, float pan)
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return false;

    return Queue({CommandType::GainPan, id, gain, pan});
}

bool SoundInstanceManager::IsPlaying(SoundInstanceId id) const noexcept
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return false;

    for (std::size_t index = 0; index < m_capacity; ++index)
    {
        const SoundInstanceId firstId =
            m_snapshots[index].id.load(std::memory_order_acquire);
        if (firstId != id)
            continue;

        const SoundInstanceState state =
            m_snapshots[index].state.load(std::memory_order_acquire);
        const SoundInstanceId secondId =
            m_snapshots[index].id.load(std::memory_order_acquire);
        if (firstId == secondId)
            return state == SoundInstanceState::Playing;
    }

    return false;
}

void SoundInstanceManager::Clear()
{
    {
        std::lock_guard<std::mutex> lock(m_commandMutex);
        m_commandHead = 0;
        m_commandCount = 0;
    }

    for (const InstanceEntry& entry : m_instances)
    {
        entry.instance->Stop();
        UnpublishSnapshot(entry.snapshotIndex);
    }

    m_instances.clear();
    m_pool.Clear();
}

std::size_t SoundInstanceManager::Capacity() const noexcept
{
    return m_pool.Capacity();
}

std::size_t SoundInstanceManager::Size() const noexcept
{
    return m_pool.Size();
}

ProcessedSoundInstance*
SoundInstanceManager::Create(SoundInstanceDescription description)
{
    ProcessedSoundInstance* instance =
        m_pool.Emplace(m_processorChain, std::move(description));
    if (instance == nullptr)
        return nullptr;

    if (!instance->Create(NextInstanceId()))
    {
        m_pool.Release(instance);
        return nullptr;
    }

    return instance;
}

ProcessedSoundInstance* SoundInstanceManager::Find(SoundInstanceId id) noexcept
{
    const auto it = std::find_if(m_instances.begin(), m_instances.end(),
                                 [id](const InstanceEntry& entry)
                                 { return entry.instance->Id() == id; });
    return it == m_instances.end() ? nullptr : it->instance;
}

bool SoundInstanceManager::Queue(Command command)
{
    std::lock_guard<std::mutex> lock(m_commandMutex);
    if (m_commandCount == m_commands.size())
        return false;

    const std::size_t index =
        (m_commandHead + m_commandCount) % m_commands.size();
    m_commands[index] = command;
    ++m_commandCount;
    return true;
}

std::size_t SoundInstanceManager::DrainCommands(
    std::array<Command, MAX_QUEUED_COMMANDS>& commands)
{
    std::lock_guard<std::mutex> lock(m_commandMutex);
    const std::size_t count = m_commandCount;
    for (std::size_t index = 0; index < count; ++index)
    {
        commands[index] =
            m_commands[(m_commandHead + index) % m_commands.size()];
    }

    m_commandHead = 0;
    m_commandCount = 0;
    return count;
}

void SoundInstanceManager::ProcessCommands()
{
    std::array<Command, MAX_QUEUED_COMMANDS> commands;
    const std::size_t count = DrainCommands(commands);

    for (std::size_t index = 0; index < count; ++index)
    {
        const Command& command = commands[index];
        if (command.type == CommandType::StopAll)
        {
            for (const InstanceEntry& entry : m_instances)
                entry.instance->Stop();
            continue;
        }

        ProcessedSoundInstance* instance = Find(command.id);
        if (instance == nullptr)
            continue;

        switch (command.type)
        {
        case CommandType::Stop:
            instance->Stop();
            break;
        case CommandType::GainPan:
            instance->Parameters().pan = command.pan;
            instance->SetVolumeMultiplier(command.gain);
            break;
        case CommandType::StopAll:
            break;
        }
    }
}

std::size_t SoundInstanceManager::AcquireSnapshotIndex() const noexcept
{
    for (std::size_t index = 0; index < m_capacity; ++index)
    {
        if (m_snapshots[index].id.load(std::memory_order_acquire) ==
            INVALID_SOUND_INSTANCE_ID)
        {
            return index;
        }
    }

    return INVALID_SNAPSHOT_INDEX;
}

void SoundInstanceManager::PublishSnapshot(std::size_t index,
                                           SoundInstanceId id,
                                           SoundInstanceState state) noexcept
{
    m_snapshots[index].state.store(state, std::memory_order_release);
    m_snapshots[index].id.store(id, std::memory_order_release);
}

void SoundInstanceManager::UnpublishSnapshot(std::size_t index) noexcept
{
    m_snapshots[index].state.store(SoundInstanceState::Stopped,
                                   std::memory_order_release);
    m_snapshots[index].id.store(INVALID_SOUND_INSTANCE_ID,
                                std::memory_order_release);
}

SoundInstanceId SoundInstanceManager::NextInstanceId() noexcept
{
    const SoundInstanceId id = m_nextInstanceId++;
    if (m_nextInstanceId == INVALID_SOUND_INSTANCE_ID)
        m_nextInstanceId = 1;
    return id;
}

} // namespace forg::audio
