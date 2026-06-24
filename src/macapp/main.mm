// main.mm : macOS port of the Win32 sample app (src/winapp).
// Reads config.yml to pick the renderer plugin and window geometry,
// then renders the demo scene (lit cylinder) in a continuous loop.

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
#include "forg/control/SceneControl.h"
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

static forg::Light s_Light = {};

// Camera control sensitivities (tune to taste; flip a sign to invert an axis).
static const float kOrbitSpeed = 0.01f; // radians per pixel of left-drag
static const float kTruckSpeed = 0.01f; // world units per pixel of right-drag
static const float kZoomSpeed = 0.30f;  // world units per scroll line
static const float kMinTargetDistance =
    0.5f; // keep the camera off the target when zooming

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    AppSettings m_settings;
    forg::Engine m_engine;
    forg::scene::Model* m_model;

    forg::Color m_clear_color;
    forg::net::CommandQueue* m_cmd_queue;
    forg::net::HttpControlServer* m_control_server;

    NSWindow* m_window;
    NSView* m_view;
    NSTimer* m_timer;
    id m_event_monitor;
}
- (instancetype)initWithSettings:(const AppSettings&)settings;
- (void)render;
@end

static bool RenderEngineFrame(forg::Engine& engine, void* userData);

@implementation AppDelegate

- (instancetype)initWithSettings:(const AppSettings&)settings
{
    self = [super init];
    if (self)
    {
        m_settings = settings;
        m_model = 0;
        m_clear_color = forg::Color(0.75f, 0.75f, 0.75f);
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

    m_engine.SetRenderCallback(&RenderEngineFrame, self);

    forg::IRenderDevice* device = m_engine.Device();
    forg::scene::MeshNode& modelNode = m_engine.Scene().CreateMeshNode();
    m_model = &modelNode.GetModel();
    m_model->SetMesh(
        forg::geometry::Mesh::Cylinder(device, 1.0f, 2.0f, 5.0f, 10, 40));
    DBG_MSG("Cylinder created. Vertices: %d, Faces: %d\n",
            m_model->GetMesh()->GetNumVertices(),
            m_model->GetMesh()->GetNumFaces());

    // Set up a white point light (same setup as the Win32 sample).
    s_Light.Type = forg::LightType::Point;
    s_Light.Diffuse.r = 1.0f;
    s_Light.Diffuse.g = 1.0f;
    s_Light.Diffuse.b = 0.0f;
    s_Light.Ambient.r = 1.0f;
    s_Light.Ambient.g = 1.0f;
    s_Light.Ambient.b = 1.0f;
    s_Light.Specular.r = 1.0f;
    s_Light.Specular.g = 1.0f;
    s_Light.Specular.b = 1.0f;
    s_Light.Position.X = 5.0f;
    s_Light.Position.Y = 5.0f;
    s_Light.Position.Z = -1.0f;
    s_Light.Attenuation0 = 1.0f;
    s_Light.Range = 1000.0f;

    device->SetLight(0, &s_Light);
    device->LightEnable(0, true);

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
        // Orbit around the cylinder. x = yaw, y = pitch.
        m_engine.Camera().Orbit(-event.deltaX * kOrbitSpeed,
                                event.deltaY * kOrbitSpeed);
        break;

    case NSEventTypeRightMouseDragged:
        // Strafe the camera and its target parallel to the view plane.
        m_engine.Camera().Truck(-event.deltaX * kTruckSpeed,
                                event.deltaY * kTruckSpeed);
        break;

    case NSEventTypeScrollWheel:
    {
        // Dolly the camera toward/away from the target, clamped so it never
        // crosses the target (Camera::Dolly's own guard is disabled).
        float dolly = (float)event.scrollingDeltaY * kZoomSpeed;
        forg::Camera& camera = m_engine.Camera();
        float distance = (camera.get_Target() - camera.get_Position()).Length();
        if (dolly > distance - kMinTargetDistance)
        {
            dolly = distance - kMinTargetDistance;
        }
        camera.Dolly(dolly, 0.0f);
        break;
    }

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

// port of Viewport::Render (without the font/UI overlay)
- (void)render
{
    forg::IRenderDevice* device = m_engine.Device();
    if (device == 0)
        return;

    device->SetLight(0, &s_Light);
    device->SetRenderState(forg::RenderStates_Lighting, true);

    m_engine.Scene().Render(device);
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
            forg::control::SceneControlContext ctx;
            ctx.camera = &m_engine.Camera();
            ctx.model = m_model;
            ctx.light = &s_Light;
            ctx.clearColor = &m_clear_color;
            ctx.device = m_engine.Device();

            std::string body = forg::control::DispatchCommand(ctx, item.cmd);
            if (item.reply)
            {
                item.reply->set_value(body);
            }
        }
    }

    m_engine.SetClearColor(m_clear_color);
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

    m_model = 0;
    m_engine.SetRenderCallback(nullptr, nullptr);
    m_engine.Shutdown();
}

@end

static bool RenderEngineFrame(forg::Engine& engine, void* userData)
{
    (void)engine;

    AppDelegate* delegate = (AppDelegate*)userData;
    if (delegate == nil)
        return false;

    [delegate render];
    return true;
}

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
