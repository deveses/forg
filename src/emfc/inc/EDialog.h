  // EDialog.h: interface for the EDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDIALOG_H__1D26B2E9_2397_4567_A027_47DC4801497C__INCLUDED_)
#define AFX_EDIALOG_H__1D26B2E9_2397_4567_A027_47DC4801497C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EWnd.h"

namespace emfc {

class EMFC_API EOpenFileDialog
{
    enum { MAX_FILE_NAME = 260 };

    char m_filename[MAX_FILE_NAME];

public:
    const char* GetFileName() const { return m_filename; } 

public:
    int ShowDialog();
};

class EMFC_API EDialog : public EWnd  
{
    static BOOL CALLBACK EDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate, HWND pParentWnd, HINSTANCE hInst);
	LRESULT DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam );
	virtual void OnCancel( );
	virtual void OnOK( );
	UINT m_nIDHelp;
	LPCTSTR m_lpszTemplateName;
	EDialog(UINT nIDTemplate, HWND pParentWnd);
	LPCDLGTEMPLATE m_lpDialogTemplate;
	BOOL CreateIndirect(HGLOBAL hDialogTemplate, HWND pParentWnd, HINSTANCE hInst);
	void EndDialog( int nResult );
	virtual int DoModal( );
	virtual BOOL OnInitDialog( );
	BOOL Create( LPCTSTR lpszTemplateName, HWND pParentWnd = NULL );
	BOOL Create( UINT nIDTemplate, HWND pParentWnd = NULL );
	EDialog();
	virtual ~EDialog();
};

/*
BOOL CALLBACK EDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
  ) ;
  */

}

#endif // !defined(AFX_EDIALOG_H__1D26B2E9_2397_4567_A027_47DC4801497C__INCLUDED_)
