// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
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
    int m_nVersionMajor;
    int m_nVersionMinor;
    int m_nVersionRelease;

  public:
    int ReadExtensions();
    bool HasCapability(int capability) const;

  private:
    int GetExtension(int ext_num);
};

} // namespace forg
