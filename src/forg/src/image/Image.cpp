#include "forg_pch.h"

#include "image/Image.h"

#include "debug/dbg.h"
#include "image/bmp/bmp.h"
#include "image/dds/dds.h"
#include "image/ppm/ppm.h"
#include "math/Math.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

using namespace forg::math;

namespace forg {

enum class ImageMagic : uint
{
    Bmp = 0x4d42,
    Ppm = 0x3650,
    Dds = 0x20534444,
};

enum class ImageFileType
{
    NotFound = -1,
    Unknown = 0,
    Bmp,
    Dds,
    Ppm,
};

////////////////////////////////////////////////////////////////////////////////

/*
enum
{
    CLAMP = 0,
    MIRROR,
    WRAP,
};

static inline void verify_image_coords(int& x, int& y, int width, int height,
int mode)
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

static inline vec4 sample_cubic(CTexture2D* tex, uint pos_x, uint pos_y, uint
dst_width, uint dst_height)
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

static inline vec4 sample_point(CTexture2D* tex, uint pos_x, uint pos_y, uint
dst_width, uint dst_height)
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

static void Resize_NearestNeighbor(Color4b* src, uint src_width,
                                   uint src_height, Color4b* dst,
                                   uint dst_width, uint dst_height)
{
    float sx = (float)src_width / dst_width;
    float sy = (float)src_height / dst_height;

    for (uint h = 0; h < dst_height; h++)
    {
        uint dst_off = h * dst_width;
        uint src_off = h * sy * src_width;

        for (uint w = 0; w < dst_width; w++)
        {
            uint x = w * sx;

            dst[dst_off + w] = src[src_off + x];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static ImageFileType detect_file_type(std::string_view filename)
{
    const std::string path(filename);
    FILE* f = std::fopen(path.c_str(), "rb");

    if (f != NULL)
    {
        uint magic_number = 0;
        uint magic_lword = 0;

        std::fread(&magic_number, 4, 1, f);
        std::fclose(f);

        magic_lword = magic_number & 0xffff;

        switch (static_cast<ImageMagic>(magic_lword))
        {
        case ImageMagic::Bmp:
            return ImageFileType::Bmp;
        case ImageMagic::Ppm:
            return ImageFileType::Ppm;
        default:
            break;
        }

        switch (static_cast<ImageMagic>(magic_number))
        {
        case ImageMagic::Dds:
            return ImageFileType::Dds;
        default:
            break;
        }

        return ImageFileType::Unknown;
    }

    return ImageFileType::NotFound;
}

//////////////////////////////////////////////////////////////////////////

static bool has_extension(std::string_view filename, std::string_view extension)
{
    if (filename.size() < extension.size())
        return false;

    filename = filename.substr(filename.size() - extension.size());
    for (std::string_view::size_type i = 0; i < extension.size(); ++i)
    {
        const char a = static_cast<char>(
            std::tolower(static_cast<unsigned char>(filename[i])));
        const char b = static_cast<char>(
            std::tolower(static_cast<unsigned char>(extension[i])));
        if (a != b)
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

Image::Image() : m_width(0), m_height(0), m_num_mipmaps(1) {}

Image::~Image() { Clean(); }

void Image::Clean()
{
    m_data.clear();
    m_num_mipmaps = 1;
}

bool Image::Load(std::string_view filename)
{
    Clean();

    ImageDescription img_info{};
    std::unique_ptr<Color4b[]> img_data;
    const std::string path(filename);

    switch (detect_file_type(filename))
    {
    case ImageFileType::Bmp:
    {
        img_data.reset(LoadBmp(path.c_str(), &img_info));
    }
    break;

    case ImageFileType::Dds:
    {
        img_data.reset(LoadDds(path.c_str(), &img_info));
    }
    break;

    case ImageFileType::Ppm:
    {
        img_data.reset(LoadPpm(path.c_str(), &img_info));
    }
    break;

    case ImageFileType::NotFound:
        DBG_MSG("[Image] <%s>: File not found!\n", path.c_str());
        break;

    default:
        DBG_MSG("[Image] <%s>: Unsupported image format!\n", path.c_str());
    }

    if (img_data)
    {
        m_width = img_info.Width;
        m_height = img_info.Height;

        const uint size = GetWidth() * GetHeight() * sizeof(Color4b);
        m_data.resize(m_num_mipmaps);
        m_data[0].resize(size);
        std::memcpy(m_data[0].data(), img_data.get(), size);

        return true;
    }

    return false;
}

bool Image::Save(std::string_view filename) const
{
    if (m_data.empty() || m_data[0].empty())
        return false;

    if (has_extension(filename, ".ppm"))
    {
        return SavePpm(filename, reinterpret_cast<const Color4b*>(m_data[0].data()),
                       m_width, m_height, m_width * sizeof(Color4b));
    }

    if (has_extension(filename, ".bmp"))
    {
        return SaveBmp(filename, reinterpret_cast<const Color4b*>(m_data[0].data()),
                       m_width, m_height, m_width * sizeof(Color4b));
    }

    DBG_MSG("[Image] <%s>: Unsupported image format!\n",
            std::string(filename).c_str());
    return false;
}

const char* Image::GetData(uint _level) const
{
    if (_level < m_data.size())
        return m_data[_level].data();

    return NULL;
}

uint Image::GetSize(uint _level) const
{
    uint w = m_width >> _level;
    uint h = m_height >> _level;

    w |= (-(w == 0)) & 1;
    h |= (-(h == 0)) & 1;

    return w * h * 4;
}

uint Image::GenerateMipmaps()
{
    if (m_data.empty())
        return 0;

    uint w = m_width;
    uint h = m_height;

    int num_w = Math::bit_log2(m_width);
    int num_h = Math::bit_log2(m_height);

    uint levels = Math::bit_max(num_w, num_h) + 1;

    std::vector<std::vector<char>> new_data(levels);

    new_data[0] = std::move(m_data[0]);

    for (uint l = 1; l < levels; l++)
    {
        w >>= 1;
        h >>= 1;

        w |= (-(w == 0)) & 1;
        h |= (-(h == 0)) & 1;

        new_data[l].resize(w * h * 4);

        Resize_NearestNeighbor((Color4b*)new_data[0].data(), m_width, m_height,
                               (Color4b*)new_data[l].data(), w, h);
        // memset(new_data[l], l*10, w*h*4);
    }

    m_data = std::move(new_data);
    m_num_mipmaps = levels;

    return m_num_mipmaps;
}

} // namespace forg
