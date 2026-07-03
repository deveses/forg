#include "forg_pch.h"

#include "image/Image.h"

#include "debug/dbg.h"
#include "image/bmp/bmp.h"
#include "image/dds/dds.h"
#include "image/ppm/ppm.h"
#include "math/Math.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

using namespace forg::math;

namespace forg {

enum class ImageMagic : u32
{
    Bmp = 0x4d42,
    Ppm = 0x3650,
    Dds = 0x20534444,
};

enum class ImageFileType : std::int8_t
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

static inline vec4 sample_cubic(CTexture2D* tex, u32 pos_x, u32 pos_y, u32
dst_width, u32 dst_height)
{
    // bicubic filter
    vec4 p;
    u32 tex_width = tex->GetWidth();
    u32 tex_height = tex->GetHeight();
    float sx = (float)tex_width / dst_width;
    float sy = (float)tex_height / dst_height;

    float x = pos_x*sx;
    float y = pos_y*sy;
    float dx = x - (u32)x;
    float dy = y - (u32)y;

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

static inline vec4 sample_point(CTexture2D* tex, u32 pos_x, u32 pos_y, u32
dst_width, u32 dst_height)
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

static void Resize_NearestNeighbor(Color4b* src, u32 src_width, u32 src_height,
                                   Color4b* dst, u32 dst_width, u32 dst_height)
{
    for (u32 h = 0; h < dst_height; h++)
    {
        const std::size_t dst_off = static_cast<std::size_t>(h) * dst_width;
        const u32 src_y = static_cast<u32>(
            (static_cast<std::uint64_t>(h) * src_height) / dst_height);
        const std::size_t src_off = static_cast<std::size_t>(src_y) * src_width;

        for (u32 w = 0; w < dst_width; w++)
        {
            const u32 x = static_cast<u32>(
                (static_cast<std::uint64_t>(w) * src_width) / dst_width);

            dst[dst_off + w] = src[src_off + x];
        }
    }
}

static std::optional<std::size_t> CalculatePixelCount(u32 width, u32 height)
{
    if (height != 0 && width > std::numeric_limits<std::size_t>::max() / height)
    {
        return std::nullopt;
    }

    return static_cast<std::size_t>(width) * height;
}

static u32 ClampImageCoord(int value, u32 limit)
{
    if (value < 0)
        return 0;

    const u32 coord = static_cast<u32>(value);
    return coord < limit ? coord : limit - 1;
}

static byte FloatToByte(float value)
{
    if (value <= 0.0f)
        return 0;

    if (value >= 255.0f)
        return 255;

    return static_cast<byte>(std::lround(value));
}

////////////////////////////////////////////////////////////////////////////////

static ImageFileType detect_file_type(std::string_view filename)
{
    const std::string path(filename);
    FILE* f = std::fopen(path.c_str(), "rb");

    if (f != NULL)
    {
        u32 magic_number = 0;
        u32 magic_lword = 0;

        const bool read_magic = std::fread(&magic_number, 4, 1, f) == 1;
        std::fclose(f);

        if (!read_magic)
            return ImageFileType::Unknown;

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
        const std::optional<std::size_t> pixel_count =
            CalculatePixelCount(img_info.Width, img_info.Height);
        if (!pixel_count ||
            *pixel_count >
                std::numeric_limits<std::size_t>::max() / sizeof(Color4b))
        {
            return false;
        }

        const std::size_t size = *pixel_count * sizeof(Color4b);
        m_width = img_info.Width;
        m_height = img_info.Height;

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
        return SavePpm(filename,
                       reinterpret_cast<const Color4b*>(m_data[0].data()),
                       m_width, m_height, m_width * sizeof(Color4b));
    }

    if (has_extension(filename, ".bmp"))
    {
        return SaveBmp(filename,
                       reinterpret_cast<const Color4b*>(m_data[0].data()),
                       m_width, m_height, m_width * sizeof(Color4b));
    }

    DBG_MSG("[Image] <%s>: Unsupported image format!\n",
            std::string(filename).c_str());
    return false;
}

const char* Image::GetData(u32 _level) const
{
    if (_level < m_data.size())
        return m_data[_level].data();

    return NULL;
}

u32 Image::GetSize(u32 _level) const
{
    u32 w = m_width >> _level;
    u32 h = m_height >> _level;

    w |= (-(w == 0)) & 1;
    h |= (-(h == 0)) & 1;

    return w * h * 4;
}

bool Image::ApplyConvolution(std::span<const float> _kernel, u32 _kernel_width,
                             u32 _kernel_height, float _scale, float _bias,
                             bool _preserve_alpha)
{
    if (_kernel.empty() || _kernel_width == 0 || _kernel_height == 0 ||
        (_kernel_width % 2) == 0 || (_kernel_height % 2) == 0 ||
        m_data.empty() || m_data[0].empty() || m_width == 0 || m_height == 0)
    {
        return false;
    }

    if (_kernel_height > std::numeric_limits<u32>::max() / _kernel_width)
    {
        return false;
    }

    const std::size_t kernel_size =
        static_cast<std::size_t>(_kernel_width) * _kernel_height;
    if (_kernel.size() != kernel_size)
    {
        return false;
    }

    if (m_width > static_cast<u32>(std::numeric_limits<int>::max()) ||
        m_height > static_cast<u32>(std::numeric_limits<int>::max()) ||
        _kernel_width > static_cast<u32>(std::numeric_limits<int>::max()) ||
        _kernel_height > static_cast<u32>(std::numeric_limits<int>::max()))
    {
        return false;
    }

    const std::optional<std::size_t> pixel_count =
        CalculatePixelCount(m_width, m_height);
    if (!pixel_count || *pixel_count > std::numeric_limits<std::size_t>::max() /
                                           sizeof(Color4b))
    {
        return false;
    }

    const std::size_t data_size = *pixel_count * sizeof(Color4b);
    if (m_data[0].size() < data_size)
    {
        return false;
    }

    const Color4b* src = reinterpret_cast<const Color4b*>(m_data[0].data());
    std::vector<char> filtered(data_size);
    Color4b* dst = reinterpret_cast<Color4b*>(filtered.data());

    const int kernel_center_x = static_cast<int>(_kernel_width / 2);
    const int kernel_center_y = static_cast<int>(_kernel_height / 2);

    for (u32 y = 0; y < m_height; ++y)
    {
        for (u32 x = 0; x < m_width; ++x)
        {
            Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

            for (u32 ky = 0; ky < _kernel_height; ++ky)
            {
                const int sample_y = static_cast<int>(y) +
                                     static_cast<int>(ky) - kernel_center_y;
                const u32 clamped_y = ClampImageCoord(sample_y, m_height);

                for (u32 kx = 0; kx < _kernel_width; ++kx)
                {
                    const int sample_x = static_cast<int>(x) +
                                         static_cast<int>(kx) - kernel_center_x;
                    const u32 clamped_x = ClampImageCoord(sample_x, m_width);
                    const Color4b& sample =
                        src[static_cast<std::size_t>(clamped_y) * m_width +
                            clamped_x];
                    const float weight =
                        _kernel[static_cast<std::size_t>(ky) * _kernel_width +
                                kx];

                    color.r += sample.r * weight;
                    color.g += sample.g * weight;
                    color.b += sample.b * weight;
                    color.a += sample.a * weight;
                }
            }

            const std::size_t dst_index =
                static_cast<std::size_t>(y) * m_width + x;
            Color4b& pixel = dst[dst_index];
            Color4f scaled = color * _scale + Color4f(_bias);
            pixel.r = FloatToByte(scaled.r);
            pixel.g = FloatToByte(scaled.g);
            pixel.b = FloatToByte(scaled.b);
            pixel.a = _preserve_alpha ? src[dst_index].a
                                      : FloatToByte(scaled.a);
        }
    }

    m_data.resize(1);
    m_data[0] = std::move(filtered);
    m_num_mipmaps = 1;

    return true;
}

bool Image::ApplyBoxBlur()
{
    static constexpr float kernel[] = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    return ApplyConvolution(kernel, 3, 3, 1.0f / 9.0f);
}

bool Image::ApplyGaussianBlur()
{
    static constexpr float kernel[] = {
        1.0f, 2.0f, 1.0f, 2.0f, 4.0f, 2.0f, 1.0f, 2.0f, 1.0f,
    };

    return ApplyConvolution(kernel, 3, 3, 1.0f / 16.0f);
}

bool Image::ApplySharpen()
{
    static constexpr float kernel[] = {
        0.0f, -1.0f, 0.0f, -1.0f, 5.0f, -1.0f, 0.0f, -1.0f, 0.0f,
    };

    return ApplyConvolution(kernel, 3, 3);
}

bool Image::ApplyEdgeDetect()
{
    static constexpr float kernel[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f,
    };

    return ApplyConvolution(kernel, 3, 3);
}

u32 Image::GenerateMipmaps()
{
    if (m_data.empty())
        return 0;

    u32 w = m_width;
    u32 h = m_height;

    const u32 num_w = Math::bit_log2(m_width);
    const u32 num_h = Math::bit_log2(m_height);

    const u32 levels = (num_w > num_h ? num_w : num_h) + 1U;

    std::vector<std::vector<char>> new_data(levels);

    new_data[0] = std::move(m_data[0]);

    for (u32 l = 1; l < levels; l++)
    {
        w >>= 1;
        h >>= 1;

        w |= (-(w == 0)) & 1;
        h |= (-(h == 0)) & 1;

        new_data[l].resize(static_cast<std::size_t>(w) * h * sizeof(Color4b));

        Resize_NearestNeighbor(
            reinterpret_cast<Color4b*>(new_data[0].data()), m_width, m_height,
            reinterpret_cast<Color4b*>(new_data[l].data()), w, h);
        // memset(new_data[l], l*10, w*h*4);
    }

    m_data = std::move(new_data);
    m_num_mipmaps = levels;

    return m_num_mipmaps;
}

} // namespace forg
