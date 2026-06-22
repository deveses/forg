// winapp.cpp : Defines the entry point for the application.
//

#include "winapp.h"
#include "Viewport.h"
#include "stdafx.h"

#include "forg.h"
#include "forg/script/yaml/YAMLParser.h"

#include <cstring>

namespace {

struct AppConfig
{
    const char* DefaultDriver = "glrenderer.dll";
    char Driver[MAX_PATH] = {};
    int Width = 100;
    int Height = 100;
    int X = 10;
    int Y = 10;
};

struct LoadedRendererPlugin
{
    HMODULE Module = NULL;
    forg::RendererPluginBinding Binding;
    forg::IRenderer* Renderer = nullptr;
};

void ShowError(LPCTSTR message)
{
    MessageBox(NULL, message, _T("Error"), MB_OK | MB_ICONERROR);
}

void ShowRendererPluginError(forg::RendererPluginStatus status)
{
    switch (status)
    {
    case forg::RendererPluginStatus::MissingSymbols:
        ShowError(_T("The renderer plugin does not expose renderer symbols."));
        break;
    case forg::RendererPluginStatus::NullDescriptor:
        ShowError(_T("The renderer plugin returned a null descriptor."));
        break;
    case forg::RendererPluginStatus::TruncatedDescriptor:
        ShowError(_T("The renderer plugin descriptor is truncated."));
        break;
    case forg::RendererPluginStatus::UnsupportedVersion:
        ShowError(_T("The renderer plugin version is unsupported."));
        break;
    case forg::RendererPluginStatus::MissingCreate:
        ShowError(_T("The renderer plugin is missing its create callback."));
        break;
    case forg::RendererPluginStatus::MissingDestroy:
        ShowError(_T("The renderer plugin is missing its destroy callback."));
        break;
    case forg::RendererPluginStatus::FactoryFailed:
        ShowError(_T("The renderer plugin failed to create a renderer."));
        break;
    case forg::RendererPluginStatus::DestroyFailed:
        ShowError(_T("The renderer plugin failed to destroy the renderer."));
        break;
    case forg::RendererPluginStatus::Ok:
        break;
    }
}

void ShowLastError()
{
    LPVOID message = NULL;
    DWORD error = GetLastError();

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      reinterpret_cast<LPTSTR>(&message), 0, NULL) != 0)
    {
        MessageBox(NULL, static_cast<LPCTSTR>(message), _T("Error"),
                   MB_OK | MB_ICONERROR);
        LocalFree(message);
    }
    else
    {
        ShowError(_T("An unknown Win32 error occurred."));
    }
}

bool ChangeToExecutableDirectory()
{
    TCHAR path[MAX_PATH] = {};
    DWORD length = GetModuleFileName(NULL, path, MAX_PATH);
    if (length == 0)
        return false;

    if (length >= MAX_PATH)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return false;
    }

    TCHAR* lastSlash = _tcsrchr(path, _T('\\'));
    if (lastSlash == nullptr)
        return true;

    *lastSlash = _T('\0');
    return SetCurrentDirectory(path) != FALSE;
}

AppConfig LoadConfig()
{
    AppConfig config;
    strncpy_s(config.Driver, config.DefaultDriver, _TRUNCATE);

    forg::script::yaml::YAMLParser parser;
    parser.Open("config.yml");
    forg::script::yaml::YAMLDocument* document = parser.Parse();

    if (!document)
        return config;

    if (forg::script::yaml::YAMLNode* node = document->FindNode("renderer"))
    {
        if (forg::script::yaml::YAMLNode* driver =
                node->FindAttribute("driver"))
        {
            strncpy_s(config.Driver, driver->GetContent().c_str(), _TRUNCATE);
        }
    }

    if (forg::script::yaml::YAMLNode* node = document->FindNode("window"))
    {
        if (forg::script::yaml::YAMLNode* width = node->FindAttribute("width"))
            config.Width = atoi(width->GetContent().c_str());

        if (forg::script::yaml::YAMLNode* height =
                node->FindAttribute("height"))
            config.Height = atoi(height->GetContent().c_str());

        if (forg::script::yaml::YAMLNode* posx = node->FindAttribute("posx"))
            config.X = atoi(posx->GetContent().c_str());

        if (forg::script::yaml::YAMLNode* posy = node->FindAttribute("posy"))
            config.Y = atoi(posy->GetContent().c_str());
    }

    return config;
}

bool LoadRendererPlugin(const char* driver, LoadedRendererPlugin& plugin)
{
    plugin = {};
    plugin.Module = LoadLibraryA(driver);
    if (plugin.Module == NULL)
    {
        ShowLastError();
        return false;
    }

    auto getDescriptor = reinterpret_cast<forg::PFGETRENDERERPLUGINDESCRIPTOR>(
        GetProcAddress(plugin.Module, "forgGetRendererPluginDescriptor"));
    auto legacyCreateRenderer = reinterpret_cast<forg::PFCREATERENDERER>(
        GetProcAddress(plugin.Module, "forgCreateRenderer"));

    forg::RendererPluginStatus status = forg::ProbeRendererPlugin(
        getDescriptor, legacyCreateRenderer, plugin.Binding);
    if (status != forg::RendererPluginStatus::Ok)
    {
        ShowRendererPluginError(status);
        FreeLibrary(plugin.Module);
        plugin = {};
        return false;
    }

    status = forg::CreateRendererFromPlugin(plugin.Binding, plugin.Renderer);
    if (status != forg::RendererPluginStatus::Ok)
    {
        ShowRendererPluginError(status);
        FreeLibrary(plugin.Module);
        plugin = {};
        return false;
    }

    return true;
}

void UnloadRendererPlugin(LoadedRendererPlugin& plugin)
{
    if (plugin.Renderer != nullptr)
    {
        const forg::RendererPluginStatus status =
            forg::DestroyRendererFromPlugin(plugin.Binding, plugin.Renderer);
        if (status != forg::RendererPluginStatus::Ok)
            ShowRendererPluginError(status);
        plugin.Renderer = nullptr;
    }

    if (plugin.Module != NULL)
    {
        FreeLibrary(plugin.Module);
        plugin.Module = NULL;
    }
}

int Run(Viewport& viewport)
{
    MSG msg = {};
    bool running = true;

    while (running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (running)
            viewport.OnPaint();
    }

    return static_cast<int>(msg.wParam);
}

} // namespace

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX commonControls = {};
    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&commonControls);

    if (!ChangeToExecutableDirectory())
    {
        ShowLastError();
        return 1;
    }

    AppConfig config = LoadConfig();
    LoadedRendererPlugin plugin;
    if (!LoadRendererPlugin(config.Driver, plugin))
        return 1;

    int result = 1;
    {
        Viewport viewport;
        if (viewport.Create(plugin.Renderer, config.X, config.Y, config.Width,
                            config.Height, NULL) != 0)
        {
            ShowLastError();
            UnloadRendererPlugin(plugin);
            return 1;
        }

        viewport.ShowWindow(nCmdShow);
        viewport.SetFocus();
        result = Run(viewport);
    }

    UnloadRendererPlugin(plugin);

    return result;
}
