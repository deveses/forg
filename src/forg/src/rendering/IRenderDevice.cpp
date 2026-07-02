#include "forg_pch.h"

#include "image/ppm/ppm.h"
#include "rendering/IRenderDevice.h"

#include <vector>

namespace forg {

int IRenderDevice::SaveBackBuffer(std::string_view filename)
{
    BackBuffer backBuffer;
    const int result = GetBackBuffer(backBuffer);
    if (result != FORG_OK)
        return result;

    return WriteBackBufferToFile(filename, backBuffer);
}

int IRenderDevice::WriteBackBufferToFile(std::string_view filename,
                                         const BackBuffer& backBuffer)
{
    if (filename.empty() || backBuffer.Width == 0 || backBuffer.Height == 0 ||
        backBuffer.RowPitch < backBuffer.Width * 4 ||
        backBuffer.Format != BackBufferPixelFormat::RGBA8 ||
        backBuffer.Pixels.size() < backBuffer.RowPitch * backBuffer.Height)
    {
        return FORG_INVALID_CALL;
    }

    std::vector<Color4b> pixels(backBuffer.Width * backBuffer.Height);
    for (uint y = 0; y < backBuffer.Height; ++y)
    {
        const unsigned char* row =
            backBuffer.Pixels.data() + y * backBuffer.RowPitch;
        for (uint x = 0; x < backBuffer.Width; ++x)
        {
            Color4b& pixel = pixels[y * backBuffer.Width + x];
            pixel.r = row[x * 4 + 0];
            pixel.g = row[x * 4 + 1];
            pixel.b = row[x * 4 + 2];
            pixel.a = row[x * 4 + 3];
        }
    }

    return SavePpm(filename, pixels.data(), backBuffer.Width, backBuffer.Height,
                   backBuffer.Width * sizeof(Color4b))
               ? FORG_OK
               : FORG_INVALID_CALL;
}

} // namespace forg
