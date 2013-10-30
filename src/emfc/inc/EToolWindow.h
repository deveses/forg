// EToolWindow.h: interface for the EToolWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ETOOLWINDOW_H__BDF7B673_8705_43A5_8618_435E98E06BD9__INCLUDED_)
#define AFX_ETOOLWINDOW_H__BDF7B673_8705_43A5_8618_435E98E06BD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EWnd.h"

namespace emfc {

#define DOCK_EXACT  1 
#define DOCK_LEFT   2 
#define DOCK_RIGHT  4 
#define DOCK_TOP    8 
#define DOCK_BOTTOM 16

class EMFC_API EToolWindow : public EWnd  
{
public:
	RECT m_rtBeforeDock;
	POINT m_lastDockPosition;
	DWORD Create(LPCTSTR lpWindowName, int x, int y, int nWidth, int nHeight, DWORD dwStyle, HWND hWndParent);
	int m_dockMode;
	void SetDockMode(int nMode);
	void UndockWindow();
	void OnClose();
	BOOL m_isDocked;
	BOOL DockWindow(int x, int y);
	EToolWindow();
	virtual ~EToolWindow();

private:
	LONG m_winLongOrginal;
};

}

#endif // !defined(AFX_ETOOLWINDOW_H__BDF7B673_8705_43A5_8618_435E98E06BD9__INCLUDED_)
