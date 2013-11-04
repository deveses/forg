// winapp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winapp.h"
#include "Viewport.h"
//#include "glrenderer/glrenderer.h"
#include "forg/forg.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////////

class CWinApp : public emfc::EApplication  
{
	Viewport* m_winMain;
    forg::IRenderer* m_renderer;

public:
	BOOL InitApplication();
	CWinApp();
	virtual ~CWinApp();
    virtual BOOL OnIdle( LONG lCount );
};

CWinApp::CWinApp()
{
    m_winMain = 0;
    m_renderer = 0;
}

CWinApp::~CWinApp()
{
    delete m_winMain;
    delete m_renderer;
    m_winMain = 0;
    m_renderer = 0;
}

BOOL CWinApp::InitApplication()
{
    /*okna glowne*/
    /*Usually an application should use SW_SHOW for the Y parameter
      because SW_SHOW allows the proper functioning for WS_MAXIMIZE 
      and WS_MINIMIZE styles.*/

    HMODULE module = 0;
    forg::script::xml::XMLParser config;

    config.Open("config.xml");
    forg::script::xml::XMLDocument* xml_doc = config.Parse();

    if (xml_doc)
    {
        forg::script::xml::XMLNode* xml_node = xml_doc->FindNode("renderer");

        if (xml_node)
        {
            forg::script::xml::XMLNode* xml_att = xml_node->FindAttribute("driver");

            if (xml_att)
            {                   
                module = LoadLibrary(xml_att->GetContent().c_str());
            }
        }
    }
    
    //HMODULE module = LoadLibrary("glrenderer_vc_d.dll");
    //HMODULE module = LoadLibrary("swrenderer_vc_d.dll");
    if (module == 0) return FALSE;

    forg::PFCREATERENDERER pfCreateRenderer = (forg::PFCREATERENDERER)GetProcAddress(module, "forgCreateRenderer");
    if (pfCreateRenderer == 0) return FALSE;

    //m_renderer = forgCreateRenderer();
    m_renderer = pfCreateRenderer();
    if (m_renderer == 0) return FALSE;

    m_winMain = new Viewport();
    //if (m_winMain.Create("EWindow","3Ditor",CW_USEDEFAULT ,SW_MAXIMIZE,480,460,WS_SIZEBOX|WS_MAXIMIZEBOX|WS_MAXIMIZE|WS_TILEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS ))
    if (m_winMain->Create(m_renderer, 10, 10, 1280,720, NULL))
        return FALSE;

//    m_winMainWnd=&m_winMain;
//    m_appMain=this;
//    HICON ic1=LoadIcon(IDI_ICON1BIG);
//    m_winMain.SetIcon(ic1,TRUE);
    //menu glowne
//    HMENU m=LoadMenu(m_hInstance,MAKEINTRESOURCE(IDR_MENU_MAIN));
//    m_winMain.SetMenu(m);

    //m_winMain.m_winTabPanel.Create(m_winMain.m_hWnd);
    //m_winMain.m_winTabPanel.ShowWindow(SW_SHOW);

    //m_winMain.m_winViewControlPanel.Create(101,51,m_winMain.m_hWnd);
    //m_winMain.m_winViewControlPanel.FitToParent();
    //m_winMain.m_winViewControlPanel.ShowWindow(SW_SHOW);

    //m_winMain.m_winTimePanel.Create(176+9,51,m_winMain.m_hWnd);
    //m_winMain.m_winTimePanel.FitToParent();
    //m_winMain.m_winTimePanel.ShowWindow(SW_SHOW);

    //m_winMain.m_winStatusPanel.Create(m_winMain.m_hWnd);
    //m_winMain.m_winStatusPanel.FitToParent();
    //m_winMain.m_winStatusPanel.ShowWindow(SW_SHOW);

    //okno-panel operacyjny
 //   if (m_winMain.CreateCommandPanel(120,0,192,400))
 //       return FALSE;
 //   
 //   //dokujemy panel
 //   m_winMain.m_winCommandPanel.SetDockMode(DOCK_RIGHT);
 //   m_winMain.m_winCommandPanel.DockWindow(0,0);
 //   m_winMain.m_winCommandPanel.FitToParent();
	//m_winMain.m_winCommandPanel.GetWindowRect(&((m_winMain.m_winCommandPanel).m_rtBeforeDock));
 //   m_winMain.m_winCommandPanel.ShowWindow(SW_SHOW);

 //   m_winMain.m_animScroll.Create(0,400,800,18,WS_CHILD,m_winMain.m_hWnd);
 //   m_winMain.m_animScroll.FitToParent();
 //   m_winMain.m_animScroll.ShowWindow(SW_SHOW);

 //   m_winMain.CreateViewPanel(0,0,100,100);
 //   m_winMain.m_winViewPanel.FitToParent();

    m_winMain->ShowWindow(SW_SHOW);
    m_winMain->SetFocus();
    
    return TRUE;
}

BOOL CWinApp::OnIdle( LONG lCount )
{
    m_winMain->OnPaint();

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

    CWinApp app;

    app.InitCommonControls();
    if (! app.InitApplication()) app.ErrorBox();
    else app.Run();

	return 0;
}
