// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/ProcessedSoundInstance.h"

#include "forg/audio/SoundInstanceProcessorChain.h"

#include <utility>

namespace forg::audio {
namespace {

bool IsTerminal(SoundInstanceState state)
{
    return state == SoundInstanceState::Complete ||
           state == SoundInstanceState::Failed ||
           state == SoundInstanceState::Stopped;
}

} // namespace

ProcessedSoundInstance::ProcessedSoundInstance(
    std::shared_ptr<SoundInstanceProcessorChain> chain,
    SoundInstanceDescription description)
    : m_chain(std::move(chain)), m_source(std::move(description.source)),
      m_looping(description.looping),
      m_parameters(std::move(description.parameters))
{
}

ProcessedSoundInstance::~ProcessedSoundInstance() { DestroyProcessors(); }

bool ProcessedSoundInstance::Queue(Command command)
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

std::size_t ProcessedSoundInstance::DrainCommands(
    std::array<Command, MAX_QUEUED_COMMANDS>& commands)
{
    std::lock_guard<std::mutex> lock(m_commandMutex);
    const std::size_t count = m_commandCount;
    for (std::size_t i = 0; i < count; ++i)
        commands[i] = m_commands[(m_commandHead + i) % m_commands.size()];

    m_commandHead = 0;
    m_commandCount = 0;
    return count;
}

SoundInstanceState ProcessedSoundInstance::State() const noexcept
{
    return m_state;
}

SoundInstanceId ProcessedSoundInstance::Id() const noexcept { return m_id; }

bool ProcessedSoundInstance::IsActive() const noexcept
{
    return !IsTerminal(m_state);
}

bool ProcessedSoundInstance::IsPending() const noexcept
{
    return m_state == SoundInstanceState::Pending || m_hasPending;
}

bool ProcessedSoundInstance::IsComplete() const noexcept
{
    return m_state == SoundInstanceState::Complete;
}

bool ProcessedSoundInstance::IsFailed() const noexcept
{
    return m_state == SoundInstanceState::Failed;
}

bool ProcessedSoundInstance::IsStopped() const noexcept
{
    return m_state == SoundInstanceState::Stopped;
}

bool ProcessedSoundInstance::HasPendingWork() const noexcept
{
    return m_hasPending;
}

SoundInstanceLifecyclePhase
ProcessedSoundInstance::PendingPhase() const noexcept
{
    return m_pendingPhase;
}

std::size_t ProcessedSoundInstance::PendingProcessorIndex() const noexcept
{
    return m_pendingIndex;
}

SoundProcessingParameters& ProcessedSoundInstance::Parameters() noexcept
{
    return m_parameters;
}

const SoundProcessingParameters&
ProcessedSoundInstance::Parameters() const noexcept
{
    return m_parameters;
}

std::shared_ptr<IAudioSource> ProcessedSoundInstance::Source() const
{
    return m_source;
}

bool ProcessedSoundInstance::Looping() const noexcept { return m_looping; }

std::shared_ptr<SoundInstanceProcessorChain>
ProcessedSoundInstance::ProcessorChain() const
{
    return m_chain;
}

std::size_t ProcessedSoundInstance::ProcessorContextCount() const noexcept
{
    return m_contextCount;
}

SoundProcessorContext*
ProcessedSoundInstance::ProcessorContext(std::size_t index) noexcept
{
    if (index >= m_contextCount)
        return nullptr;

    return &m_contexts[index];
}

const SoundProcessorContext*
ProcessedSoundInstance::ProcessorContext(std::size_t index) const noexcept
{
    if (index >= m_contextCount)
        return nullptr;

    return &m_contexts[index];
}

void* ProcessedSoundInstance::SenderObject() const
{
    return ResolveSenderObject();
}

void* ProcessedSoundInstance::EmitterObject() const
{
    return ResolveEmitterObject();
}

void ProcessedSoundInstance::SetSenderObject(void* sender) noexcept
{
    m_sender = sender;
}

void ProcessedSoundInstance::SetEmitterObject(void* emitter) noexcept
{
    m_emitter = emitter;
}

bool ProcessedSoundInstance::Create(SoundInstanceId id)
{
    if (IsTerminal(m_state))
        return false;

    if (m_created)
        return true;

    if (m_hasPending && m_pendingPhase != SoundInstanceLifecyclePhase::Create)
    {
        return false;
    }

    if (!m_creationStarted)
    {
        m_id = id;
        const std::size_t contextCount =
            m_chain != nullptr ? m_chain->Count() : 0;
        if (contextCount > m_contexts.size())
        {
            TransitionTo(SoundInstanceState::Failed);
            return false;
        }
        m_contextCount = contextCount;
        m_creationStarted = true;
    }

    const SoundProcessingResult result =
        m_hasPending ? (ResumePendingWork() ? SoundProcessingResult::Continue
                                            : SoundProcessingResult::Wait)
                     : RunPhase(SoundInstanceLifecyclePhase::Create, 0);
    if (result == SoundProcessingResult::Continue && !m_hasPending &&
        !IsTerminal(m_state))
    {
        m_created = true;
        m_state = SoundInstanceState::Pending;
    }

    return m_created;
}

bool ProcessedSoundInstance::Play()
{
    if (IsTerminal(m_state) || !m_created)
        return false;

    if (!ResumePendingWork())
        return false;

    if (IsTerminal(m_state))
        return false;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Play, 0);
    if (result == SoundProcessingResult::Continue)
        m_state = SoundInstanceState::Playing;

    return m_state == SoundInstanceState::Playing;
}

void ProcessedSoundInstance::Update()
{
    std::array<Command, MAX_QUEUED_COMMANDS> commands;
    const std::size_t commandCount = DrainCommands(commands);
    for (std::size_t i = 0; i < commandCount; ++i)
    {
        const Command& command = commands[i];
        if (IsTerminal(m_state))
            break;

        switch (command.type)
        {
        case CommandType::Stop:
            Stop();
            break;
        case CommandType::Pause:
            Pause();
            break;
        case CommandType::Resume:
            Resume();
            break;
        case CommandType::Volume:
            SetVolumeMultiplier(command.volumeMultiplier);
            break;
        }
    }

    if (!ResumePendingWork())
        return;

    if (m_state != SoundInstanceState::Playing)
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Update, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_state))
    {
        m_state = SoundInstanceState::Playing;
    }
}

void ProcessedSoundInstance::Stop()
{
    if (IsTerminal(m_state))
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Stop, 0);
    if (result == SoundProcessingResult::Continue)
        TransitionTo(SoundInstanceState::Stopped);
}

void ProcessedSoundInstance::Pause()
{
    if (IsTerminal(m_state) || m_state == SoundInstanceState::Paused ||
        !m_created)
    {
        return;
    }

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Pause, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_state))
    {
        m_state = SoundInstanceState::Paused;
    }
}

void ProcessedSoundInstance::Resume()
{
    if (IsTerminal(m_state) || m_state == SoundInstanceState::Playing ||
        !m_created)
    {
        return;
    }

    if (!ResumePendingWork())
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Resume, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_state))
    {
        m_state = SoundInstanceState::Playing;
    }
}

void ProcessedSoundInstance::SetVolumeMultiplier(float volumeMultiplier)
{
    if (IsTerminal(m_state) || !m_created || !m_parameters.volumeChangesEnabled)
    {
        return;
    }

    m_parameters.volumeMultiplier = volumeMultiplier;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Volume, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_state))
    {
        if (m_state == SoundInstanceState::Pending && !m_hasPending)
        {
            m_state = SoundInstanceState::Pending;
        }
    }
}

bool ProcessedSoundInstance::RequestStop()
{
    return Queue({CommandType::Stop, 1.0f});
}

bool ProcessedSoundInstance::RequestPause()
{
    return Queue({CommandType::Pause, 1.0f});
}

bool ProcessedSoundInstance::RequestResume()
{
    return Queue({CommandType::Resume, 1.0f});
}

bool ProcessedSoundInstance::RequestVolumeMultiplier(float volumeMultiplier)
{
    return Queue({CommandType::Volume, volumeMultiplier});
}

void* ProcessedSoundInstance::ResolveSenderObject() const { return m_sender; }

void* ProcessedSoundInstance::ResolveEmitterObject() const { return m_emitter; }

SoundProcessingResult
ProcessedSoundInstance::CallProcessor(SoundInstanceLifecyclePhase phase,
                                      SoundInstanceProcessor& processor,
                                      SoundProcessorContext& context)
{
    switch (phase)
    {
    case SoundInstanceLifecyclePhase::Create:
        return processor.OnCreate(*this, context);
    case SoundInstanceLifecyclePhase::Play:
        return processor.OnPlay(*this, context);
    case SoundInstanceLifecyclePhase::Update:
        return processor.OnUpdate(*this, context);
    case SoundInstanceLifecyclePhase::Stop:
        return processor.OnStop(*this, context);
    case SoundInstanceLifecyclePhase::Pause:
        return processor.OnPause(*this, context);
    case SoundInstanceLifecyclePhase::Resume:
        return processor.OnResume(*this, context);
    case SoundInstanceLifecyclePhase::Volume:
        return processor.OnVolumeChanged(*this, context);
    case SoundInstanceLifecyclePhase::Destroy:
        break;
    }

    return SoundProcessingResult::Continue;
}

SoundProcessingResult
ProcessedSoundInstance::RunPhase(SoundInstanceLifecyclePhase phase,
                                 std::size_t startIndex)
{
    if (m_chain == nullptr)
        return SoundProcessingResult::Continue;

    const std::size_t count = m_chain->Count();
    for (std::size_t i = startIndex; i < count && i < m_contextCount; i++)
    {
        SoundProcessorContext& context = m_contexts[i];
        if (context.bypassed && phase != SoundInstanceLifecyclePhase::Destroy)
            continue;

        SoundInstanceProcessor* processor = m_chain->Processor(i);
        if (processor == nullptr)
            continue;

        const SoundProcessingResult result =
            CallProcessor(phase, *processor, context);

        switch (result)
        {
        case SoundProcessingResult::Continue:
            break;
        case SoundProcessingResult::Bypass:
            context.bypassed = true;
            break;
        case SoundProcessingResult::Wait:
            m_hasPending = true;
            m_pendingPhase = phase;
            m_pendingIndex = i;
            m_state = SoundInstanceState::Pending;
            return result;
        case SoundProcessingResult::Stop:
            TransitionTo(SoundInstanceState::Stopped);
            return result;
        case SoundProcessingResult::Fail:
            TransitionTo(SoundInstanceState::Failed);
            return result;
        case SoundProcessingResult::Complete:
            TransitionTo(SoundInstanceState::Complete);
            return result;
        }
    }

    m_hasPending = false;
    return SoundProcessingResult::Continue;
}

bool ProcessedSoundInstance::ResumePendingWork()
{
    if (!m_hasPending)
        return true;

    const SoundInstanceLifecyclePhase phase = m_pendingPhase;
    const SoundProcessingResult result = RunPhase(phase, m_pendingIndex);
    if (result != SoundProcessingResult::Continue)
        return false;

    switch (phase)
    {
    case SoundInstanceLifecyclePhase::Play:
    case SoundInstanceLifecyclePhase::Resume:
        if (!IsTerminal(m_state))
            m_state = SoundInstanceState::Playing;
        break;
    case SoundInstanceLifecyclePhase::Pause:
        if (!IsTerminal(m_state))
            m_state = SoundInstanceState::Paused;
        break;
    case SoundInstanceLifecyclePhase::Stop:
        TransitionTo(SoundInstanceState::Stopped);
        break;
    case SoundInstanceLifecyclePhase::Create:
        if (!IsTerminal(m_state))
        {
            m_created = true;
            m_state = SoundInstanceState::Pending;
        }
        break;
    case SoundInstanceLifecyclePhase::Update:
    case SoundInstanceLifecyclePhase::Volume:
    case SoundInstanceLifecyclePhase::Destroy:
        if (!IsTerminal(m_state) && m_state == SoundInstanceState::Pending)
        {
            m_state = SoundInstanceState::Pending;
        }
        break;
    }

    return !m_hasPending;
}

void ProcessedSoundInstance::TransitionTo(SoundInstanceState state)
{
    m_hasPending = false;
    m_state = state;
    if (IsTerminal(state))
        DestroyProcessors();
}

void ProcessedSoundInstance::DestroyProcessors()
{
    if (m_destroyed)
        return;

    m_destroyed = true;
    if (!m_creationStarted || m_chain == nullptr)
        return;

    const std::size_t count = m_chain->Count();
    for (std::size_t i = 0; i < count && i < m_contextCount; i++)
    {
        SoundInstanceProcessor* processor = m_chain->Processor(i);
        if (processor != nullptr)
            processor->OnDestroy(*this, m_contexts[i]);
    }
}

} // namespace forg::audio
