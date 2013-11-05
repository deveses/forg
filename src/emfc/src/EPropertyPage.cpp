// EPropertyPage.cpp: implementation of the EPropertyPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EPropertyPage.h"


namespace emfc {

INT_PTR CALLBACK EPropertyPage::EPropDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
  ) 
{
    EPropertyPage *ed=NULL;
    
    ed=(EPropertyPage *)::GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
    if (ed==NULL && lParam==0L) return 0; 
    
    if (uMsg==WM_INITDIALOG) {
        if (lParam>0) {
            ed=(EPropertyPage *)(((PROPSHEETPAGE *)lParam)->lParam);
            ed->m_hWnd=hwndDlg;
            ed->SetWindowLongPointer(GWLP_USERDATA, (LONG_PTR)ed);
        }
        return ed->OnInitDialog();
    } else if (ed->m_hWnd!=NULL)
        return ed->WindowProc(uMsg,wParam,lParam);

    return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EPropertyPage::EPropertyPage()
{

}

EPropertyPage::~EPropertyPage()
{

}

EPropertyPage::EPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption)
{
    Construct(lpszTemplateName,nIDCaption);
}


void EPropertyPage::Construct(LPCTSTR lpszTemplateName, UINT nIDCaption)
{
   memset(&m_psp, 0, sizeof(m_psp));
   m_psp.dwSize = sizeof(m_psp);
   m_psp.dwFlags = PSP_USECALLBACK;
   m_psp.pszTemplate = lpszTemplateName;
   m_psp.pfnDlgProc = EPropDialogProc;
   m_psp.lParam = (LPARAM)this;
//   m_psp.pfnCallback = AfxPropPageCallback;
/*   if (nIDCaption != 0)
   {
      VERIFY(m_strCaption.LoadString(nIDCaption));
      m_psp.pszTitle = m_strCaption;
      m_psp.dwFlags |= PSP_USETITLE;
   }*/
//   if (AfxHelpEnabled())
 //     m_psp.dwFlags |= PSP_HASHELP;
  // if (HIWORD(lpszTemplateName) == 0)
   //   m_nIDHelp = LOWORD((DWORD)lpszTemplateName);
   m_lpszTemplateName = m_psp.pszTemplate;
//   m_bFirstSetActive = TRUE;

}

}
