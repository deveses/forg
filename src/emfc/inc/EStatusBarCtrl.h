// EStatusBarCtrl.h: interface for the EStatusBarCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ESTATUSBARCTRL_H__24914957_E874_42D4_90BA_83FA29CBC12C__INCLUDED_)
#define AFX_ESTATUSBARCTRL_H__24914957_E874_42D4_90BA_83FA29CBC12C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#include "EWnd.h"

namespace emfc {

class EMFC_API EStatusBarCtrl : public EWnd  
{
public:
	BOOL SetParts( int nParts, int* pWidths );
	BOOL SetText(LPCTSTR lpszText, int nPane, int nType);
	DWORD Create( DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID );
	EStatusBarCtrl();
	virtual ~EStatusBarCtrl();

};

}

#endif // !defined(AFX_ESTATUSBARCTRL_H__24914957_E874_42D4_90BA_83FA29CBC12C__INCLUDED_)
