#include "forg_pch.h"

#include "image/bmp/bmp.h"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <string>

namespace forg {

//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

    typedef std::uint16_t WORD;
    typedef std::uint32_t DWORD;
    typedef std::uint8_t UCHAR;
    typedef UCHAR BYTE;
    typedef std::int32_t LONG;
    typedef void* LPVOID;
    typedef std::int32_t FXPT2DOT30;

    enum BMPCOMPRESSION
    {
        BMPC_RGB = 0,
        BMPC_RLE8,
        BMPC_RLE4,
        BMPC_BITFIELDS,
        BMPC_JPEG,
        BMPC_PNG
    };

    // #define BI_RGB        0L
    // #define BI_RLE8       1L
    // #define BI_RLE4       2L
    // #define BI_BITFIELDS  3L
    // #define BI_JPEG       4L
    // #define BI_PNG        5L

#pragma pack(2)

    typedef struct
    {
        /// Specifies the file type, must be BM.
        WORD bfType;
        /// Specifies the size, in bytes, of the bitmap file.
        DWORD bfSize;
        WORD bfReserved1;
        WORD bfReserved2;
        /// Specifies the offset, in bytes, from the beginning of the
        /// BITMAPFILEHEADER structure to the bitmap bits.
        DWORD bfOffBits;
    } BMPFILEHEADER;

#pragma pack()

    typedef struct
    {
        DWORD bcSize;
        WORD bcWidth;
        WORD bcHeight;
        WORD bcPlanes;
        WORD bcBitCount;
    } BMPCOREHEADER;

    typedef struct
    {
        DWORD biSize;
        LONG biWidth;
        LONG biHeight;
        WORD biPlanes;
        WORD biBitCount;
        DWORD biCompression;
        /**
         * Specifies the size, in bytes, of the image. This may be set to zero
         * for BI_RGB bitmaps. Windows 98/Me, Windows 2000/XP: If biCompression
         * is BI_JPEG or BI_PNG, biSizeImage indicates the size of the JPEG or
         * PNG image buffer, respectively.
         */
        DWORD biSizeImage;
        LONG biXPelsPerMeter;
        LONG biYPelsPerMeter;
        /**
        Specifies the number of color indexes in the color table that are
        actually used by the bitmap. If this value is zero, the bitmap uses the
        maximum number of colors corresponding to the value of the biBitCount
        member for the compression mode specified by biCompression. If biClrUsed
        is nonzero and the biBitCount member is less than 16, the biClrUsed
        member specifies the actual number of colors the graphics engine or
        device driver accesses. If biBitCount is 16 or greater, the biClrUsed
        member specifies the size of the color table used to optimize
        performance of the system color palettes. If biBitCount equals 16 or 32,
        the optimal color palette starts immediately following the three DWORD
        masks.

        When the bitmap array immediately follows the BITMAPINFO structure, it
        is a packed bitmap. Packed bitmaps are referenced by a single pointer.
        Packed bitmaps require that the biClrUsed member must be either zero or
        the actual size of the color table.
        */
        DWORD biClrUsed;
        DWORD biClrImportant;
    } BMPINFOHEADER;

#pragma pack(1)

    typedef struct
    {
        BYTE rgbtBlue;
        BYTE rgbtGreen;
        BYTE rgbtRed;
    } RGB_TRIPLE;

#pragma pack()

    typedef struct
    {
        BYTE rgbBlue;
        BYTE rgbGreen;
        BYTE rgbRed;
        BYTE rgbReserved;
    } RGB_QUAD;

    typedef struct
    {
        BMPINFOHEADER bmiHeader;
        RGB_QUAD bmiColors[1];
    } BMPINFO;

    typedef struct
    {
        BMPCOREHEADER bmciHeader;
        RGB_TRIPLE bmciColors[1];
    } BMPCOREINFO;

    typedef struct
    {
        FXPT2DOT30 ciexyzX;
        FXPT2DOT30 ciexyzY;
        FXPT2DOT30 ciexyzZ;
    } CIE_XYZ;

    typedef struct
    {
        CIE_XYZ ciexyzRed;
        CIE_XYZ ciexyzGreen;
        CIE_XYZ ciexyzBlue;
    } CIE_XYZ_TRIPLE;

    typedef struct
    {
        LONG bmType;
        LONG bmWidth;
        LONG bmHeight;
        LONG bmWidthBytes;
        WORD bmPlanes;
        WORD bmBitsPixel;
        LPVOID bmBits;
    } BMP;

    typedef struct
    {
        DWORD bV4Size;
        LONG bV4Width;
        LONG bV4Height;
        WORD bV4Planes;
        WORD bV4BitCount;
        DWORD bV4V4Compression;
        DWORD bV4SizeImage;
        LONG bV4XPelsPerMeter;
        LONG bV4YPelsPerMeter;
        DWORD bV4ClrUsed;
        DWORD bV4ClrImportant;
        DWORD bV4RedMask;
        DWORD bV4GreenMask;
        DWORD bV4BlueMask;
        DWORD bV4AlphaMask;
        DWORD bV4CSType;
        CIE_XYZ_TRIPLE bV4Endpoints;
        DWORD bV4GammaRed;
        DWORD bV4GammaGreen;
        DWORD bV4GammaBlue;
    } BMPV4HEADER;

    typedef struct
    {
        DWORD bV5Size;
        LONG bV5Width;
        LONG bV5Height;
        WORD bV5Planes;
        WORD bV5BitCount;
        DWORD bV5Compression;
        DWORD bV5SizeImage;
        LONG bV5XPelsPerMeter;
        LONG bV5YPelsPerMeter;
        DWORD bV5ClrUsed;
        DWORD bV5ClrImportant;
        DWORD bV5RedMask;
        DWORD bV5GreenMask;
        DWORD bV5BlueMask;
        DWORD bV5AlphaMask;
        DWORD bV5CSType;
        CIE_XYZ_TRIPLE bV5Endpoints;
        DWORD bV5GammaRed;
        DWORD bV5GammaGreen;
        DWORD bV5GammaBlue;
        DWORD bV5Intent;
        DWORD bV5ProfileData;
        DWORD bV5ProfileSize;
        DWORD bV5Reserved;
    } BMPV5HEADER;

#ifdef __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////

/// Force upcasting from type Y to type T
template <class T, class Y> void FORCE_UPCAST(T& a, Y& b) { a = *((T*)&b); }

static DWORD BmpRowPitchBytes(uint width, uint bpp)
{
    return ((width * bpp + 31) / 32) * 4;
}

Color4b* LoadBmp(const char* filename, ImageDescription* bmp_info)
{
    BMPFILEHEADER bmfh{};
    BMPINFOHEADER bmih{};
    BMPV4HEADER bmi4{};
    BMPV5HEADER bmi5{};
    // RGB_QUAD             *aColors = 0;

    FILE* f = std::fopen(filename, "rb");

    if (f == 0)
        return NULL;

    bool hdr_read = false;
    DWORD bihSize = 0;
    WORD bmpBpp = 0;
    LONG bmpWidth = 0;
    LONG bmpHeight = 0;

    std::fread(&bmfh, 1, sizeof(BMPFILEHEADER), f);
    std::fread(&bihSize, 1, sizeof(DWORD), f);

    switch (bihSize)
    {
    case sizeof(BMPINFOHEADER):
        bmih.biSize = bihSize;
        std::fread((BYTE*)&bmih + sizeof(DWORD), 1,
                   sizeof(BMPINFOHEADER) - sizeof(DWORD), f);
        bmpBpp = bmih.biBitCount;
        bmpWidth = bmih.biWidth;
        bmpHeight = bmih.biHeight;
        hdr_read = true;
        break;
    case sizeof(BMPV4HEADER):
        bmi4.bV4Size = bihSize;
        std::fread((BYTE*)&bmi4 + sizeof(DWORD), 1,
                   sizeof(BMPV4HEADER) - sizeof(DWORD), f);
        bmpBpp = bmi4.bV4BitCount;
        bmpWidth = bmi4.bV4Width;
        bmpHeight = bmi4.bV4Height;
        hdr_read = true;
        break;
    case sizeof(BMPV5HEADER):
        bmi5.bV5Size = bihSize;
        std::fread((BYTE*)&bmi5 + sizeof(DWORD), 1,
                   sizeof(BMPV5HEADER) - sizeof(DWORD), f);
        bmpBpp = bmi5.bV5BitCount;
        bmpWidth = bmi5.bV5Width;
        bmpHeight = bmi5.bV5Height;
        hdr_read = true;
        break;
    }

    const bool top_down = bmpHeight < 0;
    const uint image_width = bmpWidth > 0 ? static_cast<uint>(bmpWidth) : 0;
    const uint image_height =
        bmpHeight < 0 ? static_cast<uint>(-bmpHeight) : static_cast<uint>(bmpHeight);

    if (bmp_info)
    {
        bmp_info->Width = image_width;
        bmp_info->Height = image_height;
        bmp_info->Bpp = bmpBpp;
    }

    RGB_QUAD color_table[256];
    std::unique_ptr<Color4b[]> aBitmapBits;

    if (hdr_read && bmfh.bfType == 0x4d42 && image_width > 0 &&
        image_height > 0)
    {
        aBitmapBits = std::make_unique<Color4b[]>(image_width * image_height);

        const DWORD pitch = BmpRowPitchBytes(image_width, bmpBpp);
        std::unique_ptr<BYTE[]> buff_in =
            std::make_unique<BYTE[]>(pitch * image_height);

        switch (bmpBpp)
        {
        case 1:
            std::fread(color_table, sizeof(RGB_QUAD), 2, f);
            break;
        case 4:
            std::fread(color_table, sizeof(RGB_QUAD), 16, f);
            break;
        case 8:
            std::fread(color_table, sizeof(RGB_QUAD), 256, f);
            break;
        }

        std::fseek(f, bmfh.bfOffBits, SEEK_SET);

        // fill buffer from end to begin. image in bitmap is saved from bottom
        // to top
        for (uint i = 0; i < image_height; i++)
        {
            const uint dst_row = top_down ? i : image_height - i - 1;
            std::fread(buff_in.get() + (pitch * dst_row), 1, pitch, f);
        }

        switch (bmpBpp)
        {
        case 1:
            // color_table -> pixel
            for (uint h = 0; h < image_height; h++)
            {
                for (uint w = 0; w < image_width; w++)
                {
                    int cindex =
                        buff_in[(h * pitch) + (w >> 3)]; // byte = 8 colors
                    int shift = w & 0x07; // current color/bit (0-7)

                    cindex >>= shift;

                    cindex = cindex & 0x01;

                    FORCE_UPCAST(aBitmapBits[h * image_width + w],
                                 color_table[cindex]);
                }
            }
            break;
        case 4:
            // color_table -> pixel
            for (uint h = 0; h < image_height; h++)
            {
                for (uint w = 0; w < image_width; w++)
                {
                    int cindex = buff_in[(h * pitch) + (w >> 1)];
                    int odd = w & 0x01;

                    if (!odd)
                    {
                        cindex >>= 4;
                    }

                    cindex = cindex & 0x0f;

                    FORCE_UPCAST(aBitmapBits[h * image_width + w],
                                 color_table[cindex]);
                }
            }
            break;
        case 8:
            // color_table -> pixel
            for (uint h = 0; h < image_height; h++)
            {
                for (uint w = 0; w < image_width; w++)
                {
                    FORCE_UPCAST(aBitmapBits[h * image_width + w],
                                 color_table[buff_in[h * pitch + w]]);
                    aBitmapBits[h * image_width + w].a = 255;
                }
            }
            break;
        case 24:
            for (uint h = 0; h < image_height; h++)
            {
                for (uint w = 0; w < image_width; w++)
                {
                    uint off = h * image_width + w;
                    RGB_TRIPLE* p =
                        reinterpret_cast<RGB_TRIPLE*>(buff_in.get() +
                                                       h * pitch) +
                        w;

                    aBitmapBits[off].r = p->rgbtRed;
                    aBitmapBits[off].g = p->rgbtGreen;
                    aBitmapBits[off].b = p->rgbtBlue;
                    aBitmapBits[off].a = 255;
                }
            }
            break;
        }
    }

    std::fclose(f);

    return aBitmapBits.release();
}

bool SaveBmp(std::string_view filename, const Color4b* pixels, uint width,
             uint height, uint rowPitchBytes)
{
    if (filename.empty() || pixels == nullptr || width == 0 || height == 0)
        return false;

    if (rowPitchBytes == 0)
        rowPitchBytes = width * sizeof(Color4b);
    if (rowPitchBytes < width * sizeof(Color4b))
        return false;

    if (width > static_cast<uint>(std::numeric_limits<LONG>::max()) ||
        height > static_cast<uint>(std::numeric_limits<LONG>::max()))
        return false;

    const DWORD dst_pitch = BmpRowPitchBytes(width, 24);
    if (height > std::numeric_limits<DWORD>::max() / dst_pitch)
        return false;
    const DWORD image_size = dst_pitch * height;

    const DWORD file_header_size = sizeof(BMPFILEHEADER);
    const DWORD info_header_size = sizeof(BMPINFOHEADER);
    const DWORD pixel_offset = file_header_size + info_header_size;
    if (image_size > std::numeric_limits<DWORD>::max() - pixel_offset)
        return false;

    const std::string path(filename);
    FILE* f = std::fopen(path.c_str(), "wb");
    if (f == nullptr)
        return false;

    BMPFILEHEADER file_header{};
    file_header.bfType = 0x4d42;
    file_header.bfSize = pixel_offset + image_size;
    file_header.bfOffBits = pixel_offset;

    BMPINFOHEADER info_header{};
    info_header.biSize = info_header_size;
    info_header.biWidth = static_cast<LONG>(width);
    info_header.biHeight = static_cast<LONG>(height);
    info_header.biPlanes = 1;
    info_header.biBitCount = 24;
    info_header.biCompression = BMPC_RGB;
    info_header.biSizeImage = image_size;

    bool ok = std::fwrite(&file_header, sizeof(file_header), 1, f) == 1 &&
              std::fwrite(&info_header, sizeof(info_header), 1, f) == 1;

    const BYTE padding[3] = {};
    const uint padding_size = dst_pitch - width * sizeof(RGB_TRIPLE);
    for (uint row = 0; ok && row < height; ++row)
    {
        const uint src_y = height - row - 1;
        const Color4b* src =
            reinterpret_cast<const Color4b*>(
                reinterpret_cast<const BYTE*>(pixels) + src_y * rowPitchBytes);

        for (uint x = 0; ok && x < width; ++x)
        {
            const RGB_TRIPLE out = {src[x].b, src[x].g, src[x].r};
            ok = std::fwrite(&out, sizeof(out), 1, f) == 1;
        }

        if (ok && padding_size > 0)
            ok = std::fwrite(padding, padding_size, 1, f) == 1;
    }

    ok = std::fclose(f) == 0 && ok;
    return ok;
}

} // namespace forg
