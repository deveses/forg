#ifndef FORG_ENGINE_H
#define FORG_ENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/base.h"
#include "forg/rendering/Color.h"

#include <memory>
#include <string>
#include <string_view>

namespace forg {

class IRenderDevice;
class IRenderer;
class Engine;

namespace scene {
class Scene;
}

struct EngineConfig
{
    std::string RendererDriver;
    uint BackBufferWidth = 100;
    uint BackBufferHeight = 100;
};

struct EngineFrameStats
{
    uint64 FrameIndex = 0;
    double DeltaSeconds = 0.0;
    double ElapsedSeconds = 0.0;
    uint FPS = 0;
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

    bool Update(double deltaSeconds);
    bool Render();
    bool Frame();
    void Resize(uint width, uint height);
    void SetClearColor(const Color& color);
    void Shutdown();

    void SetUpdateCallback(EngineUpdateCallback callback, void* userData);
    void SetRenderCallback(EngineRenderCallback callback, void* userData);

    scene::Scene& Scene();
    const scene::Scene& Scene() const;

    IRenderDevice* Device() const;
    IRenderer* Renderer() const;
    const EngineConfig& Config() const;
    const EngineFrameStats& FrameStats() const;

    std::string_view LastError() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forg

#endif // FORG_ENGINE_H
