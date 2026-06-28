#include "forg_pch.h"

#include "forg/Engine.h"

#include "PerformanceCounter.h"
#include "forg/Input.h"
#include "forg/rendering/IRenderDevice.h"
#include "forg/rendering/IRenderer.h"
#include "forg/rendering/Camera.h"
#include "forg/rendering/CameraOrbitController.h"
#include "forg/scene/Scene.h"
#include "forg/control/SceneControl.h"
#include "forg/net/CommandQueue.h"
#include "forg/net/HttpControlServer.h"
#include "forg/scene/Model.h"
#include "forg/script/yaml/YAMLParser.h"
#include "forg/script/yaml/YAMLSerializer.h"

#include <charconv>
#include <sstream>
#include <string_view>
#include <system_error>
#include <utility>

#ifdef FORG_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace forg {

namespace {

bool ParsePositiveUint(std::string_view text, uint& value)
{
    if (text.empty())
        return false;

    uint parsed = 0;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto [ptr, ec] = std::from_chars(begin, end, parsed);

    if (ec != std::errc() || ptr != end || parsed == 0)
        return false;

    value = parsed;
    return true;
}

#ifdef FORG_PLATFORM_WINDOWS
using PluginModule = HMODULE;

PluginModule OpenPluginModule(std::string_view driver, std::string& error)
{
    const std::string driverText(driver);
    HMODULE module = LoadLibraryA(driverText.c_str());
    if (module == nullptr)
    {
        std::ostringstream stream;
        stream << "Unable to load renderer <" << driverText << ">";
        error = stream.str();
    }
    return module;
}

template <typename T> T PluginSymbol(PluginModule module, const char* name)
{
    return reinterpret_cast<T>(GetProcAddress(module, name));
}

bool ClosePluginModule(PluginModule module, std::string& error)
{
    if (module == nullptr)
        return true;

    if (FreeLibrary(module) != 0)
        return true;

    error = "Unable to unload renderer plugin";
    return false;
}
#else
using PluginModule = void*;

std::string PluginPath(std::string_view driver)
{
    if (driver.find('/') != std::string::npos)
        return std::string(driver);

    std::string path = "./";
    path.append(driver);
    return path;
}

PluginModule OpenPluginModule(std::string_view driver, std::string& error)
{
    const std::string path = PluginPath(driver);
    void* module = dlopen(path.c_str(), RTLD_NOW);
    if (module == nullptr)
    {
        std::ostringstream stream;
        stream << "Unable to load renderer <" << std::string(driver)
               << ">: " << dlerror();
        error = stream.str();
    }
    return module;
}

template <typename T> T PluginSymbol(PluginModule module, const char* name)
{
    return reinterpret_cast<T>(dlsym(module, name));
}

bool ClosePluginModule(PluginModule module, std::string& error)
{
    if (module == nullptr)
        return true;

    if (dlclose(module) == 0)
        return true;

    std::ostringstream stream;
    stream << "Unable to unload renderer plugin: " << dlerror();
    error = stream.str();
    return false;
}
#endif

class PluginModuleHandle
{
  public:
    PluginModuleHandle() = default;
    ~PluginModuleHandle()
    {
        std::string ignored;
        Reset(ignored);
    }

    PluginModuleHandle(const PluginModuleHandle&) = delete;
    PluginModuleHandle& operator=(const PluginModuleHandle&) = delete;

    PluginModule Get() const { return m_module; }
    bool Empty() const { return m_module == nullptr; }

    void Attach(PluginModule module)
    {
        std::string ignored;
        Reset(ignored);
        m_module = module;
    }

    bool Reset(std::string& error)
    {
        PluginModule module = std::exchange(m_module, nullptr);
        return ClosePluginModule(module, error);
    }

  private:
    PluginModule m_module = nullptr;
};

class RenderDeviceHandle
{
  public:
    RenderDeviceHandle() = default;
    ~RenderDeviceHandle() { Reset(); }

    RenderDeviceHandle(const RenderDeviceHandle&) = delete;
    RenderDeviceHandle& operator=(const RenderDeviceHandle&) = delete;

    IRenderDevice* Get() const { return m_device; }

    void Attach(IRenderDevice* device)
    {
        Reset();
        m_device = device;
    }

    void Reset()
    {
        IRenderDevice* device = std::exchange(m_device, nullptr);
        if (device != nullptr)
            device->Release();
    }

  private:
    IRenderDevice* m_device = nullptr;
};

class RendererHandle
{
  public:
    RendererHandle() = default;
    ~RendererHandle()
    {
        std::string ignored;
        Reset(ignored);
    }

    RendererHandle(const RendererHandle&) = delete;
    RendererHandle& operator=(const RendererHandle&) = delete;

    IRenderer* Get() const { return m_renderer; }

    void Attach(IRenderer* renderer, const RendererPluginBinding& binding)
    {
        std::string ignored;
        Reset(ignored);
        m_renderer = renderer;
        m_binding = binding;
    }

    bool Reset(std::string& error)
    {
        IRenderer* renderer = std::exchange(m_renderer, nullptr);
        const RendererPluginBinding binding = m_binding;
        m_binding = {};

        if (renderer == nullptr)
            return true;

        const RendererPluginStatus status =
            DestroyRendererFromPlugin(binding, renderer);
        if (status == RendererPluginStatus::Ok)
            return true;

        std::ostringstream stream;
        stream << "Unable to destroy renderer: "
               << RendererPluginStatusName(status);
        error = stream.str();
        return false;
    }

  private:
    IRenderer* m_renderer = nullptr;
    RendererPluginBinding m_binding;
};

} // namespace

struct Engine::Impl
{
    EngineConfig config;
    bool configLoaded = false;
    scene::Scene scene;
    PluginModuleHandle module;
    RendererHandle renderer;
    RenderDeviceHandle device;
    EngineFrameStats frameStats;
    PerformanceCounter frameClock;
    PerformanceCounter fpsClock;
    forg::Camera camera;
    forg::CameraOrbitController cameraController;
    Color clearColor = Color(0.75f, 0.75f, 0.75f);
    Light light = Engine::DefaultLight();
    bool lightEnabled = true;
    scene::Model* activeModel = nullptr;
    uint fpsFrameCounter = 0;
    EngineUpdateCallback updateCallback = nullptr;
    void* updateUserData = nullptr;
    EngineRenderCallback renderCallback = nullptr;
    void* renderUserData = nullptr;
    std::unique_ptr<net::CommandQueue> controlQueue;
    std::unique_ptr<net::HttpControlServer> controlServer;
    std::string lastError;

    bool IsInitialized() const
    {
        return !module.Empty() || renderer.Get() != nullptr ||
               device.Get() != nullptr;
    }

    void SetError(std::string_view message) { lastError.assign(message); }

    void ClearError() { lastError.clear(); }

    bool RequireInitialized()
    {
        if (device.Get() != nullptr)
            return true;

        SetError("Engine is not initialized");
        return false;
    }

    void ResetFrameState()
    {
        frameStats = {};
        fpsFrameCounter = 0;
        frameClock.Start();
        fpsClock.Start();
    }

    bool LoadConfig(std::string_view filename)
    {
        if (IsInitialized())
        {
            SetError("Cannot load config while engine is initialized");
            return false;
        }

        if (filename.empty())
        {
            SetError("Config filename is empty");
            return false;
        }

        const std::string filenameText(filename);
        script::yaml::YAMLParser parser;
        if (!parser.Open(filenameText.c_str()))
        {
            std::ostringstream stream;
            stream << "Unable to open config <" << filenameText << ">";
            SetError(stream.str());
            return false;
        }

        script::yaml::YAMLDocument* document = parser.Parse();
        if (document == nullptr)
        {
            std::ostringstream stream;
            stream << "Unable to parse config <" << filenameText << ">";
            SetError(stream.str());
            parser.Close();
            return false;
        }

        EngineConfig nextConfig;

        if (const char* driver = script::yaml::FindNodeAttributeValue(
                document, "renderer", "driver"))
            nextConfig.RendererDriver = driver;

        if (const char* width = script::yaml::FindNodeAttributeValue(
                document, "window", "width"))
        {
            if (!ParsePositiveUint(width, nextConfig.BackBufferWidth))
            {
                SetError("Invalid window.width in config");
                parser.Close();
                return false;
            }
        }

        if (const char* height = script::yaml::FindNodeAttributeValue(
                document, "window", "height"))
        {
            if (!ParsePositiveUint(height, nextConfig.BackBufferHeight))
            {
                SetError("Invalid window.height in config");
                parser.Close();
                return false;
            }
        }

        config = nextConfig;
        configLoaded = true;
        ClearError();
        parser.Close();
        return true;
    }

    bool Initialize(HWIN window)
    {
        if (IsInitialized())
        {
            SetError("Engine is already initialized");
            return false;
        }

        if (!configLoaded)
        {
            SetError("Engine config is not loaded");
            return false;
        }

        if (config.RendererDriver.empty())
        {
            SetError("No renderer driver specified in config");
            return false;
        }

        std::string error;
        PluginModule loadedModule =
            OpenPluginModule(config.RendererDriver, error);
        if (loadedModule == nullptr)
        {
            SetError(error);
            return false;
        }
        module.Attach(loadedModule);

        auto getDescriptor = PluginSymbol<PFGETRENDERERPLUGINDESCRIPTOR>(
            module.Get(), "forgGetRendererPluginDescriptor");
        auto createRenderer =
            PluginSymbol<PFCREATERENDERER>(module.Get(), "forgCreateRenderer");

        RendererPluginBinding binding;
        RendererPluginStatus status =
            ProbeRendererPlugin(getDescriptor, createRenderer, binding);
        if (status != RendererPluginStatus::Ok)
        {
            std::ostringstream stream;
            stream << "Incompatible renderer plugin <" << config.RendererDriver
                   << ">: " << RendererPluginStatusName(status);
            SetError(stream.str());
            CleanupAfterInitializeFailure();
            return false;
        }

        IRenderer* createdRenderer = nullptr;
        status = CreateRendererFromPlugin(binding, createdRenderer);
        if (status != RendererPluginStatus::Ok)
        {
            std::ostringstream stream;
            stream << "Unable to create renderer <" << config.RendererDriver
                   << ">: " << RendererPluginStatusName(status);
            SetError(stream.str());
            CleanupAfterInitializeFailure();
            return false;
        }
        renderer.Attach(createdRenderer, binding);

        RENDER_PARAMETERS parameters = {};
        parameters.BackBufferWidth = config.BackBufferWidth;
        parameters.BackBufferHeight = config.BackBufferHeight;
        parameters.PresentationInterval = 0;

        IRenderDevice* createdDevice =
            renderer.Get()->CreateDevice(window, &parameters);
        if (createdDevice == nullptr)
        {
            SetError("Unable to create render device");
            CleanupAfterInitializeFailure();
            return false;
        }
        device.Attach(createdDevice);

        device.Get()->SetRenderState(RenderStates_CullMode, Cull_Clockwise);
        device.Get()->SetRenderState(RenderStates_ShadeMode, ShadeMode_Gouraud);
        device.Get()->SetRenderState(RenderStates_Lighting, true);
        device.Get()->SetRenderState(RenderStates_FillMode, FillMode_Solid);
        device.Get()->SetRenderState(RenderStates_SourceBlend,
                                     Blend_SourceAlpha);
        device.Get()->SetRenderState(RenderStates_DestinationBlend,
                                     Blend_InvSourceAlpha);

        ResetFrameState();
        ClearError();
        return true;
    }

    bool LoadScene(std::string_view filename)
    {
        if (!RequireInitialized())
            return false;

        if (filename.empty())
        {
            SetError("Scene filename is empty");
            return false;
        }

        io::YAMLSerializer serializer;
        if (!serializer.OpenRead(filename))
        {
            std::ostringstream stream;
            stream << "Unable to open scene <" << std::string(filename) << ">";
            SetError(stream.str());
            return false;
        }

        if (!scene.Load(serializer))
        {
            std::ostringstream stream;
            stream << "Unable to load scene <" << std::string(filename) << ">";
            SetError(stream.str());
            return false;
        }

        if (!scene.LoadResources(device.Get()))
        {
            std::ostringstream stream;
            stream << "Unable to load scene resources <"
                   << std::string(filename) << ">";
            SetError(stream.str());
            return false;
        }

        activeModel = nullptr;
        ClearError();
        return true;
    }

    bool StartControlServer(std::string_view bindAddr, int port)
    {
        if (bindAddr.empty())
        {
            SetError("Control server bind address is empty");
            return false;
        }

        if (port <= 0 || port > 65535)
        {
            SetError("Control server port is invalid");
            return false;
        }

        StopControlServer();

        controlQueue.reset(new net::CommandQueue());
        controlServer.reset(new net::HttpControlServer(std::string(bindAddr),
                                                       port, *controlQueue));
        if (!controlServer->Start())
        {
            controlServer.reset();
            controlQueue.reset();
            SetError("Control server failed to start");
            return false;
        }

        ClearError();
        return true;
    }

    void StopControlServer()
    {
        if (controlServer)
        {
            controlServer->Stop();
            controlServer.reset();
        }
        controlQueue.reset();
    }

    bool ControlServerRunning() const { return controlServer != nullptr; }

    uint PumpControlCommands()
    {
        if (!controlQueue)
            return 0;

        uint count = 0;
        net::QueueItem item;
        while (controlQueue->TryPop(item))
        {
            std::string body = DispatchCommand(item.cmd);
            if (item.reply)
                item.reply->set_value(body);
            ++count;
        }
        return count;
    }

    bool Update(double deltaSeconds, Engine& engine)
    {
        if (!RequireInitialized())
            return false;

        PumpControlCommands();
        scene.Update(deltaSeconds);
        if (updateCallback != nullptr &&
            !updateCallback(engine, deltaSeconds, updateUserData))
        {
            SetError("Engine update callback failed");
            return false;
        }

        ClearError();
        return true;
    }

    bool Render(Engine& engine)
    {
        if (!RequireInitialized())
            return false;

        PerformanceCounter renderTimer;
        renderTimer.Start();

        IRenderDevice* renderDevice = device.Get();
        Matrix4 view;
        camera.GetViewMatrix(view);
        renderDevice->SetTransform(TransformType_View, view);

        Matrix4 projection;
        camera.GetProjectionMatrix(projection);
        renderDevice->SetTransform(TransformType_Projection, projection);

        renderDevice->Clear(ClearFlags_Target | ClearFlags_ZBuffer, clearColor,
                            1.0f, 0);
        renderDevice->BeginScene();

        if (lightEnabled)
            renderDevice->SetLight(0, &light);
        renderDevice->LightEnable(0, lightEnabled);
        renderDevice->SetRenderState(RenderStates_Lighting, lightEnabled);

        scene.Render(renderDevice);

        bool callbackOk = true;
        if (renderCallback != nullptr)
            callbackOk = renderCallback(engine, renderUserData);

        renderDevice->EndScene();
        renderDevice->Present();

        renderTimer.Stop();
        renderTimer.GetDurationInUs(frameStats.LastRenderTimeUs);

        if (!callbackOk)
        {
            SetError("Engine render callback failed");
            return false;
        }

        ++frameStats.FrameIndex;
        ++fpsFrameCounter;

        uint64 fpsDurationMs = 0;
        fpsClock.GetDurationInMs(fpsDurationMs);
        if (fpsDurationMs >= 1000)
        {
            frameStats.FPS = fpsFrameCounter;
            fpsFrameCounter = 0;
            fpsClock.Start();
            DBG_MSG("fps %d time: %lld us\n", frameStats.FPS,
                    frameStats.LastRenderTimeUs);
        }

        ClearError();
        return true;
    }

    bool Frame(Engine& engine)
    {
        if (!RequireInitialized())
            return false;

        uint64 elapsedUs = 0;
        frameClock.GetDurationInUs(elapsedUs);
        frameClock.Start();

        frameStats.DeltaSeconds = static_cast<double>(elapsedUs) / 1000000.0;
        frameStats.ElapsedSeconds += frameStats.DeltaSeconds;

        return Update(frameStats.DeltaSeconds, engine) && Render(engine);
    }

    void Resize(uint width, uint height)
    {
        if (device.Get() == nullptr)
        {
            SetError("Engine is not initialized");
            return;
        }

        device.Get()->SetViewport(0, 0, width, height);
        device.Get()->Reset();
        camera.set_ScreenSize(static_cast<float>(width),
                              static_cast<float>(height));
        ClearError();
    }

    bool HandleInput(const InputEvent& event)
    {
        if (event.Type == InputEventType::PointerDrag)
        {
            if (event.Button == InputButton::Left)
            {
                cameraController.OrbitPixels(camera, event.DeltaX,
                                             event.DeltaY);
                ClearError();
                return true;
            }
            if (event.Button == InputButton::Right)
            {
                cameraController.TruckPixels(camera, event.DeltaX,
                                             event.DeltaY);
                ClearError();
                return true;
            }
        }
        else if (event.Type == InputEventType::Scroll)
        {
            cameraController.ZoomLines(camera, event.ScrollDelta);
            ClearError();
            return true;
        }

        SetError("Unsupported input event");
        return false;
    }

    void SetClearColor(const Color& color) { clearColor = color; }

    bool SetLight(uint index, const Light& nextLight)
    {
        if (index != 0)
        {
            SetError("Only light 0 is supported");
            return false;
        }

        light = nextLight;
        if (device.Get() != nullptr)
        {
            device.Get()->SetLight(index, &light);
            device.Get()->LightEnable(index, lightEnabled);
        }

        ClearError();
        return true;
    }

    bool EnableLight(uint index, bool enabled)
    {
        if (index != 0)
        {
            SetError("Only light 0 is supported");
            return false;
        }

        lightEnabled = enabled;
        if (device.Get() != nullptr)
            device.Get()->LightEnable(index, lightEnabled);

        ClearError();
        return true;
    }

    Light* GetLight(uint index)
    {
        if (index != 0)
            return nullptr;
        return &light;
    }

    const Light* GetLight(uint index) const
    {
        if (index != 0)
            return nullptr;
        return &light;
    }

    std::string DispatchCommand(const net::Command& cmd)
    {
        control::SceneControlContext ctx;
        ctx.camera = &camera;
        ctx.model = activeModel;
        ctx.light = &light;
        ctx.clearColor = &clearColor;
        ctx.device = device.Get();
        ctx.inputHandler = [](const InputEvent& event, void* userData)
        { return static_cast<Impl*>(userData)->HandleInput(event); };
        ctx.inputUserData = this;
        return control::DispatchCommand(ctx, cmd);
    }

    void CleanupAfterInitializeFailure()
    {
        const std::string primaryError = lastError;
        Shutdown();
        lastError = primaryError;
    }

    void Shutdown()
    {
        std::string shutdownError;

        StopControlServer();
        scene.ClearNodes();
        activeModel = nullptr;

        device.Reset();

        std::string rendererError;
        if (!renderer.Reset(rendererError) && shutdownError.empty())
            shutdownError = rendererError;

        std::string unloadError;
        if (!module.Reset(unloadError) && shutdownError.empty())
            shutdownError = unloadError;

        if (shutdownError.empty())
            ClearError();
        else
            SetError(shutdownError);

        ResetFrameState();
    }
};

Engine::Engine() : m_impl(new Impl()) {}

Engine::~Engine() { m_impl->Shutdown(); }

bool Engine::LoadConfig(std::string_view filename)
{
    return m_impl->LoadConfig(filename);
}

bool Engine::Initialize(HWIN window) { return m_impl->Initialize(window); }

bool Engine::Initialize(HWIN window, std::string_view configFilename)
{
    if (!LoadConfig(configFilename))
        return false;

    return Initialize(window);
}

bool Engine::LoadScene(std::string_view filename)
{
    return m_impl->LoadScene(filename);
}

bool Engine::StartControlServer(std::string_view bindAddr, int port)
{
    return m_impl->StartControlServer(bindAddr, port);
}

void Engine::StopControlServer() { m_impl->StopControlServer(); }

bool Engine::ControlServerRunning() const
{
    return m_impl->ControlServerRunning();
}

uint Engine::PumpControlCommands() { return m_impl->PumpControlCommands(); }

bool Engine::Update(double deltaSeconds)
{
    return m_impl->Update(deltaSeconds, *this);
}

bool Engine::Render() { return m_impl->Render(*this); }

bool Engine::Frame() { return m_impl->Frame(*this); }

void Engine::Resize(uint width, uint height) { m_impl->Resize(width, height); }

bool Engine::HandleInput(const InputEvent& event)
{
    return m_impl->HandleInput(event);
}

void Engine::SetClearColor(const Color& color) { m_impl->SetClearColor(color); }

const Color& Engine::ClearColor() const { return m_impl->clearColor; }

void Engine::Shutdown() { m_impl->Shutdown(); }

void Engine::SetUpdateCallback(EngineUpdateCallback callback, void* userData)
{
    m_impl->updateCallback = callback;
    m_impl->updateUserData = userData;
}

void Engine::SetRenderCallback(EngineRenderCallback callback, void* userData)
{
    m_impl->renderCallback = callback;
    m_impl->renderUserData = userData;
}

Light Engine::DefaultLight()
{
    Light light = {};
    light.Type = LightType::Point;
    light.Diffuse.r = 1.0f;
    light.Diffuse.g = 1.0f;
    light.Diffuse.b = 0.0f;
    light.Ambient.r = 1.0f;
    light.Ambient.g = 1.0f;
    light.Ambient.b = 1.0f;
    light.Specular.r = 1.0f;
    light.Specular.g = 1.0f;
    light.Specular.b = 1.0f;
    light.Position.X = 5.0f;
    light.Position.Y = 5.0f;
    light.Position.Z = -1.0f;
    light.Attenuation0 = 1.0f;
    light.Range = 1000.0f;
    return light;
}

bool Engine::SetLight(uint index, const Light& light)
{
    return m_impl->SetLight(index, light);
}

bool Engine::EnableLight(uint index, bool enabled)
{
    return m_impl->EnableLight(index, enabled);
}

Light* Engine::GetLight(uint index) { return m_impl->GetLight(index); }

const Light* Engine::GetLight(uint index) const
{
    return m_impl->GetLight(index);
}

void Engine::SetActiveModel(scene::Model* model)
{
    m_impl->activeModel = model;
}

scene::Model* Engine::ActiveModel() const { return m_impl->activeModel; }

std::string Engine::DispatchCommand(const net::Command& cmd)
{
    return m_impl->DispatchCommand(cmd);
}

scene::Scene& Engine::Scene() { return m_impl->scene; }

const scene::Scene& Engine::Scene() const { return m_impl->scene; }

forg::Camera& Engine::Camera() { return m_impl->camera; }

const forg::Camera& Engine::Camera() const { return m_impl->camera; }

IRenderDevice* Engine::Device() const { return m_impl->device.Get(); }

IRenderer* Engine::Renderer() const { return m_impl->renderer.Get(); }

const EngineConfig& Engine::Config() const { return m_impl->config; }

const EngineFrameStats& Engine::FrameStats() const
{
    return m_impl->frameStats;
}

std::string_view Engine::LastError() const { return m_impl->lastError; }

} // namespace forg
