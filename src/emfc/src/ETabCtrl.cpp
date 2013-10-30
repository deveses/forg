// ETabCtrl.cpp: implementation of the ETabCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ETabCtrl.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


ETabCtrl::ETabCtrl()
{

}

ETabCtrl::~ETabCtrl()
{
}

DWORD ETabCtrl::Create(LPCTSTR lpWindowName, int x, int y, int nWidth, int nHeight, DWORD dwStyle, HWND hWndParent)
{
    return EWnd::Create(WC_TABCONTROL,lpWindowName,x,y,nWidth,nHeight,WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|dwStyle ,0,hWndParent);
}

BOOL ETabCtrl::InsertItem(int nItem, TCITEM *pTabCtrlItem)
{
    return (TabCtrl_InsertItem(m_hWnd,nItem,pTabCtrlItem)>=0);
}

BOOL ETabCtrl::DeleteItem(int iItem)
{
    return TabCtrl_DeleteItem(m_hWnd,iItem);
}

int ETabCtrl::GetCurSel()
{
    return TabCtrl_GetCurSel(m_hWnd);
}

void ETabCtrl::AdjustRect(BOOL bLarger, LPRECT lpRect)
{
    TabCtrl_AdjustRect(m_hWnd,bLarger,lpRect);
}

BOOL ETabCtrl::DeleteAllItems()
{
    return TabCtrl_DeleteAllItems(m_hWnd);
}

HIMAGELIST ETabCtrl::SetImageList(HIMAGELIST hImageList)
{
    return TabCtrl_SetImageList(m_hWnd,hImageList);
}


DWORD ETabCtrl::SetItemSize(int cx, int cy)
{
    return TabCtrl_SetItemSize(m_hWnd,cx,cy);
}

void ETabCtrl::SetPadding(int cx, int cy)
{
    TabCtrl_SetPadding(m_hWnd,cx,cy);
}

HIMAGELIST ETabCtrl::GetImageList()
{
    return TabCtrl_GetImageList(m_hWnd);
}


int ETabCtrl::HitTest(TCHITTESTINFO *pHitTestInfo)
{
    return TabCtrl_HitTest(m_hWnd,pHitTestInfo);
}

HWND ETabCtrl::GetToolTips()
{
    return TabCtrl_GetToolTips(m_hWnd);
}

}