// EToolBarCtrl.cpp: implementation of the EToolBarCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EToolBarCtrl.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EToolBarCtrl::EToolBarCtrl()
{

}

EToolBarCtrl::~EToolBarCtrl()
{

}

DWORD EToolBarCtrl::Create(DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd)
{
    DWORD ret=EWnd::Create(TOOLBARCLASSNAME,_T(""),x,y,nWidth,nHeight,WS_CHILD|dwStyle,WS_EX_TOOLWINDOW, pParentWnd);
    SetButtonStructSize(sizeof(TBBUTTON));
    return ret;
}

int EToolBarCtrl::AddBitmap(int nNumButtons, UINT nBitmapID)
{
    TBADDBITMAP tbab;

    tbab.hInst=GetModuleHandle(NULL);
    tbab.nID=nBitmapID;

    return (int)SendMessage(m_hWnd,TB_ADDBITMAP,(WPARAM)nNumButtons,(LPARAM) (LPTBADDBITMAP) &tbab);
}



int EToolBarCtrl::AddString(UINT idString)
{
    return (int)SendMessage(m_hWnd,TB_ADDSTRING,(WPARAM)GetModuleHandle(NULL),(LPARAM) MAKELONG(idString,0));
}

int EToolBarCtrl::AddStrings(LPCTSTR lpszStrings)
{
    return (int)SendMessage(m_hWnd,TB_ADDSTRING,(WPARAM)0,(LPARAM) lpszStrings);
}

BOOL EToolBarCtrl::AddButtons(int nNumButtons, LPTBBUTTON lpButtons)
{
    return (BOOL)SendMessage(m_hWnd,TB_ADDBUTTONS,(WPARAM) (UINT) nNumButtons,(LPARAM) lpButtons);
}

void EToolBarCtrl::AutoSize()
{
    SendMessage(m_hWnd,TB_AUTOSIZE,0,0);
}

BOOL EToolBarCtrl::SetButtonSize(int dx, int dy)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETBUTTONSIZE,0,(LPARAM) MAKELONG(dx, dy) );
}

HIMAGELIST EToolBarCtrl::SetImageList(HIMAGELIST himlNew)
{
    return (HIMAGELIST)SendMessage(m_hWnd,TB_SETIMAGELIST,0,(LPARAM)(HIMAGELIST) himlNew);
}

BOOL EToolBarCtrl::SetButtonWidth(int cxMin, int cxMax)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETBUTTONWIDTH,0,(LPARAM)(DWORD) MAKELONG(cxMin,cxMax));
}

BOOL EToolBarCtrl::SetMaxTextRows(int iMaxRows)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETMAXTEXTROWS,(WPARAM)(INT) iMaxRows,0);
}

DWORD EToolBarCtrl::SetExtendedStyle(DWORD dwExStyle)
{
    return (DWORD)SendMessage(m_hWnd,TB_SETEXTENDEDSTYLE,0,(LPARAM)dwExStyle);
}



DWORD EToolBarCtrl::SetPadding(int cx, int cy)
{
    return (DWORD)SendMessage(m_hWnd,TB_SETPADDING,0,MAKELPARAM(cx, cy));
}

BOOL EToolBarCtrl::SetButtonInfo(int nID, TBBUTTONINFO *ptbbi)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETBUTTONINFO,(WPARAM)(INT) nID,(LPARAM)(LPTBBUTTONINFO) ptbbi);
}

BOOL EToolBarCtrl::GetItemRect(int nIndex, LPRECT lpRect)
{
    return (BOOL)SendMessage(m_hWnd,TB_GETITEMRECT,(WPARAM) nIndex,(LPARAM) (LPRECT) lpRect);
}

UINT EToolBarCtrl::CommandToIndex(UINT nID)
{
    return (UINT)SendMessage(m_hWnd,TB_COMMANDTOINDEX,(WPARAM) nID,0);
}

BOOL EToolBarCtrl::InsertButton(int nIndex, LPTBBUTTON lpButton)
{
    return (BOOL)SendMessage(m_hWnd,TB_INSERTBUTTON,(WPARAM) nIndex,(LPARAM) (LPTBBUTTON) lpButton);
}

int EToolBarCtrl::GetButtonCount()
{
    return (int)SendMessage(m_hWnd,TB_BUTTONCOUNT,0,0);
}

void EToolBarCtrl::SetButtonStructSize(int nSize)
{
    SendMessage(m_hWnd,TB_BUTTONSTRUCTSIZE,(WPARAM)nSize,0);
}

BOOL EToolBarCtrl::ChangeBitmap(UINT idButton, UINT iBitmap)
{
    return (BOOL)SendMessage(m_hWnd,TB_CHANGEBITMAP,(WPARAM)idButton,(LPARAM)MAKELPARAM(iBitmap,0));
}

BOOL EToolBarCtrl::CheckButton(int nID, BOOL bCheck)
{
    return (BOOL)SendMessage(m_hWnd,TB_CHECKBUTTON,(WPARAM)nID,(LPARAM)MAKELONG(bCheck, 0));

}

void EToolBarCtrl::Customize()
{
    SendMessage(m_hWnd,TB_CUSTOMIZE,0,0);
}

BOOL EToolBarCtrl::DeleteButton(int nIndex)
{
    return (BOOL)SendMessage(m_hWnd,TB_DELETEBUTTON,(WPARAM)nIndex,0);

}

BOOL EToolBarCtrl::EnableButton(int nID, BOOL bEnable)
{
    return (BOOL)SendMessage(m_hWnd,TB_ENABLEBUTTON,(WPARAM)nID,(LPARAM)MAKELONG(bEnable, 0));

}

BOOL EToolBarCtrl::GetAnchorHighlight()
{
    return (BOOL)SendMessage(m_hWnd,TB_GETANCHORHIGHLIGHT,0,0);

}

BOOL EToolBarCtrl::HideButton(int nID, BOOL bHide)
{
    return (BOOL)SendMessage(m_hWnd,TB_HIDEBUTTON,(WPARAM)nID,(LPARAM)MAKELONG(bHide, 0));

}

BOOL EToolBarCtrl::Indeterminate(int nID, BOOL bIndeterminate)
{
    return (BOOL)SendMessage(m_hWnd,TB_INDETERMINATE,(WPARAM)nID,(LPARAM)MAKELONG(bIndeterminate, 0));

}

BOOL EToolBarCtrl::PressButton(int nID, BOOL bPress)
{
    return (BOOL)SendMessage(m_hWnd,TB_PRESSBUTTON,(WPARAM)nID,(LPARAM)MAKELONG(bPress, 0));

}

BOOL EToolBarCtrl::SetAnchorHighlight(BOOL fAnchor)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETANCHORHIGHLIGHT,(WPARAM)(BOOL)fAnchor,0);

}

BOOL EToolBarCtrl::SetCmdID(int nIndex, UINT nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETCMDID,(WPARAM) (UINT) nIndex,(LPARAM) (UINT) nID);

}

int EToolBarCtrl::GetState(int nID)
{
    return (int)SendMessage(m_hWnd,TB_GETSTATE,(WPARAM) nID, 0);
    
}

BOOL EToolBarCtrl::SetState(int nID, UINT nState)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETSTATE,(WPARAM) nID, (LPARAM) MAKELONG(nState, 0));

}

BOOL EToolBarCtrl::IsButtonPressed(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONPRESSED,(WPARAM) nID, 0);

}


BOOL EToolBarCtrl::IsButtonChecked(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONCHECKED,(WPARAM) nID, 0);

}

BOOL EToolBarCtrl::IsButtonEnabled(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONENABLED,(WPARAM) nID, 0);

}

BOOL EToolBarCtrl::IsButtonHidden(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONHIDDEN,(WPARAM) nID, 0);

}

BOOL EToolBarCtrl::IsButtonHighlighted(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONHIGHLIGHTED,(WPARAM) nID, 0);

}

BOOL EToolBarCtrl::IsButtonIndeterminate(int nID)
{
    return (BOOL)SendMessage(m_hWnd,TB_ISBUTTONINDETERMINATE,(WPARAM) nID, 0);

}

BOOL EToolBarCtrl::SetBitmapSize(int dxBitmap, int dyBitmap)
{
    return (BOOL)SendMessage(m_hWnd,TB_SETBITMAPSIZE, 0, (LPARAM) MAKELONG(dxBitmap, dyBitmap));

}

}