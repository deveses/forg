#include "forg_pch.h"

#include "image/dds/dds.h"

namespace forg {

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long UINT;
typedef unsigned char UCHAR;
typedef UCHAR BYTE;

// Pixelformat flags
#define DDPF_ALPHAPIXELS                        0x00000001l
#define DDPF_FOURCC                             0x00000004l
#define DDPF_RGB                                0x00000040l

#define DDSD_CAPS               0x00000001l     // default
#define DDSD_HEIGHT             0x00000002l     // Required in every .dds file.
#define DDSD_WIDTH              0x00000004l     // Required in every .dds file.
#define DDSD_PITCH              0x00000008l     // Required when pitch is provided for an uncompressed texture.
#define DDSD_PIXELFORMAT        0x00001000l     // Required in every .dds file.
#define DDSD_MIPMAPCOUNT        0x00020000l     // Required in a mipmapped texture.
#define DDSD_LINEARSIZE         0x00080000l     // Required when pitch is provided for a compressed texture.
#define DDSD_DEPTH              0x00800000l     // Required in a depth texture.


#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

/*
* FOURCC codes for DX compressed-texture pixel formats
*/
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////

struct DDS_PIXELFORMAT {
    UINT dwSize;
    UINT dwFlags;
    UINT dwFourCC;
    UINT dwRGBBitCount;
    UINT dwRBitMask;
    UINT dwGBitMask;
    UINT dwBBitMask;
    UINT dwRGBAlphaBitMask;
};

struct DDS_HEADER
{
    UINT dwSize;
    UINT dwFlags;
    UINT dwHeight;
    UINT dwWidth;
    UINT dwLinearSize;
    UINT dwDepth;
    UINT dwMipMapCount;
    UINT dwReserved1[11];
    DDS_PIXELFORMAT ddpf;
    UINT dwCaps;
    UINT dwCaps2;
    UINT dwCaps3;
    UINT dwCaps4;
    UINT dwReserved2;
};

struct DXT1BLOCK
{
    unsigned short color_0;
    unsigned short color_1;
    unsigned char indices[4];
};

//////////////////////////////////////////////////////////////////////////

#define GET_BITS(value, bit_index, bit_count) ( (value)>>(bit_index) & ((1<<(bit_count))-1) )
#define GET_BLUE_R5G6B5(x) ( (x & 0x1f) )
#define GET_GREEN_R5G6B5(x) ( ((x>>5) & 0x3f) )
#define GET_RED_R5G6B5(x) ( ((x>>11) & 0x1f) )

static int first_bit_num(unsigned int _value)
{
    unsigned int r = 32;

    // result is in 0-31, so we need 5 pow2 numbers to write it
    // 1,2,4,8,16 - with these, we can construct any number from range 0-31

    // we zeroing from right to left by shifting

    // if there is something to zero
    if (_value & 0x0000FFFF)    // 00000000 00000000 11111111 11111111
    {
        _value <<= 16; r -= 16;
        // value = XXXXXXXX XXXXXXXX 00000000 00000000
    }
    // else there are zeros already

    if (_value & 0x00FF00FF)    // 00000000 11111111 00000000 11111111
    {
        _value <<= 8; r -= 8;
        // value = XXXXXXXX 00000000 XXXXXXXX 00000000
    }

    if (_value & 0x0F0F0F0F)    // 00001111 00001111 00001111 00001111
    {
        _value <<= 4; r -= 4;
        // value = XXXX0000 XXXX0000 XXXX0000 XXXX0000
    }

    if (_value & 0x33333333)    // 00110011 00110011 00110011 00110011
    {
        _value <<= 2; r -= 2;
        // value = XX00XX00 XX00XX00 XX00XX00 XX00XX00
    }

    if (_value & 0x55555555)    // 01010101 01010101 01010101 01010101
    {
        _value <<= 1; r -= 1;
        // value = X0X0X0X0 X0X0X0X0 X0X0X0X0 X0X0X0X0
    }

    if (_value)
        r--;


    return r;
}

inline void endian_swap(unsigned short& x)
{
    x = (x>>8) |
        (x<<8);
}

inline void endian_swap(unsigned int& x)
{
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

static void Unpack565(WORD packed, Color4b& _color)
{
    _color.b = (packed >> 11) & 0x1f;
    _color.g = (packed >> 5) & 0x3f;
    _color.r = packed & 0x1f;

    _color.r = (_color.r << 3) | (_color.r >> 2);
    _color.g = (_color.g << 2) | (_color.g >> 4);
    _color.b = (_color.b << 3) | (_color.b >> 2);

    _color.a = 0xff;
}

void DecodeDXT1(Color4b* _output, uint width, uint height, char* _input)
{
    uint hblocks = width >> 2;
    uint vblocks = height >> 2;

    DXT1BLOCK* block = (DXT1BLOCK*)_input;

    Color4b color_table[4];

    for (uint v=0; v<vblocks; v++)
    {
        for (uint h=0; h<hblocks; h++)
        {
            Unpack565(block->color_0, color_table[0]);
            Unpack565(block->color_1, color_table[1]);

            if (color_table[0] < color_table[1])
            {
                color_table[2].r= ((int)color_table[0].r + color_table[1].r)/2;
                color_table[2].g= ((int)color_table[0].g + color_table[1].g)/2;
                color_table[2].b= ((int)color_table[0].b + color_table[1].b)/2;
                color_table[3] = 0;
            }
            else
            {
                color_table[2].r = (((int)color_table[0].r*2) + (int)color_table[1].r)/3;
                color_table[2].g = (((int)color_table[0].g*2) + (int)color_table[1].g)/3;
                color_table[2].b = (((int)color_table[0].b*2) + (int)color_table[1].b)/3;
                color_table[2].a = 0xff;

                color_table[3].r = ((int)color_table[0].r + ((int)color_table[1].r*2))/3;
                color_table[3].g = ((int)color_table[0].g + ((int)color_table[1].g*2))/3;
                color_table[3].b = ((int)color_table[0].b + ((int)color_table[1].b*2))/3;
                color_table[3].a = 0xff;
            }

            Color4b* out = _output + (v<<2)*width + (h<<2);

            // 4 rows
            for (int i=0; i<4; i++)
            {
                int row = block->indices[i];

                // 4 columns - 4 indices per byte
                for (int c=0; c<4; c++)
                {
                    out[c] = color_table[ row & 0x3 ];

                    row >>= 2;
                }

                out += width;
            }

            block++;
        }
    }
}

Color4b* LoadDds(const char* filename, ImageDescription* bmp_info)
{
    FILE* f = fopen(filename, "r+b");

    if (! f)
        return NULL;

    fseek(f, 0, SEEK_END);
    uint file_size = ftell(f);

    char* file_data = new char[file_size];

    fseek(f, 0, SEEK_SET);
    fread(file_data, 1, file_size, f);
    fclose(f);

    DWORD dwMagicNumber = *(DWORD*)(file_data);
    if( dwMagicNumber != 0x20534444 )
        return NULL;

    // setup the pointers in the process request
    DDS_HEADER* pSurfDesc = (DDS_HEADER*)( file_data + sizeof(DWORD) );
    char* pBitData  = file_data + sizeof(DWORD) + sizeof(DDS_HEADER);
    //uint pBitSize   = file_size - sizeof(DWORD) - sizeof(DDS_HEADER);

    bmp_info->Width = pSurfDesc->dwWidth;
    bmp_info->Height = pSurfDesc->dwHeight;

    uint num_pixels = bmp_info->Width*bmp_info->Height;
    Color4b* aBitmapBits = new Color4b[num_pixels];
    int dxt = 0;

    if (pSurfDesc->ddpf.dwFlags & DDPF_FOURCC)
    {
        switch(pSurfDesc->ddpf.dwFourCC)
        {
        case FOURCC_DXT1:
            dxt = 1;
            break;
        case FOURCC_DXT3:
            dxt = 3;
            break;
        case FOURCC_DXT4:
            dxt = 4;
            break;
        case FOURCC_DXT5:
            dxt = 5;
            break;
        }
    } else
    if (pSurfDesc->ddpf.dwFlags & DDPF_RGB)
    {
        uint pix_size = (pSurfDesc->ddpf.dwRGBBitCount >> 3);
        char* dds_data = pBitData;
        int off_r = first_bit_num(pSurfDesc->ddpf.dwRBitMask);
        int off_g = first_bit_num(pSurfDesc->ddpf.dwGBitMask);
        int off_b = first_bit_num(pSurfDesc->ddpf.dwBBitMask);
        int off_a = first_bit_num(pSurfDesc->ddpf.dwRGBAlphaBitMask);

        for (uint i=0; i<num_pixels; i++)
        {
            uint pix = *((uint*)dds_data);

            aBitmapBits[i].b = byte((pix & pSurfDesc->ddpf.dwRBitMask) >> off_r);
            aBitmapBits[i].g = byte((pix & pSurfDesc->ddpf.dwGBitMask) >> off_g);
            aBitmapBits[i].r = byte((pix & pSurfDesc->ddpf.dwBBitMask) >> off_b);

            if (pSurfDesc->ddpf.dwFlags & DDPF_ALPHAPIXELS)
            {
                aBitmapBits[i].a = byte((pix & pSurfDesc->ddpf.dwRGBAlphaBitMask) >> off_a);;
            } else
            {
                aBitmapBits[i].a = 0xff;
            }

            dds_data += pix_size;
        }

        //memcpy(aBitmapBits, pBitData, bmp_info->Width*bmp_info->Height*3);
    }

    switch(dxt)
    {
    case 1:
        DecodeDXT1(aBitmapBits, bmp_info->Width, bmp_info->Height, pBitData);
        break;
    }

    delete [] file_data;

    return aBitmapBits;
}


}
