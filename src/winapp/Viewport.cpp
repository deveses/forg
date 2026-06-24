// Viewport.cpp: implementation of the Viewport class.
//
//////////////////////////////////////////////////////////////////////

#include "Viewport.h"
#include "stdafx.h"

#include <commdlg.h>

forg::Light s_Light = {0};

namespace {

const float kOrbitSpeed = 0.01f;
const float kTruckSpeed = 0.01f;
const float kZoomSpeed = 0.30f;
const float kMinTargetDistance = 0.5f;

} // namespace

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Viewport::Viewport()
{
    m_hWnd = 0;
    m_hInstance = 0;
    m_engine = 0;
    m_device = 0;
    m_model_node = 0;
    m_font = 0;
    m_bMouseCaptured = FALSE;
    m_bLMBDown = FALSE;
    m_fullscreen = false;
    m_show_gui = 1;
    m_hasLastMousePoint = false;
    m_lastMousePoint = {0, 0};

}

Viewport::~Viewport()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = 0;
    }

    m_Dialog.Close();

    if (m_font)
    {
        delete m_font;
        m_font = 0;
    }

    if (m_engine)
        m_engine->SetRenderCallback(nullptr, nullptr);
}

DWORD Viewport::Create(forg::Engine& engine, int x, int y, int nWidth,
                       int nHeight, HWND hParent)
{
    m_engine = &engine;
    m_hInstance = GetModuleHandle(NULL);

    DWORD style = WS_SIZEBOX | WS_MAXIMIZEBOX |
                  /*WS_MAXIMIZE|*/ WS_TILEDWINDOW | WS_CLIPCHILDREN |
                  WS_CLIPSIBLINGS;
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

    const LPCTSTR class_name = TEXT("ForgViewport");
    WNDCLASSEX wc;
    if (!GetClassInfoEx(m_hInstance, class_name, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.hInstance = m_hInstance;
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = Viewport::StaticWindowProc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wc.lpszClassName = class_name;

        if (!RegisterClassEx(&wc))
            return GetLastError();
    }

    RECT windowRect = {0, 0, nWidth, nHeight};
    if (!AdjustWindowRectEx(&windowRect, style, FALSE, exstyle))
        return GetLastError();

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    m_hWnd = CreateWindowEx(exstyle, class_name, TEXT("View"), style, x, y,
                            windowWidth, windowHeight, hParent, NULL,
                            m_hInstance, this);
    if (m_hWnd == NULL)
        return GetLastError();

    if (!m_engine->Initialize(m_hWnd, "config.yml"))
        return 1;

    m_device = m_engine->Device();
    m_engine->SetRenderCallback(&Viewport::RenderEngineFrame, this);

    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    OnSize(SIZE_RESTORED, clientRect.right - clientRect.left,
           clientRect.bottom - clientRect.top);

    m_model_node = &m_engine->Scene().CreateMeshNode();
    m_model_node->GetModel().SetMesh(
        forg::geometry::Mesh::Cylinder(m_device, 1.0f, 2.0f, 5.0f, 10, 40));
    DBG_MSG("Cylinder created. Vertices: %d, Faces: %d\n",
            m_model_node->GetModel().GetMesh()->GetNumVertices(),
            m_model_node->GetModel().GetMesh()->GetNumFaces());

    forg::FontDescription fd = {12, 0, 0, 1, false, 0, 0, 0, 0, (""),
                                //"../bin/test.ttf"
                                ("c:/windows/fonts/arial.ttf")};
    m_font = forg::Font::CreateIndirect(m_device, &fd);

    // m_Dialog.Init(m_device, "../bin/data/ui/dxutcontrols.dds");
    m_Dialog.Init(m_device, "data/ui/debug_texture2.dds");
    m_Dialog.Load("data/ui/dialog.xml");
    m_Dialog.AddButton(0, 100, 15, 50, 30);
    m_Dialog.AddSlider(1, 180, 15, 80, 30);
    m_Dialog.AddKnob(2, 0, 15, 30, 30);
    m_Dialog.AddComboBox(3, 300, 15, 100, 30);

    // Set up a white point light.
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

    // Position it high in the scene and behind the user.
    // Remember, these coordinates are in world space, so
    // the user could be anywhere in world space, too.
    // For the purposes of this example, assume the user
    // is at the origin of world space.
    s_Light.Position.X = 5.0f;
    s_Light.Position.Y = 5.0f;
    s_Light.Position.Z = -1.0f;

    // Don't attenuate.
    s_Light.Attenuation0 = 1.0f;
    s_Light.Range = 1000.0f;

    m_device->SetLight(0, &s_Light);
    m_device->LightEnable(0, true);

    UpdateWindow(m_hWnd);
    return 0;
}

BOOL Viewport::ShowWindow(int nCmdShow)
{
    return ::ShowWindow(m_hWnd, nCmdShow);
}

HWND Viewport::SetFocus() { return ::SetFocus(m_hWnd); }

void Viewport::Invalidate(BOOL bErase) { InvalidateRect(m_hWnd, NULL, bErase); }

LRESULT CALLBACK Viewport::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                            LPARAM lParam)
{
    Viewport* viewport =
        reinterpret_cast<Viewport*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (uMsg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCT*>(lParam);
        viewport = reinterpret_cast<Viewport*>(create->lpCreateParams);
        viewport->m_hWnd = hWnd;
        SetWindowLongPtr(hWnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>(viewport));
    }

    if (viewport)
        return viewport->WindowProc(uMsg, wParam, lParam);

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Viewport::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    POINTS points;

    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hWnd, &ps);
        EndPaint(m_hWnd, &ps);
        OnPaint();
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_LBUTTONDOWN:
        points = MAKEPOINTS(lParam);
        m_lastMousePoint = points;
        m_hasLastMousePoint = true;
        SetCapture(m_hWnd);
        pt.x = points.x;
        pt.y = points.y;
        OnLButtonDown(static_cast<UINT>(wParam), pt);
        return 0;
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        m_lastMousePoint = MAKEPOINTS(lParam);
        m_hasLastMousePoint = true;
        SetCapture(m_hWnd);
        return 0;
    case WM_LBUTTONUP:
        points = MAKEPOINTS(lParam);
        pt.x = points.x;
        pt.y = points.y;
        OnLButtonUp(static_cast<UINT>(wParam), pt);
        if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0)
        {
            ReleaseCapture();
            m_hasLastMousePoint = false;
        }
        return 0;
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0)
        {
            ReleaseCapture();
            m_hasLastMousePoint = false;
        }
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(static_cast<UINT>(wParam), MAKEPOINTS(lParam));
        return 0;
    case WM_MOUSEWHEEL:
        OnMouseWheel(static_cast<UINT>(wParam), MAKEPOINTS(lParam),
                     GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;
    case WM_SIZE:
        OnSize(static_cast<UINT>(wParam), LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KEYDOWN:
        OnKeyDown(static_cast<UINT>(wParam), lParam & 0xffff, lParam >> 16);
        return 0;
    case WM_KEYUP:
        OnKeyUp(static_cast<UINT>(wParam), lParam & 0xffff, lParam >> 16);
        return 0;
    case WM_CLOSE:
        DestroyWindow(m_hWnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_NCDESTROY:
    {
        HWND hWnd = m_hWnd;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
        m_hWnd = 0;
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    default:
        return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    }
}

void Viewport::Cleanup()
{
    /*
    if( g_pd3dDevice != NULL)
        g_pd3dDevice->Release();

    if( g_pD3D != NULL)
        g_pD3D->Release();
        */
}

void Viewport::ToggleFullscreen()
{
    int width = 1280;
    int height = 800;
    int bits = 32;
    bool fullscreen = !m_fullscreen;

    DWORD dwExStyle; // Window Extended Style
    DWORD dwStyle;   // Window Style
    RECT WindowRect; // Grabs Rectangle Upper Left / Lower Right Values

    if (fullscreen) // Attempt Fullscreen Mode?
    {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);

        DEVMODE dmScreenSettings; // Device Mode
        memset(&dmScreenSettings, 0,
               sizeof(dmScreenSettings)); // Makes Sure Memory's Cleared
        dmScreenSettings.dmSize =
            sizeof(dmScreenSettings);           // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth = width;   // Selected Screen Width
        dmScreenSettings.dmPelsHeight = height; // Selected Screen Height
        dmScreenSettings.dmBitsPerPel = bits;   // Selected Bits Per Pixel
        dmScreenSettings.dmFields =
            DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets
        // Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) !=
            DISP_CHANGE_SUCCESSFUL)
        {
            fullscreen = false;
        }
    }

    if (fullscreen) // Are We Still In Fullscreen Mode?
    {
        dwExStyle = WS_EX_APPWINDOW; // Window Extended Style
        dwStyle = WS_POPUP;          // Windows Style
                                     // ShowCursor(FALSE);
        // // Hide Mouse Pointer

        WindowRect.left = (long)0;      // Set Left Value To 0
        WindowRect.right = (long)width; // Set Right Value To Requested Width
        WindowRect.top = (long)0;       // Set Top Value To 0
        WindowRect.bottom =
            (long)height; // Set Bottom Value To Requested Height
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; // Window Extended Style
        dwStyle = WS_OVERLAPPEDWINDOW;                  // Windows Style

        WindowRect.left = (long)0;     // Set Left Value To 0
        WindowRect.right = (long)400;  // Set Right Value To Requested Width
        WindowRect.top = (long)0;      // Set Top Value To 0
        WindowRect.bottom = (long)400; // Set Bottom Value To Requested Height
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE,
                       dwExStyle); // Adjust Window To True Requested Size

    SetWindowLong(m_hWnd, GWL_STYLE,
                  dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
    SetWindowPos(m_hWnd, HWND_TOP, 0, 0, WindowRect.right - WindowRect.left,
                 WindowRect.bottom - WindowRect.top, SWP_SHOWWINDOW);

    m_fullscreen = fullscreen;
}

void Viewport::OnSize(UINT nType, int cx, int cy)
{
    if (m_engine != 0)
        m_engine->Resize(cx, cy);
}

void Viewport::Render()
{
    if (m_engine == NULL || m_device == NULL)
        return;

    m_device->SetLight(0, &s_Light);
    m_device->SetRenderState(forg::RenderStates_Lighting, true);

    m_engine->Scene().Render(m_device);

    RenderUI();
}

void Viewport::RenderUI()
{
    if (m_show_gui > 0)
    {
        m_device->SetRenderState(forg::RenderStates_Lighting, false);

        forg::Viewport vp;
        m_device->GetViewport(&vp);
        forg::Rectangle r = {0, 0, static_cast<int>(vp.Width),
                             static_cast<int>(vp.Height)};

        if (m_font)
        {
            char str[512];

            sprintf(str,
                    "%u fps   camera pos: %.3f %.3f %.3f  dir: %.3f %.3f %.3f",
                    m_engine->FrameStats().FPS,
                    m_engine->Camera().get_Position().X,
                    m_engine->Camera().get_Position().Y,
                    m_engine->Camera().get_Position().Z,
                    m_engine->Camera().get_Target().X,
                    m_engine->Camera().get_Target().Y,
                    m_engine->Camera().get_Target().Z);

            m_font->DrawText2(str, -1, &r, 0, forg::Color4b(255, 255, 0, 255));
        }

        if (m_show_gui == 1)
        {
            m_Dialog.Render();
        }
    }
}

void Viewport::OnPaint()
{
    ValidateRect(m_hWnd, NULL);
}

void Viewport::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
    case VK_ESCAPE:
        m_show_gui = (m_show_gui + 1) % 3;
        break;
    case VK_F1:
        ToggleFullscreen();
        break;
    case VK_LEFT:
        break;
    case VK_RIGHT:
        break;
    case 'O':
    case 'o':
    {
        char filename[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = filename;
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

        if (GetOpenFileNameA(&ofn))
        {
            if (m_model_node != 0)
                m_model_node->GetModel().Load(filename, m_device);
        }
    }
    break;
    }

    Invalidate(0);
}

void Viewport::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
    case VK_DOWN:
        m_Dialog.GetKnob(2)->SetValue(m_Dialog.GetKnob(2)->GetValue() - 1);
        break;
    case VK_UP:
        m_Dialog.GetKnob(2)->SetValue(m_Dialog.GetKnob(2)->GetValue() + 1);
        break;
    }

    Invalidate(0);
}

void Viewport::OnLButtonDown(UINT nFlags, POINT point)
{
    m_bLMBDown = TRUE;
    /*jesli nie jestesmy w trybie "full size" to zmieniamy
      zaznaczone okna na okno aktualnie znajdujace sie pod kursorem*/
    /*
        if (! m_winMainWnd->m_winViewPanel.m_bOnlyActive) {
            m_winMainWnd->m_winViewPanel.m_hActiveWnd=m_hWnd;
            m_winMainWnd->m_winViewPanel.Invalidate();
        }
        */
}

void Viewport::OnLButtonUp(UINT nFlags, POINT point) { m_bLMBDown = FALSE; }

void Viewport::OnMouseWheel(UINT nFlags, POINTS point, int delta)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);

    float dolly = (static_cast<float>(delta) / WHEEL_DELTA) * kZoomSpeed;
    forg::Camera& camera = m_engine->Camera();
    float distance = (camera.get_Target() - camera.get_Position()).Length();
    if (dolly > distance - kMinTargetDistance)
    {
        dolly = distance - kMinTargetDistance;
    }

    camera.Dolly(dolly, 0.0f);
    Invalidate(0);
}

void Viewport::OnMouseMove(UINT nFlags, POINTS point)
{
    {
        forg::Point forg_point = {point.x, point.y};
        m_Dialog.HandleMouse(forg::ui::EMouseEvent::Move, forg_point, 0, 0);
    }

    if (!m_hasLastMousePoint)
    {
        m_lastMousePoint = point;
        m_hasLastMousePoint = true;
        return;
    }

    float dx = static_cast<float>(point.x - m_lastMousePoint.x);
    float dy = static_cast<float>(point.y - m_lastMousePoint.y);
    m_lastMousePoint = point;

    if (nFlags & MK_LBUTTON)
    {
        m_engine->Camera().Orbit(-dx * kOrbitSpeed, dy * kOrbitSpeed);
    }
    else if (nFlags & MK_RBUTTON)
    {
        m_engine->Camera().Truck(-dx * kTruckSpeed, dy * kTruckSpeed);
    }

    // UpdateWindow();
    Invalidate(0);

    // int action=ACTION_NONE;
    /*
    ClientToScreen(m_hWnd,&p);

    if (! m_bMouseCaptured) {
        m_bMouseCaptured=TRUE;
        SetCapture();

        if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON3)) {
            //select
            hCur=m_appMain->LoadStandardCursor(IDC_ARROW);
            action=ACTION_MOVE;
        } else if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON4))
    {
            //move
            hCur=m_appMain->LoadCursor(IDC_CURSOR2);
            action=ACTION_MOVE;
        } else if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON5))
    {
            //rotate
            hCur=m_appMain->LoadCursor(IDC_CURSOR3);
            action=ACTION_ROTATE;
        } else if
    (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON1))
    {
            //zoom
            hCur=m_appMain->LoadCursor(IDC_CURSOR6);
            action=ACTION_ZOOM;
        } else if
    (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON2))
    {
            //zoom all
            hCur=m_appMain->LoadCursor(IDC_CURSOR13);
            action=ACTION_ZOOMALL;
        } else if
    (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON5))
    {
            //field-of-view
            hCur=m_appMain->LoadCursor(IDC_CURSOR12);
            action=ACTION_FOV;
        } else if
    (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON6))
    {
            //pan
            hCur=m_appMain->LoadCursor(IDC_CURSOR5);
            action=ACTION_PAN;
        } else if
    (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON7))
    {
            //arc rotate
            hCur=m_appMain->LoadCursor(IDC_CURSOR8);
            action=ACTION_ARCROTATE;
        }

        if (hCur!=NULL) SetCursor(hCur);
    }

    //wyliczamy procenty
    GetClientRect(&rect);
    rect.right--;
    rect.bottom--;
    tmpx=((200*(double)point.x)/(double)rect.right)-100;
    tmpy=((200*(double)point.y)/(double)rect.bottom)-100;

    //sprawdzamy czy juz czas zeby zwolnic mysz
    GetWindowRect(&rect);
    if (m_bMouseCaptured && (m_hWnd!=WindowFromPoint(p))) {
        ReleaseCapture();
        m_bMouseCaptured=FALSE;
        //(m_winMainWnd->m_winStatusPanel).ClearPositionStatus();
        SetCursor(LoadCursor(NULL,IDC_ARROW));
        OnMouseRelease(tmpx,tmpy);
        return;
    }
    */

    // OnMouseCapture(action,tmpx,tmpy);
    /*jesli to okno jest wybrane to wypisujemy wspolrzedne*/
    /*
    if ((m_winMainWnd->m_winViewPanel).m_hActiveWnd==m_hWnd) {
        sprintf(str,"%.1f%%",tmpx);
        (m_winMainWnd->m_winStatusPanel).m_winStatus2.SetText(str,0,0);
        sprintf(str,"%.1f%%",tmpy);
        (m_winMainWnd->m_winStatusPanel).m_winStatus3.SetText(str,0,0);
        sprintf(str,"0.0");
        (m_winMainWnd->m_winStatusPanel).m_winStatus4.SetText(str,0,0);
    }
    */
}

void Viewport::OnMouseCapture(int nAction, double fPosX, double fPosY) {}

void Viewport::OnMouseRelease(double fPosX, double fPosY) {}

bool Viewport::RenderEngineFrame(forg::Engine& engine, void* userData)
{
    UNREFERENCED_PARAMETER(engine);

    Viewport* viewport = static_cast<Viewport*>(userData);
    if (viewport == 0)
        return false;

    viewport->Render();
    return true;
}
