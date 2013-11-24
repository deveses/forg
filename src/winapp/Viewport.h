#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "emfc\inc\EWnd.h"

namespace forg { namespace scene {

class Model
{
    geometry::Mesh::ExtendedMaterialVec m_materials;
    core::vector< ITexture* > m_textures;
    
    math::Matrix4 m_mesh_tm;
    geometry::Mesh::MeshPtr m_mesh;

public:
    int Load(const char* _name, IRenderDevice* _device);

    void Render(IRenderDevice* _device);
};

}}


class Viewport : public emfc::EWnd  
{
    forg::IRenderDevice* m_device;
    forg::Camera m_camera;

    forg::math::Matrix4 m_mesh_tm;
	forg::geometry::Mesh::MeshPtr m_mesh;
    forg::scene::Model m_model;

    forg::Font* m_font;
    forg::ui::CUIDialog m_Dialog;

    bool m_fullscreen;
	bool m_show_gui;
	BOOL m_bLMBDown;
	BOOL m_bMouseCaptured;

    int m_fps;
    int m_frame_counter;
    forg::PerformanceCounter m_perf_count;

public:
	Viewport();
	virtual ~Viewport();

public:
	void OnMouseRelease(double fPosX, double fPosY);
	void OnMouseCapture(int nAction, double fPosX, double fPosY);
	void OnLButtonUp(UINT nFlags, POINT point);
	void Render();
	void Cleanup();
    void ToggleFullscreen();
	//LPDIRECT3DDEVICE9 g_pd3dDevice;
	//LPDIRECT3D9 g_pD3D;

	DWORD Create(forg::IRenderer* renderer, int x, int y, int nWidth, int nHeight, HWND hParent);

    // Overrides
public:
    virtual void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual void OnMouseMove(UINT nFlags, POINTS point);
    virtual void OnMouseWheel(UINT nFlags, POINTS point, int delta);
	virtual void OnLButtonDown(UINT nFlags, POINT point);
    virtual void OnSize(UINT nType, int cx, int cy);
	virtual void OnPaint();

private:
    void RenderUI();
};