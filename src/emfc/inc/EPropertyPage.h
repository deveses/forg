// EPropertyPage.h: interface for the EPropertyPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EPROPERTYPAGE_H__1B76A107_AC27_4152_B242_19F68B687ECA__INCLUDED_)
#define AFX_EPROPERTYPAGE_H__1B76A107_AC27_4152_B242_19F68B687ECA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EDialog.h"

namespace emfc {

class EMFC_API EPropertyPage : public EDialog  
{
    static BOOL CALLBACK EPropDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 

public:
	PROPSHEETPAGE m_psp;
	void Construct( LPCTSTR lpszTemplateName, UINT nIDCaption = 0 );
	EPropertyPage( LPCTSTR lpszTemplateName, UINT nIDCaption = 0 );
	EPropertyPage();
	virtual ~EPropertyPage();

};

}

#endif // !defined(AFX_EPROPERTYPAGE_H__1B76A107_AC27_4152_B242_19F68B687ECA__INCLUDED_)
