#include "Renderer.h"
#include "RenderDevice.h"
#ifdef _WIN32
#include <Windows.h>
#endif

namespace forg {

class SWRenderer
	: public IRenderer
{
public:
    SWRenderer() {};
    virtual ~SWRenderer(){} ;

    LPCTSTR get_Name() { return _T("Software renderer"); }

	//can return derived class because covariant return types
	IRenderDevice* CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters);
};

IRenderDevice* SWRenderer::CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters)
{
    SWRenderDevice* dev = new SWRenderDevice(hWindow);

    if (dev->Initialize(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight) != FORG_OK)
    {
        dev->Release();
        dev = 0;
    }

    return dev;
}

}

extern "C" {
    forg::IRenderer* forgCreateRenderer()
    {
        return (new forg::SWRenderer());
    }
}
