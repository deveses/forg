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

#ifndef _GL_DEVICE_CAPS_H_
#define _GL_DEVICE_CAPS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <forg.h>

namespace forg {

using namespace forg::core;

enum GLCaps
{
    /// GL_EXT_texture3D
    GLCaps_Texture3D = 0,

    /// GL_EXT_bgra
    GLCaps_BGRA,

    /// GL_EXT_packed_pixels
    GLCaps_PackedPixels,

    /// GL_EXT_separate_specular_color
    GLCaps_SeparateSpecularColor,

    /// GL_ARB_multitexture
	GLCaps_Multitexture,

    /// GL_ARB_texture_compression
    GLCaps_TextureCompression,

    /// GL_ARB_texture_cube_map
    GLCaps_TextureCubeMap,

    /// GL_SGIS_generate_mipmap
    GLCaps_GenerateMipmap,

    /// GL_ARB_vertex_program,
    GLCaps_VertexProgram,

    /// GL_ARB_depth_texture
    GLCaps_DepthTexture,

    /// GL_ARB_vertex_buffer_object
    GLCaps_VertexBufferObject,

    /// GL_ARB_shader_objects
    GLCaps_ShaderObjects,

    /// GL_ARB_texture_non_power_of_two
    GLCaps_TextureNonPowerOfTwo,

    /// GL_ARB_shading_language_100
    GLCaps_ShadingLanguage100,

    /// GL_ARB_pixel_buffer_object
    GLCaps_PixelBufferObject,

#ifdef _WIN32
	GLCaps_SwapControl,
#endif
	GLCaps_Count
};

class GLDeviceCaps
{
public:
	GLDeviceCaps(void);
	~GLDeviceCaps(void);

private:
	BitArray m_capabilities;

public:
	int	m_nVersionMajor;
	int	m_nVersionMinor;
	int	m_nVersionRelease;

public:
	int ReadExtensions();
	bool HasCapability(int capability) const;

private:
	int GetExtension(int ext_num);
};

}

#endif  // _GL_DEVICE_CAPS_H_
