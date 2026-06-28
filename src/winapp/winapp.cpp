// winapp.cpp : Defines the entry point for the application.
//

#include "winapp.h"
#include "Viewport.h"
#include "stdafx.h"

#include "forg.h"
#include "forg/script/yaml/YAMLParser.h"

#include <string>

namespace {

struct AppConfig
{
    int Width = 100;
    int Height = 100;
    int X = 10;
    int Y = 10;
    bool ControlEnabled = false;
    int ControlPort = 8080;
};

void ShowError(LPCTSTR message)
{
    MessageBox(NULL, message, _T("Error"), MB_OK | MB_ICONERROR);
}

void ShowEngineError(const forg::Engine& engine)
{
#ifdef _UNICODE
    MessageBoxA(NULL, std::string(engine.LastError()).c_str(), "Error",
                MB_OK | MB_ICONERROR);
#else
    ShowError(std::string(engine.LastError()).c_str());
#endif
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

    forg::script::yaml::YAMLParser parser;
    parser.Open("config.yml");
    forg::script::yaml::YAMLDocument* document = parser.Parse();

    if (!document)
        return config;

    if (const char* width = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "width"))
        config.Width = atoi(width);

    if (const char* height = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "height"))
        config.Height = atoi(height);

    if (const char* posx = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "posx"))
        config.X = atoi(posx);

    if (const char* posy = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "posy"))
        config.Y = atoi(posy);

    if (const char* enabled = forg::script::yaml::FindNodeAttributeValue(
            document, "controlserver", "enabled"))
        config.ControlEnabled = std::string(enabled) == "true";

    if (const char* port = forg::script::yaml::FindNodeAttributeValue(
            document, "controlserver", "port"))
        config.ControlPort = atoi(port);

    return config;
}

int Run(forg::Engine& engine)
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
            engine.Frame();
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
    forg::Engine engine;

    int result = 1;
    {
        Viewport viewport;
        if (viewport.Create(engine, config.X, config.Y, config.Width,
                            config.Height, NULL) != 0)
        {
            if (std::string(engine.LastError()).empty())
                ShowLastError();
            else
                ShowEngineError(engine);
            return 1;
        }

        if (config.ControlEnabled &&
            !engine.StartControlServer("127.0.0.1", config.ControlPort))
        {
            ShowEngineError(engine);
        }

        viewport.ShowWindow(nCmdShow);
        viewport.SetFocus();
        result = Run(engine);
    }

    engine.Shutdown();

    return result;
}
