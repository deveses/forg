// EEdit.cpp: implementation of the EEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EEdit.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EEdit::EEdit()
{

}

EEdit::~EEdit()
{

}

DWORD EEdit::Create(DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID)
{
    return EWnd::Create(_T("EDIT"),_T(""),x,y,nWidth,nHeight,dwStyle,0,pParentWnd);
}

void EEdit::SetLimitText(UINT nMax)
{
    SendMessage(m_hWnd,EM_SETLIMITTEXT,(WPARAM)nMax,0);
}

int EEdit::GetLineCount( ) const
{
    return SendMessage(m_hWnd,EM_GETLINECOUNT,0,0);
}

void EEdit::LineScroll(int nLines, int nChars)
{
    SendMessage(m_hWnd,EM_LINESCROLL,nChars,nLines);
}

}