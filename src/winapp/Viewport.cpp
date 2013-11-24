// Viewport.cpp: implementation of the Viewport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Viewport.h"

forg::Light s_Light = {0};

namespace forg { namespace scene {

    int Model::Load(const char* _name, IRenderDevice* _device)
    {
        //auto_ptr<ITexture> old_tex = m_texture;

        m_materials.clear();
        m_textures.clear();

        m_mesh = geometry::Mesh::FromFile(_name, 0, _device, m_materials);

        if (m_mesh == 0)
        {
            return false;
        }

        m_mesh_tm = Matrix4::Identity;

        // setup textures' dir

        string base_dir = _name;

        string::size_type last_slash = base_dir.find_last_of('/');

        if (last_slash == string::npos)
        {
            last_slash = base_dir.find_last_of('\\');
        }

        if (last_slash != string::npos)
        {
            base_dir.erase(last_slash+1);
        } else
        {
            base_dir.clear();
        }

        // load textures

        for (uint i=0; i<m_materials.size(); i++)
        {
            string tfn = base_dir + m_materials[i].TextureFilename;


            ITexture* tex = ITexture::FromFile(_device, tfn.c_str());

            m_textures.push_back( tex );

            if (tex == NULL)
                DBG_MSG(__T("Failed to load texture <%s>!\n"), tfn.c_str());
        }

        return FORG_OK;
    }


    void Model::Render(IRenderDevice* _device)
    {
        if (m_mesh.is_null())
            return;

        _device->SetTransform(TransformType_World, m_mesh_tm);

        if (m_materials.size() > 0)
        {
             for (uint i=0; i<m_materials.size(); i++)
            {
                _device->SetMaterial( &m_materials[i].Material3D );
                _device->SetTexture(0, m_textures[i] );
                m_mesh->DrawSubset(i);
            }
        } else
        {
            _device->SetTexture(0, 0);
            m_mesh->DrawSubset(0);
        }
    }
}}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Viewport::Viewport()
{
    m_device = 0;
    m_font = 0;
    m_bMouseCaptured=FALSE;
    m_bLMBDown=FALSE;
    m_fullscreen = false;
	m_show_gui = true;

    m_fps = 0;
    m_frame_counter = 0;
    m_perf_count.Start();
}

Viewport::~Viewport()
{
    m_Dialog.Close();

    if (m_font)
    {
        delete m_font;
        m_font = 0;
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
}

DWORD Viewport::Create(forg::IRenderer* renderer, int x, int y, int nWidth, int nHeight, HWND hParent)
{
    DWORD ret;

    DWORD style = WS_SIZEBOX|WS_MAXIMIZEBOX|/*WS_MAXIMIZE|*/WS_TILEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    ret=EWnd::Create(TEXT("Viewport"),TEXT("View"),x,y,nWidth,nHeight, style, exstyle, hParent);

    if (ret!=0) return ret;

    forg::RENDER_PARAMETERS rp;

    rp.BackBufferWidth = nWidth;
    rp.BackBufferHeight = nHeight;

    m_device = renderer->CreateDevice(m_hWnd, &rp);
    if (m_device == 0) return 1;

    m_device->SetRenderState(forg::RenderStates_CullMode, forg::Cull_Clockwise);
	m_device->SetRenderState(forg::RenderStates_ShadeMode, forg::ShadeMode_Gouraud);
	m_device->SetRenderState(forg::RenderStates_Lighting, true);
    m_device->SetRenderState(forg::RenderStates_FillMode, forg::FillMode_Solid);

    m_device->SetRenderState(forg::RenderStates_SourceBlend, forg::Blend_SourceAlpha);
    m_device->SetRenderState(forg::RenderStates_DestinationBlend, forg::Blend_InvSourceAlpha);

    m_mesh = forg::geometry::Mesh::Cylinder(m_device, 1.0f, 2.0f, 5.0f, 5, 20);
    
    forg::FontDescription fd =
    {
        12,
        0,
        0,
        1,
        false,
        0,
        0,
        0,
        0,
        (""),
        //"../bin/test.ttf"
        ("c:/windows/fonts/arial.ttf")
    };
    m_font = forg::Font::CreateIndirect(m_device, &fd);

    //m_Dialog.Init(m_device, "../bin/data/ui/dxutcontrols.dds");
    m_Dialog.Init(m_device, "data/ui/debug_texture2.dds");
	m_Dialog.Load("data/ui/dialog.xml");
    m_Dialog.AddButton(0, 100, 15, 50, 30);
    m_Dialog.AddSlider(1, 180, 15, 80, 30);
    m_Dialog.AddKnob(2, 0, 15, 30, 30);
    m_Dialog.AddComboBox(3, 300, 15, 100, 30);

        // Set up a white point light.
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
    s_Light.Range        = 1000.0f;

    m_device->SetLight(0, &s_Light);
    m_device->LightEnable(0, true);

    UpdateWindow();
    return ret;
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

    DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
    
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);

		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
            fullscreen = false;
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		//ShowCursor(FALSE);										// Hide Mouse Pointer

        WindowRect.left=(long)0;			// Set Left Value To 0
	    WindowRect.right=(long)width;		// Set Right Value To Requested Width
	    WindowRect.top=(long)0;				// Set Top Value To 0
	    WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style

        WindowRect.left=(long)0;			// Set Left Value To 0
	    WindowRect.right=(long)400;		// Set Right Value To Requested Width
	    WindowRect.top=(long)0;				// Set Top Value To 0
	    WindowRect.bottom=(long)400;		// Set Bottom Value To Requested Height
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

    SetWindowLong(GWL_STYLE, dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    SetWindowLong(GWL_EXSTYLE, dwExStyle);
    SetWindowPos(HWND_TOP, 0, 0, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, SWP_SHOWWINDOW);

    m_fullscreen = fullscreen;
}

void  Viewport::OnSize(UINT nType, int cx, int cy)
{
    if (m_device != 0)
    {
        m_device->SetViewport(0, 0, cx, cy);
        m_device->Reset();
    }

    //m_glRenderer.FitViewport(size.GetWidth(), size.GetHeight());
    m_camera.set_ScreenSize(cx, cy);
}

void Viewport::Render()
{
    if (m_device == NULL) return;

    forg::Matrix4 mlook;
    m_camera.GetViewMatrix(mlook);
	m_device->SetTransform(forg::TransformType_View, mlook);

	forg::Matrix4 mproj;
    m_camera.GetProjectionMatrix(mproj);
	m_device->SetTransform(forg::TransformType_Projection, mproj);

  	m_device->Clear(forg::ClearFlags_Target | forg::ClearFlags_ZBuffer, forg::Color(0.75f, 0.75f, 0.75f), 1.0f, 0);
	m_device->BeginScene();

    m_device->SetLight(0, &s_Light);
    m_device->SetRenderState(forg::RenderStates_Lighting, true);
    if (m_mesh != 0)
    {
        //Matrix4 mat;
        //mat.Scale(0.01f, 0.01f, 0.01f);
        //mat.Translate(0.0f, 2.0f, 0.0f);
        //mat.Scale(10.0f, 10.0f, 10.0f);
        m_device->SetTexture(0, 0);
        m_device->SetTransform(forg::TransformType_World, m_mesh_tm);
        m_mesh->DrawSubset(0);
    }

    m_model.Render(m_device);

	RenderUI();

  	m_device->EndScene();
	m_device->Present();
}

void Viewport::RenderUI()
{
    m_device->SetRenderState(forg::RenderStates_Lighting, false);

    forg::Viewport vp;
    m_device->GetViewport(&vp);
    forg::Rectangle r = {0, 0, vp.Width, vp.Height};

    if (m_font)
    {
        char str[512];

        sprintf(str, "%d fps   camera pos: %.3f %.3f %.3f  dir: %.3f %.3f %.3f",
                m_fps,
                m_camera.get_Position().X, m_camera.get_Position().Y, m_camera.get_Position().Z,
                m_camera.get_Target().X, m_camera.get_Target().Y, m_camera.get_Target().Z);

        m_font->DrawText2(str, -1, &r, 0, forg::Color4b(255, 255, 0, 255));
    }

	if (m_show_gui)
	{
		m_Dialog.Render();
	}
}

void Viewport::OnPaint()
{
    Render();
    ValidateRect( m_hWnd, NULL );

    m_frame_counter++;

    forg::uint64 duration = 0;
    m_perf_count.GetDurationInMs(duration);
	if (duration >= 1000)
	{
        m_fps = m_frame_counter;
        m_frame_counter = 0;
        m_perf_count.Start();
        //DBG_MSG("fps %d\n", m_fps);
    }
}

void Viewport::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    switch (nChar)
    {
	case VK_ESCAPE:
		m_show_gui = !m_show_gui;
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
            emfc::EOpenFileDialog ofd;
            if (IDOK == ofd.ShowDialog())
            {
                m_model.Load(ofd.GetFileName(), m_device);
            }
        }
        break;
    }

    Invalidate(0);
}

void Viewport::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    switch (nChar)
    {
    case VK_DOWN:
        m_Dialog.GetKnob(2)->SetValue(m_Dialog.GetKnob(2)->GetValue()-1);
        break;
    case VK_UP:
        m_Dialog.GetKnob(2)->SetValue(m_Dialog.GetKnob(2)->GetValue()+1);
        break;
    }

    Invalidate(0);
}

void Viewport::OnLButtonDown(UINT nFlags, POINT point)
{
    m_bLMBDown=TRUE;
    /*jesli nie jestesmy w trybie "full size" to zmieniamy
      zaznaczone okna na okno aktualnie znajdujace sie pod kursorem*/
/*
    if (! m_winMainWnd->m_winViewPanel.m_bOnlyActive) {
        m_winMainWnd->m_winViewPanel.m_hActiveWnd=m_hWnd;
        m_winMainWnd->m_winViewPanel.Invalidate();
    }
    */
}

void Viewport::OnLButtonUp(UINT nFlags, POINT point)
{
    m_bLMBDown=FALSE;
}

void Viewport::OnMouseWheel(UINT nFlags, POINTS point, int delta)
{
        float d = 0.01f * delta;

        if (MK_RBUTTON & nFlags)
        {
            float d = 0.01f * delta;

            forg::Vector3 move_dir = s_Light.Position;

            move_dir.Normalize();

            move_dir *= d;

            s_Light.Position += move_dir;
           
        } else
        {
            m_camera.Dolly(d, d);
        }

        Invalidate(0);
}

void Viewport::OnMouseMove(UINT nFlags, POINTS point)
{
    char str[30];
    RECT rect;
    POINT p={point.x,point.y};
    double tmpx,tmpy;
    HCURSOR hCur=NULL;

    static float last_x = 0.0f;
    static float last_y = 0.0f;

    {
        forg::Point forg_point = {point.x, point.y};
        m_Dialog.HandleMouse(forg::ui::EMouseEvent::Move, forg_point, 0, 0);
    }

    if (nFlags&MK_LBUTTON)
    {
        m_camera.Pan((last_x - point.x)*0.002, (point.y - last_y)*0.002);
    }
    if (nFlags&MK_MBUTTON)
    {
            m_camera.Truck((last_x - point.x)*0.004, (point.y - last_y)*0.004);
    }
    if (nFlags&MK_RBUTTON)
    {
            float x = (last_x - point.x)*0.002;
            float y = (last_y - point.y)*0.002;

            forg::Quaternion qx;
            forg::Quaternion::RotationAxis(qx, forg::Vector3(0.0f, 1.0f, 0.0f), x);

            forg::Vector3 ay;
            ay.Cross(s_Light.Position, forg::Vector3(0.0f, 1.0f, 0.0f));

            forg::Quaternion qy;
            forg::Quaternion::RotationAxis(qy, ay, y);

            forg::Vector3::TransformCoordinate(s_Light.Position, s_Light.Position, qx*qy);
    }
	//UpdateWindow();
    Invalidate(0);
    last_x = point.x;
    last_y = point.y;

    //int action=ACTION_NONE;
    /*
    ClientToScreen(m_hWnd,&p);

    if (! m_bMouseCaptured) {
        m_bMouseCaptured=TRUE;
        SetCapture();
        
        if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON3)) {
            //select
            hCur=m_appMain->LoadStandardCursor(IDC_ARROW);
            action=ACTION_MOVE;
        } else if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON4)) {
            //move
            hCur=m_appMain->LoadCursor(IDC_CURSOR2);
            action=ACTION_MOVE;
        } else if (m_winMainWnd->m_winTabPanel.IsButtonChecked(IDC_TB_BUTTON5)) {
            //rotate
            hCur=m_appMain->LoadCursor(IDC_CURSOR3);
            action=ACTION_ROTATE;
        } else if (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON1)) {
            //zoom
            hCur=m_appMain->LoadCursor(IDC_CURSOR6);
            action=ACTION_ZOOM;
        } else if (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON2)) {
            //zoom all
            hCur=m_appMain->LoadCursor(IDC_CURSOR13);
            action=ACTION_ZOOMALL;
        } else if (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON5)) {
            //field-of-view
            hCur=m_appMain->LoadCursor(IDC_CURSOR12);
            action=ACTION_FOV;
        } else if (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON6)) {
            //pan
            hCur=m_appMain->LoadCursor(IDC_CURSOR5);
            action=ACTION_PAN;
        } else if (m_winMainWnd->m_winViewControlPanel.IsButtonChecked(IDC_VIEWCONTROL_BUTTON7)) {
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
   
    //OnMouseCapture(action,tmpx,tmpy);
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


void Viewport::OnMouseCapture(int nAction, double fPosX, double fPosY)
{

}

void Viewport::OnMouseRelease(double fPosX, double fPosY)
{

}
