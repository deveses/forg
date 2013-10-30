// EImageList.cpp: implementation of the EImageList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EImageList.h"

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EImageList::EImageList()
{
    m_hImageList=NULL;
}

EImageList::~EImageList()
{
    if (m_hImageList!=NULL) {
        Destroy();
    }
}

BOOL EImageList::Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow)
{
    m_hImageList=ImageList_Create(cx,cy,nFlags,nInitial,nGrow);
    if (m_hImageList==NULL) return FALSE;
    return TRUE;
}

int EImageList::Add(HBITMAP hbmImage, HBITMAP hbmMask)
{
    return ImageList_Add(m_hImageList,hbmImage, hbmMask);
}

BOOL EImageList::RemoveAll()
{
    return ImageList_RemoveAll(m_hImageList);
}

BOOL EImageList::Attach(HIMAGELIST hImageList)
{
    m_hImageList=hImageList;
    return TRUE;
}

COLORREF EImageList::SetBkColor(COLORREF clrBk)
{
    return ImageList_SetBkColor(m_hImageList,clrBk);
}

BOOL EImageList::Destroy()
{
    return ImageList_Destroy(m_hImageList);
}

int EImageList::Add(HBITMAP hbmImage, COLORREF crMask)
{
    return ImageList_AddMasked(m_hImageList,hbmImage,crMask);
}

int EImageList::GetImageCount() const
{
    return ImageList_GetImageCount(m_hImageList);
}

BOOL EImageList::Draw(HDC pdc, int nImage, POINT pt, UINT nStyle)
{
    return ImageList_Draw(m_hImageList,nImage,pdc,pt.x,pt.y,nStyle);
}

BOOL EImageList::Draw(HDC pdc, int nImage, int x, int y, UINT nStyle)
{
    return ImageList_Draw(m_hImageList,nImage,pdc,x,y,nStyle);

}

}