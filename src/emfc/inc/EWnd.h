// EWindow.h: interface for the EWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EWINDOW_H__C8C45B8D_0EC4_42FE_9340_1589D360C277__INCLUDED_)
#define AFX_EWINDOW_H__C8C45B8D_0EC4_42FE_9340_1589D360C277__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#include "base.h"

namespace emfc {

const int MIN_WINDOW_WIDTH = 480;
const int MIN_WINDOW_HEIGHT = 360;

#define WM_QUERYAFXWNDPROC  0x0360  // lResult = 1 if processed by AfxWndProc
#define WM_SIZEPARENT       0x0361  // lParam = &AFX_SIZEPARENTPARAMS
#define WM_SETMESSAGESTRING 0x0362  // wParam = nIDS (or 0),
                           // lParam = lpszOther (or NULL)
#define WM_IDLEUPDATECMDUI  0x0363  // wParam == bDisableIfNoHandler
#define WM_INITIALUPDATE    0x0364  // (params unused) - sent to children
#define WM_COMMANDHELP      0x0365  // lResult = TRUE/FALSE,
                           // lParam = dwContext
#define WM_HELPHITTEST      0x0366  // lResult = dwContext,
                           // lParam = MAKELONG(x,y)
#define WM_EXITHELPMODE     0x0367  // (params unused)
#define WM_RECALCPARENT     0x0368  // force RecalcLayout on frame window
                           //  (only for inplace frame windows)
#define WM_SIZECHILD        0x0369  // special notify from COleResizeBar
                           // wParam = ID of child window
                           // lParam = lpRectNew (new position/size)
#define WM_KICKIDLE         0x036A  // (params unused) causes idles to kick in
#define WM_QUERYCENTERWND   0x036B  // lParam = HWND to use as centering parent
#define WM_DISABLEMODAL     0x036C  // lResult = 0, disable during modal state
                           // lResult = 1, don't disable
#define WM_FLOATSTATUS      0x036D  // wParam combination of FS_* flags below

// WM_ACTIVATETOPLEVEL is like WM_ACTIVATEAPP but works with hierarchies
//   of mixed processes (as is the case with OLE in-place activation)
#define WM_ACTIVATETOPLEVEL 0x036E  // wParam = nState (like WM_ACTIVATE)
                           // lParam = pointer to HWND[2]
                           //  lParam[0] = hWnd getting WM_ACTIVATE
                           //  lParam[1] = hWndOther

#define WM_QUERY3DCONTROLS  0x036F  // lResult != 0 if 3D controls wanted

// Note: Messages 0x0370, 0x0371, and 0x372 were incorrectly used by
//  some versions of Windows.  To remain compatible, MFC does not
//  use messages in that range.
#define WM_RESERVED_0370    0x0370
#define WM_RESERVED_0371    0x0371
#define WM_RESERVED_0372    0x0372

// WM_SOCKET_NOTIFY and WM_SOCKET_DEAD are used internally by MFC's
// Windows sockets implementation.  For more information, see sockcore.cpp
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374

// same as WM_SETMESSAGESTRING except not popped if IsTracking()
#define WM_POPMESSAGESTRING 0x0375

// WM_HELPPROMPTADDR is used internally to get the address of
//  m_dwPromptContext from the associated frame window. This is used
//  during message boxes to setup for F1 help while that msg box is
//  displayed. lResult is the address of m_dwPromptContext.
#define WM_HELPPROMPTADDR   0x0376


// CWnd::m_nFlags (generic to CWnd)
#define WF_TOOLTIPS         0x0001  // window is enabled for tooltips
#define WF_TEMPHIDE         0x0002  // window is temporarily hidden
#define WF_STAYDISABLED     0x0004  // window should stay disabled
#define WF_MODALLOOP        0x0008  // currently in modal loop
#define WF_CONTINUEMODAL    0x0010  // modal loop should continue running
#define WF_OLECTLCONTAINER  0x0100  // some descendant is an OLE control
#define WF_TRACKINGTOOLTIPS 0x0400  // window is enabled for tracking tooltips

// CWnd::m_nFlags (specific to CFrameWnd)
#define WF_STAYACTIVE       0x0020  // look active even though not active
#define WF_NOPOPMSG         0x0040  // ignore WM_POPMESSAGESTRING calls
#define WF_MODALDISABLE     0x0080  // window is disabled
#define WF_KEEPMINIACTIVE   0x0200  // stay activate even though you are deactivated

// flags for CWnd::RunModalLoop
#define MLF_NOIDLEMSG       0x0001  // don't send WM_ENTERIDLE messages
#define MLF_NOKICKIDLE      0x0002  // don't send WM_KICKIDLE messages
#define MLF_SHOWONIDLE      0x0004  // show window if not visible at idle time



class EMFC_API WndMessage {
public: 
    UINT uMsg;    
    WPARAM wParam;
    LPARAM lParam;
    LRESULT lResult;
    WndMessage(){}
    virtual ~WndMessage(){}
};

class EDataExchange;

class EMFC_API EWnd  
{
protected:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	ATOM m_aWndClass;
	HWND m_hWndParent;

public:
	EWnd();
	virtual ~EWnd();

public:
	int m_nModalResult;
	UINT m_nFlags;
	WndMessage m_lastMessage;

    virtual BOOL PreTranslateMessage( MSG* pMsg );
	void GetDlgItem( int nID, HWND* phWnd );
	virtual void DoDataExchange(EDataExchange* pDX);
	virtual void OnInitMenu( HMENU pMenu );
	BOOL PostMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 );
	void UpdateWindow();
	void EndModalLoop(int nResult);
	BOOL ContinueModal();
	int RunModalLoop(DWORD dwFlags);
	virtual LRESULT DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam );
	BOOL UpdateData(BOOL bSaveAndValidate);
	void SendMessageToDescendants( UINT message, WPARAM wParam = 0, LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE );

	virtual void OnActivate( UINT nState, HWND pWndOther, BOOL bMinimized );
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	virtual void OnSysCommand( UINT nID, LPARAM lParam );
	virtual void OnRButtonUp(UINT nFlags, POINT point);
	virtual void OnRButtonDown(UINT nFlags, POINT point);
	virtual void OnRButtonDblClk(UINT nFlags, POINTS point);
	virtual void OnCaptureChanged(HWND pWnd);
	virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
    virtual void OnMouseMove( UINT nFlags, POINTS point );
    virtual void OnMouseWheel( UINT nFlags, POINTS point, int nDelta );
	virtual void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual HBRUSH OnCtlColor( HDC pDC, HWND pWnd, UINT nCtlColor );
    virtual void OnPaint();
	virtual void OnLButtonUp( UINT nFlags, POINT point );
	virtual void OnLButtonDown( UINT nFlags, POINT point );
	virtual void OnParentNotify( UINT message, LPARAM lParam );
	virtual void OnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu );
    virtual void OnSizing( UINT nSide, LPRECT lpRect );
	virtual void OnSize(UINT nType, int cx, int cy);
	virtual void OnMoving( UINT nSide, LPRECT lpRect );
	virtual void OnMove(int x, int y);
	virtual void OnLButtonDblClk(UINT nFlags, POINTS point);
	virtual void OnClose();
   	virtual void OnTimer( UINT nIDEvent );


	void CenterWindow(EWnd* pAlternateOwner = NULL);
	BOOL ExecuteDlgInit(LPVOID lpResource);
	BOOL ExecuteDlgInit(LPCTSTR lpszResourceName);
	HWND GetDlgItem( int nID ) const;
	EWnd * FromHandlePermanent( HWND hWnd );
	HWND GetSafeHwnd() const;
	HMENU GetMenu();
	DWORD GetStyle( );
	HWND GetCapture( );
	HWND SetCapture( );
	void SetWindowText( LPCTSTR lpszString );
	void GetClientRect( LPRECT lpRect ) ;
	void GetWindowRect( LPRECT lpRect ) ;
	BOOL ModifyStyle( DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0 );
	LONG GetWindowLong( int nIndex ) ;
	LONG SetWindowLong( int nIndex, LONG dwNewLong );
	BOOL ModifyStyleEx( DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0 );
	void SetFont(HFONT pFont, BOOL bRedraw = TRUE );
	void Invalidate( BOOL bErase=TRUE);
	HWND ChildWindowFromPoint( POINT point, UINT nFlags=CWP_ALL );
	HDC GetDC();
	HICON SetIcon( HICON hIcon, BOOL bBigIcon );
	void SetBkBrush(HBRUSH hBrush);
	BOOL SetWindowPos( HWND pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags );
	HDC GetWindowDC();
	HWND SetFocus();    
    BOOL SetMenu(HMENU hMenu);
	BOOL DestroyWindow();
	virtual LRESULT WindowProc(UINT uMsg,  WPARAM wParam, LPARAM lParam);
	BOOL ShowWindow(int nCmdShow);
	DWORD Create(LPCTSTR lpszClassName,
        LPCTSTR lpWindowName, int x, int y,
        int nWidth=MIN_WINDOW_WIDTH, int nHeight=MIN_WINDOW_HEIGHT, 
        DWORD dwStyle=0, DWORD dwExStyle=0, 
        HWND hWndParent=NULL);

};

class EMFC_API EDataExchange  
{

public:
   BOOL m_bSaveAndValidate;   // TRUE => save and validate data
   EWnd* m_pDlgWnd;           // container usually a dialog

   HWND PrepareCtrl(int nIDC);     // return HWND of control
   HWND PrepareEditCtrl(int nIDC); // return HWND of control
   void Fail();                    // will throw exception

   EDataExchange(EWnd* pDlgWnd, BOOL bSaveAndValidate);

   HWND m_hWndLastControl;    // last control used (for validation)
   BOOL m_bEditLastControl;   // last control was an edit item


};

void EMFC_API DDX_Text(EDataExchange* pDX, int nIDC, DWORD& value);
void EMFC_API DDX_Text(EDataExchange* pDX, int nIDC, LPTSTR value, int nMaxLen);
void EMFC_API DDX_Check(EDataExchange* pDX, int nIDC, int& value);
void EMFC_API DDX_Radio(EDataExchange* pDX, int nIDC, int& value);
void EMFC_API DDX_LBString(EDataExchange* pDX, int nIDC, LPTSTR value);
void EMFC_API DDX_CBString(EDataExchange* pDX, int nIDC, LPTSTR value);
void EMFC_API DDX_LBIndex(EDataExchange* pDX, int nIDC, int& index);
void EMFC_API DDX_CBIndex(EDataExchange* pDX, int nIDC, int& index);
void EMFC_API DDX_LBStringExact(EDataExchange* pDX, int nIDC, LPTSTR value);
void EMFC_API DDX_CBStringExact(EDataExchange* pDX, int nIDC, LPTSTR value);
void EMFC_API DDX_Scroll(EDataExchange* pDX, int nIDC, int& value);
void EMFC_API DDX_Slider(EDataExchange* pDX, int nIDC, int& value);

}

#endif // !defined(AFX_EWINDOW_H__C8C45B8D_0EC4_42FE_9340_1589D360C277__INCLUDED_)
