// EImageList.h: interface for the EImageList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EIMAGELIST_H__1F524274_775C_468C_970D_972506B36A56__INCLUDED_)
#define AFX_EIMAGELIST_H__1F524274_775C_468C_970D_972506B36A56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace emfc {

class EMFC_API EImageList  
{
public:
	BOOL Draw( HDC pdc, int nImage, int x, int y, UINT nStyle );
	BOOL Draw( HDC pdc, int nImage, POINT pt, UINT nStyle );
	int GetImageCount( ) const;
	int Add(HBITMAP hbmImage, COLORREF crMask );
	BOOL Destroy();
	COLORREF SetBkColor(COLORREF clrBk);
	BOOL Attach(HIMAGELIST hImageList);
	BOOL RemoveAll();
	int Add(HBITMAP hbmImage, HBITMAP hbmMask);
	BOOL Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow);
	HIMAGELIST m_hImageList;
	EImageList();
	virtual ~EImageList();

};

}

#endif // !defined(AFX_EIMAGELIST_H__1F524274_775C_468C_970D_972506B36A56__INCLUDED_)
