// EApplication.cpp: implementation of the EApplication class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EApplication.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EApplication::EApplication()
{
    m_hInstance=GetModuleHandle(NULL);
}

EApplication::~EApplication()
{

}

DWORD EApplication::Run()
{
    BOOL bIdle = TRUE;
    LONG lIdleCount = 0;

   // acquire and dispatch messages until a WM_QUIT message is received.
   for (;;)
   {
      // phase1: check to see if we can do idle work
      while (bIdle &&
         !::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
      {
         // call OnIdle while in bIdle state
         if (!OnIdle(lIdleCount++))
            bIdle = FALSE; // assume "no idle" state
      }
            // phase2: pump messages while available
      do
      {
         // pump message, but quit on WM_QUIT
         if (!PumpMessage())
            return ExitInstance();

         // reset "no idle" state after pumping "normal" message
         if (IsIdleMessage(&m_msgCur))
         {
            bIdle = TRUE;
            lIdleCount = 0;
         }

      } while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));

   }
/*
	while (! done){
   		if (GetMessage(&msg,NULL,0,0)>=0) {
            if (msg.message==WM_QUIT) {
                done=true;
            } else {
         		TranslateMessage(&msg);
				DispatchMessage(&msg); 
			}
        } else {
            ErrorBox(0);
        }
    }*/

    return 0;
}

DWORD EApplication::Create(HWND hWnd)
{
    return 0;
}

DWORD EApplication::ErrorBox(DWORD nr)
{
	LPVOID lpMsgBuf;

	if (FormatMessage( 
		 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		 FORMAT_MESSAGE_FROM_SYSTEM | 
		 FORMAT_MESSAGE_IGNORE_INSERTS,
		 NULL,
		 (nr ? nr : GetLastError()),
		 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		 (LPTSTR) &lpMsgBuf,
		 0,
		 NULL 
		 )!=0) {
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
		LocalFree( lpMsgBuf );
	} else return GetLastError();
    return 0;
}

BOOL EApplication::InitApplication()
{

    return TRUE;
}

void EApplication::InitCommonControls()
{
    INITCOMMONCONTROLSEX icc;


    icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
}

HICON EApplication::LoadIcon(LPCTSTR lpszResourceName)
{
    return ::LoadIcon(m_hInstance,lpszResourceName);
}

HICON EApplication::LoadIcon(UINT nIDResource)
{
    return ::LoadIcon(m_hInstance,MAKEINTRESOURCE(nIDResource));
}

HCURSOR EApplication::LoadCursor(UINT nIDResource)
{
    return ::LoadCursor(m_hInstance,MAKEINTRESOURCE(nIDResource));
}

HCURSOR EApplication::LoadStandardCursor(LPCTSTR lpszCursorName)
{
    return ::LoadCursor(NULL,lpszCursorName);
}

BOOL EApplication::OnIdle(LONG lCount)
{
    return FALSE;
}

BOOL EApplication::IsIdleMessage(MSG *pMsg)
{
    return pMsg->message != WM_PAINT && pMsg->message != 0x0118;
}

int EApplication::ExitInstance()
{
    return m_msgCur.wParam;
}

BOOL EApplication::PumpMessage()
{
    int i=::GetMessage(&m_msgCur, NULL, NULL, NULL);
    if (i<=0) {
        if (i<0) ErrorBox();
        return FALSE;
    }

    if (/*m_msgCur.message != WM_KICKIDLE &&*/ !PreTranslateMessage(&m_msgCur)) {
      ::TranslateMessage(&m_msgCur);
      ::DispatchMessage(&m_msgCur);
    }
   return TRUE;
}

BOOL EApplication::PreTranslateMessage(MSG *pMsg)
{

    return FALSE;
}

}