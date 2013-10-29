#include "GLDeviceCaps.h"

#include "GLFunc.h"
//#include <string>

//#ifdef _WIN32
//#include <Windows.h>
//#endif
//
//#define GL_GLEXT_PROTOTYPES 1
//#include <gl/gl.h>
//#include <gl/glext.h>
//
//#ifdef _WIN32
//#include <gl/wglext.h>
//#endif

/*
+ ARB – Extensions officially approved by the OpenGL Architecture Review Board
+ EXT – Extensions agreed upon by multiple OpenGL vendors
+ HP – Hewlett-Packard
+ IBM – International Business Machines
+ KTX – Kinetix, maker of 3D Studio Max
+ INTEL – Intel
+ NV – NVIDIA Corporation
+ MESA – Brian Paul’s freeware portable OpenGL implementation
+ SGI – Silicon Graphics
+ SGIX – Silicon Graphics (experimental)
+ SUN – Sun Microsystems
+ WIN – Microsoft
*/

namespace forg {

using namespace forg::core;
using namespace forg::debug;
//using std::string;

#ifdef _DEBUG
#define GLCAPS_CHECK_AND_SETUP(str, ext, cap)\
	{\
	if (! m_capabilities[cap] && str == (StringTokenizer<char>::string_type)_T(#ext))\
	{\
		m_capabilities.Set(cap, GetExtension(cap)!=0 );\
		DbgOutputString(_T(""#ext" \n"));\
	}\
	}
#else
#define GLCAPS_CHECK_AND_SETUP(str, ext, cap)\
	{\
	if (! m_capabilities[cap] && str == (string)_T(#ext))\
	{\
		m_capabilities.Set(cap, GetExtension(cap)!=0 );\
	}\
	}
#endif

#ifdef WIN32
#define GET_GLPROC(type, proc) (proc = (type)wglGetProcAddress(#proc))
#else
#define GET_GLPROC(type, proc) (proc = (type)#proc)
#endif

GLDeviceCaps::GLDeviceCaps(void)
: m_capabilities(GLCaps_Count, false)
{
}

GLDeviceCaps::~GLDeviceCaps(void)
{
}


int GLDeviceCaps::ReadExtensions()
{
	int gl_error = 0;

	char* vendor = (char *)glGetString(GL_VENDOR);
	char* renderer = (char *)glGetString(GL_RENDERER);
	char* version = (char *)glGetString(GL_VERSION);
	char* extensions = (char *)glGetString(GL_EXTENSIONS);

	if ((gl_error = glGetError()) != 0)
	{
		return 1;
	}
	else
	{
	    DBG_MSG("vendor: %s\n", vendor);
	    DBG_MSG("renderer: %s\n", renderer);
	    DBG_MSG("version: %s\n", version);
	}

    DBG_MSG("\nextensions: %s\n", extensions);

	StringTokenizer<char> tokenizer(extensions, " ");

	//checking every token in extensions string
	while (tokenizer.HasMoreTokens())
	{
		StringTokenizer<char>::string_type ext = tokenizer.NextToken();

        GLCAPS_CHECK_AND_SETUP(ext, GL_EXT_texture3D, GLCaps_Texture3D);
        GLCAPS_CHECK_AND_SETUP(ext, GL_EXT_bgra, GLCaps_BGRA);
        GLCAPS_CHECK_AND_SETUP(ext, GL_EXT_packed_pixels, GLCaps_PackedPixels);
        GLCAPS_CHECK_AND_SETUP(ext, GL_EXT_separate_specular_color, GLCaps_SeparateSpecularColor);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_multitexture, GLCaps_Multitexture);
        GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_texture_compression, GLCaps_TextureCompression);
        GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_texture_cube_map, GLCaps_TextureCubeMap);
        GLCAPS_CHECK_AND_SETUP(ext, GL_SGIS_generate_mipmap, GLCaps_GenerateMipmap);
        GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_depth_texture, GLCaps_DepthTexture);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_vertex_buffer_object, GLCaps_VertexBufferObject);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_vertex_program, GLCaps_VertexProgram);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_texture_non_power_of_two, GLCaps_TextureNonPowerOfTwo);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_shader_objects, GLCaps_ShaderObjects);
		GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_shading_language_100, GLCaps_ShadingLanguage100);
        GLCAPS_CHECK_AND_SETUP(ext, GL_ARB_pixel_buffer_object, GLCaps_PixelBufferObject);
#ifdef _WIN32
		GLCAPS_CHECK_AND_SETUP(ext, WGL_EXT_swap_control, GLCaps_SwapControl);
#endif
	}

	return 0;
}

int GLDeviceCaps::GetExtension(int ext_num)
{
	switch(ext_num) {
	case GLCaps_Multitexture:
		break;
	case GLCaps_VertexBufferObject:
		GET_GLPROC(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
		GET_GLPROC(PFNGLDELETEBUFFERSARBPROC, glDeleteBuffersARB);
		GET_GLPROC(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
		GET_GLPROC(PFNGLISBUFFERARBPROC, glIsBufferARB);
		GET_GLPROC(PFNGLBUFFERDATAARBPROC, glBufferDataARB);
		GET_GLPROC(PFNGLBUFFERSUBDATAARBPROC, glBufferSubDataARB);
		GET_GLPROC(PFNGLGETBUFFERSUBDATAARBPROC, glGetBufferSubDataARB);
		GET_GLPROC(PFNGLMAPBUFFERARBPROC, glMapBufferARB);
		GET_GLPROC(PFNGLUNMAPBUFFERARBPROC, glUnmapBufferARB);
		GET_GLPROC(PFNGLGETBUFFERPARAMETERIVARBPROC, glGetBufferParameterivARB);
		GET_GLPROC(PFNGLGETBUFFERPOINTERVARBPROC, glGetBufferPointervARB);
		break;
	case GLCaps_VertexProgram:
		GET_GLPROC(PFNGLPROGRAMSTRINGARBPROC, glProgramStringARB);
		GET_GLPROC(PFNGLBINDPROGRAMARBPROC, glBindProgramARB);
		GET_GLPROC(PFNGLDELETEPROGRAMSARBPROC, glDeleteProgramsARB);
		GET_GLPROC(PFNGLGENPROGRAMSARBPROC, glGenProgramsARB);
		break;
	case GLCaps_ShaderObjects:
		GET_GLPROC(PFNGLDELETEOBJECTARBPROC, glDeleteObjectARB);
		GET_GLPROC(PFNGLGETHANDLEARBPROC, glGetHandleARB);
		GET_GLPROC(PFNGLDETACHOBJECTARBPROC, glDetachObjectARB);
		GET_GLPROC(PFNGLCREATESHADEROBJECTARBPROC, glCreateShaderObjectARB);
		GET_GLPROC(PFNGLSHADERSOURCEARBPROC, glShaderSourceARB);
		GET_GLPROC(PFNGLCOMPILESHADERARBPROC, glCompileShaderARB);
		GET_GLPROC(PFNGLCREATEPROGRAMOBJECTARBPROC, glCreateProgramObjectARB);
		GET_GLPROC(PFNGLATTACHOBJECTARBPROC, glAttachObjectARB);
		GET_GLPROC(PFNGLLINKPROGRAMARBPROC, glLinkProgramARB);
		GET_GLPROC(PFNGLUSEPROGRAMOBJECTARBPROC, glUseProgramObjectARB);
		GET_GLPROC(PFNGLVALIDATEPROGRAMARBPROC, glValidateProgramARB);
        GET_GLPROC(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
        GET_GLPROC(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
		GET_GLPROC(PFNGLUNIFORM1FARBPROC, glUniform1fARB);
		GET_GLPROC(PFNGLUNIFORM2FARBPROC, glUniform2fARB);
		GET_GLPROC(PFNGLUNIFORM3FARBPROC, glUniform3fARB);
		GET_GLPROC(PFNGLUNIFORM4FARBPROC, glUniform4fARB);
		GET_GLPROC(PFNGLUNIFORM1IARBPROC, glUniform1iARB);
		GET_GLPROC(PFNGLUNIFORM2IARBPROC, glUniform2iARB);
		GET_GLPROC(PFNGLUNIFORM3IARBPROC, glUniform3iARB);
		GET_GLPROC(PFNGLUNIFORM4IARBPROC, glUniform4iARB);
		GET_GLPROC(PFNGLUNIFORM1FVARBPROC, glUniform1fvARB);
		GET_GLPROC(PFNGLUNIFORM2FVARBPROC, glUniform2fvARB);
		GET_GLPROC(PFNGLUNIFORM3FVARBPROC, glUniform3fvARB);
		GET_GLPROC(PFNGLUNIFORM4FVARBPROC, glUniform4fvARB);
		GET_GLPROC(PFNGLUNIFORM1IVARBPROC, glUniform1ivARB);
		GET_GLPROC(PFNGLUNIFORM2IVARBPROC, glUniform2ivARB);
		GET_GLPROC(PFNGLUNIFORM3IVARBPROC, glUniform3ivARB);
		GET_GLPROC(PFNGLUNIFORM4IVARBPROC, glUniform4ivARB);
		GET_GLPROC(PFNGLUNIFORMMATRIX2FVARBPROC, glUniformMatrix2fvARB);
		GET_GLPROC(PFNGLUNIFORMMATRIX3FVARBPROC, glUniformMatrix3fvARB);
		GET_GLPROC(PFNGLUNIFORMMATRIX4FVARBPROC, glUniformMatrix4fvARB);
		GET_GLPROC(PFNGLGETOBJECTPARAMETERFVARBPROC, glGetObjectParameterfvARB);
		GET_GLPROC(PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB);
		GET_GLPROC(PFNGLGETINFOLOGARBPROC, glGetInfoLogARB);
		GET_GLPROC(PFNGLGETATTACHEDOBJECTSARBPROC, glGetAttachedObjectsARB);
		GET_GLPROC(PFNGLGETUNIFORMLOCATIONARBPROC, glGetUniformLocationARB);
		GET_GLPROC(PFNGLGETACTIVEUNIFORMARBPROC, glGetActiveUniformARB);
		GET_GLPROC(PFNGLGETUNIFORMFVARBPROC, glGetUniformfvARB);
		GET_GLPROC(PFNGLGETUNIFORMIVARBPROC, glGetUniformivARB);
		GET_GLPROC(PFNGLGETSHADERSOURCEARBPROC, glGetShaderSourceARB);
        GET_GLPROC(PFNGLGETSHADERIVPROC, glGetShaderiv);
        GET_GLPROC(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
		break;
#ifdef _WIN32
	case GLCaps_SwapControl:
		GET_GLPROC(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
		GET_GLPROC(PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);
		break;
#endif
	}

	return 1;
}

bool GLDeviceCaps::HasCapability(int capability) const
{
	if (capability<0 || capability>=GLCaps_Count)
		return false;

	return m_capabilities[capability];
}

}
