#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "forg.h"
#include "forg/fs/Filesystem.h"
#include "forg/script/yaml/YAMLParser.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits.h>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {

struct AppConfig
{
    int Width = 800;
    int Height = 600;
    int X = 100;
    int Y = 100;
    bool ControlEnabled = false;
    int ControlPort = 8080;
};

struct AppState
{
    forg::Engine Engine;
#ifdef FORG_USE_FREETYPE
    forg::Font* Font = nullptr;
#endif
};

bool ChangeToExecutableDirectory()
{
    char path[PATH_MAX] = {};
    ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length < 0)
        return false;

    path[length] = '\0';
    std::filesystem::path executablePath(path);
    std::filesystem::path directory = executablePath.parent_path();
    if (directory.empty())
        return true;

    return chdir(directory.c_str()) == 0;
}

bool LoadConfig(AppConfig& config)
{
    forg::script::yaml::YAMLParser parser;
    if (!parser.Open("config.yml"))
    {
        std::cerr << "Unable to load config.yml!\n";
        return false;
    }

    forg::script::yaml::YAMLDocument* document = parser.Parse();
    if (document == nullptr)
    {
        std::cerr << "Unable to parse config.yml!\n";
        parser.Close();
        return false;
    }

    if (const char* width = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "width"))
        config.Width = std::atoi(width);

    if (const char* height = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "height"))
        config.Height = std::atoi(height);

    if (const char* posx = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "posx"))
        config.X = std::atoi(posx);

    if (const char* posy = forg::script::yaml::FindNodeAttributeValue(
            document, "window", "posy"))
        config.Y = std::atoi(posy);

    if (const char* enabled = forg::script::yaml::FindNodeAttributeValue(
            document, "controlserver", "enabled"))
        config.ControlEnabled = std::string(enabled) == "true";

    if (const char* port = forg::script::yaml::FindNodeAttributeValue(
            document, "controlserver", "port"))
        config.ControlPort = std::atoi(port);

    parser.Close();
    return true;
}

bool RenderEngineFrame(forg::Engine& engine, void* userData)
{
    AppState* state = static_cast<AppState*>(userData);
    if (state == nullptr || engine.Device() == nullptr)
        return false;

    engine.Device()->SetRenderState(forg::RenderStates_Lighting, false);

#ifdef FORG_USE_FREETYPE
    if (state->Font)
    {
        forg::Viewport vp;
        engine.Device()->GetViewport(&vp);
        forg::Rectangle r = {0, 0, static_cast<int>(vp.Width),
                             static_cast<int>(vp.Height)};

        char text[512];
        std::string_view rendererName = engine.RendererPluginName();
        if (rendererName.empty())
            rendererName = "Unknown Renderer";

        std::snprintf(
            text, sizeof(text),
            "%u fps   renderer: %.*s   camera pos: %.3f %.3f "
            "%.3f  dir: %.3f %.3f %.3f",
            engine.FrameStats().FPS, static_cast<int>(rendererName.size()),
            rendererName.data(), engine.Camera().get_Position().X,
            engine.Camera().get_Position().Y, engine.Camera().get_Position().Z,
            engine.Camera().get_Target().X, engine.Camera().get_Target().Y,
            engine.Camera().get_Target().Z);

        state->Font->DrawText2(text, -1, &r, 0,
                               forg::Color4b(255, 255, 255, 255));
    }
#endif

    return true;
}

void HandleMouseMotion(forg::Engine& engine, const SDL_MouseMotionEvent& event)
{
    if (event.state & SDL_BUTTON_LMASK)
    {
        engine.HandleInput({forg::InputEventType::PointerDrag,
                            forg::InputButton::Left,
                            static_cast<float>(event.xrel),
                            static_cast<float>(event.yrel), 0.0f});
    }
    else if (event.state & SDL_BUTTON_RMASK)
    {
        engine.HandleInput({forg::InputEventType::PointerDrag,
                            forg::InputButton::Right,
                            static_cast<float>(event.xrel),
                            static_cast<float>(event.yrel), 0.0f});
    }
}

void HandleMouseWheel(forg::Engine& engine, const SDL_MouseWheelEvent& event)
{
    float delta = static_cast<float>(event.y);
#if SDL_VERSION_ATLEAST(2, 0, 18)
    if (event.preciseY != 0.0f)
        delta = event.preciseY;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
    if (event.direction == SDL_MOUSEWHEEL_FLIPPED)
        delta = -delta;
#endif

    engine.HandleInput({forg::InputEventType::Scroll, forg::InputButton::None,
                        0.0f, 0.0f, delta});
}

bool InitializeEngine(AppState& state, SDL_Window* window,
                      const AppConfig& config)
{
    state.Engine.Filesystem().Mount("data:", "data",
                                    forg::fs::MountPermissions::ReadOnly);

    if (!state.Engine.Initialize((forg::HWIN)window, "config.yml"))
    {
        std::cerr << state.Engine.LastError() << "\n";
        return false;
    }
    state.Engine.SetRenderCallback(&RenderEngineFrame, &state);

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    state.Engine.Resize(static_cast<forg::u32>(width),
                        static_cast<forg::u32>(height));

    if (!state.Engine.LoadScene("scene.yml"))
    {
        std::cerr << state.Engine.LastError() << "\n";
        return false;
    }

    if (!state.Engine.LoadScene("data:ui/dialog.yml", 1))
    {
        std::cerr << state.Engine.LastError() << "\n";
        return false;
    }

#ifdef FORG_USE_FREETYPE
    std::filesystem::path fontPath;
    const std::string fontPathText =
        state.Engine.Filesystem().ResolveReadPath(
            "data:fonts/Roboto-Regular.ttf", fontPath)
            ? fontPath.string()
            : std::string();
    forg::FontDescription fd = {20, 0, 0, 1, false, 0, 0, 0, 0, (""), ("")};
    std::snprintf(fd.FontPath, sizeof(fd.FontPath), "%s", fontPathText.c_str());
    state.Font = forg::Font::CreateIndirect(state.Engine.Device(), &fd);
#endif

    if (config.ControlEnabled)
    {
        if (!state.Engine.StartControlServer("127.0.0.1", config.ControlPort))
        {
            std::cerr << "Control server failed to start on port "
                      << config.ControlPort << ": " << state.Engine.LastError()
                      << "\n";
        }
        else
        {
            std::cout << "Control server listening on http://127.0.0.1:"
                      << config.ControlPort << "\n";
        }
    }

    return true;
}

int Run(AppState& state)
{
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
#if SDL_VERSION_ATLEAST(2, 0, 4)
                SDL_CaptureMouse(SDL_TRUE);
#endif
                break;
            case SDL_MOUSEBUTTONUP:
#if SDL_VERSION_ATLEAST(2, 0, 4)
                if (SDL_GetMouseState(nullptr, nullptr) == 0)
                    SDL_CaptureMouse(SDL_FALSE);
#endif
                break;
            case SDL_MOUSEMOTION:
                HandleMouseMotion(state.Engine, event.motion);
                break;
            case SDL_MOUSEWHEEL:
                HandleMouseWheel(state.Engine, event.wheel);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    running = false;
                else if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    if (width > 0 && height > 0)
                    {
                        state.Engine.Resize(static_cast<forg::u32>(width),
                                            static_cast<forg::u32>(height));
                    }
                }
                break;
            default:
                break;
            }
        }

        if (running && !state.Engine.Frame())
        {
            std::cerr << state.Engine.LastError() << "\n";
            return 1;
        }
    }

    return 0;
}

} // namespace

int main(int, char*[])
{
    if (!ChangeToExecutableDirectory())
    {
        std::cerr << "Unable to change to executable directory.\n";
        return 1;
    }

    AppConfig config;
    if (!LoadConfig(config))
        return 1;

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "View", config.X, config.Y, config.Width, config.Height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    AppState state;
    int result = 1;
    if (InitializeEngine(state, window, config))
        result = Run(state);

#ifdef FORG_USE_FREETYPE
    delete state.Font;
    state.Font = nullptr;
#endif
    state.Engine.SetRenderCallback(nullptr, nullptr);
    state.Engine.Shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return result;
}
