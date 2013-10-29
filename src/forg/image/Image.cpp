#include "forg_pch.h"

#include "image/Image.h"

#include "image/bmp/bmp.h"
#include "image/dds/dds.h"
#include "debug/dbg.h"
#include "math/Math.h"

using namespace forg::math;

namespace forg {

#define MAGIC_BMP 0x4d42

#define MAGIC_DDS 0x20534444

#define IMAGE_NOT_FOUND -1
#define IMAGE_UNKNOWN 0
#define IMAGE_BMP 1
#define IMAGE_DDS 2


////////////////////////////////////////////////////////////////////////////////

static inline float spline_cube(float value)
{
    return value <= 0.0f ? 0.0f : value * value * value;
}

static inline float spline_weight(float value)
{
    return (spline_cube(value + 2) -
        4 * spline_cube(value + 1) +
        6 * spline_cube(value) -
        4 * spline_cube(value - 1)) / 6;
}
/*
enum
{
    CLAMP = 0,
    MIRROR,
    WRAP,
};

static inline void verify_image_coords(int& x, int& y, int width, int height, int mode)
{
    switch (mode)
    {
    case MIRROR:
        y = y < 0
            ? min(-y, height - 1)
            : y >= height
            ? min(2*(height - 1) - y, height - 1)
            : y;

        x = x < 0
            ? min(-x, width - 1)
            : x >= width
            ? min(2*(width - 1) - x, width -1)
            : x;
        break;
    case WRAP:
        y = y < 0
            ? max(0, (height - 1 - y))
            : y >= height
            ? min(y - height - 1, height - 1)
            : y;

        x = x < 0
            ? max(0, width - 1 - x)
            : x >= width
            ? min(x - width - 1, width -1)
            : x;
        break;
    case CLAMP:
    default:
        y = y < 0
            ? 0
            : y >= height
            ? height - 1
            : y;

        x = x < 0
            ? 0
            : x >= width
            ? width - 1
            : x;
        break;
    }
}

static inline vec4 sample_cubic(CTexture2D* tex, uint pos_x, uint pos_y, uint dst_width, uint dst_height)
{
    // bicubic filter
    vec4 p;
    uint tex_width = tex->GetWidth();
    uint tex_height = tex->GetHeight();
    float sx = (float)tex_width / dst_width;
    float sy = (float)tex_height / dst_height;

    float x = pos_x*sx;
    float y = pos_y*sy;
    float dx = x - (uint)x;
    float dy = y - (uint)y;

    vec4 new_pix(0.0f);

    for (int m=-1; m<=2; m++)
    {
        for (int n=-1; n<=2; n++)
        {
            // computing source image coordinates with clamping edge pixels
            int yoff = (int)(y + m);
            int xoff = (int)(x + n);

            verify_image_coords(xoff, yoff, tex_width, tex_height, CLAMP);

            // P(x) = x > 0 ? x : 0;
            // R(x)=( P(x+2)^3 - 4*P(x+1)^3 + 6*P(x)^3 - 4*P(x-1)^3 ) / 6;

            float weight = spline_weight(m - dx)*spline_weight(dy - n);
            float* pix_in = (float*)tex->Pixel(xoff, yoff);

            new_pix[0] += pix_in[0] * weight;
            new_pix[1] += pix_in[1] * weight;
            new_pix[2] += pix_in[2] * weight;
            new_pix[3] += pix_in[3] * weight;
        }
    }

    // add 0.5 to round it up
    p.x = new_pix[0];
    p.y = new_pix[1];
    p.z = new_pix[2];
    p.w = new_pix[3];

//     p.x = new_pix[2] / 255.0f;
//     p.y = new_pix[1] / 255.0f;
//     p.z = new_pix[0] / 255.0f;
//     p.w = new_pix[3] / 255.0f;

    return p;
}

static inline vec4 sample_point(CTexture2D* tex, uint pos_x, uint pos_y, uint dst_width, uint dst_height)
{
    vec4 p;

    pos_x = pos_x * (tex->GetWidth() / dst_width);
    pos_y = pos_y * (tex->GetHeight() / dst_height);

    float* d = (float*)tex->Pixel(pos_x, pos_y, 0);

    p.x = d[0];
    p.y = d[1];
    p.z = d[2];
    p.w = d[3];

    return p;
}
*/

static void Resize_NearestNeighbor(Color4b* src, uint src_width, uint src_height, Color4b* dst, uint dst_width, uint dst_height)
{
    float sx = (float)src_width / dst_width;
    float sy = (float)src_height / dst_height;

    for (uint h=0; h<dst_height; h++)
    {
        uint dst_off = h * dst_width;
        uint src_off = h * sy * src_width;

        for (uint w=0; w<dst_width; w++)
        {
            uint x = w*sx;

            dst[ dst_off + w ] = src[ src_off + x ];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////


static int detect_file_type(const char* _filename)
{
    FILE* f = fopen(_filename, "r+b");

    if (f != NULL)
    {
        uint magic_number = 0;
        uint magic_lword = 0;
        uint magic_hword = 0;

        fread(&magic_number, 4, 1, f);
        fclose(f);

        magic_lword = magic_number & 0xffff;
        magic_hword = (magic_number>>16) & 0xffff;

        switch(magic_lword)
        {
        case MAGIC_BMP:
            return IMAGE_BMP;
        }

        switch(magic_number)
        {
        case MAGIC_DDS:
            return IMAGE_DDS;
        }

        return IMAGE_UNKNOWN;
    }

    return IMAGE_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////

Image::Image()
: m_data(0)
{
    m_num_mipmaps = 1;
}

Image::~Image()
{
    Clean();
}

void Image::Clean()
{
    if (m_data)
    {
        for (uint i=0; i<m_num_mipmaps; i++)
        {
            delete [] m_data[i];
        }

        delete [] m_data;
    }
}

bool Image::Load(const char* _filename)
{
    ImageDescription img_info;
    Color4b* img_data = NULL;

    switch( detect_file_type(_filename) )
    {
    case IMAGE_BMP:
        {
            img_data = LoadBmp(_filename, &img_info);
        }
        break;

    case IMAGE_DDS:
        {
            img_data = LoadDds(_filename, &img_info);
        }
        break;

    case IMAGE_NOT_FOUND:
        DBG_MSG("[Image] <%s>: File not found!\n", _filename);
        break;

    default:
        DBG_MSG("[Image] <%s>: Unsupported image format!\n", _filename);
    }


    if (img_data)
    {
        m_width = img_info.Width;
        m_height = img_info.Height;

        m_data = new char*[m_num_mipmaps];

        m_data[0] = new char[GetWidth()*GetHeight()*sizeof(Color4b)];

        memcpy(m_data[0], img_data, GetWidth()*GetHeight()*sizeof(Color4b));

        delete [] img_data;

        return true;
    }


    return false;
}

const char* Image::GetData(uint _level) const
{
    if (_level < m_num_mipmaps)
        return m_data[_level];

    return NULL;
}

uint Image::GetSize(uint _level) const
{
    uint w = m_width >> _level;
    uint h = m_height >> _level;

    w |= (-(w == 0)) & 1;
    h |= (-(h == 0)) & 1;

    return w*h*4;
}

uint Image::GenerateMipmaps()
{
    uint w = m_width;
    uint h = m_height;

    int num_w = Math::bit_log2(m_width);
    int num_h = Math::bit_log2(m_height);

    uint levels = Math::bit_max(num_w, num_h) + 1;

    char** new_data = new char*[levels];

    new_data[0] = m_data[0];

    for (uint l=1; l < levels; l++)
    {
        w >>= 1;
        h >>= 1;

        w |= (-(w == 0)) & 1;
        h |= (-(h == 0)) & 1;

        new_data[l] = new char[w*h*4];

        Resize_NearestNeighbor((Color4b*)new_data[0], m_width, m_height, (Color4b*)new_data[l], w, h);
        //memset(new_data[l], l*10, w*h*4);
    }

    for (uint l=1; l < m_num_mipmaps; l++)
    {
        delete [] m_data[l];
    }

    delete [] m_data;

    m_data = new_data;
    m_num_mipmaps = levels;

    return m_num_mipmaps;
}

}
