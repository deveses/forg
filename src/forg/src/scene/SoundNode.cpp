#include "forg_pch.h"

#include "scene/SoundNode.h"

#include "forg/audio/AudioFileWav.h"
#include "forg/audio/AudioManager.h"
#include "forg/fs/Filesystem.h"
#include "forg/io/ISerializer.h"
#include "scene/SoundEmitterNode.h"

#include <algorithm>
#include <cstring>
#include <string>

namespace forg::scene {
namespace {

bool StringEquals(const core::string& lhs, const char* rhs)
{
    return std::strcmp(lhs.c_str(), rhs) == 0;
}

bool ValueOrDefault(io::ISerializer& serializer, const char* name, float& value)
{
    float loaded = value;
    if (serializer.Value(name, loaded))
        value = loaded;
    return true;
}

const SoundEmitterNode* FindEmitterAncestor(const TreeNode* node)
{
    for (const TreeNode* parent = node->Parent(); parent != nullptr;
         parent = parent->Parent())
    {
        const SoundEmitterNode* emitter =
            dynamic_cast<const SoundEmitterNode*>(parent);
        if (emitter != nullptr)
            return emitter;
    }
    return nullptr;
}

} // namespace

const char* SoundSourceTypeName(SoundSourceType type)
{
    switch (type)
    {
    case SoundSourceType::Generator:
        return "generator";
    case SoundSourceType::File:
        return "file";
    case SoundSourceType::None:
        break;
    }
    return "none";
}

bool SoundSourceTypeFromName(const core::string& name, SoundSourceType& type)
{
    if (StringEquals(name, "none"))
        type = SoundSourceType::None;
    else if (StringEquals(name, "generator"))
        type = SoundSourceType::Generator;
    else if (StringEquals(name, "file"))
        type = SoundSourceType::File;
    else
        return false;

    return true;
}

const char* SoundNode::TypeName() const { return "SoundNode"; }

bool SoundNode::Save(io::ISerializer& serializer) const
{
    if (!SceneNode::Save(serializer) || !serializer.BeginObject("sound"))
        return false;

    core::string source(SoundSourceTypeName(m_sourceType));
    core::string waveform(std::string(audio::AudioWaveformName(m_waveform)).c_str());
    core::string file = m_file;
    float frequency = m_frequency;
    float amplitude = m_amplitude;
    int looping = m_looping ? 1 : 0;
    int autoplay = m_autoplay ? 1 : 0;
    float gain = m_gain;

    if (!serializer.Value("source", source) ||
        !serializer.Value("waveform", waveform) ||
        !serializer.Value("frequency", frequency) ||
        !serializer.Value("amplitude", amplitude) ||
        !serializer.Value("file", file) ||
        !serializer.Value("looping", looping) ||
        !serializer.Value("autoplay", autoplay) ||
        !serializer.Value("gain", gain))
    {
        return false;
    }

    return serializer.EndObject();
}

bool SoundNode::Load(io::ISerializer& serializer)
{
    if (!SceneNode::Load(serializer) || !serializer.BeginObject("sound"))
        return false;

    core::string sourceName("none");
    if (!serializer.Value("source", sourceName))
        return false;

    SoundSourceType sourceType = SoundSourceType::None;
    if (!SoundSourceTypeFromName(sourceName, sourceType))
        return false;

    core::string waveformName("sine");
    serializer.Value("waveform", waveformName);

    float frequency = m_frequency;
    float amplitude = m_amplitude;
    float gain = m_gain;
    ValueOrDefault(serializer, "frequency", frequency);
    ValueOrDefault(serializer, "amplitude", amplitude);
    ValueOrDefault(serializer, "gain", gain);

    core::string file;
    serializer.Value("file", file);

    int looping = 0;
    serializer.Value("looping", looping);
    int autoplay = 0;
    serializer.Value("autoplay", autoplay);

    if (!serializer.EndObject())
        return false;

    m_looping = looping != 0;
    m_autoplay = autoplay != 0;
    SetGain(gain);

    switch (sourceType)
    {
    case SoundSourceType::Generator:
        SetGenerator(audio::AudioWaveformFromName(waveformName.c_str()),
                     frequency, amplitude);
        break;
    case SoundSourceType::File:
        SetFile(file.c_str());
        break;
    case SoundSourceType::None:
        RequestStopForSourceChange();
        m_sourceType = SoundSourceType::None;
        m_source.reset();
        break;
    }

    return true;
}

void SoundNode::RequestStopForSourceChange()
{
    if (m_voice >= 0)
    {
        m_stopRequested = true;
        m_playRequested = false;
    }
}

void SoundNode::SetGenerator(audio::AudioWaveform waveform, float frequencyHz,
                             float amplitude)
{
    std::shared_ptr<audio::AudioGenerator> generator =
        std::make_shared<audio::AudioGenerator>();
    generator->SetWaveform(waveform);
    generator->SetFrequency(frequencyHz);
    generator->SetAmplitude(amplitude);

    RequestStopForSourceChange();

    m_sourceType = SoundSourceType::Generator;
    m_waveform = generator->Waveform();
    m_frequency = generator->Frequency();
    m_amplitude = generator->Amplitude();
    m_file = "";
    m_source = generator;
}

void SoundNode::SetFile(std::string_view path)
{
    const std::string pathText(path);

    RequestStopForSourceChange();

    m_sourceType = SoundSourceType::File;
    m_file = pathText.c_str();
    m_source.reset();
}

SoundSourceType SoundNode::SourceType() const { return m_sourceType; }

std::shared_ptr<audio::IAudioSource> SoundNode::Source() const
{
    return m_source;
}

const core::string& SoundNode::File() const { return m_file; }

audio::AudioWaveform SoundNode::Waveform() const { return m_waveform; }

float SoundNode::Frequency() const { return m_frequency; }

float SoundNode::Amplitude() const { return m_amplitude; }

void SoundNode::SetLooping(bool looping) { m_looping = looping; }

bool SoundNode::Looping() const { return m_looping; }

void SoundNode::SetAutoplay(bool autoplay) { m_autoplay = autoplay; }

bool SoundNode::Autoplay() const { return m_autoplay; }

void SoundNode::SetGain(float gain)
{
    m_gain = std::clamp(gain, 0.0f, 1.0f);
}

float SoundNode::Gain() const { return m_gain; }

void SoundNode::Play()
{
    m_playRequested = true;
    m_stopRequested = false;
}

void SoundNode::Stop()
{
    m_stopRequested = true;
    m_playRequested = false;
}

bool SoundNode::IsPlaying() const { return m_voice >= 0; }

bool SoundNode::LoadResources(const fs::Filesystem& filesystem)
{
    if (m_sourceType != SoundSourceType::File)
        return true;

    std::filesystem::path nativePath;
    if (!filesystem.ResolveReadPath(m_file.c_str(), nativePath))
        return false;

    std::shared_ptr<audio::AudioFileWav> file =
        std::make_shared<audio::AudioFileWav>();
    if (!file->Open(nativePath.string()))
        return false;

    RequestStopForSourceChange();
    m_source = file;
    return true;
}

void SoundNode::SyncAudio(audio::AudioManager& manager,
                          const math::Vector3& listenerPosition,
                          const math::Vector3& listenerRight, bool hasListener)
{
    if (m_autoplay && !m_autoplayConsumed && m_source != nullptr)
    {
        m_autoplayConsumed = true;
        if (m_voice < 0 && !m_stopRequested)
            m_playRequested = true;
    }

    if (m_stopRequested)
    {
        if (m_voice >= 0)
            manager.Stop(m_voice);
        m_voice = -1;
        m_stopRequested = false;
    }

    if (m_voice >= 0 && !manager.IsPlaying(m_voice))
        m_voice = -1; // finished one-shot, voice reclaimed by the manager

    float gain = m_gain;
    float pan = 0.0f;

    const SoundEmitterNode* emitter = FindEmitterAncestor(this);
    if (emitter != nullptr && hasListener)
    {
        const math::Vector3 delta = emitter->Position() - listenerPosition;
        const float distance = delta.Length();
        const float reference = emitter->ReferenceDistance();

        gain *= reference / (std::max)(distance, reference);

        if (distance > 0.0001f)
        {
            math::Vector3 direction = delta;
            direction.Normalize();
            pan = std::clamp(math::Vector3::Dot(direction, listenerRight), -1.0f,
                             1.0f);
        }
    }

    if (m_playRequested)
    {
        m_playRequested = false;
        if (m_voice < 0 && m_source != nullptr)
        {
            m_source->Reset();
            m_voice = manager.Play(m_source, m_looping, gain, pan);
        }
    }
    else if (m_voice >= 0)
    {
        manager.SetGainPan(m_voice, gain, pan);
    }
}

} // namespace forg::scene
