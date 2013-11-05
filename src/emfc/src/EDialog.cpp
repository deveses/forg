                           // EDialog.cpp: implementation of the EDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EDialog.h"

namespace emfc {


int EOpenFileDialog::ShowDialog()
{
    OPENFILENAME ofn;       // common dialog box structure

    m_filename[0] = 0;

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = m_filename;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(m_filename);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (TRUE == GetOpenFileName(&ofn))
    {
        return IDOK;
    }

    return IDCANCEL;
}

INT_PTR CALLBACK EDialog::EDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
  ) 
{
    EDialog *ed=NULL;
    
    ed=(EDialog *)::GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
    if (uMsg==WM_INITDIALOG) {
        ed=(EDialog *)lParam;
        //SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)ed);
        if (ed!=NULL) {
            ed->m_hWnd=hwndDlg;
            ed->OnInitDialog();
        }
    }  
    if (ed==NULL) return 0; 
    if (ed->m_hWnd!=NULL) {
        return ed->WindowProc(uMsg,wParam,lParam);
    }

    return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EDialog::EDialog()
{
    m_lpDialogTemplate=NULL;
    m_nFlags=0;
}

EDialog::EDialog(UINT nIDTemplate, HWND pParentWnd)
{
    m_hWndParent = pParentWnd;
    m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
    m_nIDHelp = nIDTemplate;
}

EDialog::~EDialog()
{
    if (m_hWnd!=NULL) EndDialog(0);
    m_hWnd=NULL;
    SetWindowLongPointer(GWLP_USERDATA, NULL);
}

BOOL EDialog::Create(UINT nIDTemplate, HWND pParentWnd)
{
    return Create(MAKEINTRESOURCE(nIDTemplate),pParentWnd);
}

BOOL EDialog::Create(LPCTSTR lpszTemplateName, HWND pParentWnd)
{
    m_hInstance=GetModuleHandle(NULL);
    m_hWndParent=pParentWnd;

    HGLOBAL hDialogTemplate = NULL;
    HRSRC hResource = ::FindResource(m_hInstance, lpszTemplateName, RT_DIALOG);
    hDialogTemplate = LoadResource(m_hInstance, hResource);

    if (! CreateIndirect(hDialogTemplate,pParentWnd,m_hInstance)) return FALSE;
    
    FreeResource(hDialogTemplate);

    return TRUE;
}

BOOL EDialog::CreateIndirect(HGLOBAL hDialogTemplate, HWND pParentWnd, HINSTANCE hInst)
{
    LPCDLGTEMPLATE lpDialogTemplate = NULL;
    if (hDialogTemplate != NULL)
        lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
        m_lpDialogTemplate=lpDialogTemplate;

    CreateIndirect(lpDialogTemplate,pParentWnd,hInst);

   if (hDialogTemplate != NULL)
   {
      UnlockResource(hDialogTemplate);
   }
    if (m_hWnd==NULL) return FALSE;

    return TRUE;
}

BOOL EDialog::CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, HWND pParentWnd, HINSTANCE hInst)
{
    if (lpDialogTemplate==NULL/* || hInst==NULL*/) return FALSE;
    m_hWnd=::CreateDialogIndirectParam(hInst,lpDialogTemplate,pParentWnd,EDialogProc,(LPARAM)this);
    m_nModalResult=-1;
    m_nFlags |= WF_CONTINUEMODAL;
    if (m_hWnd==NULL) return FALSE;
    SetWindowLongPointer(GWLP_USERDATA, (LONG)this);
    return TRUE;
}


int EDialog::DoModal()
{
   // load resource as necessary
   LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
   HGLOBAL hDialogTemplate =NULL;
   HINSTANCE hInst = NULL;
   
   hInst=NULL;//AfxGetResourceHandle(); //m_hInstance; //GetModuleHandle(NULL);
   if (m_lpszTemplateName != NULL)
   {
      HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
      hDialogTemplate = LoadResource(hInst, hResource);
   }
   if (hDialogTemplate != NULL)
      lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

   // return -1 in case of failure to load the dialog template resource
   if (lpDialogTemplate == NULL)
      return -1;
/*
   // disable parent (before creating dialog)
   HWND hWndParent = PreModal();
   AfxUnhookWindowCreate();*/
   BOOL bEnableParent = FALSE;
   if (m_hWndParent != NULL && ::IsWindowEnabled(m_hWndParent))
   {
      ::EnableWindow(m_hWndParent, FALSE);
      bEnableParent = TRUE;
   }


      // create modeless dialog
  //    AfxHookWindowCreate(this);
      if (CreateIndirect(lpDialogTemplate,
                  m_hWndParent, hInst))
      {
         if (m_nFlags & WF_CONTINUEMODAL)
         {
            // enter modal loop
            DWORD dwFlags = MLF_SHOWONIDLE;
            if (GetStyle() & DS_NOIDLEMSG)
               dwFlags |= MLF_NOIDLEMSG;
            RunModalLoop(dwFlags);
         }

         // hide the window before enabling the parent, etc.
         if (m_hWnd != NULL)
            SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
               SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
      }
/*   }
   CATCH_ALL(e)
   {
      DELETE_EXCEPTION(e);
      m_nModalResult = -1;
   }
   END_CATCH_ALL

   if (bEnableParent)
      ::EnableWindow(hWndParent, TRUE);
   if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
      ::SetActiveWindow(hWndParent);
*/
   // destroy modal window
         DestroyWindow();
//         PostModal();
      
         // unlock/free resources as necessary
         if (m_lpszTemplateName != NULL) {
            UnlockResource(hDialogTemplate);
            FreeResource(hDialogTemplate);  
         }

   return m_nModalResult;
}


void EDialog::EndDialog(int nResult)
{
   if (m_nFlags & (WF_MODALLOOP|WF_CONTINUEMODAL))
      EndModalLoop(nResult);
    ::EndDialog(m_hWnd,nResult);
}

BOOL EDialog::OnInitDialog()
{
   BOOL bDlgInit;

   bDlgInit = ExecuteDlgInit(m_lpszTemplateName);

   if (!bDlgInit)
   {
//      TRACE0("Warning: ExecuteDlgInit failed during dialog init.\n");
      EndDialog(-1);
      return FALSE;
   }

   // transfer data into the dialog from member variables
   if (!UpdateData(FALSE))
   {
    //  TRACE0("Warning: UpdateData failed during dialog init.\n");
      EndDialog(-1);
      return FALSE;
   }

   // enable/disable help button automatically
 /*  CWnd* pHelpButton = GetDlgItem(ID_HELP);
   if (pHelpButton != NULL)
      pHelpButton->ShowWindow(AfxHelpEnabled() ? SW_SHOW : SW_HIDE);
*/
   return TRUE;    // set focus to first one
}


void EDialog::OnOK()
{
    if (!UpdateData(TRUE))
    {
        return;
    }
    EndDialog(IDOK);
}

void EDialog::OnCancel()
{
    EndDialog(IDCANCEL);
}

LRESULT EDialog::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    return 0;
}


BOOL EDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam); // notification code 
    WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier 
    HWND hwndCtl = (HWND) lParam;      // handle of control 

    if (wID==IDOK) {
        OnOK();
        return TRUE;
    }else if (wID==IDCANCEL) {
        OnCancel();
        return TRUE;
    }

    return FALSE;
}

}