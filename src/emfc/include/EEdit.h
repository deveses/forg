// EEdit.h: interface for the EEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EEDIT_H__3F1B4085_48BB_4DD0_BF32_7869E3CB2F02__INCLUDED_)
#define AFX_EEDIT_H__3F1B4085_48BB_4DD0_BF32_7869E3CB2F02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EWnd.h"

namespace emfc {

class EMFC_API EEdit : public EWnd  
{
public:
	void LineScroll( int nLines, int nChars = 0 );
    int GetLineCount( ) const;
	void SetLimitText( UINT nMax );
	DWORD Create( DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND pParentWnd, UINT nID );
	EEdit();
	virtual ~EEdit();

};

}

#endif // !defined(AFX_EEDIT_H__3F1B4085_48BB_4DD0_BF32_7869E3CB2F02__INCLUDED_)
