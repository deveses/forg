// EStatusBarCtrl.cpp: implementation of the EStatusBarCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EStatusBarCtrl.h"

#include <CommCtrl.h>

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EStatusBarCtrl::EStatusBarCtrl()
{

}

EStatusBarCtrl::~EStatusBarCtrl()
{

}

DWORD EStatusBarCtrl::Create(DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID)
{
    return EWnd::Create(STATUSCLASSNAME,_T(""),x,y,nWidth,nHeight,dwStyle,0,pParentWnd);
}

BOOL EStatusBarCtrl::SetText(LPCTSTR lpszText, int nPane, int nType)
{
    return (BOOL)SendMessage(m_hWnd,SB_SETTEXT,(WPARAM) (nPane | nType),(LPARAM) lpszText);
}

BOOL EStatusBarCtrl::SetParts(int nParts, int *pWidths)
{
    return (BOOL)SendMessage(m_hWnd,SB_SETPARTS,(WPARAM) nParts,(LPARAM) (LPINT) pWidths);
}

}