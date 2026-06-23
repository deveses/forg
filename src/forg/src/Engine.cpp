#include "forg_pch.h"

#include "forg/Engine.h"

#include "forg/rendering/IRenderer.h"
#include "forg/scene/Scene.h"
#include "forg/script/yaml/YAMLParser.h"

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
    std::string lastError;

    bool IsInitialized() const
    {
        return !module.Empty() || renderer.Get() != nullptr ||
               device.Get() != nullptr;
    }

    void SetError(std::string_view message) { lastError.assign(message); }

    void ClearError() { lastError.clear(); }

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

        if (script::yaml::YAMLNode* rendererNode =
                document->FindNode("renderer"))
        {
            if (script::yaml::YAMLNode* driver =
                    rendererNode->FindAttribute("driver"))
            {
                nextConfig.RendererDriver = driver->GetContent().c_str();
            }
        }

        if (script::yaml::YAMLNode* windowNode = document->FindNode("window"))
        {
            if (script::yaml::YAMLNode* width =
                    windowNode->FindAttribute("width"))
            {
                if (!ParsePositiveUint(width->GetContent().c_str(),
                                       nextConfig.BackBufferWidth))
                {
                    SetError("Invalid window.width in config");
                    parser.Close();
                    return false;
                }
            }

            if (script::yaml::YAMLNode* height =
                    windowNode->FindAttribute("height"))
            {
                if (!ParsePositiveUint(height->GetContent().c_str(),
                                       nextConfig.BackBufferHeight))
                {
                    SetError("Invalid window.height in config");
                    parser.Close();
                    return false;
                }
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

        ClearError();
        return true;
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

        scene.ClearNodes();

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

void Engine::Shutdown() { m_impl->Shutdown(); }

scene::Scene& Engine::Scene() { return m_impl->scene; }

const scene::Scene& Engine::Scene() const { return m_impl->scene; }

IRenderDevice* Engine::Device() const { return m_impl->device.Get(); }

IRenderer* Engine::Renderer() const { return m_impl->renderer.Get(); }

const EngineConfig& Engine::Config() const { return m_impl->config; }

std::string_view Engine::LastError() const { return m_impl->lastError; }

} // namespace forg
