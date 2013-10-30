// EButton.h: interface for the EButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EBUTTON_H__A4E43DB2_1DC0_42BD_8B53_B798050FB110__INCLUDED_)
#define AFX_EBUTTON_H__A4E43DB2_1DC0_42BD_8B53_B798050FB110__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EWnd.h"

namespace emfc {

class EMFC_API EButton : public EWnd  
{
public:
	UINT GetState( );
	BOOL SetState( BOOL bHighlight );
	void SetCheck( int nCheck );
	DWORD Create( LPCTSTR lpszCaption, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID );
	EButton();
	virtual ~EButton();

};

}

#endif // !defined(AFX_EBUTTON_H__A4E43DB2_1DC0_42BD_8B53_B798050FB110__INCLUDED_)
