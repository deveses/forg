// EApplication.h: interface for the EApplication class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EAPPLICATION_H__8428BD42_D1EE_41B7_ADEA_5734EF2BDF37__INCLUDED_)
#define AFX_EAPPLICATION_H__8428BD42_D1EE_41B7_ADEA_5734EF2BDF37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace emfc {

class EMFC_API EApplication
{
public:
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	BOOL PumpMessage();
	int ExitInstance();
	BOOL IsIdleMessage(MSG* pMsg);
	virtual BOOL OnIdle( LONG lCount );
	HCURSOR LoadStandardCursor( LPCTSTR lpszCursorName );
	HCURSOR LoadCursor( UINT nIDResource );
	HICON LoadIcon( UINT nIDResource );
	HICON LoadIcon( LPCTSTR lpszResourceName );
	HINSTANCE m_hInstance;
	static void InitCommonControls();
	virtual BOOL InitApplication();
	static DWORD ErrorBox(DWORD nr=0);
	DWORD Create(HWND hWnd);
	DWORD Run();
	EApplication();
	virtual ~EApplication();

private:
	MSG m_msgCur;
};

}

#endif // !defined(AFX_EAPPLICATION_H__8428BD42_D1EE_41B7_ADEA_5734EF2BDF37__INCLUDED_)
