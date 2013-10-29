// winapp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winapp.h"
#include "Viewport.h"
//#include "glrenderer/glrenderer.h"
#include "forg.h"

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

    forg::script::xml::XMLParser config;

    config.Open("../bin/config.xml");
    config.Parse();
    
    HMODULE module = LoadLibrary("glrenderer_vc_d.dll");
    //HMODULE module = LoadLibrary("swrenderer_vc_d.dll");
    if (module == 0) return FALSE;

    forg::PFCREATERENDERER pfCreateRenderer = (forg::PFCREATERENDERER)GetProcAddress(module, "forgCreateRenderer");
    if (pfCreateRenderer == 0) return FALSE;

    //m_renderer = forgCreateRenderer();
    m_renderer = pfCreateRenderer();
    if (m_renderer == 0) return FALSE;

    m_winMain = new Viewport();
    //if (m_winMain.Create("EWindow","3Ditor",CW_USEDEFAULT ,SW_MAXIMIZE,480,460,WS_SIZEBOX|WS_MAXIMIZEBOX|WS_MAXIMIZE|WS_TILEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS ))
    if (m_winMain->Create(m_renderer, 10, 10, 800,600, NULL))
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

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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
	/*UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;*/
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINAPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
