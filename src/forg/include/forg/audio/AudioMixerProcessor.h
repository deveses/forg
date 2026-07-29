// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "forg/api.h"
#include "forg/audio/SoundInstanceProcessor.h"

namespace forg::audio {

class AudioMixer;

class FORG_API AudioMixerProcessor final : public SoundInstanceProcessor
{
  public:
    explicit AudioMixerProcessor(AudioMixer& mixer) noexcept;

    int VoiceId(const ProcessedSoundInstance& instance) const noexcept;

    SoundProcessingResult OnCreate(ProcessedSoundInstance& instance,
                                   SoundProcessorContext& context) override;
    SoundProcessingResult OnPlay(ProcessedSoundInstance& instance,
                                 SoundProcessorContext& context) override;
    SoundProcessingResult OnUpdate(ProcessedSoundInstance& instance,
                                   SoundProcessorContext& context) override;
    SoundProcessingResult OnStop(ProcessedSoundInstance& instance,
                                 SoundProcessorContext& context) override;
    SoundProcessingResult
    OnVolumeChanged(ProcessedSoundInstance& instance,
                    SoundProcessorContext& context) override;
    void OnDestroy(ProcessedSoundInstance& instance,
                   SoundProcessorContext& context) override;

  private:
    AudioMixer& m_mixer;

    void ReleaseVoice(SoundProcessorContext& context) noexcept;
};

} // namespace forg::audio
