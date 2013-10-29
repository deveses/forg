#include "GLRenderer.h"
#include "GLFunc.h"
//#include "..\inc\GLRenderDevice.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
//#include <GL/GLU.h>

namespace forg {

class GLRenderer
	: public IRenderer
{
public:
    GLRenderer() {};
    virtual ~GLRenderer(){} ;

    LPCTSTR get_Name() { return _T("OpenGL renderer"); }

	//can return derived class because covariant return types
	GLRenderDevice* CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters);
};

GLRenderDevice* GLRenderer::CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters)
{
    RENDER_PARAMETERS params;

#ifdef _WIN32
		PIXELFORMATDESCRIPTOR pfdTarget = {
			sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
				1,                     // version number
				PFD_DRAW_TO_WINDOW |   // support window
				PFD_SUPPORT_OPENGL |   // support OpenGL
				PFD_DOUBLEBUFFER,      // double buffered
				PFD_TYPE_RGBA,         // RGBA type
				32,                    // 24-bit color depth
				0, 0, 0, 0, 0, 0,      // color bits ignored
				0,                     // no alpha buffer
				0,                     // shift bit ignored
				0,                     // no accumulation buffer
				0, 0, 0, 0,            // accum bits ignored
				24,                    // 32-bit z-buffer
				0,                     // no stencil buffer
				0,                     // no auxiliary buffer
				PFD_MAIN_PLANE,        // main layer
				0,                     // reserved
				0, 0, 0                // layer masks ignored
		};

		HWND hWin = (HWND)hWindow;

	//	if (! SetClassLongPtr(hWindow, GCL_STYLE, CS_HREDRAW | CS_VREDRAW | CS_OWNDC))
	//		return false;
		HDC hDC = GetDC(hWin);
		if (hDC == NULL)
			return 0;

        PIXELFORMATDESCRIPTOR pfd;
        ZeroMemory(&pfd,sizeof(pfd));
        pfd.nSize=sizeof(pfd);
        pfd.nVersion=1;

        int numFormats = DescribePixelFormat(hDC, 1, sizeof(pfd), &pfd);
        if (numFormats == 0)
            return 0;

        for (int i=1; i<=numFormats; i++)
        {
            ZeroMemory(&pfd, sizeof(pfd));
            pfd.nSize=sizeof(pfd);
            pfd.nVersion=1;

            DescribePixelFormat(hDC, i, sizeof(pfd), &pfd);

            int bpp=pfd.cColorBits;
            int depth=pfd.cDepthBits;
            bool pal=(pfd.iPixelType==PFD_TYPE_COLORINDEX);
            bool mcd=((pfd.dwFlags & PFD_GENERIC_FORMAT) && (pfd.dwFlags & PFD_GENERIC_ACCELERATED));
            bool soft=((pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
            bool icd=(!(pfd.dwFlags & PFD_GENERIC_FORMAT) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
            bool opengl=(pfd.dwFlags & PFD_SUPPORT_OPENGL);
            bool window=(pfd.dwFlags & PFD_DRAW_TO_WINDOW);
            bool bitmap=(pfd.dwFlags & PFD_DRAW_TO_BITMAP);
            bool dbuff=(pfd.dwFlags & PFD_DOUBLEBUFFER);

            DBG_MSG("%d: bpp: %d, depth: %d, opengl: %d, windowed: %d, accelerated: %d\n", i, bpp, depth, opengl, window, !soft);

        //    unsigned int q=0;
        //    if (opengl && window) q=q+0x8000;
        //    if (wdepth==-1 || (wdepth>0 && depth>0)) q=q+0x4000;
        //    if (wdbl==-1 || (wdbl==0 && !dbuff) || (wdbl==1 && dbuff)) q=q+0x2000;
        //    if (wacc==-1 || (wacc==0 && soft) || (wacc==1 && (mcd || icd))) q=q+0x1000;
        //    if (mcd || icd) q=q+0x0040; if (icd) q=q+0x0002;
        //    if (wbpp==-1 || (wbpp==bpp)) q=q+0x0800;
        //    if (bpp>=16) q=q+0x0020; if (bpp==16) q=q+0x0008;
        //    if (wdepth==-1 || (wdepth==depth)) q=q+0x0400;
        //    if (depth>=16) q=q+0x0010; if (depth==16) q=q+0x0004;
        //    if (!pal) q=q+0x0080;
        //    if (bitmap) q=q+0x0001;
        //    if (q>maxqual) {maxqual=q; maxindex=i;max_bpp=bpp; max_depth=depth; max_dbl=dbuff?1:0; max_acc=soft?0:1;}
        }

		int pformat = ChoosePixelFormat(hDC,&pfdTarget);
		if (! pformat)
			return 0;

		SetPixelFormat(hDC, pformat, &pfdTarget);

		HGLRC hRC = wglGetCurrentContext();

		if (hRC == NULL)
        {
            hRC = wglCreateContext(hDC);
        }

		if (hRC == NULL)
			return 0;

		wglMakeCurrent(hDC, hRC);

		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


        glShadeModel(GL_SMOOTH);
		//glDisable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);	//need for proper culling
		glEnable(GL_LINE_SMOOTH);  //SHOULD BE CALLED EXPLICTE BU USER
        glEnable(GL_POINT_SMOOTH);
		//glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

//		glViewport(0,0,100,100);
//		glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();

		GLRenderDevice* gldev = new GLRenderDevice();
		gldev->m_hWnd = hWindow;
		gldev->m_hDC = hDC;
		gldev->m_hRC = hRC;

        // TODO: set proper interval, depending on RENDER_PARAMETERS
//		const GLDeviceCaps* caps = gldev->get_DeviceCaps();
//		if (caps->HasCapability(GLCaps_SwapControl))
//		{
//			wglSwapIntervalEXT(0);
//		}

		return gldev;
#else
    return NULL;
#endif

}

}

forg::IRenderer* forgCreateRenderer()
{
    return (new forg::GLRenderer());
}

