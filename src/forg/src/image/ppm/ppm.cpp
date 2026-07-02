#include "forg_pch.h"

#include "image/ppm/ppm.h"

#include <cctype>
#include <fstream>
#include <limits>
#include <memory>
#include <string>

namespace forg {

namespace {

bool ReadToken(std::istream& in, std::string& token)
{
    token.clear();

    while (true)
    {
        int ch = in.peek();
        if (ch == EOF)
            return false;

        if (std::isspace(static_cast<unsigned char>(ch)))
        {
            in.get();
            continue;
        }

        if (ch == '#')
        {
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        break;
    }

    while (true)
    {
        int ch = in.peek();
        if (ch == EOF || std::isspace(static_cast<unsigned char>(ch)))
            break;

        token.push_back(static_cast<char>(in.get()));
    }

    return !token.empty();
}

bool ReadUInt(std::istream& in, u32& value)
{
    std::string token;
    if (!ReadToken(in, token))
        return false;

    std::size_t pos = 0;
    unsigned long parsed = 0;
    try
    {
        parsed = std::stoul(token, &pos, 10);
    }
    catch (...)
    {
        return false;
    }

    if (pos != token.size() || parsed > std::numeric_limits<u32>::max())
        return false;

    value = static_cast<u32>(parsed);
    return true;
}

} // namespace

Color4b* LoadPpm(const char* filename, ImageDescription* ppm_info)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
        return nullptr;

    std::string magic;
    u32 width = 0;
    u32 height = 0;
    u32 maxValue = 0;
    if (!ReadToken(in, magic) || magic != "P6" || !ReadUInt(in, width) ||
        !ReadUInt(in, height) || !ReadUInt(in, maxValue) || maxValue != 255 ||
        width == 0 || height == 0 ||
        width > std::numeric_limits<u32>::max() / height)
    {
        return nullptr;
    }

    int separator = in.get();
    if (separator == EOF ||
        !std::isspace(static_cast<unsigned char>(separator)))
    {
        return nullptr;
    }

    std::unique_ptr<Color4b[]> pixels(new Color4b[width * height]);
    for (u32 i = 0; i < width * height; ++i)
    {
        char rgb[3] = {};
        if (!in.read(rgb, sizeof(rgb)))
            return nullptr;

        pixels[i].r = static_cast<byte>(static_cast<unsigned char>(rgb[0]));
        pixels[i].g = static_cast<byte>(static_cast<unsigned char>(rgb[1]));
        pixels[i].b = static_cast<byte>(static_cast<unsigned char>(rgb[2]));
        pixels[i].a = 255;
    }

    if (ppm_info != nullptr)
    {
        ppm_info->Width = width;
        ppm_info->Height = height;
        ppm_info->Bpp = 24;
    }

    return pixels.release();
}

bool SavePpm(std::string_view filename, const Color4b* pixels, u32 width,
             u32 height, u32 rowPitchBytes)
{
    if (filename.empty() || pixels == nullptr || width == 0 || height == 0)
        return false;

    if (rowPitchBytes == 0)
        rowPitchBytes = width * sizeof(Color4b);
    if (rowPitchBytes < width * sizeof(Color4b))
        return false;

    std::ofstream out(std::string(filename), std::ios::binary);
    if (!out)
        return false;

    out << "P6\n" << width << " " << height << "\n255\n";
    for (u32 y = 0; y < height; ++y)
    {
        const Color4b* row = reinterpret_cast<const Color4b*>(
            reinterpret_cast<const unsigned char*>(pixels) + y * rowPitchBytes);
        for (u32 x = 0; x < width; ++x)
        {
            const char rgb[3] = {static_cast<char>(row[x].r),
                                 static_cast<char>(row[x].g),
                                 static_cast<char>(row[x].b)};
            out.write(rgb, sizeof(rgb));
        }
    }

    return out.good();
}

} // namespace forg
