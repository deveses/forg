#include "forg_pch.h"

#include "image/bmp/bmp.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace forg {
namespace {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;

constexpr u16 BmpSignature = 0x4d42;
constexpr u32 BmpCompressionRgb = 0;
constexpr u32 BmpFileHeaderSize = 14;
constexpr u32 BmpInfoHeaderSize = 40;
constexpr u16 SaveBpp = 24;

struct BmpPaletteEntry
{
    byte r = 0;
    byte g = 0;
    byte b = 0;
    byte a = 255;
};

struct BmpHeader
{
    u32 pixelOffset = 0;
    uint width = 0;
    uint height = 0;
    u16 bpp = 0;
    u32 compression = 0;
    u32 colorsUsed = 0;
    bool topDown = false;
    bool coreHeader = false;
};

bool ReadBytes(std::istream& in, void* data, std::size_t size)
{
    return static_cast<bool>(in.read(static_cast<char*>(data), size));
}

bool WriteBytes(std::ostream& out, const void* data, std::size_t size)
{
    return static_cast<bool>(out.write(static_cast<const char*>(data), size));
}

bool ReadU8(std::istream& in, u8& value)
{
    char byte_value = 0;
    if (!in.get(byte_value))
        return false;

    value = static_cast<u8>(static_cast<unsigned char>(byte_value));
    return true;
}

bool ReadU16Le(std::istream& in, u16& value)
{
    std::array<u8, 2> bytes{};
    if (!ReadBytes(in, bytes.data(), bytes.size()))
        return false;

    value = static_cast<u16>(bytes[0] | (bytes[1] << 8));
    return true;
}

bool ReadU32Le(std::istream& in, u32& value)
{
    std::array<u8, 4> bytes{};
    if (!ReadBytes(in, bytes.data(), bytes.size()))
        return false;

    value = static_cast<u32>(bytes[0]) | (static_cast<u32>(bytes[1]) << 8) |
            (static_cast<u32>(bytes[2]) << 16) |
            (static_cast<u32>(bytes[3]) << 24);
    return true;
}

bool ReadI32Le(std::istream& in, i32& value)
{
    u32 raw = 0;
    if (!ReadU32Le(in, raw))
        return false;

    value = static_cast<i32>(raw);
    return true;
}

bool WriteU16Le(std::ostream& out, u16 value)
{
    const std::array<u8, 2> bytes = {
        static_cast<u8>(value & 0xff),
        static_cast<u8>((value >> 8) & 0xff),
    };
    return WriteBytes(out, bytes.data(), bytes.size());
}

bool WriteU32Le(std::ostream& out, u32 value)
{
    const std::array<u8, 4> bytes = {
        static_cast<u8>(value & 0xff),
        static_cast<u8>((value >> 8) & 0xff),
        static_cast<u8>((value >> 16) & 0xff),
        static_cast<u8>((value >> 24) & 0xff),
    };
    return WriteBytes(out, bytes.data(), bytes.size());
}

bool WriteI32Le(std::ostream& out, i32 value)
{
    return WriteU32Le(out, static_cast<u32>(value));
}

bool SkipBytes(std::istream& in, std::streamoff count)
{
    if (count <= 0)
        return true;

    in.seekg(count, std::ios::cur);
    return static_cast<bool>(in);
}

bool CheckedRowPitch(uint width, uint bpp, u32& pitch)
{
    const std::uint64_t bits = static_cast<std::uint64_t>(width) * bpp;
    const std::uint64_t bytes = ((bits + 31) / 32) * 4;
    if (bytes > std::numeric_limits<u32>::max())
        return false;

    pitch = static_cast<u32>(bytes);
    return true;
}

bool CheckedImageSize(uint width, uint height, uint bpp, u32& rowPitch,
                      u32& imageSize)
{
    if (!CheckedRowPitch(width, bpp, rowPitch))
        return false;

    const std::uint64_t size = static_cast<std::uint64_t>(rowPitch) * height;
    if (size > std::numeric_limits<u32>::max())
        return false;

    imageSize = static_cast<u32>(size);
    return true;
}

bool ReadCoreHeader(std::istream& in, BmpHeader& header)
{
    u16 width = 0;
    u16 height = 0;
    u16 planes = 0;
    if (!ReadU16Le(in, width) || !ReadU16Le(in, height) ||
        !ReadU16Le(in, planes) || !ReadU16Le(in, header.bpp))
    {
        return false;
    }

    if (planes != 1 || width == 0 || height == 0)
        return false;

    header.width = width;
    header.height = height;
    header.compression = BmpCompressionRgb;
    header.coreHeader = true;
    return true;
}

bool ReadInfoHeader(std::istream& in, u32 headerSize, BmpHeader& header)
{
    i32 width = 0;
    i32 height = 0;
    u16 planes = 0;
    u32 unusedImageSize = 0;
    i32 unusedPelsPerMeter = 0;
    u32 unusedClrImportant = 0;
    if (!ReadI32Le(in, width) || !ReadI32Le(in, height) ||
        !ReadU16Le(in, planes) || !ReadU16Le(in, header.bpp) ||
        !ReadU32Le(in, header.compression) || !ReadU32Le(in, unusedImageSize) ||
        !ReadI32Le(in, unusedPelsPerMeter) ||
        !ReadI32Le(in, unusedPelsPerMeter) ||
        !ReadU32Le(in, header.colorsUsed) || !ReadU32Le(in, unusedClrImportant))
    {
        return false;
    }

    if (planes != 1 || width <= 0 || height == 0 ||
        height == std::numeric_limits<i32>::min() ||
        header.compression != BmpCompressionRgb)
    {
        return false;
    }

    header.width = static_cast<uint>(width);
    header.height =
        height < 0 ? static_cast<uint>(-height) : static_cast<uint>(height);
    header.topDown = height < 0;

    return SkipBytes(in, static_cast<std::streamoff>(headerSize) -
                             BmpInfoHeaderSize);
}

bool ReadHeader(std::istream& in, BmpHeader& header)
{
    u16 signature = 0;
    u32 unusedFileSize = 0;
    u16 unusedReserved = 0;
    u32 dibHeaderSize = 0;
    if (!ReadU16Le(in, signature) || !ReadU32Le(in, unusedFileSize) ||
        !ReadU16Le(in, unusedReserved) || !ReadU16Le(in, unusedReserved) ||
        !ReadU32Le(in, header.pixelOffset) || !ReadU32Le(in, dibHeaderSize))
    {
        return false;
    }

    if (signature != BmpSignature)
        return false;

    if (dibHeaderSize == 12)
        return ReadCoreHeader(in, header);

    if (dibHeaderSize >= BmpInfoHeaderSize)
        return ReadInfoHeader(in, dibHeaderSize, header);

    return false;
}

uint PaletteEntryCount(const BmpHeader& header)
{
    if (header.bpp > 8)
        return 0;

    const uint maxColors = 1u << header.bpp;
    if (header.colorsUsed == 0)
        return maxColors;

    return header.colorsUsed < maxColors ? static_cast<uint>(header.colorsUsed)
                                         : maxColors;
}

bool ReadPalette(std::istream& in, const BmpHeader& header,
                 std::vector<BmpPaletteEntry>& palette)
{
    const uint count = PaletteEntryCount(header);
    palette.resize(count);

    for (BmpPaletteEntry& entry : palette)
    {
        u8 b = 0;
        u8 g = 0;
        u8 r = 0;
        u8 unused = 0;
        if (!ReadU8(in, b) || !ReadU8(in, g) || !ReadU8(in, r))
            return false;

        if (!header.coreHeader && !ReadU8(in, unused))
            return false;

        entry.r = r;
        entry.g = g;
        entry.b = b;
    }

    return true;
}

bool ReadRows(std::istream& in, const BmpHeader& header, u32 rowPitch,
              std::vector<u8>& rows)
{
    rows.resize(static_cast<std::size_t>(rowPitch) * header.height);
    in.seekg(header.pixelOffset, std::ios::beg);
    if (!in)
        return false;

    for (uint fileRow = 0; fileRow < header.height; ++fileRow)
    {
        const uint dstRow =
            header.topDown ? fileRow : header.height - fileRow - 1;
        if (!ReadBytes(
                in, rows.data() + static_cast<std::size_t>(dstRow) * rowPitch,
                rowPitch))
        {
            return false;
        }
    }

    return true;
}

void DecodeIndexedRows(const BmpHeader& header, const std::vector<u8>& rows,
                       u32 rowPitch,
                       const std::vector<BmpPaletteEntry>& palette,
                       Color4b* pixels)
{
    for (uint y = 0; y < header.height; ++y)
    {
        const u8* row = rows.data() + static_cast<std::size_t>(y) * rowPitch;
        for (uint x = 0; x < header.width; ++x)
        {
            uint paletteIndex = 0;
            if (header.bpp == 8)
            {
                paletteIndex = row[x];
            }
            else if (header.bpp == 4)
            {
                const u8 packed = row[x / 2];
                paletteIndex = (x % 2) == 0 ? packed >> 4 : packed & 0x0f;
            }
            else
            {
                const u8 packed = row[x / 8];
                paletteIndex = (packed >> (7 - (x % 8))) & 0x01;
            }

            if (paletteIndex >= palette.size())
                continue;

            const BmpPaletteEntry& color = palette[paletteIndex];
            Color4b& pixel = pixels[y * header.width + x];
            pixel.r = color.r;
            pixel.g = color.g;
            pixel.b = color.b;
            pixel.a = color.a;
        }
    }
}

void DecodeRgbRows(const BmpHeader& header, const std::vector<u8>& rows,
                   u32 rowPitch, Color4b* pixels)
{
    const uint bytesPerPixel = header.bpp / 8;
    for (uint y = 0; y < header.height; ++y)
    {
        const u8* row = rows.data() + static_cast<std::size_t>(y) * rowPitch;
        for (uint x = 0; x < header.width; ++x)
        {
            const u8* src = row + static_cast<std::size_t>(x) * bytesPerPixel;
            Color4b& pixel = pixels[y * header.width + x];
            pixel.r = src[2];
            pixel.g = src[1];
            pixel.b = src[0];
            pixel.a = header.bpp == 32 ? src[3] : 255;
        }
    }
}

bool IsSupportedBpp(u16 bpp)
{
    return bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32;
}

bool DecodePixels(std::istream& in, const BmpHeader& header, Color4b* pixels)
{
    if (!IsSupportedBpp(header.bpp))
        return false;

    u32 rowPitch = 0;
    if (!CheckedRowPitch(header.width, header.bpp, rowPitch))
        return false;

    std::vector<BmpPaletteEntry> palette;
    if (!ReadPalette(in, header, palette))
        return false;

    std::vector<u8> rows;
    if (!ReadRows(in, header, rowPitch, rows))
        return false;

    switch (header.bpp)
    {
    case 1:
    case 4:
    case 8:
        DecodeIndexedRows(header, rows, rowPitch, palette, pixels);
        return true;
    case 24:
    case 32:
        DecodeRgbRows(header, rows, rowPitch, pixels);
        return true;
    default:
        return false;
    }
}

bool WriteBmpHeader(std::ostream& out, uint width, uint height, u32 imageSize)
{
    const u32 pixelOffset = BmpFileHeaderSize + BmpInfoHeaderSize;
    const u32 fileSize = pixelOffset + imageSize;

    return WriteU16Le(out, BmpSignature) && WriteU32Le(out, fileSize) &&
           WriteU16Le(out, 0) && WriteU16Le(out, 0) &&
           WriteU32Le(out, pixelOffset) && WriteU32Le(out, BmpInfoHeaderSize) &&
           WriteI32Le(out, static_cast<i32>(width)) &&
           WriteI32Le(out, static_cast<i32>(height)) && WriteU16Le(out, 1) &&
           WriteU16Le(out, SaveBpp) && WriteU32Le(out, BmpCompressionRgb) &&
           WriteU32Le(out, imageSize) && WriteI32Le(out, 0) &&
           WriteI32Le(out, 0) && WriteU32Le(out, 0) && WriteU32Le(out, 0);
}

bool WriteBmpPixels(std::ostream& out, const Color4b* pixels, uint width,
                    uint height, uint rowPitchBytes, u32 dstRowPitch)
{
    const std::array<u8, 3> padding{};
    const uint paddingSize = dstRowPitch - width * 3;

    for (uint row = 0; row < height; ++row)
    {
        const uint srcY = height - row - 1;
        const Color4b* src = reinterpret_cast<const Color4b*>(
            reinterpret_cast<const u8*>(pixels) + srcY * rowPitchBytes);

        for (uint x = 0; x < width; ++x)
        {
            const std::array<u8, 3> bgr = {src[x].b, src[x].g, src[x].r};
            if (!WriteBytes(out, bgr.data(), bgr.size()))
                return false;
        }

        if (paddingSize > 0 && !WriteBytes(out, padding.data(), paddingSize))
        {
            return false;
        }
    }

    return true;
}

} // namespace

Color4b* LoadBmp(const char* filename, ImageDescription* bmp_info)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
        return nullptr;

    BmpHeader header;
    if (!ReadHeader(in, header) || header.width == 0 || header.height == 0)
        return nullptr;

    if (bmp_info != nullptr)
    {
        bmp_info->Width = header.width;
        bmp_info->Height = header.height;
        bmp_info->Bpp = header.bpp;
    }

    if (header.width > std::numeric_limits<std::size_t>::max() / header.height)
    {
        return nullptr;
    }

    const std::size_t pixelCount =
        static_cast<std::size_t>(header.width) * header.height;
    auto pixels = std::make_unique<Color4b[]>(pixelCount);
    if (!DecodePixels(in, header, pixels.get()))
        return nullptr;

    return pixels.release();
}

bool SaveBmp(std::string_view filename, const Color4b* pixels, uint width,
             uint height, uint rowPitchBytes)
{
    if (filename.empty() || pixels == nullptr || width == 0 || height == 0)
        return false;

    const std::uint64_t minRowPitch =
        static_cast<std::uint64_t>(width) * sizeof(Color4b);
    if (minRowPitch > std::numeric_limits<uint>::max())
        return false;

    if (rowPitchBytes == 0)
        rowPitchBytes = static_cast<uint>(minRowPitch);
    if (rowPitchBytes < minRowPitch)
        return false;

    if (width > static_cast<uint>(std::numeric_limits<i32>::max()) ||
        height > static_cast<uint>(std::numeric_limits<i32>::max()))
    {
        return false;
    }

    u32 dstRowPitch = 0;
    u32 imageSize = 0;
    if (!CheckedImageSize(width, height, SaveBpp, dstRowPitch, imageSize))
        return false;

    if (imageSize > std::numeric_limits<u32>::max() -
                        (BmpFileHeaderSize + BmpInfoHeaderSize))
    {
        return false;
    }

    std::ofstream out(std::string(filename), std::ios::binary);
    if (!out)
        return false;

    return WriteBmpHeader(out, width, height, imageSize) &&
           WriteBmpPixels(out, pixels, width, height, rowPitchBytes,
                          dstRowPitch) &&
           out.good();
}

} // namespace forg
