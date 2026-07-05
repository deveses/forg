// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "forg.h"

namespace forg {

using namespace forg::core;

class GLTexture
{
    // Nested
  public:
    typedef void* (GLTexture::*PFNLOCKRECT)(u32, u32);
    typedef int (GLTexture::*PFNUNLOCKRECT)(u32);

    typedef void (GLTexture::*PFNCREATETEXTURE)(void);
    typedef void (GLTexture::*PFNRELEASETEXTURE)(void);

    // 'structors
  public:
    GLTexture(IRenderDevice* device, u32 width, u32 height, u32 numLevels,
              u32 usage, u32 format, u32 pool);

    ~GLTexture(void);

    // Attributes
  private:
    IRenderDevice* m_device;

    u32 m_width;
    u32 m_height;
    u32 m_levels;
    u32 m_usage;
    u32 m_format;
    u32 m_pool;
    byte** m_data;
    u32 m_id;
    u32* m_buffers;

    PFNCREATETEXTURE Create;
    PFNRELEASETEXTURE Release;

    PFNLOCKRECT LockRectInternal;
    PFNUNLOCKRECT UnlockRectInternal;

    // Properties
  public:
    u32 get_TextureID() const { return m_id; }

    // Helpers
  private:
    void CreateSysMem();
    void ReleaseSysMem();
    void* LockRectSysMem(u32 Level, u32 Flags);
    int UnlockRectSysMem(u32 Level);

    void CreatePBO();
    void ReleasePBO();
    void* LockRectPBO(u32 Level, u32 Flags);
    int UnlockRectPBO(u32 Level);

    // ITexture implementation
  public:
    u32 GetLevelCount() { return m_levels; };

    int GetLevelDesc(u32 Level, SurfaceDescription* Description) const;

    void* LockRect(u32 Level, u32 Flags);

    int UnlockRect(u32 Level);
};

class ITextureGLImpl : public ITexture
{
    // 'structors
  private:
    ITextureGLImpl() {};
    ~ITextureGLImpl();

  public:
    static ITextureGLImpl* Create(IRenderDevice* device, u32 width, u32 height,
                                  u32 numLevels, u32 usage, u32 format,
                                  u32 pool);

    // Attributes
  private:
    GLTexture* m_texture;
    u32 m_refCount;

  public:
    GLTexture* get_Texture() { return m_texture; }

    // ITexture implementation
  public:
    u32 GetLevelCount() { return m_texture->GetLevelCount(); };

    int GetLevelDesc(u32 Level, SurfaceDescription* Description) const
    {
        return m_texture->GetLevelDesc(Level, Description);
    };

    void* LockRect(u32 Level, u32 Flags)
    {
        return m_texture->LockRect(Level, Flags);
    };

    int UnlockRect(u32 Level) { return m_texture->UnlockRect(Level); };
};

} // namespace forg
