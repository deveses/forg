// main.mm : macOS port of the Win32 sample app (src/winapp).
// Reads config.yml to pick the renderer plugin and window geometry, loads the
// scene from scene.yml, then renders it in a continuous loop.

// Cocoa must come first: forg's base.h defines macros (null, IN, OUT)
// that break the system headers if they are seen earlier.
#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <MacTypes.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>

#include "forg.h"
#include "forg/script/yaml/YAMLParser.h"

struct AppSettings
{
    int winWidth = 100;
    int winHeight = 100;
    int winX = 10;
    int winY = 10;
    bool controlEnabled = false;
    int controlPort = 8080;
};

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    AppSettings m_settings;
    forg::Engine m_engine;
    forg::Font* m_font;

    NSWindow* m_window;
    NSView* m_view;
    NSTimer* m_timer;
    id m_event_monitor;
}
- (instancetype)initWithSettings:(const AppSettings&)settings;
- (bool)renderEngineFrame:(forg::Engine&)engine;
@end

static bool RenderEngineFrame(forg::Engine& engine, void* userData)
{
    AppDelegate* delegate = static_cast<AppDelegate*>(userData);
    if (delegate == nil)
        return false;

    return [delegate renderEngineFrame:engine];
}

@implementation AppDelegate

- (instancetype)initWithSettings:(const AppSettings&)settings
{
    self = [super init];
    if (self)
    {
        m_settings = settings;
        m_font = nullptr;
    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    int winWidth = m_settings.winWidth;
    int winHeight = m_settings.winHeight;

    // config posx/posy are from the top-left corner (Win32 convention)
    CGFloat screenHeight = [NSScreen mainScreen].frame.size.height;
    NSRect contentRect =
        NSMakeRect(m_settings.winX, screenHeight - m_settings.winY - winHeight,
                   winWidth, winHeight);

    NSWindowStyleMask style =
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
        NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    m_window = [[NSWindow alloc] initWithContentRect:contentRect
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    [m_window setTitle:@"View"];

    m_view = [m_window contentView];
    [m_view setWantsLayer:YES];
    [m_view setPostsFrameChangedNotifications:YES];

    if (!m_engine.Initialize((forg::HWIN)m_view, "config.yml"))
    {
        std::cerr << m_engine.LastError() << "\n";
        [NSApp terminate:nil];
        return;
    }
    m_engine.SetRenderCallback(&RenderEngineFrame, self);

    if (!m_engine.LoadScene("scene.yml"))
    {
        std::cerr << m_engine.LastError() << "\n";
        [NSApp terminate:nil];
        return;
    }

    // Optional runtime control server: a background HTTP endpoint queues
    // commands that are applied on this (main) thread by Engine::Frame().
    if (m_settings.controlEnabled)
    {
        if (!m_engine.StartControlServer("127.0.0.1", m_settings.controlPort))
        {
            std::cerr << "Control server failed to start on port "
                      << m_settings.controlPort << ": " << m_engine.LastError()
                      << "\n";
        }
        else
        {
            std::cout << "Control server listening on http://127.0.0.1:"
                      << m_settings.controlPort << "\n";
        }
    }

#ifdef FORG_USE_FREETYPE
    forg::FontDescription fd = {12,
                                0,
                                0,
                                1,
                                false,
                                0,
                                0,
                                0,
                                0,
                                (""),
                                ("data/fonts/Roboto-Regular.ttf")};
    m_font = forg::Font::CreateIndirect(m_engine.Device(), &fd);
#endif

    if (!m_engine.LoadScene("data/ui/dialog.yml", 1))
    {
        std::cerr << m_engine.LastError() << "\n";
        [NSApp terminate:nil];
        return;
    }

    [self onResize];

    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(viewFrameChanged:)
               name:NSViewFrameDidChangeNotification
             object:m_view];

    m_timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
                                               target:self
                                             selector:@selector(onIdle:)
                                             userInfo:nil
                                              repeats:YES];

    [m_window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    // Turntable camera controls: left-drag orbits, right-drag trucks, scroll
    // zooms. A local monitor sees the deltas without subclassing the renderer's
    // content view.
    NSEventMask mask = NSEventMaskLeftMouseDragged |
                       NSEventMaskRightMouseDragged | NSEventMaskScrollWheel;
    // MRC build: __unsafe_unretained avoids a retain cycle; the monitor is
    // removed in applicationWillTerminate, before the delegate is deallocated.
    __unsafe_unretained AppDelegate* unretainedSelf = self;
    m_event_monitor = [NSEvent
        addLocalMonitorForEventsMatchingMask:mask
                                     handler:^NSEvent*(NSEvent* event) {
                                       [unretainedSelf handleCameraEvent:event];
                                       return event;
                                     }];
}

// Maps mouse/scroll deltas onto the engine's normalized input API.
- (void)handleCameraEvent:(NSEvent*)event
{
    switch (event.type)
    {
    case NSEventTypeLeftMouseDragged:
        m_engine.HandleInput({forg::InputEventType::PointerDrag,
                              forg::InputButton::Left, (float)event.deltaX,
                              (float)event.deltaY, 0.0f});
        break;

    case NSEventTypeRightMouseDragged:
        m_engine.HandleInput({forg::InputEventType::PointerDrag,
                              forg::InputButton::Right, (float)event.deltaX,
                              (float)event.deltaY, 0.0f});
        break;

    case NSEventTypeScrollWheel:
        m_engine.HandleInput({forg::InputEventType::Scroll,
                              forg::InputButton::None, 0.0f, 0.0f,
                              (float)event.scrollingDeltaY});
        break;

    default:
        break;
    }
}

// port of Viewport::OnSize
- (void)onResize
{
    NSSize size = m_view.bounds.size;

    m_engine.Resize((forg::uint)size.width, (forg::uint)size.height);
}

- (void)viewFrameChanged:(NSNotification*)notification
{
    [self onResize];
}

- (bool)renderEngineFrame:(forg::Engine&)engine
{
    forg::IRenderDevice* device = engine.Device();
    if (device == nullptr)
        return false;

    device->SetRenderState(forg::RenderStates_Lighting, false);

    if (m_font)
    {
        forg::Viewport vp;
        device->GetViewport(&vp);
        forg::Rectangle r = {0, 0, static_cast<int>(vp.Width),
                             static_cast<int>(vp.Height)};

        char str[512];
        std::snprintf(
            str, sizeof(str),
            "%u fps   camera pos: %.3f %.3f %.3f  dir: %.3f %.3f %.3f",
            engine.FrameStats().FPS, engine.Camera().get_Position().X,
            engine.Camera().get_Position().Y, engine.Camera().get_Position().Z,
            engine.Camera().get_Target().X, engine.Camera().get_Target().Y,
            engine.Camera().get_Target().Z);

        m_font->DrawText2(str, -1, &r, 0, forg::Color4b(255, 255, 0, 255));
    }

    return true;
}

// port of Viewport::OnPaint + CWinApp::OnIdle
- (void)onIdle:(NSTimer*)timer
{
    m_engine.Frame();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    [m_timer invalidate];
    m_timer = nil;

    if (m_event_monitor)
    {
        [NSEvent removeMonitor:m_event_monitor];
        m_event_monitor = nil;
    }

    m_engine.SetRenderCallback(nullptr, nullptr);

    if (m_font)
    {
        delete m_font;
        m_font = nullptr;
    }

    m_engine.Shutdown();
}

@end

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////

static void ChangeToResourcesDirectory()
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resBundle = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char bundlePath[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resBundle, TRUE, (UInt8*)bundlePath,
                                         PATH_MAX))
    {
        std::cout << "resource path: " << bundlePath << "\n";
        chdir(bundlePath);
    }
    CFRelease(resBundle);
}

// port of the config parsing in CWinApp::InitApplication
static bool LoadSettings(AppSettings& settings)
{
    forg::script::yaml::YAMLParser config;

    config.Open("config.yml");
    forg::script::yaml::YAMLDocument* yaml_doc = config.Parse();

    if (yaml_doc == 0)
    {
        std::cerr << "Unable to load config.yml!\n";
        return false;
    }

    if (const char* width = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "window", "width"))
        settings.winWidth = atoi(width);

    if (const char* height = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "window", "height"))
        settings.winHeight = atoi(height);

    if (const char* posx = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "window", "posx"))
        settings.winX = atoi(posx);

    if (const char* posy = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "window", "posy"))
        settings.winY = atoi(posy);

    if (const char* enabled = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "controlserver", "enabled"))
    {
        settings.controlEnabled = std::string(enabled) == "true";
    }

    if (const char* port = forg::script::yaml::FindNodeAttributeValue(
            yaml_doc, "controlserver", "port"))
        settings.controlPort = atoi(port);

    return true;
}

int main(int, char*[])
{
    ChangeToResourcesDirectory();

    AppSettings settings;
    if (!LoadSettings(settings))
    {
        return 1;
    }

    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    AppDelegate* delegate = [[AppDelegate alloc] initWithSettings:settings];
    [app setDelegate:delegate];

    [app run];

    return 0;
}
