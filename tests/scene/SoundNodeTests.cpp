#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "forg/audio/AudioDefs.h"
#include "forg/audio/AudioManager.h"
#include "forg/audio/WaveFile.h"
#include "forg/fs/Filesystem.h"
#include "forg/io/MemorySerializer.h"
#include "forg/scene/Scene.h"
#include "forg/script/yaml/YAMLSerializer.h"

namespace {

class NullAudioOutput : public forg::audio::IAudioOutput
{
  public:
    bool CanWrite() override { return true; }
    void Write(char*, unsigned int) override {}
    bool Init() override { return true; }
    void Release() override {}
};

void PopulateSoundScene(forg::scene::Scene& source)
{
    forg::scene::SoundEmitterNode& emitter = source.CreateSoundEmitterNode();
    forg::scene::SoundNode& tone = source.CreateSoundNode();
    forg::scene::SoundNode& clip = source.CreateSoundNode();
    REQUIRE(emitter.AddChild(tone));

    emitter.SetPosition(forg::math::Vector3(1.0f, 2.0f, 3.0f));
    emitter.SetReferenceDistance(4.0f);

    tone.SetGenerator(forg::audio::AudioWaveform::Square, 220.0f, 0.25f);
    tone.SetLooping(true);
    tone.SetAutoplay(true);
    tone.SetGain(0.75f);

    clip.SetFile("sounds/step.wav");
}

void RequireSoundScene(const forg::scene::Scene& target)
{
    REQUIRE(target.NodeCount() == 3);

    const forg::scene::SoundEmitterNode* emitter =
        dynamic_cast<const forg::scene::SoundEmitterNode*>(target.Node(0));
    REQUIRE(emitter != nullptr);
    REQUIRE(emitter->Position().X == 1.0f);
    REQUIRE(emitter->Position().Y == 2.0f);
    REQUIRE(emitter->Position().Z == 3.0f);
    REQUIRE(emitter->ReferenceDistance() == 4.0f);

    const forg::scene::SoundNode* tone =
        dynamic_cast<const forg::scene::SoundNode*>(target.Node(1));
    REQUIRE(tone != nullptr);
    REQUIRE(tone->Parent() == emitter);
    REQUIRE(tone->SourceType() == forg::scene::SoundSourceType::Generator);
    REQUIRE(tone->Waveform() == forg::audio::AudioWaveform::Square);
    REQUIRE(tone->Frequency() == 220.0f);
    REQUIRE(tone->Amplitude() == 0.25f);
    REQUIRE(tone->Looping());
    REQUIRE(tone->Autoplay());
    REQUIRE(tone->Gain() == 0.75f);

    const forg::scene::SoundNode* clip =
        dynamic_cast<const forg::scene::SoundNode*>(target.Node(2));
    REQUIRE(clip != nullptr);
    REQUIRE(clip->Parent() == &target);
    REQUIRE(clip->SourceType() == forg::scene::SoundSourceType::File);
    REQUIRE(clip->File() == "sounds/step.wav");
    REQUIRE_FALSE(clip->Looping());
    REQUIRE_FALSE(clip->Autoplay());
}

template <typename T> void WriteValue(std::ofstream& out, T value)
{
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void WriteMixerRateWaveFile(const std::filesystem::path& path)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    REQUIRE(out);

    const std::vector<short> samples = {1000, -1000, 2000, -2000};
    const unsigned int format_size = 16;
    const unsigned int data_size =
        static_cast<unsigned int>(samples.size() * sizeof(short));
    const unsigned int riff_size = 4 + (8 + format_size) + (8 + data_size);

    WriteValue(out, static_cast<unsigned int>(RiffID));
    WriteValue(out, riff_size);
    WriteValue(out, static_cast<unsigned int>(WaveID));
    WriteValue(out, static_cast<unsigned int>(FormatID));
    WriteValue(out, format_size);
    WriteValue(out, static_cast<unsigned short>(1));       // PCM
    WriteValue(out, static_cast<unsigned short>(1));       // mono
    WriteValue(out, static_cast<unsigned int>(44100));     // sample rate
    WriteValue(out, static_cast<unsigned int>(44100 * 2)); // byte rate
    WriteValue(out, static_cast<unsigned short>(2));       // block align
    WriteValue(out, static_cast<unsigned short>(16));      // bits per sample
    WriteValue(out, static_cast<unsigned int>(DataID));
    WriteValue(out, data_size);
    out.write(reinterpret_cast<const char*>(samples.data()), data_size);
}

} // namespace

TEST_CASE("Scene round-trips sound nodes through memory",
          "[scene][sound][serialization]")
{
    forg::scene::Scene source;
    PopulateSoundScene(source);

    forg::io::MemorySerializer serializer;
    REQUIRE(source.Save(serializer));
    REQUIRE(serializer.ResetReading());

    forg::scene::Scene target;
    REQUIRE(target.Load(serializer));
    RequireSoundScene(target);
}

TEST_CASE("Scene round-trips sound nodes through YAML text",
          "[scene][sound][serialization][yaml]")
{
    forg::scene::Scene source;
    PopulateSoundScene(source);

    forg::io::YAMLSerializer writer;
    REQUIRE(source.Save(writer));

    std::string text;
    REQUIRE(writer.SaveToString(text));
    REQUIRE(text.find("SoundNode") != std::string::npos);
    REQUIRE(text.find("SoundEmitterNode") != std::string::npos);

    forg::io::YAMLSerializer reader;
    REQUIRE(reader.LoadFromString(text));

    forg::scene::Scene target;
    REQUIRE(target.Load(reader));
    RequireSoundScene(target);
}

TEST_CASE("Scene loads WAV resources for file sound nodes", "[scene][sound]")
{
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / "forg_soundnode_data";
    std::filesystem::create_directories(dir);
    WriteMixerRateWaveFile(dir / "step.wav");

    forg::fs::Filesystem filesystem;
    REQUIRE(filesystem.Mount("data:", dir));

    forg::scene::Scene scene;
    forg::scene::SoundNode& clip = scene.CreateSoundNode();
    clip.SetFile("data:step.wav");

    REQUIRE(scene.LoadResources(filesystem, nullptr));
    REQUIRE(clip.Source() != nullptr);
    REQUIRE(clip.Source()->Channels() == 1);

    forg::scene::SoundNode& missing = scene.CreateSoundNode();
    missing.SetFile("data:missing.wav");
    REQUIRE_FALSE(scene.LoadResources(filesystem, nullptr));

    std::filesystem::remove_all(dir);
}

TEST_CASE("SoundNode SyncAudio drives the audio manager",
          "[scene][sound][audio]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::scene::Scene scene;
    forg::scene::SoundNode& tone = scene.CreateSoundNode();
    tone.SetGenerator(forg::audio::AudioWaveform::Sine, 440.0f, 0.5f);
    tone.SetLooping(true);

    REQUIRE_FALSE(tone.IsPlaying());

    scene.UpdateAudio(manager);
    REQUIRE_FALSE(tone.IsPlaying()); // no play request yet

    tone.Play();
    scene.UpdateAudio(manager);
    REQUIRE(tone.IsPlaying());

    tone.Stop();
    scene.UpdateAudio(manager);
    REQUIRE_FALSE(tone.IsPlaying());
}

TEST_CASE("SoundNode autoplay starts on first sync", "[scene][sound][audio]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::scene::Scene scene;
    forg::scene::SoundNode& tone = scene.CreateSoundNode();
    tone.SetGenerator(forg::audio::AudioWaveform::Sine, 440.0f, 0.5f);
    tone.SetLooping(true);
    tone.SetAutoplay(true);

    scene.UpdateAudio(manager);
    REQUIRE(tone.IsPlaying());

    tone.Stop();
    scene.UpdateAudio(manager);
    REQUIRE_FALSE(tone.IsPlaying()); // autoplay is consumed, does not restart
    scene.UpdateAudio(manager);
    REQUIRE_FALSE(tone.IsPlaying());
}

TEST_CASE("SoundNode pans toward an emitter to the listener's right",
          "[scene][sound][audio]")
{
    class CapturingAudioOutput : public forg::audio::IAudioOutput
    {
      public:
        std::vector<char> written;
        bool CanWrite() override { return true; }
        void Write(char* data, unsigned int size) override
        {
            written.assign(data, data + size);
        }
        bool Init() override { return true; }
        void Release() override {}
    };

    CapturingAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::scene::Scene scene;

    // Camera at origin looking down -Z (right-handed): +X is to the right.
    forg::scene::CameraNode& camera = scene.CreateCameraNode();
    camera.GetCamera().set_Position(forg::math::Vector3(0.0f, 0.0f, 0.0f));
    camera.GetCamera().set_Target(forg::math::Vector3(0.0f, 0.0f, -1.0f));

    forg::scene::SoundEmitterNode& emitter = scene.CreateSoundEmitterNode();
    emitter.SetPosition(forg::math::Vector3(5.0f, 0.0f, 0.0f));

    forg::scene::SoundNode& tone = scene.CreateSoundNode();
    REQUIRE(emitter.AddChild(tone));
    tone.SetGenerator(forg::audio::AudioWaveform::Sine, 440.0f, 1.0f);
    tone.SetLooping(true);
    tone.Play();

    scene.UpdateAudio(manager);
    REQUIRE(tone.IsPlaying());
    manager.Update();

    REQUIRE_FALSE(output.written.empty());
    std::vector<short> samples(output.written.size() / sizeof(short));
    std::memcpy(samples.data(), output.written.data(), output.written.size());

    long long leftEnergy = 0;
    long long rightEnergy = 0;
    for (std::size_t i = 0; i + 1 < samples.size(); i += 2)
    {
        leftEnergy += std::abs(static_cast<long long>(samples[i]));
        rightEnergy += std::abs(static_cast<long long>(samples[i + 1]));
    }

    // Emitter fully to the right: right channel carries the signal.
    REQUIRE(rightEnergy > 0);
    REQUIRE(leftEnergy == 0);
}

TEST_CASE("SoundNode keeps a replaced source alive until the next sync",
          "[scene][sound][audio]")
{
    NullAudioOutput output;
    forg::audio::AudioManager manager;
    REQUIRE(manager.InitWithOutput(&output));

    forg::scene::Scene scene;
    forg::scene::SoundNode& tone = scene.CreateSoundNode();
    tone.SetGenerator(forg::audio::AudioWaveform::Sine, 440.0f, 0.5f);
    tone.SetLooping(true);
    tone.Play();

    scene.UpdateAudio(manager);
    REQUIRE(tone.IsPlaying());
    manager.Update(); // mixer pulls from the first generator

    // Replace the source while the voice is still attached; the mixer must
    // keep reading valid memory until the next sync (ASan guards this).
    tone.SetGenerator(forg::audio::AudioWaveform::Square, 220.0f, 0.5f);
    manager.Update();

    scene.UpdateAudio(manager); // stops the stale voice, frees the old source
    REQUIRE_FALSE(tone.IsPlaying());

    tone.Play();
    scene.UpdateAudio(manager);
    REQUIRE(tone.IsPlaying());
    manager.Update();

    tone.Stop();
    scene.UpdateAudio(manager);
    REQUIRE_FALSE(tone.IsPlaying());
}
