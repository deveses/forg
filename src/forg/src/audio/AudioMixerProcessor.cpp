// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/AudioMixerProcessor.h"

#include "forg/audio/AudioMixer.h"
#include "forg/audio/IAudioSource.h"
#include "forg/audio/ProcessedSoundInstance.h"
#include "forg/audio/SoundInstanceProcessorChain.h"

namespace forg::audio {
namespace {

struct AudioMixerProcessorState
{
    std::shared_ptr<IAudioSource> source;
    int voiceId = AudioMixer::INVALID_VOICE;
    bool looping = false;
};

} // namespace

AudioMixerProcessor::AudioMixerProcessor(AudioMixer& mixer) noexcept
    : m_mixer(mixer)
{
}

int AudioMixerProcessor::VoiceId(
    const ProcessedSoundInstance& instance) const noexcept
{
    const std::shared_ptr<SoundInstanceProcessorChain> chain =
        instance.ProcessorChain();
    if (chain == nullptr)
        return AudioMixer::INVALID_VOICE;

    for (std::size_t i = 0; i < chain->Count(); i++)
    {
        if (chain->Processor(i) != this)
            continue;

        const SoundProcessorContext* context = instance.ProcessorContext(i);
        if (context == nullptr || context->storage.Empty())
            return AudioMixer::INVALID_VOICE;

        const AudioMixerProcessorState* state =
            context->Get<AudioMixerProcessorState>();
        return state != nullptr ? state->voiceId : AudioMixer::INVALID_VOICE;
    }

    return AudioMixer::INVALID_VOICE;
}

SoundProcessingResult
AudioMixerProcessor::OnCreate(ProcessedSoundInstance& instance,
                              SoundProcessorContext& context)
{
    std::shared_ptr<IAudioSource> source = instance.Source();
    if (source == nullptr)
        return SoundProcessingResult::Fail;

    AudioMixerProcessorState* state =
        context.Emplace<AudioMixerProcessorState>();
    if (state == nullptr)
        return SoundProcessingResult::Fail;

    state->source = std::move(source);
    state->looping = instance.Looping();
    return SoundProcessingResult::Continue;
}

SoundProcessingResult
AudioMixerProcessor::OnPlay(ProcessedSoundInstance& instance,
                            SoundProcessorContext& context)
{
    AudioMixerProcessorState* state = context.Get<AudioMixerProcessorState>();
    if (state == nullptr)
        return SoundProcessingResult::Fail;

    if (state->voiceId != AudioMixer::INVALID_VOICE)
        return SoundProcessingResult::Continue;

    if (!m_mixer.IsInitialized())
        return SoundProcessingResult::Fail;

    state->voiceId = m_mixer.AcquireVoice();
    if (state->voiceId == AudioMixer::INVALID_VOICE)
        return SoundProcessingResult::Wait;

    const SoundProcessingParameters& parameters = instance.Parameters();
    m_mixer.SetStreamSource(static_cast<unsigned int>(state->voiceId),
                            state->source, state->looping);
    m_mixer.SetStreamGainPan(static_cast<unsigned int>(state->voiceId),
                             parameters.volumeMultiplier, parameters.pan);

    return SoundProcessingResult::Continue;
}

SoundProcessingResult
AudioMixerProcessor::OnUpdate(ProcessedSoundInstance&,
                              SoundProcessorContext& context)
{
    AudioMixerProcessorState* state = context.Get<AudioMixerProcessorState>();
    if (state == nullptr || state->voiceId == AudioMixer::INVALID_VOICE)
        return SoundProcessingResult::Fail;

    if (state->source != nullptr &&
        !m_mixer.IsStreamActive(static_cast<unsigned int>(state->voiceId)))
    {
        return SoundProcessingResult::Complete;
    }

    return SoundProcessingResult::Continue;
}

SoundProcessingResult
AudioMixerProcessor::OnStop(ProcessedSoundInstance&,
                            SoundProcessorContext& context)
{
    ReleaseVoice(context);
    return SoundProcessingResult::Continue;
}

SoundProcessingResult
AudioMixerProcessor::OnVolumeChanged(ProcessedSoundInstance& instance,
                                     SoundProcessorContext& context)
{
    AudioMixerProcessorState* state = context.Get<AudioMixerProcessorState>();
    if (state == nullptr || state->voiceId == AudioMixer::INVALID_VOICE)
        return SoundProcessingResult::Continue;

    const SoundProcessingParameters& parameters = instance.Parameters();
    m_mixer.SetStreamGainPan(static_cast<unsigned int>(state->voiceId),
                             parameters.volumeMultiplier, parameters.pan);
    return SoundProcessingResult::Continue;
}

void AudioMixerProcessor::OnDestroy(ProcessedSoundInstance&,
                                    SoundProcessorContext& context)
{
    ReleaseVoice(context);
}

void AudioMixerProcessor::ReleaseVoice(SoundProcessorContext& context) noexcept
{
    AudioMixerProcessorState* state = context.Get<AudioMixerProcessorState>();
    if (state == nullptr || state->voiceId == AudioMixer::INVALID_VOICE)
        return;

    m_mixer.ReleaseVoice(state->voiceId);
    state->voiceId = AudioMixer::INVALID_VOICE;
}

} // namespace forg::audio
