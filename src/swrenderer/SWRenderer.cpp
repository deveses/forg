#include "SWRenderer.h"
#include "SWRenderDevice.h"
#ifdef _WIN32
#include <Windows.h>
#endif

//#include <CL/cl.h>

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

forg::IRenderer* forgCreateRenderer()
{
    return (new forg::SWRenderer());
}
