// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/SoundInstanceManager.h"
#include "forg_pch.h"

#include <algorithm>
#include <utility>

namespace forg::audio {

SoundInstanceManager::SoundInstanceManager(std::size_t capacity)
    : m_pool(capacity)
{
    m_instances.reserve(capacity);
}

SoundInstanceManager::~SoundInstanceManager() { StopAll(); }

ProcessedSoundInstance* SoundInstanceManager::Create(
    std::shared_ptr<SoundInstanceProcessorChain> processorChain,
    SoundInstanceDescription description)
{
    ProcessedSoundInstance* instance =
        m_pool.Emplace(std::move(processorChain), std::move(description));
    if (instance == nullptr)
        return nullptr;

    if (!instance->Create(NextInstanceId()))
    {
        m_pool.Release(instance);
        return nullptr;
    }

    m_instances.push_back(instance);
    return instance;
}

bool SoundInstanceManager::Destroy(ProcessedSoundInstance* instance)
{
    const auto it = std::find(m_instances.begin(), m_instances.end(), instance);
    if (it == m_instances.end())
        return false;

    m_instances.erase(it);
    return m_pool.Release(instance);
}

ProcessedSoundInstance* SoundInstanceManager::Find(SoundInstanceId id) noexcept
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return nullptr;

    const auto it = std::find_if(m_instances.begin(), m_instances.end(),
                                 [id](const ProcessedSoundInstance* instance)
                                 { return instance->Id() == id; });
    return it == m_instances.end() ? nullptr : *it;
}

const ProcessedSoundInstance*
SoundInstanceManager::Find(SoundInstanceId id) const noexcept
{
    if (id == INVALID_SOUND_INSTANCE_ID)
        return nullptr;

    const auto it = std::find_if(m_instances.begin(), m_instances.end(),
                                 [id](const ProcessedSoundInstance* instance)
                                 { return instance->Id() == id; });
    return it == m_instances.end() ? nullptr : *it;
}

void SoundInstanceManager::Update()
{
    for (auto it = m_instances.begin(); it != m_instances.end();)
    {
        ProcessedSoundInstance* instance = *it;
        instance->Update();
        if (!instance->IsActive())
        {
            it = m_instances.erase(it);
            m_pool.Release(instance);
        }
        else
        {
            ++it;
        }
    }
}

void SoundInstanceManager::Stop(SoundInstanceId id)
{
    ProcessedSoundInstance* instance = Find(id);
    if (instance == nullptr)
        return;

    instance->Stop();
    Destroy(instance);
}

void SoundInstanceManager::StopAll()
{
    for (ProcessedSoundInstance* instance : m_instances)
        instance->Stop();

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

SoundInstanceId SoundInstanceManager::NextInstanceId() noexcept
{
    const SoundInstanceId id = m_nextInstanceId++;
    if (m_nextInstanceId == INVALID_SOUND_INSTANCE_ID)
        m_nextInstanceId = 1;
    return id;
}

} // namespace forg::audio
