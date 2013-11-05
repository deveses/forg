// EWindow.cpp: implementation of the EWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EWnd.h"
#include <winbase.h>
#include <stdio.h>

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


LRESULT CALLBACK EWindowProc(
  HWND hWnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
  ) {
    EWnd *ew=NULL;

    ew=(EWnd *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
    if (ew!=NULL) return ew->WindowProc(uMsg,wParam,lParam);
    else {
	    switch (uMsg) {
            case WM_CLOSE: 
      	        PostQuitMessage(0);
                return 0; 
	    }
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
   }
   //return 0;
  }
 




EWnd::EWnd()
{
    m_hWnd=NULL;
    m_hWndParent=NULL;
    m_hInstance=NULL;
    m_nFlags=0;
}

EWnd::~EWnd()
{
    if (m_hWnd) {
        UnregisterClass((LPCTSTR)(m_aWndClass & 0xffff),m_hInstance);
    }
}

DWORD EWnd::Create(LPCTSTR lpszClassName,
                      LPCTSTR lpWindowName, 
                      int x, int y, 
                      int nWidth, 
                      int nHeight, 
                      DWORD dwStyle, 
                      DWORD dwExStyle, 
                      HWND hWndParent)
{
    WNDCLASSEX wc;

    m_hInstance=GetModuleHandle(NULL);
    m_hWndParent=hWndParent;
    if (! GetClassInfoEx(m_hInstance,lpszClassName,&wc) &&
        ! GetClassInfoEx(NULL,lpszClassName,&wc)) {
        memset(&wc,0,sizeof(WNDCLASSEX));
        wc.cbSize=sizeof(WNDCLASSEX);
        wc.hInstance=m_hInstance;
        wc.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
        wc.lpfnWndProc=(WNDPROC)EWindowProc;
        wc.hCursor=LoadCursor(NULL,IDC_ARROW);
        wc.hbrBackground=/*CreateSolidBrush(RGB(192,192,192));*/GetSysColorBrush(COLOR_3DFACE);//(HBRUSH)GetStockObject(LTGRAY_BRUSH);
        wc.lpszMenuName=NULL;
        wc.lpszClassName=lpszClassName;
        m_aWndClass=RegisterClassEx(&wc);
        if (! m_aWndClass) return GetLastError();
    }

    m_hWnd=CreateWindowEx(dwExStyle, lpszClassName, lpWindowName,
   							dwStyle, x, y,
                      nWidth,nHeight,hWndParent,NULL,m_hInstance,NULL);
    if (m_hWnd==NULL) return GetLastError();

    SetWindowLongPointer(GWLP_USERDATA, (LONG_PTR)this);
    return 0;
}

BOOL EWnd::ShowWindow(int nCmdShow)
{
    return ::ShowWindow(m_hWnd,nCmdShow);
}

LRESULT EWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    
    m_lastMessage.uMsg=uMsg;
    m_lastMessage.wParam=wParam;
    m_lastMessage.lParam=lParam;
    m_lastMessage.lResult=0;

    switch (uMsg) {
        case WM_PAINT :
            OnPaint();
            break;
        case WM_LBUTTONDBLCLK:
            OnLButtonDblClk(wParam, MAKEPOINTS(lParam));
            break;
        case WM_LBUTTONDOWN:
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam); 
            OnLButtonDown(wParam,pt);
            break;        
        case WM_LBUTTONUP:
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam); 
            OnLButtonUp(wParam,pt);
            break;        
		case WM_RBUTTONDBLCLK:
            OnRButtonDblClk(wParam, MAKEPOINTS(lParam));
            break;
        case WM_RBUTTONDOWN:
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam); 
            OnRButtonDown(wParam,pt);
            break;        
        case WM_RBUTTONUP:
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam); 
            OnRButtonUp(wParam,pt);
            break;
        case WM_MOUSEMOVE:
            OnMouseMove(wParam,MAKEPOINTS(lParam));
            break;
        case WM_MOUSEWHEEL:
            OnMouseWheel(wParam,MAKEPOINTS(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
            break;
        case WM_CAPTURECHANGED:
            OnCaptureChanged((HWND)lParam);
            break;
        case WM_MOVE:
            OnMove((int)(short) LOWORD(lParam),(int)(short) HIWORD(lParam));
            break;
        case WM_MOVING:
            OnMoving(wParam,(LPRECT) lParam);
            break;
        case WM_SIZE:
            OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZING:
            OnSizing(wParam, (LPRECT) lParam);
            break;
        case WM_MENUSELECT :
            OnMenuSelect((UINT) LOWORD(wParam),(UINT) HIWORD(wParam), (HMENU) lParam);
            break;
        case WM_PARENTNOTIFY :
            OnParentNotify(LOWORD(wParam),lParam);
            break;
        case WM_ACTIVATE :
            OnActivate(LOWORD(wParam),(HWND) lParam,(BOOL) HIWORD(wParam));
            break;
        case WM_COMMAND:
            if (OnCommand(wParam,lParam)) 
                return 0;
            return DefWindowProc(uMsg,wParam,lParam);
        case WM_NOTIFY:
            if (! OnNotify(wParam,lParam,&m_lastMessage.lResult))
                DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
            break;    
        case WM_KEYDOWN:
            OnKeyDown((UINT) wParam,lParam & 0xffff,lParam>>16);
            return 0;
        case WM_KEYUP:
            OnKeyUp((UINT) wParam,lParam & 0xffff,lParam>>16);
            return 0;
        case WM_CTLCOLORBTN:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_BTN);
        case WM_CTLCOLORDLG:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_DLG);
        case WM_CTLCOLOREDIT:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_EDIT);
        case WM_CTLCOLORLISTBOX:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_LISTBOX);
        case WM_CTLCOLORMSGBOX:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_MSGBOX);
        case WM_CTLCOLORSCROLLBAR:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_SCROLLBAR);
        case WM_CTLCOLORSTATIC:
            return (LRESULT)OnCtlColor((HDC)wParam, (HWND)lParam, CTLCOLOR_STATIC);
        case WM_SYSCOMMAND :
            OnSysCommand(wParam,lParam);
            break;
        case WM_TIMER :
            OnTimer(wParam);
            break;
        case WM_INITMENU :
            OnInitMenu((HMENU)wParam);
            break;
        case WM_CLOSE : OnClose();

        default:
             return DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
    }
    //DefWindowProc(m_hWnd,uMsg,wParam,lParam);
    return m_lastMessage.lResult;
}

BOOL EWnd::DestroyWindow()
{
    BOOL r;
    r= ::DestroyWindow(m_hWnd);
    if (r) m_hWnd=NULL;
    return r;
}

HWND EWnd::ChildWindowFromPoint(POINT point, UINT nFlags)
{
    return ::ChildWindowFromPointEx(m_hWnd,point,nFlags);
}

BOOL EWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
    LONG l=GetWindowLong(GWL_STYLE);
    LONG t=0xffffffff;
    BOOL ret;
    
    //style do usuniecia
    t^=dwRemove;      
    l&=t;
    //style dodane
    l|=dwAdd;
    
    ret=(SetWindowLong(GWL_STYLE,l)!=0);
    if (nFlags!=0) SetWindowPos(NULL,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    return ret;
}


BOOL EWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
    LONG l=GetWindowLong(GWL_EXSTYLE);
    LONG t=0xffffffff;
    BOOL ret;
    
    //style do usuniecia
    t^=dwRemove;      
    l&=t;
    //style dodane
    l|=dwAdd;
    
    ret=(SetWindowLong(GWL_EXSTYLE,l)!=0);
    if (nFlags!=0) SetWindowPos(NULL,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    return ret;
}

inline HWND EWnd::GetSafeHwnd() const
{
	return this == NULL ? NULL : m_hWnd;
}

void EWnd::CenterWindow(EWnd* pAlternateOwner)
{
	DWORD dwStyle = GetStyle();
	HWND hWndCenter = pAlternateOwner->GetSafeHwnd();
	if (pAlternateOwner == NULL)
	{
		if (dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(m_hWnd);
		else
			hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
		if (hWndCenter != NULL)
		{
			// let parent determine alternate center window
			HWND hWndTemp =
				(HWND)::SendMessage(hWndCenter, WM_QUERYCENTERWND, 0, 0);
			if (hWndTemp != NULL)
				hWndCenter = hWndTemp;
		}
	}

	// get coordinates of the window relative to its parent
	RECT rcDlg;
	GetWindowRect(&rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	if (!(dwStyle & WS_CHILD))
	{
		// don't center against invisible or minimized windows
		if (hWndCenter != NULL)
		{
			DWORD dwStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if (!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		MONITORINFO mi;
		mi.cbSize = sizeof(mi);

		// center within appropriate monitor coordinates
		if (hWndCenter == NULL)
		{
			HWND hwDefault = NULL; //AfxGetMainWnd()->GetSafeHwnd();

			GetMonitorInfo(
				MonitorFromWindow(hwDefault, MONITOR_DEFAULTTOPRIMARY), &mi);
			rcCenter = mi.rcWork;
			rcArea = mi.rcWork;
		}
		else
		{
			::GetWindowRect(hWndCenter, &rcCenter);
			GetMonitorInfo(
				MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST), &mi);
			rcArea = mi.rcWork;
		}
	}
	else
	{
		// center within parent client coordinates
		hWndParent = ::GetParent(m_hWnd);
		//ASSERT(::IsWindow(hWndParent));

		::GetClientRect(hWndParent, &rcArea);
		//ASSERT(::IsWindow(hWndCenter));
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	// find dialog's upper left based on rcCenter
	int rcDlgWidth = rcDlg.right - rcDlg.left;
	int rcDlgHeight = rcDlg.bottom - rcDlg.top;
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - rcDlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - rcDlgHeight / 2;

	// if the dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + rcDlgWidth > rcArea.right)
		xLeft = rcArea.right - rcDlgWidth;

	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + rcDlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - rcDlgHeight;

	// map screen coordinates to child coordinates
	SetWindowPos(NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


HWND EWnd::SetFocus()
{
    return (::SetFocus(m_hWnd));
}

void EWnd::SetFont(HFONT pFont, BOOL bRedraw)
{
    SendMessage(m_hWnd,WM_SETFONT,(WPARAM) pFont,MAKELPARAM(bRedraw, 0));
}


BOOL EWnd::SetMenu(HMENU hMenu)
{
    return (::SetMenu(m_hWnd,hMenu));
}

void EWnd::SetBkBrush(HBRUSH hBrush)
{
    SetClassLongPtr(m_hWnd,GCLP_HBRBACKGROUND,(LONG_PTR) hBrush);
}

HICON EWnd::SetIcon(HICON hIcon, BOOL bBigIcon)
{
    return (HICON)SendMessage(m_hWnd,WM_SETICON,(bBigIcon ? ICON_BIG : ICON_SMALL),(LPARAM)hIcon);
}

BOOL EWnd::SetWindowPos(HWND pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
    return (::SetWindowPos(m_hWnd,pWndInsertAfter,x,y,cx,cy,nFlags));
}

LONG EWnd::SetWindowLong(int nIndex, LONG dwNewLong)
{
    return ::SetWindowLong(m_hWnd,nIndex,dwNewLong);
}

LONG_PTR EWnd::SetWindowLongPointer(int nIndex, LONG_PTR dwNewLong)
{
	return ::SetWindowLongPtr(m_hWnd, nIndex, dwNewLong);
}

void EWnd::SetWindowText(LPCTSTR lpszString)
{
    ::SetWindowText(m_hWnd,lpszString);
}

HWND EWnd::SetCapture()
{
    return ::SetCapture(m_hWnd);
}

HWND EWnd::GetCapture()
{
    return ::GetCapture();
}

HMENU EWnd::GetMenu()
{
	return ::GetMenu(m_hWnd);
}


HDC EWnd::GetWindowDC()
{
    return (::GetWindowDC(m_hWnd));
}


HDC EWnd::GetDC()
{
    return ::GetDC(m_hWnd);
}

LONG EWnd::GetWindowLong(int nIndex) 
{
    return ::GetWindowLong(m_hWnd,nIndex);
}

LONG_PTR EWnd::GetWindowLongPointer(int nIndex)
{
	return ::GetWindowLongPtr(m_hWnd, nIndex);
}

void EWnd::GetWindowRect(LPRECT lpRect)
{
    ::GetWindowRect(m_hWnd,lpRect);
}

void EWnd::GetClientRect(LPRECT lpRect)
{
    ::GetClientRect(m_hWnd,lpRect);
}

HWND EWnd::GetDlgItem(int nID) const
{
    return ::GetDlgItem(m_hWnd,nID);
}

void EWnd::GetDlgItem(int nID, HWND *phWnd)
{
    HWND hwnd=GetDlgItem(nID);
    *phWnd=hwnd;
}

EWnd * EWnd::FromHandlePermanent(HWND hWnd)
{
    EWnd *pWnd=NULL;

    pWnd=(EWnd *)::GetWindowLongPtr(hWnd,GWLP_USERDATA);

    return pWnd;
}


void EWnd::Invalidate(BOOL bErase)
{
    ::InvalidateRect(m_hWnd,NULL,bErase);
}

void EWnd::OnClose()
{
    DestroyWindow();
    PostQuitMessage(0);
}

void EWnd::OnMove(int x, int y)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
}

void EWnd::OnMoving(UINT nSide, LPRECT lpRect)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnSize(UINT nType, int cx, int cy)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnSizing(UINT nSide, LPRECT lpRect)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}



void EWnd::OnParentNotify(UINT message, LPARAM lParam)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnLButtonDblClk(UINT nFlags, POINTS point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
}

void EWnd::OnLButtonDown(UINT nFlags, POINT point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnLButtonUp(UINT nFlags, POINT point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnRButtonDblClk(UINT nFlags, POINTS point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
}

void EWnd::OnRButtonDown(UINT nFlags, POINT point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnRButtonUp(UINT nFlags, POINT point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}

void EWnd::OnPaint()
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}


void EWnd::OnActivate(UINT nState, HWND pWndOther, BOOL bMinimized)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);

}


BOOL EWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}





HBRUSH EWnd::OnCtlColor(HDC pDC, HWND pWnd, UINT nCtlColor)
{
    return (HBRUSH)DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam);
}

void EWnd::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 
}


void EWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 
}

void EWnd::OnMouseMove(UINT nFlags, POINTS point)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}

void EWnd::OnMouseWheel(UINT nFlags, POINTS point, int delta)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}

void EWnd::OnCaptureChanged(HWND pWnd)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}

BOOL EWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    //DefWindowProc(m_hWnd,m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 
    return 0;
}


void EWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}


void EWnd::OnTimer(UINT nIDEvent)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}

void EWnd::OnInitMenu(HMENU pMenu)
{
    DefWindowProc(m_lastMessage.uMsg,m_lastMessage.wParam,m_lastMessage.lParam); 

}




BOOL EWnd::ExecuteDlgInit(LPCTSTR lpszResourceName)
{
   // find resource handle
    LPVOID lpResource = NULL;
    HGLOBAL hResource = NULL;
   if (lpszResourceName != NULL)
   {
       /*szukamy tylko w aktualnym procesie,
        w mfc szuka takze po powiazanych dllach*/
      HINSTANCE hInst = GetModuleHandle(NULL); //AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
      HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DIALOG);
      if (hDlgInit != NULL) {
         // load it
         hResource = LoadResource(hInst, hDlgInit);
         if (hResource == NULL) return FALSE;
         // lock it
         lpResource = LockResource(hResource);
      }
   }

   // execute it
   BOOL bResult=ExecuteDlgInit(lpResource);

   // cleanup
   if (lpResource != NULL && hResource != NULL)
   {
      UnlockResource(hResource);
      FreeResource(hResource);
   }
   return bResult;
}





BOOL EWnd::ExecuteDlgInit(LPVOID lpResource)
{
  BOOL bSuccess = TRUE;
  /* if (lpResource != NULL)
   {
      UNALIGNED WORD* lpnRes = (WORD*)lpResource;
      while (bSuccess && *lpnRes != 0)
      {
         WORD nIDC = *lpnRes++;
         WORD nMsg = *lpnRes++;
         DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;

         // In Win32 the WM_ messages have changed.  They have
         // to be translated from the 32-bit values to 16-bit
         // values here.

         #define WIN16_LB_ADDSTRING  0x0401
         #define WIN16_CB_ADDSTRING  0x0403
         #define AFX_CB_ADDSTRING    0x1234

         // unfortunately, WIN16_CB_ADDSTRING == CBEM_INSERTITEM
         if (nMsg == AFX_CB_ADDSTRING)
            nMsg = CBEM_INSERTITEM;
         else if (nMsg == WIN16_LB_ADDSTRING)
            nMsg = LB_ADDSTRING;
         else if (nMsg == WIN16_CB_ADDSTRING)
            nMsg = CB_ADDSTRING;

         // check for invalid/unknown message types
//#ifdef _AFX_NO_OCC_SUPPORT
         ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
            nMsg == CBEM_INSERTITEM);
#else
         ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
            nMsg == CBEM_INSERTITEM ||
            nMsg == WM_OCC_LOADFROMSTREAM ||
            nMsg == WM_OCC_LOADFROMSTREAM_EX ||
            nMsg == WM_OCC_LOADFROMSTORAGE ||
            nMsg == WM_OCC_LOADFROMSTORAGE_EX ||
            nMsg == WM_OCC_INITNEW);
#endif

#ifdef _DEBUG
         // For AddStrings, the count must exactly delimit the
         // string, including the NULL termination.  This check
         // will not catch all mal-formed ADDSTRINGs, but will
         // catch some.
         if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING || nMsg == CBEM_INSERTITEM)
            ASSERT(*((LPBYTE)lpnRes + (UINT)dwLen - 1) == 0);
#endif

         if (nMsg == CBEM_INSERTITEM)
         {
            USES_CONVERSION;
            COMBOBOXEXITEM item;
            item.mask = CBEIF_TEXT;
            item.iItem = -1;
            item.pszText = A2T(LPSTR(lpnRes));

            if (::SendDlgItemMessage(m_hWnd, nIDC, nMsg, 0, (LPARAM) &item) == -1)
               bSuccess = FALSE;
         }
#ifndef _AFX_NO_OCC_SUPPORT
         else if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
#endif // !_AFX_NO_OCC_SUPPORT
         {
            // List/Combobox returns -1 for error
            if (::SendDlgItemMessageA(m_hWnd, nIDC, nMsg, 0, (LPARAM) lpnRes) == -1)
               bSuccess = FALSE;
         }


         // skip past data
         lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
      }
   }

   // send update message to all controls after all other siblings loaded
  // if (bSuccess)
  //    SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, FALSE, FALSE);
*/
   return bSuccess;
}

void EWnd::SendMessageToDescendants( UINT message, WPARAM wParam , LPARAM lParam , BOOL bDeep , BOOL bOnlyPerm )
{

}

BOOL EWnd::UpdateData(BOOL bSaveAndValidate)
{
   if (! ::IsWindow(m_hWnd)) return FALSE;

   EDataExchange dx(this, bSaveAndValidate);

//   HWND hWndOldLockout = pThreadState->m_hLockoutNotifyWindow;
//   ASSERT(hWndOldLockout != m_hWnd);   // must not recurse
//   pThreadState->m_hLockoutNotifyWindow = m_hWnd;

   BOOL bOK = FALSE;       // assume failure
 //  try {
       DoDataExchange(&dx);
       bOK = TRUE;         // it worked
  // } catch () {
 //  }

   return TRUE;
}

BOOL EWnd::PreTranslateMessage(MSG *pMsg)
{
    return FALSE;
}


LRESULT EWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(m_hWnd,message,wParam,lParam);
}

DWORD EWnd::GetStyle()
{
    return GetWindowLong(GWL_STYLE);
}

int EWnd::RunModalLoop(DWORD dwFlags)
{
    if (m_hWnd==NULL || (m_nFlags & WF_MODALLOOP)) return -1; // window must be created

   // for tracking the idle time state
   BOOL bIdle = TRUE;
   LONG lIdleCount = 0;
   BOOL bShowIdle = (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
   HWND hWndParent = ::GetParent(m_hWnd);
   m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);
   MSG  m_msgCur;
   MSG* pMsg = &m_msgCur;

   // acquire and dispatch messages until the modal state is done
   for (;;)
   {
      ContinueModal();

      // phase1: check to see if we can do idle work
      while (bIdle &&
         !::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
      {
         ContinueModal();

         // show the dialog when the message queue goes idle
         if (bShowIdle)
         {
            ShowWindow(SW_SHOWNORMAL);
            UpdateWindow();
            bShowIdle = FALSE;
         }

         // call OnIdle while in bIdle state
         if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
         {
            // send WM_ENTERIDLE to the parent
            ::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
         }
         if ((dwFlags & MLF_NOKICKIDLE) ||
            !SendMessage(m_hWnd, WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
         {
            // stop idle processing next time
            bIdle = FALSE;
         }
      }

      // phase2: pump messages while available
      do
      {
         ContinueModal();

         // pump message, but quit on WM_QUIT
         switch (GetMessage(pMsg, NULL, NULL, NULL)) {
         case 0: 
             PostQuitMessage(0);
             return -1;
         case -1:
             return -1;
         default:
             TranslateMessage(pMsg);
             DispatchMessage(pMsg);
         }

         // show the window when certain special messages rec'd
         if (bShowIdle &&
            (pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
         {
            ShowWindow(SW_SHOWNORMAL);
            UpdateWindow();
            bShowIdle = FALSE;
         }

         if (!ContinueModal())
            goto ExitModal;

         // reset "no idle" state after pumping "normal" message
         if (pMsg->message != WM_PAINT && pMsg->message != 0x0118)
         {
            bIdle = TRUE;
            lIdleCount = 0;
         }

      } while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
   }

ExitModal:
   m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
   return m_nModalResult;
}

BOOL EWnd::ContinueModal()
{
    return m_nFlags & WF_CONTINUEMODAL;
}

void EWnd::EndModalLoop(int nResult)
{
    if(m_hWnd==NULL) return;

    m_nModalResult = nResult;

    if (m_nFlags & WF_CONTINUEMODAL)
    {
        m_nFlags &= ~WF_CONTINUEMODAL;
        PostMessage(WM_NULL);
    }
}

void EWnd::UpdateWindow()
{
    ::UpdateWindow(m_hWnd);
}

BOOL EWnd::PostMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    return ::PostMessage(m_hWnd, message, wParam, lParam);
}




void EWnd::DoDataExchange(EDataExchange *pDX)
{

}





//****************data exchange

EDataExchange::EDataExchange(EWnd* pDlgWnd, BOOL bSaveAndValidate)
{
   m_bSaveAndValidate = bSaveAndValidate;
   m_pDlgWnd = pDlgWnd;
   m_hWndLastControl = NULL;
}
HWND EDataExchange::PrepareEditCtrl(int nIDC)
{
   HWND hWndCtrl = PrepareCtrl(nIDC);
   //ASSERT(hWndCtrl != NULL);
   m_bEditLastControl = TRUE;
   return hWndCtrl;
}
HWND EDataExchange::PrepareCtrl(int nIDC)
{
//   ASSERT(nIDC != 0);
//   ASSERT(nIDC != -1); // not allowed
   HWND hWndCtrl;
   m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
   if (hWndCtrl == NULL)
   {
//      TRACE1("Error: no data exchange control with ID 0x%04X.\n", nIDC);
//      ASSERT(FALSE);
//      AfxThrowNotSupportedException();
   }
   m_hWndLastControl = hWndCtrl;
   m_bEditLastControl = FALSE; // not an edit item by default
//   ASSERT(hWndCtrl != NULL);   // never return NULL handle
   return hWndCtrl;
}


/*
void DDX_TextWithFormat(EDataExchange* pDX, int nIDC,
   LPCTSTR lpszFormat, ...)
   // only supports windows output formats - no floating point
{
   va_list pData;
   va_start(pData, lpszFormat);

   HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   TCHAR szT[256];
   
   if (pDX->m_bSaveAndValidate)
   {
      // the following works for %d, %u, %ld, %lu
      ::GetWindowText(hWndCtrl, szT, sizeof(szT));
      sscanf(szT, lpszFormat, pData);

   }
   else
   {
      wvsprintf(szT, lpszFormat, pData);
         // does not support floating point numbers - see dlgfloat.cpp
      SetWindowText(hWndCtrl, szT);
   }

   va_end(pData);
}*/


void EMFC_API DDX_Text(EDataExchange* pDX, int nIDC, DWORD& value)
{
    TCHAR szT[32];
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
     
    if (pDX->m_bSaveAndValidate) {
      ::GetWindowText(hWndCtrl, szT, sizeof(szT));
	  _stscanf(szT, _T("%lu"), &value);
      //DDX_TextWithFormat(pDX, nIDC, "%lu", &value);
    } else {
      _stprintf(szT, _T("%lu"), value);
         // does not support floating point numbers - see dlgfloat.cpp
      SetWindowText(hWndCtrl, szT);
      //DDX_TextWithFormat(pDX, nIDC, "%lu", value);
   }
}
void EMFC_API DDX_Text(EDataExchange* pDX, int nIDC, LPTSTR value, int nMaxLen)
{
   //ASSERT(nMaxLen != 0);

   HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   if (pDX->m_bSaveAndValidate)
   {
      int nLen = ::GetWindowTextLength(hWndCtrl);
      int nRetrieved = ::GetWindowText(hWndCtrl, value, nMaxLen);
      //if (nLen > nRetrieved)
      //   TRACE1("Text in control ID %d is too long. Call DDV_MaxChars()!\n", nIDC);
   }
   else
   {
      SetWindowText(hWndCtrl, value);
   }
}
void EMFC_API DDX_Radio(EDataExchange* pDX, int nIDC, int& value)
   // must be first in a group of auto radio buttons
{
   HWND hWndCtrl = pDX->PrepareCtrl(nIDC);

//   ASSERT(::GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP);
//   ASSERT(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

   if (pDX->m_bSaveAndValidate)
      value = -1;     // value if none found

   // walk all children in group
   int iButton = 0;
   do
   {
      if (::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON)
      {
         // control in group is a radio button
         if (pDX->m_bSaveAndValidate)
         {
            if (::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) != 0)
            {
//               ASSERT(value == -1);    // only set once
               value = iButton;
            }
         }
         else
         {
            // select button
            ::SendMessage(hWndCtrl, BM_SETCHECK, (iButton == value), 0L);
         }
         iButton++;
      }
      else
      {
//         TRACE0("Warning: skipping non-radio button in group.\n");
      }
      hWndCtrl = ::GetWindow(hWndCtrl, GW_HWNDNEXT);

   } while (hWndCtrl != NULL &&
      !(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP));
}

void EMFC_API DDX_CBIndex(EDataExchange* pDX, int nIDC, int& index)
{
   HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
   if (pDX->m_bSaveAndValidate)
      index = (int)::SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L);
   else

      ::SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM)index, 0L);
}


}