// winapp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winapp.h"
#include "Viewport.h"

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

