// main.mm : macOS port of the Win32 sample app (src/winapp).
// Reads config.xml to pick the renderer plugin and window geometry,
// then renders the demo scene (lit cylinder) in a continuous loop.

// Cocoa must come first: forg's base.h defines macros (null, IN, OUT)
// that break the system headers if they are seen earlier.
#import <Cocoa/Cocoa.h>
#include <MacTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <unistd.h>

#include <iostream>
#include <string>

// Pulled in before forg.h so the C++ threading headers are parsed before
// base.h defines its null/IN/OUT macros.
#include "forg/net/CommandQueue.h"
#include "forg/net/HttpControlServer.h"

#include "forg.h"
#include "forg/control/SceneControl.h"

struct AppSettings
{
    std::string driver;
    int winWidth = 100;
    int winHeight = 100;
    int winX = 10;
    int winY = 10;
    bool controlEnabled = false;
    int controlPort = 8080;
};

static forg::Light s_Light = {};

// Camera control sensitivities (tune to taste; flip a sign to invert an axis).
static const float kOrbitSpeed        = 0.01f; // radians per pixel of left-drag
static const float kTruckSpeed        = 0.01f; // world units per pixel of right-drag
static const float kZoomSpeed         = 0.30f; // world units per scroll line
static const float kMinTargetDistance = 0.5f;  // keep the camera off the target when zooming

/////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    AppSettings m_settings;
    forg::IRenderer* m_renderer;
    forg::IRenderDevice* m_device;
    forg::geometry::Mesh::MeshPtr m_mesh;
    forg::Matrix4 m_mesh_tm;
    forg::Camera m_camera;
    forg::PerformanceCounter m_perf_count;
    int m_fps;
    int m_frame_counter;

    forg::Color m_clear_color;
    forg::net::CommandQueue* m_cmd_queue;
    forg::net::HttpControlServer* m_control_server;

    NSWindow* m_window;
    NSView* m_view;
    NSTimer* m_timer;
    id m_event_monitor;
}
- (instancetype)initWithSettings:(const AppSettings&)settings renderer:(forg::IRenderer*)renderer;
@end

@implementation AppDelegate

- (instancetype)initWithSettings:(const AppSettings&)settings renderer:(forg::IRenderer*)renderer
{
    self = [super init];
    if (self)
    {
        m_settings = settings;
        m_renderer = renderer;
        m_device = 0;
        m_mesh_tm = forg::Matrix4::Identity;
        m_fps = 0;
        m_frame_counter = 0;
        m_clear_color = forg::Color(0.75f, 0.75f, 0.75f);
        m_cmd_queue = 0;
        m_control_server = 0;
        m_perf_count.Start();
    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    int winWidth = m_settings.winWidth;
    int winHeight = m_settings.winHeight;

    // config posx/posy are from the top-left corner (Win32 convention)
    CGFloat screenHeight = [NSScreen mainScreen].frame.size.height;
    NSRect contentRect = NSMakeRect(m_settings.winX,
                                    screenHeight - m_settings.winY - winHeight,
                                    winWidth, winHeight);

    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                              NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    m_window = [[NSWindow alloc] initWithContentRect:contentRect
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    [m_window setTitle:@"View"];

    m_view = [m_window contentView];
    [m_view setWantsLayer:YES];
    [m_view setPostsFrameChangedNotifications:YES];

    forg::RENDER_PARAMETERS rp;
    rp.BackBufferWidth = winWidth;
    rp.BackBufferHeight = winHeight;

    m_device = m_renderer->CreateDevice((forg::HWIN)m_view, &rp);
    if (m_device == 0)
    {
        std::cerr << "Unable to create render device!\n";
        [NSApp terminate:nil];
        return;
    }

    m_device->SetRenderState(forg::RenderStates_CullMode, forg::Cull_Clockwise);
    m_device->SetRenderState(forg::RenderStates_ShadeMode, forg::ShadeMode_Gouraud);
    m_device->SetRenderState(forg::RenderStates_Lighting, true);
    m_device->SetRenderState(forg::RenderStates_FillMode, forg::FillMode_Solid);

    m_device->SetRenderState(forg::RenderStates_SourceBlend, forg::Blend_SourceAlpha);
    m_device->SetRenderState(forg::RenderStates_DestinationBlend, forg::Blend_InvSourceAlpha);

    m_mesh = forg::geometry::Mesh::Cylinder(m_device, 1.0f, 2.0f, 5.0f, 10, 40);
    DBG_MSG("Cylinder created. Vertices: %d, Faces: %d\n", m_mesh->GetNumVertices(), m_mesh->GetNumFaces());

    // Set up a white point light (same setup as the Win32 sample).
    s_Light.Type = forg::LightType::Point;
    s_Light.Diffuse.r  = 1.0f;
    s_Light.Diffuse.g  = 1.0f;
    s_Light.Diffuse.b  = 0.0f;
    s_Light.Ambient.r  = 1.0f;
    s_Light.Ambient.g  = 1.0f;
    s_Light.Ambient.b  = 1.0f;
    s_Light.Specular.r = 1.0f;
    s_Light.Specular.g = 1.0f;
    s_Light.Specular.b = 1.0f;
    s_Light.Position.X = 5.0f;
    s_Light.Position.Y = 5.0f;
    s_Light.Position.Z = -1.0f;
    s_Light.Attenuation0 = 1.0f;
    s_Light.Range        = 1000.0f;

    m_device->SetLight(0, &s_Light);
    m_device->LightEnable(0, true);

    // Optional runtime control server: a background HTTP endpoint queues
    // commands that are applied on this (main) thread in onIdle:.
    if (m_settings.controlEnabled)
    {
        m_cmd_queue = new forg::net::CommandQueue();
        m_control_server = new forg::net::HttpControlServer("127.0.0.1", m_settings.controlPort, *m_cmd_queue);
        if (! m_control_server->Start())
        {
            std::cerr << "Control server failed to start on port " << m_settings.controlPort << "\n";
        }
        else
        {
            std::cout << "Control server listening on http://127.0.0.1:" << m_settings.controlPort << "\n";
        }
    }

    [self onResize];

    [[NSNotificationCenter defaultCenter] addObserver:self
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

    // Turntable camera controls: left-drag orbits, right-drag trucks, scroll zooms.
    // A local monitor sees the deltas without subclassing the renderer's content view.
    NSEventMask mask = NSEventMaskLeftMouseDragged |
                       NSEventMaskRightMouseDragged |
                       NSEventMaskScrollWheel;
    // MRC build: __unsafe_unretained avoids a retain cycle; the monitor is removed
    // in applicationWillTerminate, before the delegate is deallocated.
    __unsafe_unretained AppDelegate* unretainedSelf = self;
    m_event_monitor = [NSEvent addLocalMonitorForEventsMatchingMask:mask
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
            m_camera.Orbit(-event.deltaX * kOrbitSpeed, event.deltaY * kOrbitSpeed);
            break;

        case NSEventTypeRightMouseDragged:
            // Strafe the camera and its target parallel to the view plane.
            m_camera.Truck(-event.deltaX * kTruckSpeed, event.deltaY * kTruckSpeed);
            break;

        case NSEventTypeScrollWheel:
        {
            // Dolly the camera toward/away from the target, clamped so it never
            // crosses the target (Camera::Dolly's own guard is disabled).
            float dolly = (float)event.scrollingDeltaY * kZoomSpeed;
            float distance = (m_camera.get_Target() - m_camera.get_Position()).Length();
            if (dolly > distance - kMinTargetDistance)
            {
                dolly = distance - kMinTargetDistance;
            }
            m_camera.Dolly(dolly, 0.0f);
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

    if (m_device != 0)
    {
        m_device->SetViewport(0, 0, (forg::uint)size.width, (forg::uint)size.height);
        m_device->Reset();
    }

    m_camera.set_ScreenSize(size.width, size.height);
}

- (void)viewFrameChanged:(NSNotification*)notification
{
    [self onResize];
}

// port of Viewport::Render (without the font/UI overlay)
- (void)render
{
    if (m_device == 0) return;

    forg::Matrix4 mlook;
    m_camera.GetViewMatrix(mlook);
    m_device->SetTransform(forg::TransformType_View, mlook);

    forg::Matrix4 mproj;
    m_camera.GetProjectionMatrix(mproj);
    m_device->SetTransform(forg::TransformType_Projection, mproj);

    m_device->Clear(forg::ClearFlags_Target | forg::ClearFlags_ZBuffer, m_clear_color, 1.0f, 0);
    m_device->BeginScene();

    m_device->SetLight(0, &s_Light);
    m_device->SetRenderState(forg::RenderStates_Lighting, true);

    if (! m_mesh.is_null())
    {
        m_device->SetTexture(0, 0);
        m_device->SetTransform(forg::TransformType_World, m_mesh_tm);
        m_mesh->DrawSubset(0);
    }

    m_device->EndScene();
    m_device->Present();
}

// port of Viewport::OnPaint + CWinApp::OnIdle
- (void)onIdle:(NSTimer*)timer
{
    // Apply any commands queued by the control server thread (main-thread only).
    if (m_cmd_queue)
    {
        forg::net::QueueItem item;
        while (m_cmd_queue->TryPop(item))
        {
            forg::control::SceneControlContext ctx;
            ctx.camera     = &m_camera;
            ctx.mesh       = &m_mesh;
            ctx.meshTm     = &m_mesh_tm;
            ctx.light      = &s_Light;
            ctx.clearColor = &m_clear_color;
            ctx.device     = m_device;

            std::string body = forg::control::DispatchCommand(ctx, item.cmd);
            if (item.reply)
            {
                item.reply->set_value(body);
            }
        }
    }

    forg::PerformanceCounter frame_profiler;
    frame_profiler.Start();
    [self render];
    frame_profiler.Stop();

    m_frame_counter++;

    forg::uint64 duration = 0;
    m_perf_count.GetDurationInMs(duration);
    if (duration >= 1000)
    {
        frame_profiler.GetDurationInUs(duration);

        m_fps = m_frame_counter;
        m_frame_counter = 0;
        m_perf_count.Start();
        DBG_MSG("fps %d time: %lld us\n", m_fps, duration);
    }
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

    if (! m_mesh.is_null())
    {
        delete m_mesh.release();
    }

    if (m_device)
    {
        m_device->Release();
        m_device = 0;
    }

    delete m_renderer;
    m_renderer = 0;
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
    if (CFURLGetFileSystemRepresentation(resBundle, TRUE, (UInt8*)bundlePath, PATH_MAX))
    {
        std::cout << "resource path: " << bundlePath << "\n";
        chdir(bundlePath);
    }
    CFRelease(resBundle);
}

// port of the config.xml parsing in CWinApp::InitApplication
static bool LoadSettings(AppSettings& settings)
{
    forg::script::xml::XMLParser config;

    config.Open("config.xml");
    forg::script::xml::XMLDocument* xml_doc = config.Parse();

    if (xml_doc == 0)
    {
        std::cerr << "Unable to load config.xml!\n";
        return false;
    }

    if (forg::script::xml::XMLNode* xml_node = xml_doc->FindNode("renderer"))
    {
        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("driver"))
        {
            settings.driver = xml_att->GetContent().c_str();
        }
    }

    if (forg::script::xml::XMLNode* xml_node = xml_doc->FindNode("window"))
    {
        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("width"))
        {
            settings.winWidth = atoi(xml_att->GetContent().c_str());
        }

        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("height"))
        {
            settings.winHeight = atoi(xml_att->GetContent().c_str());
        }

        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("posx"))
        {
            settings.winX = atoi(xml_att->GetContent().c_str());
        }

        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("posy"))
        {
            settings.winY = atoi(xml_att->GetContent().c_str());
        }
    }

    if (forg::script::xml::XMLNode* xml_node = xml_doc->FindNode("controlserver"))
    {
        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("enabled"))
        {
            settings.controlEnabled = (std::string(xml_att->GetContent().c_str()) == "true");
        }

        if (forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("port"))
        {
            settings.controlPort = atoi(xml_att->GetContent().c_str());
        }
    }

    return true;
}

static forg::IRenderer* CreateRenderer(const std::string& driver)
{
    if (driver.empty())
    {
        std::cerr << "No renderer driver specified in config.xml!\n";
        return 0;
    }

    // cwd is the resources directory; "./" keeps dlopen from searching dyld paths
    std::string path = "./" + driver;
    void* module = dlopen(path.c_str(), RTLD_NOW);
    if (module == 0)
    {
        std::cerr << "Unable to load renderer <" << driver << ">: " << dlerror() << "\n";
        return 0;
    }

    forg::PFCREATERENDERER pfCreateRenderer = (forg::PFCREATERENDERER)dlsym(module, "forgCreateRenderer");
    if (pfCreateRenderer == 0)
    {
        std::cerr << "forgCreateRenderer not found in <" << driver << ">!\n";
        return 0;
    }

    return pfCreateRenderer();
}

int main(int, char*[])
{
    ChangeToResourcesDirectory();

    AppSettings settings;
    if (! LoadSettings(settings))
    {
        return 1;
    }

    forg::IRenderer* renderer = CreateRenderer(settings.driver);
    if (renderer == 0)
    {
        return 1;
    }

    NSApplication* app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    AppDelegate* delegate = [[AppDelegate alloc] initWithSettings:settings renderer:renderer];
    [app setDelegate:delegate];

    [app run];

    return 0;
}
