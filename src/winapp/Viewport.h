#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "forg.h"

#include <windows.h>

class Viewport
{
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    forg::Engine* m_engine;
    forg::IRenderDevice* m_device;

    forg::scene::MeshNode* m_model_node;

    forg::Font* m_font;
    forg::ui::CUIDialog m_Dialog;

    bool m_fullscreen;
    int m_show_gui;
    BOOL m_bLMBDown;
    BOOL m_bMouseCaptured;
    bool m_hasLastMousePoint;
    POINTS m_lastMousePoint;

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
    // LPDIRECT3DDEVICE9 g_pd3dDevice;
    // LPDIRECT3D9 g_pD3D;

    DWORD Create(forg::Engine& engine, int x, int y, int nWidth, int nHeight,
                 HWND hParent);
    BOOL ShowWindow(int nCmdShow);
    HWND SetFocus();
    HWND GetHwnd() const { return m_hWnd; }

    // Overrides
  public:
    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void OnMouseMove(UINT nFlags, POINTS point);
    void OnMouseWheel(UINT nFlags, POINTS point, int delta);
    void OnLButtonDown(UINT nFlags, POINT point);
    void OnSize(UINT nType, int cx, int cy);
    void OnPaint();

  private:
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void Invalidate(BOOL bErase = TRUE);
    void RenderUI();
    static bool RenderEngineFrame(forg::Engine& engine, void* userData);
};
