// EToolWindow.cpp: implementation of the EToolWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EToolWindow.h"

#define abs(x) ( x>=0 ? x : -x )

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EToolWindow::EToolWindow()
{
    m_isDocked=FALSE;
    m_dockMode=DOCK_EXACT;
}

EToolWindow::~EToolWindow()
{

}



BOOL EToolWindow::DockWindow(int x, int y)
{
    RECT rp;    //rect okna do ktorego dokujemy
    RECT rc;    //rect naszego okna

    GetWindowRect(&rc);
	m_rtBeforeDock=rc;

    //jesli nie zadokowane to zmieniamy styl okna
    if (! m_isDocked) {
        m_winLongOrginal=GetWindowLong(GWL_STYLE);
        ModifyStyle(WS_CAPTION|WS_SIZEBOX,WS_CHILD);
    }

    //jesli nie ma okna nadrzednego to dokujey do pulpitu
    if (m_hWndParent==NULL) m_hWndParent=GetDesktopWindow();
    if (m_hWndParent==NULL) return FALSE;        
    ::GetClientRect(m_hWndParent,&rp);     

    //MapWindowPoints(NULL,m_hWndParent,(LPPOINT)&rp,2);
    MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);

    m_lastDockPosition.x=x;
    m_lastDockPosition.y=y;
    //podana pozycja jest wzgledem pozycji okna nadrzednego
    x+=rp.left;
    y+=rp.top;

    if (m_dockMode==DOCK_RIGHT)
        x=rp.right-(abs(rc.right-rc.left));
    else if (m_dockMode==DOCK_LEFT)
        x=0;
    else if (m_dockMode==DOCK_TOP)
        y=0;
    else if (m_dockMode==DOCK_BOTTOM)
        y=rp.bottom-(abs(rc.bottom-rc.top));
      
    SetWindowPos(HWND_TOP,x,y,0,0,SWP_NOSIZE | SWP_FRAMECHANGED);

    //wymagane po zmianie na WS_CHILD
    if (m_hWndParent!=NULL) SetParent(m_hWnd,m_hWndParent);

    SetWindowPos(HWND_TOP,x,y,0,0,SWP_NOSIZE | SWP_FRAMECHANGED);

    m_isDocked=TRUE;
    return TRUE;
}


void EToolWindow::OnClose()
{
    ShowWindow(SW_HIDE);
}

void EToolWindow::UndockWindow()
{
	ShowWindow(SW_HIDE);
    SetParent(m_hWnd,NULL);
    SetWindowLong(GWL_STYLE,m_winLongOrginal);
    SetWindowPos(HWND_TOP,m_rtBeforeDock.left,m_rtBeforeDock.top,
		m_rtBeforeDock.right-m_rtBeforeDock.left,m_rtBeforeDock.bottom-m_rtBeforeDock.top,
		SWP_FRAMECHANGED);
	ShowWindow(SW_SHOW);
    m_isDocked=FALSE;
}



void EToolWindow::SetDockMode(int nMode)
{
    m_dockMode=nMode;
}

DWORD EToolWindow::Create(LPCTSTR lpWindowName, int x, int y, int nWidth, int nHeight,DWORD dwStyle, HWND hWndParent)
{
    return (EWnd::Create(_T("EToolWindow"),lpWindowName,x,y,nWidth, nHeight,
        WS_CLIPCHILDREN|WS_SYSMENU|dwStyle,
            WS_EX_DLGMODALFRAME|WS_EX_TOOLWINDOW,
        hWndParent));
}


}