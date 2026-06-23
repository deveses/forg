#ifndef FORG_ENGINE_H
#define FORG_ENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg/base.h"

#include <memory>
#include <string>

namespace forg {

class IRenderDevice;
class IRenderer;

namespace scene {
class Scene;
}

struct EngineConfig
{
    std::string RendererDriver;
    uint BackBufferWidth = 100;
    uint BackBufferHeight = 100;
};

class FORG_API Engine
{
  public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool LoadConfig(const char* filename = "config.yml");
    bool Initialize(HWIN window);
    bool Initialize(HWIN window, const char* configFilename);

    void Shutdown();

    scene::Scene& Scene();
    const scene::Scene& Scene() const;

    IRenderDevice* Device() const;
    IRenderer* Renderer() const;
    const EngineConfig& Config() const;

    const char* LastError() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forg

#endif // FORG_ENGINE_H
