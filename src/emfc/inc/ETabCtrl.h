// ETabCtrl.h: interface for the ETabCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ETabCtrl_H__C8D03E54_103A_4F81_BD88_9E74EECABA60__INCLUDED_)
#define AFX_ETabCtrl_H__C8D03E54_103A_4F81_BD88_9E74EECABA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EWnd.h"

#include <commctrl.h>

namespace emfc {

class EMFC_API ETabCtrl : public EWnd  
{
public:
	HWND GetToolTips( );
	int HitTest( TCHITTESTINFO* pHitTestInfo );
	HIMAGELIST GetImageList();
	void SetPadding(int cx, int cy);
	DWORD SetItemSize(int cx, int cy);
	HIMAGELIST SetImageList(HIMAGELIST hImageList);
	BOOL DeleteAllItems();
	void AdjustRect( BOOL bLarger, LPRECT lpRect );
	int GetCurSel();
	BOOL DeleteItem(int iItem);
	BOOL InsertItem(int nItem, TCITEM* pTabCtrlItem);
	DWORD Create(LPCTSTR lpWindowName, int x, int y, int nWidth, int nHeight, DWORD dwStyle, HWND hWndParent);
	ETabCtrl();
	virtual ~ETabCtrl();

};

}

#endif // !defined(AFX_ETabCtrl_H__C8D03E54_103A_4F81_BD88_9E74EECABA60__INCLUDED_)
