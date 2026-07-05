// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 Slawomir Strumecki

// Plain C++ (no Metal here) so it mirrors swrenderer/SWRenderer.cpp exactly.

#include "MetalRenderDevice.h"
#include "base.h"
#include "rendering/IRenderer.h"

namespace forg {

class MetalRenderer : public IRenderer
{
  public:
    MetalRenderer() {}
    virtual ~MetalRenderer() {}

    LPCTSTR get_Name() { return _T("Metal renderer"); }

    // Covariant return type is fine; mirror SWRenderer::CreateDevice.
    IRenderDevice* CreateDevice(HWIN hWindow,
                                RENDER_PARAMETERS* pPresentationParameters);
};

IRenderDevice*
MetalRenderer::CreateDevice(HWIN hWindow,
                            RENDER_PARAMETERS* pPresentationParameters)
{
    MetalRenderDevice* dev = new MetalRenderDevice(hWindow);

    if (dev->Initialize(pPresentationParameters->BackBufferWidth,
                        pPresentationParameters->BackBufferHeight) != FORG_OK)
    {
        dev->Release();
        dev = 0;
    }

    return dev;
}

} // namespace forg

// C linkage so the macapp loader can dlsym("forgCreateRenderer") - matches the
// extern "C" declaration swrenderer exposes through SWRenderer.h.
extern "C" forg::IRenderer* forgCreateRenderer();
extern "C" int forgDestroyRenderer(forg::IRenderer* renderer);
extern "C" const forg::RendererPluginDescriptor*
forgGetRendererPluginDescriptor();

forg::IRenderer* forgCreateRenderer() { return (new forg::MetalRenderer()); }

int forgDestroyRenderer(forg::IRenderer* renderer)
{
    delete renderer;
    return FORG_OK;
}

const forg::RendererPluginDescriptor* forgGetRendererPluginDescriptor()
{
    static const forg::RendererPluginDescriptor descriptor{
        sizeof(forg::RendererPluginDescriptor), forg::RendererPluginApiVersion,
        &forgCreateRenderer, &forgDestroyRenderer, "Metal Renderer"};
    return &descriptor;
}
