#include "SWRenderDevice.h"
#include "forg.h"

#include <SDL.h>

#include <cstring>

namespace forg {
/////////////////////////////////////////////////////////////////////////////////////
// SWRenderDevice (Linux presentation: HWIN carries an SDL_Window*)
/////////////////////////////////////////////////////////////////////////////////////
struct SWRenderDevice::Impl
{
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    u32 textureWidth = 0;
    u32 textureHeight = 0;

    ~Impl() { DestroyPresentationResources(); }

    void DestroyTexture()
    {
        if (texture)
        {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        textureWidth = 0;
        textureHeight = 0;
    }

    void DestroyPresentationResources()
    {
        DestroyTexture();

        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
    }

    bool EnsurePresentationResources(SDL_Window* window, u32 width, u32 height)
    {
        if (window == nullptr)
            return false;

        if (renderer == nullptr)
        {
            renderer = SDL_CreateRenderer(window, -1,
                                          SDL_RENDERER_ACCELERATED |
                                              SDL_RENDERER_PRESENTVSYNC);
            if (renderer == nullptr)
                renderer =
                    SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

            if (renderer == nullptr)
                return false;
        }

        if (texture != nullptr && textureWidth == width &&
            textureHeight == height)
        {
            return true;
        }

        DestroyTexture();
        texture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(width), static_cast<int>(height));
        if (texture == nullptr)
            return false;

        textureWidth = width;
        textureHeight = height;
        return true;
    }
};

SWRenderDevice::SWRenderDevice(HWIN handle) : super(handle), m_impl(new Impl) {}

SWRenderDevice::~SWRenderDevice() { delete m_impl; }

int SWRenderDevice::Reset()
{
    SDL_Window* window = static_cast<SDL_Window*>(GetHWIN());
    if (window == nullptr)
        return FORG_INVALID_CALL;

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    SetBufferSize(static_cast<u32>(width), static_cast<u32>(height));
    m_impl->DestroyTexture();

    return super::Reset();
}

int SWRenderDevice::Present()
{
    u32 width = GetWidth();
    u32 height = GetHeight();
    u32* buffer = GetBuffer();

    if (buffer == nullptr || width == 0 || height == 0)
        return FORG_OK;

    SDL_Window* window = static_cast<SDL_Window*>(GetHWIN());
    if (!m_impl->EnsurePresentationResources(window, width, height))
        return FORG_INVALID_CALL;

    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(m_impl->texture, nullptr, &pixels, &pitch) != 0)
        return FORG_INVALID_CALL;

    const size_t rowBytes = static_cast<size_t>(width) * 4;
    auto* dst = static_cast<unsigned char*>(pixels);
    auto* src = reinterpret_cast<const unsigned char*>(buffer);
    for (u32 row = 0; row < height; ++row)
    {
        std::memcpy(dst + static_cast<size_t>(row) * pitch,
                    src + static_cast<size_t>(height - 1 - row) * rowBytes,
                    rowBytes);
    }

    SDL_UnlockTexture(m_impl->texture);

    SDL_RenderClear(m_impl->renderer);
    SDL_RenderCopy(m_impl->renderer, m_impl->texture, nullptr, nullptr);
    SDL_RenderPresent(m_impl->renderer);

    return FORG_OK;
}

} // namespace forg
