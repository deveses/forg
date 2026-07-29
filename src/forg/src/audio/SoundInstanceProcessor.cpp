// SPDX-License-Identifier: GPL-3.0-or-later

#include "forg/audio/SoundInstanceProcessor.h"

namespace forg::audio {

SoundInstanceProcessor::~SoundInstanceProcessor() = default;

SoundProcessingResult SoundInstanceProcessor::OnCreate(ProcessedSoundInstance&,
                                                       SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult SoundInstanceProcessor::OnPlay(ProcessedSoundInstance&,
                                                     SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult SoundInstanceProcessor::OnUpdate(ProcessedSoundInstance&,
                                                       SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult SoundInstanceProcessor::OnStop(ProcessedSoundInstance&,
                                                     SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult SoundInstanceProcessor::OnPause(ProcessedSoundInstance&,
                                                      SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult SoundInstanceProcessor::OnResume(ProcessedSoundInstance&,
                                                       SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

SoundProcessingResult
SoundInstanceProcessor::OnVolumeChanged(ProcessedSoundInstance&,
                                        SoundProcessorContext&)
{
    return SoundProcessingResult::Continue;
}

void SoundInstanceProcessor::OnDestroy(ProcessedSoundInstance&,
                                       SoundProcessorContext&)
{
}

} // namespace forg::audio
