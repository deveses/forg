// EPropertySheet.cpp: implementation of the EPropertySheet class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "EPropertySheet.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


EPropertySheet::EPropertySheet()
{
    m_strCaption=NULL;
    Construct(NULL);

}

EPropertySheet::~EPropertySheet()
{
    if (m_psh.ppsp!=NULL)
        delete [] (PROPSHEETPAGE *)m_psh.ppsp;
}

EPropertySheet::EPropertySheet(LPCTSTR pszCaption, HWND pParentWnd, UINT iSelectPage)
{
    Construct(pszCaption,pParentWnd,iSelectPage);
}

void EPropertySheet::Construct(LPCTSTR pszCaption, HWND pParentWnd, UINT iSelectPage)
{
    ZeroMemory(&m_psh,sizeof(m_psh));
    m_psh.dwSize=sizeof(m_psh);
    m_psh.nStartPage=iSelectPage;
    m_psh.dwFlags=PSH_PROPSHEETPAGE;
    m_psh.pszCaption=pszCaption;    
    m_hWndParent=pParentWnd;

}



int EPropertySheet::DoModal()
{
    int nResult=0;

    m_psh.hwndParent=m_hWndParent;

    nResult=PropertySheet(&m_psh);

    return nResult;
}

void EPropertySheet::AddPage(EPropertyPage *pPage)
{
    PROPSHEETPAGE *psp=NULL;
    
    m_psh.nPages++;

    psp=new PROPSHEETPAGE[m_psh.nPages];
    
    if (m_psh.ppsp!=NULL) {
        CopyMemory(psp,m_psh.ppsp,sizeof(PROPSHEETPAGE)*(m_psh.nPages-1));
        delete[] (PROPSHEETPAGE *)m_psh.ppsp;
    }

    CopyMemory(&psp[m_psh.nPages-1],&pPage->m_psp,sizeof(pPage->m_psp));        
    m_psh.ppsp=psp;

}

}