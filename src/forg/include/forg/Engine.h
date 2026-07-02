#ifndef FORG_ENGINE_H
#define FORG_ENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/base.h"
#include "forg/Input.h"
#include "forg/rendering/Color.h"
#include "forg/rendering/Light.h"

#include <memory>
#include <string>
#include <string_view>

namespace forg {

class IRenderDevice;
class IRenderer;
class Engine;
class Camera;

namespace fs {
class Filesystem;
}

namespace audio {
class AudioEngine;
} // namespace audio

namespace scene {
class Model;
class Scene;
} // namespace scene

namespace net {
struct Command;
}

struct EngineConfig
{
    std::string RendererDriver;
    u32 BackBufferWidth = 100;
    u32 BackBufferHeight = 100;
};

struct EngineFrameStats
{
    uint64 FrameIndex = 0;
    double DeltaSeconds = 0.0;
    double ElapsedSeconds = 0.0;
    u32 FPS = 0;
    uint64 LastRenderTimeUs = 0;
};

using EngineUpdateCallback = bool (*)(Engine& engine, double deltaSeconds,
                                      void* userData);
using EngineRenderCallback = bool (*)(Engine& engine, void* userData);

class FORG_API Engine
{
  public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool LoadConfig(std::string_view filename = "config.yml");
    bool Initialize(HWIN window);
    bool Initialize(HWIN window, std::string_view configFilename);
    bool LoadScene(std::string_view filename);
    bool LoadScene(std::string_view filename, u32 sceneIndex);
    bool StartControlServer(std::string_view bindAddr, int port);
    void StopControlServer();
    bool ControlServerRunning() const;
    u32 PumpControlCommands();

    bool Update(double deltaSeconds);
    bool Render();
    bool Frame();
    void Resize(u32 width, u32 height);
    bool HandleInput(const InputEvent& event);
    void SetClearColor(const Color& color);
    const Color& ClearColor() const;
    void Shutdown();

    void SetUpdateCallback(EngineUpdateCallback callback, void* userData);
    void SetRenderCallback(EngineRenderCallback callback, void* userData);

    static Light DefaultLight();
    bool SetLight(u32 index, const Light& light);
    bool EnableLight(u32 index, bool enabled);
    Light* GetLight(u32 index);
    const Light* GetLight(u32 index) const;

    void SetActiveModel(scene::Model* model);
    scene::Model* ActiveModel() const;
    std::string DispatchCommand(const net::Command& cmd);

    scene::Scene& Scene();
    const scene::Scene& Scene() const;
    scene::Scene& Scene(u32 sceneIndex);
    const scene::Scene& Scene(u32 sceneIndex) const;
    u32 SceneCount() const;
    audio::AudioEngine& Audio();
    const audio::AudioEngine& Audio() const;
    fs::Filesystem& Filesystem();
    const fs::Filesystem& Filesystem() const;
    forg::Camera& Camera();
    const forg::Camera& Camera() const;

    IRenderDevice* Device() const;
    IRenderer* Renderer() const;
    std::string_view RendererPluginName() const;
    const EngineConfig& Config() const;
    const EngineFrameStats& FrameStats() const;

    std::string_view LastError() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forg

#endif // FORG_ENGINE_H
