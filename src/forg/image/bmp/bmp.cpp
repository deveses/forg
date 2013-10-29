#include "forg_pch.h"

#include "image/bmp/bmp.h"

#include <stdio.h>

namespace forg {

//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned short WORD;
    typedef unsigned long DWORD;
    typedef unsigned char UCHAR;
    typedef UCHAR BYTE;
    typedef long LONG;
    typedef void* LPVOID;
    typedef long FXPT2DOT30;


    enum BMPCOMPRESSION
    {
        BMPC_RGB = 0,
        BMPC_RLE8,
        BMPC_RLE4,
        BMPC_BITFIELDS,
        BMPC_JPEG,
        BMPC_PNG
    };

    //#define BI_RGB        0L
    //#define BI_RLE8       1L
    //#define BI_RLE4       2L
    //#define BI_BITFIELDS  3L
    //#define BI_JPEG       4L
    //#define BI_PNG        5L

#pragma pack(2)

    typedef struct {
        /// Specifies the file type, must be BM.
        WORD    bfType;
        /// Specifies the size, in bytes, of the bitmap file.
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        /// Specifies the offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
        DWORD   bfOffBits;
    } BMPFILEHEADER;

#pragma pack()

    typedef struct {
        DWORD   bcSize;
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
    } BMPCOREHEADER;


    typedef struct {
        DWORD  biSize;
        LONG   biWidth;
        LONG   biHeight;
        WORD   biPlanes;
        WORD   biBitCount;
        DWORD  biCompression;
        /**
        * Specifies the size, in bytes, of the image. This may be set to zero for BI_RGB bitmaps.
        * Windows 98/Me, Windows 2000/XP: If biCompression is BI_JPEG or BI_PNG, biSizeImage indicates
        * the size of the JPEG or PNG image buffer, respectively.
        */
        DWORD  biSizeImage;
        LONG   biXPelsPerMeter;
        LONG   biYPelsPerMeter;
        /**
        Specifies the number of color indexes in the color table that are actually used by the bitmap.
        If this value is zero, the bitmap uses the maximum number of colors corresponding to the value of
        the biBitCount member for the compression mode specified by biCompression.
        If biClrUsed is nonzero and the biBitCount member is less than 16, the biClrUsed member specifies
        the actual number of colors the graphics engine or device driver accesses. If biBitCount is 16 or greater,
        the biClrUsed member specifies the size of the color table used to optimize performance of the system color palettes.
        If biBitCount equals 16 or 32, the optimal color palette starts immediately following the three DWORD masks.

        When the bitmap array immediately follows the BITMAPINFO structure, it is a packed bitmap.
        Packed bitmaps are referenced by a single pointer. Packed bitmaps require that
        the biClrUsed member must be either zero or the actual size of the color table.
        */
        DWORD  biClrUsed;
        DWORD  biClrImportant;
    } BMPINFOHEADER;

#pragma pack(1)

    typedef struct {
        BYTE rgbtBlue;
        BYTE rgbtGreen;
        BYTE rgbtRed;
    } RGB_TRIPLE;

#pragma pack()


    typedef struct {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
    } RGB_QUAD;

    typedef struct {
        BMPINFOHEADER bmiHeader;
        RGB_QUAD          bmiColors[1];
    } BMPINFO;

    typedef struct {
        BMPCOREHEADER  bmciHeader;
        RGB_TRIPLE         bmciColors[1];
    } BMPCOREINFO;

    typedef struct {
        FXPT2DOT30 ciexyzX;
        FXPT2DOT30 ciexyzY;
        FXPT2DOT30 ciexyzZ;
    } CIE_XYZ;

    typedef struct {
        CIE_XYZ  ciexyzRed;
        CIE_XYZ  ciexyzGreen;
        CIE_XYZ  ciexyzBlue;
    } CIE_XYZ_TRIPLE;

    typedef struct {
        LONG   bmType;
        LONG   bmWidth;
        LONG   bmHeight;
        LONG   bmWidthBytes;
        WORD   bmPlanes;
        WORD   bmBitsPixel;
        LPVOID bmBits;
    } BMP;

    typedef struct {
        DWORD        bV4Size;
        LONG         bV4Width;
        LONG         bV4Height;
        WORD         bV4Planes;
        WORD         bV4BitCount;
        DWORD        bV4V4Compression;
        DWORD        bV4SizeImage;
        LONG         bV4XPelsPerMeter;
        LONG         bV4YPelsPerMeter;
        DWORD        bV4ClrUsed;
        DWORD        bV4ClrImportant;
        DWORD        bV4RedMask;
        DWORD        bV4GreenMask;
        DWORD        bV4BlueMask;
        DWORD        bV4AlphaMask;
        DWORD        bV4CSType;
        CIE_XYZ_TRIPLE bV4Endpoints;
        DWORD        bV4GammaRed;
        DWORD        bV4GammaGreen;
        DWORD        bV4GammaBlue;
    } BMPV4HEADER;

    typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIE_XYZ_TRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
    } BMPV5HEADER;

#ifdef __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////

/// Force upcasting from type Y to type T
template <class T, class Y>
void FORCE_UPCAST(T& a, Y& b)
{
    a = *((T*)&b);
}

Color4b* LoadBmp(const char* filename, ImageDescription* bmp_info)
{
	BMPFILEHEADER   bmfh;
	BMPINFOHEADER   bmih;
	BMPV4HEADER     bmi4;
	BMPV5HEADER		bmi5;
	//RGB_QUAD             *aColors = 0;
	size_t data_read = 0;

	FILE* f = fopen(filename, "r+b");

	if (f == 0)
		return NULL;

	bool hdr_read = false;
	DWORD bihSize = 0;
	WORD bmpBpp = 0;
	LONG bmpWidth = 0;
	LONG bmpHeight = 0;
	DWORD bmpCompression = 0;
	DWORD bmpClrUsed = 0;

	data_read = fread(&bmfh, 1, sizeof(BMPFILEHEADER), f);
	data_read = fread(&bihSize, 1, sizeof(DWORD), f);

	switch(bihSize) {
	case sizeof(BMPINFOHEADER):
		data_read = fread((BYTE*)&bmih+sizeof(DWORD), 1, sizeof(BMPINFOHEADER)-sizeof(DWORD), f);
		bmpBpp = bmih.biBitCount;
		bmpWidth = bmih.biWidth;
		bmpHeight = bmih.biHeight;
		bmpCompression = bmih.biCompression;
		bmpClrUsed = bmih.biClrUsed;
		hdr_read = true;
		break;
	case sizeof(BMPV4HEADER):
		data_read = fread((BYTE*)&bmi4+sizeof(DWORD), 1, sizeof(BMPV4HEADER)-sizeof(DWORD), f);
		bmpBpp = bmi4.bV4BitCount;
		hdr_read = true;
		break;
	case sizeof(BMPV5HEADER):
		data_read = fread((BYTE*)&bmi5+sizeof(DWORD), 1, sizeof(BMPV5HEADER)-sizeof(DWORD), f);
		bmpBpp = bmi5.bV5BitCount;
		hdr_read = true;
		break;
	}

	if (bmp_info)
	{
		bmp_info->Width = bmpWidth;
		bmp_info->Height = bmpHeight;
		bmp_info->Bpp = bmpBpp;
	}

    RGB_QUAD color_table[256];
    Color4b *aBitmapBits = 0;

	if (hdr_read)
	{
        aBitmapBits = new Color4b[bmpWidth*bmpHeight];
        BYTE* buff_in = new BYTE[bmpWidth*bmpHeight*(bmpBpp/8)];

        int pitch = bmpWidth*(bmpBpp/8);

        switch(bmpBpp)
        {
        case 1:
            data_read = fread(color_table, sizeof(RGB_QUAD), 2, f);
            break;
        case 4:
            data_read = fread(color_table, sizeof(RGB_QUAD), 16, f);
            break;
        case 8:
            data_read = fread(color_table, sizeof(RGB_QUAD), 256, f);
            break;
        }

        // fill buffer from end to begin. image in bitmap is saved from bottom to top
        for (int i=0; i<bmpHeight; i++)
        {
            data_read = fread(buff_in + (pitch*(bmpHeight-i-1)), 1, pitch, f);
        }

        switch(bmpBpp)
        {
        case 1:
            // color_table -> pixel
            for (int h=0; h<bmpHeight; h++)
            {
                for (int w=0; w<bmpWidth; w++)
                {
                    int cindex = buff_in[(h*bmpHeight) + (w>>3)];   //byte = 8 colors
                    int shift = w & 0x07;    // current color/bit (0-7)

                    cindex >>= shift;

                    cindex = cindex & 0x01;

                    FORCE_UPCAST(aBitmapBits[h*bmpHeight + w], color_table[ cindex ]);
                }
            }
            break;
        case 4:
            // color_table -> pixel
            for (int h=0; h<bmpHeight; h++)
            {
                for (int w=0; w<bmpWidth; w++)
                {
                    int cindex = buff_in[(h*bmpHeight) + (w>>1)];
                    int odd = w & 0x01;

                    if (! odd)
                    {
                        cindex >>= 4;
                    }

                    cindex = cindex & 0x0f;

                    FORCE_UPCAST(aBitmapBits[h*bmpHeight + w], color_table[ cindex ]);
                }
            }
            break;
        case 8:
            // color_table -> pixel
            for (int h=0; h<bmpHeight; h++)
            {
                for (int w=0; w<bmpWidth; w++)
                {
                    FORCE_UPCAST(aBitmapBits[h*bmpHeight + w], color_table[ buff_in[h*bmpHeight + w] ]);
                    aBitmapBits[h*bmpHeight + w].a = 255;
                }
            }
            break;
        case 24:
            for (int h=0; h<bmpHeight; h++)
            {
                for (int w=0; w<bmpWidth; w++)
                {
                    uint off = h*bmpHeight + w;
                    RGB_TRIPLE* p = (RGB_TRIPLE*)buff_in + off;

                    aBitmapBits[off].r = p->rgbtBlue;
                    aBitmapBits[off].g = p->rgbtGreen;
                    aBitmapBits[off].b = p->rgbtRed;
                    aBitmapBits[off].a = 255;
                }
            }
            break;
        }

        delete [] buff_in;
	}

	fclose(f);


	return (aBitmapBits);
}


}
