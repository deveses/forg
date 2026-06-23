#include "forg_pch.h"

#include "forg/Engine.h"

#include "forg/rendering/IRenderer.h"
#include "forg/scene/Scene.h"
#include "forg/script/yaml/YAMLParser.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <sstream>

#ifdef FORG_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace forg {

namespace {

bool ParsePositiveUint(const char* text, uint& value)
{
    if (text == nullptr || text[0] == '\0')
        return false;

    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || parsed == 0 ||
        parsed > UINT_MAX)
    {
        return false;
    }

    value = static_cast<uint>(parsed);
    return true;
}

#ifdef FORG_PLATFORM_WINDOWS
using PluginModule = HMODULE;

PluginModule OpenPluginModule(const std::string& driver, std::string& error)
{
    HMODULE module = LoadLibraryA(driver.c_str());
    if (module == nullptr)
    {
        std::ostringstream stream;
        stream << "Unable to load renderer <" << driver << ">";
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

std::string PluginPath(const std::string& driver)
{
    if (driver.find('/') != std::string::npos)
        return driver;

    return "./" + driver;
}

PluginModule OpenPluginModule(const std::string& driver, std::string& error)
{
    const std::string path = PluginPath(driver);
    void* module = dlopen(path.c_str(), RTLD_NOW);
    if (module == nullptr)
    {
        std::ostringstream stream;
        stream << "Unable to load renderer <" << driver << ">: " << dlerror();
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

} // namespace

struct Engine::Impl
{
    EngineConfig config;
    bool configLoaded = false;
    scene::Scene scene;
    PluginModule module = nullptr;
    RendererPluginBinding binding;
    IRenderer* renderer = nullptr;
    IRenderDevice* device = nullptr;
    std::string lastError;

    bool IsInitialized() const
    {
        return module != nullptr || renderer != nullptr || device != nullptr;
    }

    void SetError(const std::string& message) { lastError = message; }

    void ClearError() { lastError.clear(); }

    bool LoadConfig(const char* filename)
    {
        if (IsInitialized())
        {
            SetError("Cannot load config while engine is initialized");
            return false;
        }

        if (filename == nullptr || filename[0] == '\0')
        {
            SetError("Config filename is empty");
            return false;
        }

        script::yaml::YAMLParser parser;
        if (!parser.Open(filename))
        {
            std::ostringstream stream;
            stream << "Unable to open config <" << filename << ">";
            SetError(stream.str());
            return false;
        }

        script::yaml::YAMLDocument* document = parser.Parse();
        if (document == nullptr)
        {
            std::ostringstream stream;
            stream << "Unable to parse config <" << filename << ">";
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
        module = OpenPluginModule(config.RendererDriver, error);
        if (module == nullptr)
        {
            SetError(error);
            return false;
        }

        auto getDescriptor = PluginSymbol<PFGETRENDERERPLUGINDESCRIPTOR>(
            module, "forgGetRendererPluginDescriptor");
        auto createRenderer =
            PluginSymbol<PFCREATERENDERER>(module, "forgCreateRenderer");

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

        status = CreateRendererFromPlugin(binding, renderer);
        if (status != RendererPluginStatus::Ok)
        {
            std::ostringstream stream;
            stream << "Unable to create renderer <" << config.RendererDriver
                   << ">: " << RendererPluginStatusName(status);
            SetError(stream.str());
            CleanupAfterInitializeFailure();
            return false;
        }

        RENDER_PARAMETERS parameters = {};
        parameters.BackBufferWidth = config.BackBufferWidth;
        parameters.BackBufferHeight = config.BackBufferHeight;
        parameters.PresentationInterval = 0;

        device = renderer->CreateDevice(window, &parameters);
        if (device == nullptr)
        {
            SetError("Unable to create render device");
            CleanupAfterInitializeFailure();
            return false;
        }

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

        if (device != nullptr)
        {
            device->Release();
            device = nullptr;
        }

        if (renderer != nullptr)
        {
            const RendererPluginStatus status =
                DestroyRendererFromPlugin(binding, renderer);
            if (status != RendererPluginStatus::Ok && shutdownError.empty())
            {
                std::ostringstream stream;
                stream << "Unable to destroy renderer: "
                       << RendererPluginStatusName(status);
                shutdownError = stream.str();
            }
            renderer = nullptr;
        }

        if (module != nullptr)
        {
            std::string unloadError;
            if (!ClosePluginModule(module, unloadError) && shutdownError.empty())
                shutdownError = unloadError;
            module = nullptr;
        }

        binding = {};

        if (shutdownError.empty())
            ClearError();
        else
            SetError(shutdownError);
    }
};

Engine::Engine() : m_impl(new Impl()) {}

Engine::~Engine() { m_impl->Shutdown(); }

bool Engine::LoadConfig(const char* filename)
{
    return m_impl->LoadConfig(filename);
}

bool Engine::Initialize(HWIN window) { return m_impl->Initialize(window); }

bool Engine::Initialize(HWIN window, const char* configFilename)
{
    if (!LoadConfig(configFilename))
        return false;

    return Initialize(window);
}

void Engine::Shutdown() { m_impl->Shutdown(); }

scene::Scene& Engine::Scene() { return m_impl->scene; }

const scene::Scene& Engine::Scene() const { return m_impl->scene; }

IRenderDevice* Engine::Device() const { return m_impl->device; }

IRenderer* Engine::Renderer() const { return m_impl->renderer; }

const EngineConfig& Engine::Config() const { return m_impl->config; }

const char* Engine::LastError() const { return m_impl->lastError.c_str(); }

} // namespace forg
