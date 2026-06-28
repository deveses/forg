// main.mm : macOS port of the Win32 sample app (src/winapp).
// Reads config.yml to pick the renderer plugin and window geometry, loads the
// scene from scene.yml, then renders it in a continuous loop.

// Cocoa must come first: forg's base.h defines macros (null, IN, OUT)
// that break the system headers if they are seen earlier.
#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <MacTypes.h>
#include <unistd.h>

#include <iostream>
#include <string>

// Pulled in before forg.h so the C++ threading headers are parsed before
// base.h defines its null/IN/OUT macros.
#include "forg/net/CommandQueue.h"
#include "forg/net/HttpControlServer.h"

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
    forg::CameraOrbitController m_camera_controller;

    forg::net::CommandQueue* m_cmd_queue;
    forg::net::HttpControlServer* m_control_server;

    NSWindow* m_window;
    NSView* m_view;
    NSTimer* m_timer;
    id m_event_monitor;
}
- (instancetype)initWithSettings:(const AppSettings&)settings;
@end

@implementation AppDelegate

- (instancetype)initWithSettings:(const AppSettings&)settings
{
    self = [super init];
    if (self)
    {
        m_settings = settings;
        m_cmd_queue = 0;
        m_control_server = 0;
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

    if (!m_engine.LoadScene("scene.yml"))
    {
        std::cerr << m_engine.LastError() << "\n";
        [NSApp terminate:nil];
        return;
    }

    // Optional runtime control server: a background HTTP endpoint queues
    // commands that are applied on this (main) thread in onIdle:.
    if (m_settings.controlEnabled)
    {
        m_cmd_queue = new forg::net::CommandQueue();
        m_control_server = new forg::net::HttpControlServer(
            "127.0.0.1", m_settings.controlPort, *m_cmd_queue);
        if (!m_control_server->Start())
        {
            std::cerr << "Control server failed to start on port "
                      << m_settings.controlPort << "\n";
        }
        else
        {
            std::cout << "Control server listening on http://127.0.0.1:"
                      << m_settings.controlPort << "\n";
        }
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

// Maps mouse/scroll deltas onto the existing Camera movement primitives.
- (void)handleCameraEvent:(NSEvent*)event
{
    switch (event.type)
    {
    case NSEventTypeLeftMouseDragged:
        m_camera_controller.OrbitPixels(m_engine.Camera(),
                                        (float)event.deltaX,
                                        (float)event.deltaY);
        break;

    case NSEventTypeRightMouseDragged:
        m_camera_controller.TruckPixels(m_engine.Camera(),
                                        (float)event.deltaX,
                                        (float)event.deltaY);
        break;

    case NSEventTypeScrollWheel:
        m_camera_controller.ZoomLines(m_engine.Camera(),
                                      (float)event.scrollingDeltaY);
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

// port of Viewport::OnPaint + CWinApp::OnIdle
- (void)onIdle:(NSTimer*)timer
{
    // Apply any commands queued by the control server thread (main-thread
    // only).
    if (m_cmd_queue)
    {
        forg::net::QueueItem item;
        while (m_cmd_queue->TryPop(item))
        {
            std::string body = m_engine.DispatchCommand(item.cmd);
            if (item.reply)
            {
                item.reply->set_value(body);
            }
        }
    }

    m_engine.Frame();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
    // Stop the server (joins its thread) before tearing down the scene it
    // touches, then free the queue it pushed to.
    if (m_control_server)
    {
        m_control_server->Stop();
        delete m_control_server;
        m_control_server = 0;
    }
    if (m_cmd_queue)
    {
        delete m_cmd_queue;
        m_cmd_queue = 0;
    }

    [m_timer invalidate];
    m_timer = nil;

    if (m_event_monitor)
    {
        [NSEvent removeMonitor:m_event_monitor];
        m_event_monitor = nil;
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

    if (forg::script::yaml::YAMLNode* yaml_node = yaml_doc->FindNode("window"))
    {
        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("width"))
        {
            settings.winWidth = atoi(yaml_att->GetContent().c_str());
        }

        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("height"))
        {
            settings.winHeight = atoi(yaml_att->GetContent().c_str());
        }

        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("posx"))
        {
            settings.winX = atoi(yaml_att->GetContent().c_str());
        }

        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("posy"))
        {
            settings.winY = atoi(yaml_att->GetContent().c_str());
        }
    }

    if (forg::script::yaml::YAMLNode* yaml_node =
            yaml_doc->FindNode("controlserver"))
    {
        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("enabled"))
        {
            settings.controlEnabled =
                (std::string(yaml_att->GetContent().c_str()) == "true");
        }

        if (forg::script::yaml::YAMLNode* yaml_att =
                yaml_node->FindAttribute("port"))
        {
            settings.controlPort = atoi(yaml_att->GetContent().c_str());
        }
    }

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
