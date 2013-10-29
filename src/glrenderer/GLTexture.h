/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef _GL_TEXTURE_H_
#define _GL_TEXTURE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "forg.h"

namespace forg{

using namespace forg::core;

class GLTexture
{
// Nested
public:
typedef void* (GLTexture::*PFNLOCKRECT)(uint, uint);
typedef int (GLTexture::*PFNUNLOCKRECT)(uint);

typedef void (GLTexture::*PFNCREATETEXTURE)(void);
typedef void (GLTexture::*PFNRELEASETEXTURE)(void);

// 'structors
public:
	GLTexture(
		IRenderDevice* device,
		uint width,
		uint height,
		uint numLevels,
		uint usage,
		uint format,
		uint pool);

	~GLTexture(void);

//Attributes
private:
	IRenderDevice* m_device;

	uint m_width;
	uint m_height;
	uint m_levels;
	uint m_usage;
	uint m_format;
	uint m_pool;
	byte** m_data;
	uint m_id;
	uint* m_buffers;

    PFNCREATETEXTURE Create;
    PFNRELEASETEXTURE Release;

    PFNLOCKRECT LockRectInternal;
    PFNUNLOCKRECT UnlockRectInternal;

// Properties
public:
    uint get_TextureID() const { return m_id; }


//Helpers
private:
    void CreateSysMem();
    void ReleaseSysMem();
    void* LockRectSysMem(uint Level, uint Flags);
    int UnlockRectSysMem(uint Level);

    void CreatePBO();
    void ReleasePBO();
    void* LockRectPBO(uint Level, uint Flags);
    int UnlockRectPBO(uint Level);

// ITexture implementation
public:
    uint GetLevelCount() { return m_levels; };

    int GetLevelDesc(uint Level, SurfaceDescription* Description) const;

    void* LockRect(uint Level, uint Flags);

    int UnlockRect(uint Level);
};

class ITextureGLImpl : public ITexture
{
// 'structors
private:
    ITextureGLImpl() {};
    ~ITextureGLImpl();

public:
	static ITextureGLImpl* Create(
		IRenderDevice* device,
		uint width,
		uint height,
		uint numLevels,
		uint usage,
		uint format,
		uint pool);

//Attributes
private:
    GLTexture* m_texture;
    uint m_refCount;

public:
    GLTexture* get_Texture() { return m_texture; }

// ITexture implementation
public:
    uint GetLevelCount() { return m_texture->GetLevelCount(); };

    int GetLevelDesc(uint Level, SurfaceDescription* Description) const { return m_texture->GetLevelDesc(Level, Description); };

    void* LockRect(uint Level, uint Flags) { return m_texture->LockRect(Level, Flags); };

    int UnlockRect(uint Level) { return m_texture->UnlockRect(Level); };
};

}

#endif  // _GL_TEXTURE_H_

