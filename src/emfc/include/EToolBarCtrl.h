// EToolBarCtrl.h: interface for the EToolBarCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ETOOLBARCTRL_H__D19EDDBA_4201_404F_A6E7_3CD51DA02F8C__INCLUDED_)
#define AFX_ETOOLBARCTRL_H__D19EDDBA_4201_404F_A6E7_3CD51DA02F8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <commctrl.h>
#include "EWnd.h"

namespace emfc {

class EMFC_API EToolBarCtrl : public EWnd  
{
public:
	BOOL SetBitmapSize(int dxBitmap, int dyBitmap);
	BOOL IsButtonIndeterminate( int nID );
	BOOL IsButtonHighlighted( int nID );
	BOOL IsButtonHidden( int nID );
	BOOL IsButtonEnabled( int nID );
	BOOL IsButtonChecked( int nID );
	BOOL SetState( int nID, UINT nState );
	int GetState( int nID );
	BOOL IsButtonPressed( int nID );
	BOOL SetCmdID( int nIndex, UINT nID );
	BOOL SetAnchorHighlight( BOOL fAnchor = TRUE );
	BOOL PressButton( int nID, BOOL bPress = TRUE );
	BOOL Indeterminate( int nID, BOOL bIndeterminate = TRUE );
	BOOL HideButton( int nID, BOOL bHide = TRUE );
	BOOL GetAnchorHighlight( );
	BOOL EnableButton( int nID, BOOL bEnable = TRUE );
	BOOL DeleteButton( int nIndex );
	void Customize();
	BOOL CheckButton( int nID, BOOL bCheck = TRUE );
	BOOL ChangeBitmap(UINT idButton, UINT iBitmap);
	void SetButtonStructSize( int nSize );
	int GetButtonCount();
	BOOL InsertButton( int nIndex, LPTBBUTTON lpButton );
	UINT CommandToIndex( UINT nID );
	BOOL GetItemRect( int nIndex, LPRECT lpRect );
	BOOL SetButtonInfo( int nID, TBBUTTONINFO* ptbbi);
	DWORD SetPadding(int cx,int cy);
	int AddStrings( LPCTSTR lpszStrings );
	DWORD SetExtendedStyle(DWORD dwExStyle);
	BOOL SetMaxTextRows( int iMaxRows );
	BOOL SetButtonWidth( int cxMin, int cxMax );
	HIMAGELIST SetImageList(HIMAGELIST himlNew);
	BOOL SetButtonSize(int dx, int dy);
	void AutoSize();
	BOOL AddButtons( int nNumButtons, LPTBBUTTON lpButtons);
	int AddString(UINT idString);
	int AddBitmap( int nNumButtons, UINT nBitmapID );
	DWORD Create( DWORD dwStyle,  int x,  int y, int nWidth, int nHeight, HWND pParentWnd);
	EToolBarCtrl();
	virtual ~EToolBarCtrl();

};

}

#endif // !defined(AFX_ETOOLBARCTRL_H__D19EDDBA_4201_404F_A6E7_3CD51DA02F8C__INCLUDED_)
