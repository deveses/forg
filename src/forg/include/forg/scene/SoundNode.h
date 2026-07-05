#ifndef FORG_SCENE_SOUNDNODE_H
#define FORG_SCENE_SOUNDNODE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <memory>
#include <string_view>

#include "audio/AudioGenerator.h"
#include "core/string.hpp"
#include "math/Vector3.h"
#include "scene/SceneNode.h"

namespace forg::audio {
class AudioManager;
}

namespace forg::fs {
class Filesystem;
}

namespace forg::scene {

enum class SoundSourceType
{
    None,
    Generator,
    File,
};

// Plays an audio source through the AudioManager. When a SoundEmitterNode
// is among its ancestors, its position drives distance attenuation and
// stereo panning relative to the listener.
//
// The node owns its source; a playing node must be stopped (or the manager
// stopped via StopAll) before it is destroyed outside engine-managed
// teardown.
class FORG_API SoundNode : public SceneNode
{
    SoundSourceType m_sourceType = SoundSourceType::None;
    audio::AudioWaveform m_waveform = audio::AudioWaveform::Sine;
    float m_frequency = 440.0f;
    float m_amplitude = 0.5f;
    core::string m_file;
    std::unique_ptr<audio::IAudioSource> m_source;

    bool m_looping = false;
    bool m_autoplay = false;
    float m_gain = 1.0f;

    bool m_playRequested = false;
    bool m_stopRequested = false;
    bool m_autoplayConsumed = false;
    int m_voice = -1;

  public:
    const char* TypeName() const override;
    bool Save(io::ISerializer& serializer) const override;
    bool Load(io::ISerializer& serializer) override;

    void SetGenerator(audio::AudioWaveform waveform, float frequencyHz,
                      float amplitude);
    void SetFile(std::string_view path);
    SoundSourceType SourceType() const;
    audio::IAudioSource* Source();
    const core::string& File() const;
    audio::AudioWaveform Waveform() const;
    float Frequency() const;
    float Amplitude() const;

    void SetLooping(bool looping);
    bool Looping() const;
    void SetAutoplay(bool autoplay);
    bool Autoplay() const;
    // Gain in [0, 1].
    void SetGain(float gain);
    float Gain() const;

    // Requests are applied on the next SyncAudio call.
    void Play();
    void Stop();
    bool IsPlaying() const;

    // Opens the audio file for file sources; generator sources need no
    // resources.
    bool LoadResources(const fs::Filesystem& filesystem);

    // Applies pending play/stop requests and refreshes gain/pan from the
    // nearest SoundEmitterNode ancestor and the listener.
    void SyncAudio(audio::AudioManager& manager,
                   const math::Vector3& listenerPosition,
                   const math::Vector3& listenerRight, bool hasListener);
};

const char* SoundSourceTypeName(SoundSourceType type);
bool SoundSourceTypeFromName(const core::string& name, SoundSourceType& type);

} // namespace forg::scene

#endif // FORG_SCENE_SOUNDNODE_H
