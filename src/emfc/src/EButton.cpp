// EButton.cpp: implementation of the EButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EButton.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EButton::EButton()
{

}

EButton::~EButton()
{

}

DWORD EButton::Create(LPCTSTR lpszCaption, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID)
{
    return EWnd::Create(_T("BUTTON"),lpszCaption,x,y,nWidth,nHeight,dwStyle,0,pParentWnd);
}

void EButton::SetCheck(int nCheck)
{
    SendMessage(m_hWnd,BM_SETCHECK, (WPARAM)nCheck,0);
}

BOOL EButton::SetState(BOOL bHighlight)
{
    return SendMessage(m_hWnd,BM_SETSTATE, (WPARAM)bHighlight,0);
}

UINT EButton::GetState()
{
    return SendMessage(m_hWnd,BM_GETSTATE, 0,0);
}

}