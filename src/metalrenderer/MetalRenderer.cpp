/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2024  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

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
extern "C" const forg::RendererPluginDescriptor*
forgGetRendererPluginDescriptor();

forg::IRenderer* forgCreateRenderer() { return (new forg::MetalRenderer()); }

const forg::RendererPluginDescriptor* forgGetRendererPluginDescriptor()
{
    static const forg::RendererPluginDescriptor descriptor{
        sizeof(forg::RendererPluginDescriptor), forg::RendererPluginApiVersion,
        &forgCreateRenderer};
    return &descriptor;
}
