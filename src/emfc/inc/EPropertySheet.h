// EPropertySheet.h: interface for the EPropertySheet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EPROPERTYSHEET_H__C1E98EC7_C987_4F34_9B7C_9258D326DDF5__INCLUDED_)
#define AFX_EPROPERTYSHEET_H__C1E98EC7_C987_4F34_9B7C_9258D326DDF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "EPropertyPage.h"

namespace emfc {

class EMFC_API EPropertySheet : public EWnd  
{
public:
	void AddPage( EPropertyPage *pPage );
	int DoModal();
	LPTSTR m_strCaption;
	PROPSHEETHEADER m_psh;
	void Construct( LPCTSTR pszCaption, HWND pParentWnd = NULL, UINT iSelectPage = 0 );
	EPropertySheet( LPCTSTR pszCaption, HWND pParentWnd = NULL, UINT iSelectPage = 0 );
	EPropertySheet();
	virtual ~EPropertySheet();

};

}

#endif // !defined(AFX_EPROPERTYSHEET_H__C1E98EC7_C987_4F34_9B7C_9258D326DDF5__INCLUDED_)
