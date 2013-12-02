#include "SWRenderDevice.h"
//#include "SWBuffers.h"
#include "forg.h"

#define NOMINMAX
#include <Windows.h>

namespace forg
{
    /////////////////////////////////////////////////////////////////////////////////////
    // SWRenderDevice
    /////////////////////////////////////////////////////////////////////////////////////
    SWRenderDevice::SWRenderDevice(HWIN handle)
        : super(handle)
    {
    }

    SWRenderDevice::~SWRenderDevice()
    {
    }

    int SWRenderDevice::Reset()
    {
        RECT rcClient;
        GetClientRect((HWND)GetHWIN(), &rcClient);

        // calculate window width/height 
        uint width = rcClient.right - rcClient.left;  
        uint height = rcClient.bottom - rcClient.top;  
        SetBufferSize(width, height);

        super::Reset();

        return FORG_OK;
    }

    int SWRenderDevice::Present()
    {
        // window color format: ARGB
        HWND hWnd = (HWND)GetHWIN();

        HDC hWindowDC = GetDC(hWnd); // get the desktop device context
        HDC hMemDC = CreateCompatibleDC(hWindowDC); // create a device context to use yourself
    
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // calculate window width/height 
        ULONG ulWindowWidth = rcClient.right - rcClient.left;  
        ULONG ulWindowHeight = rcClient.bottom - rcClient.top;  

        BITMAPINFO bi; 
        ZeroMemory(&bi,sizeof(BITMAPINFO));
        bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth=ulWindowWidth;
        bi.bmiHeader.biHeight=ulWindowHeight;
        bi.bmiHeader.biPlanes=1;
        bi.bmiHeader.biBitCount= 32;//bm.bmBitsPixel;
        bi.bmiHeader.biCompression = BI_RGB;
        bi.bmiHeader.biSizeImage = ((ulWindowWidth * bi.bmiHeader.biBitCount +31)& ~31) / 8 * ulWindowHeight; 

        // create our DIB section and select the bitmap into the dc 
        VOID *pvBits;          // pointer to DIB section 
        HBITMAP hBitmap = CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
        HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

        //for (uint y = 0; y < ulWindowHeight; y++)
        //for (uint x = 0; x < ulWindowWidth; x++)
        //    ((UINT32 *)pvBits)[x + y * ulWindowWidth] = 0xff0000ff; 

        memcpy(pvBits, GetBuffer(), ulWindowWidth*ulWindowHeight*4);

        if (!BitBlt(hWindowDC, 0, 0, ulWindowWidth, ulWindowHeight, hMemDC, 0, 0, SRCCOPY))
        //StretchBlt used to flip frame buffer
        //if (!StretchBlt(hWindowDC, 0, ulWindowHeight-1, ulWindowWidth, -1*ulWindowHeight, hMemDC, 0, 0, ulWindowWidth, ulWindowHeight, SRCCOPY))
        {
            // error
        }

        hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

        ReleaseDC((HWND)GetHWIN(), hWindowDC);
        DeleteDC(hMemDC);   
        DeleteObject(hBitmap);

        return FORG_OK;
    }

}

