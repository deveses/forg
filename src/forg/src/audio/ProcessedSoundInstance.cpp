// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/ProcessedSoundInstance.h"

#include "forg/audio/SoundInstanceProcessorChain.h"

#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace forg::audio {
namespace {

bool IsTerminal(SoundInstanceState state)
{
    return state == SoundInstanceState::Complete ||
           state == SoundInstanceState::Failed ||
           state == SoundInstanceState::Stopped;
}

} // namespace

struct ProcessedSoundInstance::Impl
{
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

    explicit Impl(std::shared_ptr<SoundInstanceProcessorChain> soundChain)
        : chain(std::move(soundChain))
    {
    }

    std::shared_ptr<SoundInstanceProcessorChain> chain;
    std::vector<SoundProcessorContext> contexts;
    SoundProcessingParameters parameters;

    SoundInstanceId id = INVALID_SOUND_INSTANCE_ID;
    SoundInstanceState state = SoundInstanceState::Pending;
    SoundInstanceLifecyclePhase pendingPhase =
        SoundInstanceLifecyclePhase::Create;
    std::size_t pendingIndex = 0;
    bool hasPending = false;
    bool creationStarted = false;
    bool created = false;
    bool destroyed = false;

    void* sender = nullptr;
    void* emitter = nullptr;

    mutable std::mutex commandMutex;
    std::deque<Command> commands;

    void Queue(Command command)
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        commands.push_back(command);
    }

    std::deque<Command> DrainCommands()
    {
        std::lock_guard<std::mutex> lock(commandMutex);
        std::deque<Command> drained;
        drained.swap(commands);
        return drained;
    }
};

ProcessedSoundInstance::ProcessedSoundInstance(
    std::shared_ptr<SoundInstanceProcessorChain> chain)
    : m_impl(std::make_unique<Impl>(std::move(chain)))
{
}

ProcessedSoundInstance::~ProcessedSoundInstance() { DestroyProcessors(); }

SoundInstanceState ProcessedSoundInstance::State() const noexcept
{
    return m_impl->state;
}

SoundInstanceId ProcessedSoundInstance::Id() const noexcept
{
    return m_impl->id;
}

bool ProcessedSoundInstance::IsActive() const noexcept
{
    return !IsTerminal(m_impl->state);
}

bool ProcessedSoundInstance::IsPending() const noexcept
{
    return m_impl->state == SoundInstanceState::Pending || m_impl->hasPending;
}

bool ProcessedSoundInstance::IsComplete() const noexcept
{
    return m_impl->state == SoundInstanceState::Complete;
}

bool ProcessedSoundInstance::IsFailed() const noexcept
{
    return m_impl->state == SoundInstanceState::Failed;
}

bool ProcessedSoundInstance::IsStopped() const noexcept
{
    return m_impl->state == SoundInstanceState::Stopped;
}

bool ProcessedSoundInstance::HasPendingWork() const noexcept
{
    return m_impl->hasPending;
}

SoundInstanceLifecyclePhase
ProcessedSoundInstance::PendingPhase() const noexcept
{
    return m_impl->pendingPhase;
}

std::size_t ProcessedSoundInstance::PendingProcessorIndex() const noexcept
{
    return m_impl->pendingIndex;
}

SoundProcessingParameters& ProcessedSoundInstance::Parameters() noexcept
{
    return m_impl->parameters;
}

const SoundProcessingParameters&
ProcessedSoundInstance::Parameters() const noexcept
{
    return m_impl->parameters;
}

std::shared_ptr<SoundInstanceProcessorChain>
ProcessedSoundInstance::ProcessorChain() const
{
    return m_impl->chain;
}

std::size_t ProcessedSoundInstance::ProcessorContextCount() const noexcept
{
    return m_impl->contexts.size();
}

SoundProcessorContext*
ProcessedSoundInstance::ProcessorContext(std::size_t index) noexcept
{
    if (index >= m_impl->contexts.size())
        return nullptr;

    return &m_impl->contexts[index];
}

const SoundProcessorContext*
ProcessedSoundInstance::ProcessorContext(std::size_t index) const noexcept
{
    if (index >= m_impl->contexts.size())
        return nullptr;

    return &m_impl->contexts[index];
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
    m_impl->sender = sender;
}

void ProcessedSoundInstance::SetEmitterObject(void* emitter) noexcept
{
    m_impl->emitter = emitter;
}

bool ProcessedSoundInstance::Create(SoundInstanceId id)
{
    if (IsTerminal(m_impl->state))
        return false;

    if (m_impl->created)
        return true;

    if (m_impl->hasPending &&
        m_impl->pendingPhase != SoundInstanceLifecyclePhase::Create)
    {
        return false;
    }

    if (!m_impl->creationStarted)
    {
        m_impl->id = id;
        const std::size_t count =
            m_impl->chain != nullptr ? m_impl->chain->Count() : 0;
        m_impl->contexts.resize(count);
        m_impl->creationStarted = true;
    }

    const SoundProcessingResult result =
        m_impl->hasPending
            ? (ResumePendingWork() ? SoundProcessingResult::Continue
                                   : SoundProcessingResult::Wait)
            : RunPhase(SoundInstanceLifecyclePhase::Create, 0);
    if (result == SoundProcessingResult::Continue && !m_impl->hasPending &&
        !IsTerminal(m_impl->state))
    {
        m_impl->created = true;
        m_impl->state = SoundInstanceState::Pending;
    }

    return m_impl->created;
}

bool ProcessedSoundInstance::Play()
{
    if (IsTerminal(m_impl->state) || !m_impl->created)
        return false;

    if (!ResumePendingWork())
        return false;

    if (IsTerminal(m_impl->state))
        return false;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Play, 0);
    if (result == SoundProcessingResult::Continue)
        m_impl->state = SoundInstanceState::Playing;

    return m_impl->state == SoundInstanceState::Playing;
}

void ProcessedSoundInstance::Update()
{
    std::deque<Impl::Command> commands = m_impl->DrainCommands();
    for (const Impl::Command& command : commands)
    {
        if (IsTerminal(m_impl->state))
            break;

        switch (command.type)
        {
        case Impl::CommandType::Stop:
            Stop();
            break;
        case Impl::CommandType::Pause:
            Pause();
            break;
        case Impl::CommandType::Resume:
            Resume();
            break;
        case Impl::CommandType::Volume:
            SetVolumeMultiplier(command.volumeMultiplier);
            break;
        }
    }

    if (!ResumePendingWork())
        return;

    if (m_impl->state != SoundInstanceState::Playing)
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Update, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_impl->state))
    {
        m_impl->state = SoundInstanceState::Playing;
    }
}

void ProcessedSoundInstance::Stop()
{
    if (IsTerminal(m_impl->state))
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Stop, 0);
    if (result == SoundProcessingResult::Continue)
        TransitionTo(SoundInstanceState::Stopped);
}

void ProcessedSoundInstance::Pause()
{
    if (IsTerminal(m_impl->state) ||
        m_impl->state == SoundInstanceState::Paused || !m_impl->created)
    {
        return;
    }

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Pause, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_impl->state))
    {
        m_impl->state = SoundInstanceState::Paused;
    }
}

void ProcessedSoundInstance::Resume()
{
    if (IsTerminal(m_impl->state) ||
        m_impl->state == SoundInstanceState::Playing || !m_impl->created)
    {
        return;
    }

    if (!ResumePendingWork())
        return;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Resume, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_impl->state))
    {
        m_impl->state = SoundInstanceState::Playing;
    }
}

void ProcessedSoundInstance::SetVolumeMultiplier(float volumeMultiplier)
{
    if (IsTerminal(m_impl->state) || !m_impl->created ||
        !m_impl->parameters.volumeChangesEnabled)
    {
        return;
    }

    m_impl->parameters.volumeMultiplier = volumeMultiplier;

    const SoundProcessingResult result =
        RunPhase(SoundInstanceLifecyclePhase::Volume, 0);
    if (result == SoundProcessingResult::Continue && !IsTerminal(m_impl->state))
    {
        if (m_impl->state == SoundInstanceState::Pending && !m_impl->hasPending)
        {
            m_impl->state = SoundInstanceState::Pending;
        }
    }
}

void ProcessedSoundInstance::RequestStop()
{
    m_impl->Queue({Impl::CommandType::Stop, 1.0f});
}

void ProcessedSoundInstance::RequestPause()
{
    m_impl->Queue({Impl::CommandType::Pause, 1.0f});
}

void ProcessedSoundInstance::RequestResume()
{
    m_impl->Queue({Impl::CommandType::Resume, 1.0f});
}

void ProcessedSoundInstance::RequestVolumeMultiplier(float volumeMultiplier)
{
    m_impl->Queue({Impl::CommandType::Volume, volumeMultiplier});
}

void* ProcessedSoundInstance::ResolveSenderObject() const
{
    return m_impl->sender;
}

void* ProcessedSoundInstance::ResolveEmitterObject() const
{
    return m_impl->emitter;
}

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
    if (m_impl->chain == nullptr)
        return SoundProcessingResult::Continue;

    const std::size_t count = m_impl->chain->Count();
    for (std::size_t i = startIndex; i < count && i < m_impl->contexts.size();
         i++)
    {
        SoundProcessorContext& context = m_impl->contexts[i];
        if (context.bypassed && phase != SoundInstanceLifecyclePhase::Destroy)
            continue;

        SoundInstanceProcessor* processor = m_impl->chain->Processor(i);
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
            m_impl->hasPending = true;
            m_impl->pendingPhase = phase;
            m_impl->pendingIndex = i;
            m_impl->state = SoundInstanceState::Pending;
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

    m_impl->hasPending = false;
    return SoundProcessingResult::Continue;
}

bool ProcessedSoundInstance::ResumePendingWork()
{
    if (!m_impl->hasPending)
        return true;

    const SoundInstanceLifecyclePhase phase = m_impl->pendingPhase;
    const SoundProcessingResult result = RunPhase(phase, m_impl->pendingIndex);
    if (result != SoundProcessingResult::Continue)
        return false;

    switch (phase)
    {
    case SoundInstanceLifecyclePhase::Play:
    case SoundInstanceLifecyclePhase::Resume:
        if (!IsTerminal(m_impl->state))
            m_impl->state = SoundInstanceState::Playing;
        break;
    case SoundInstanceLifecyclePhase::Pause:
        if (!IsTerminal(m_impl->state))
            m_impl->state = SoundInstanceState::Paused;
        break;
    case SoundInstanceLifecyclePhase::Stop:
        TransitionTo(SoundInstanceState::Stopped);
        break;
    case SoundInstanceLifecyclePhase::Create:
        if (!IsTerminal(m_impl->state))
        {
            m_impl->created = true;
            m_impl->state = SoundInstanceState::Pending;
        }
        break;
    case SoundInstanceLifecyclePhase::Update:
    case SoundInstanceLifecyclePhase::Volume:
    case SoundInstanceLifecyclePhase::Destroy:
        if (!IsTerminal(m_impl->state) &&
            m_impl->state == SoundInstanceState::Pending)
        {
            m_impl->state = SoundInstanceState::Pending;
        }
        break;
    }

    return !m_impl->hasPending;
}

void ProcessedSoundInstance::TransitionTo(SoundInstanceState state)
{
    m_impl->hasPending = false;
    m_impl->state = state;
    if (IsTerminal(state))
        DestroyProcessors();
}

void ProcessedSoundInstance::DestroyProcessors()
{
    if (m_impl->destroyed)
        return;

    m_impl->destroyed = true;
    if (!m_impl->creationStarted || m_impl->chain == nullptr)
        return;

    const std::size_t count = m_impl->chain->Count();
    for (std::size_t i = 0; i < count && i < m_impl->contexts.size(); i++)
    {
        SoundInstanceProcessor* processor = m_impl->chain->Processor(i);
        if (processor != nullptr)
            processor->OnDestroy(*this, m_impl->contexts[i]);
    }
}

} // namespace forg::audio
