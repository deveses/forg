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

#ifndef _GLFUNC_H_
#define _GLFUNC_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

//#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include "GL/glext.h"

#ifdef _WIN32
#include <gl/wglext.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

	/************************************************************************/
	/* WGL_EXT_swap_control [all]                                           */
	/************************************************************************/
	extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT; //(int interval);
	extern PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT; //(void);

#endif

	/************************************************************************/
	/* GL_ARB_vertex_buffer_object [all] (core feature in OGL 1.5)          */
	/************************************************************************/
	extern PFNGLBINDBUFFERARBPROC glBindBufferARB;	//(enum target, uint buffer)
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB; //(sizei n, const uint *buffers)
	extern PFNGLGENBUFFERSARBPROC glGenBuffersARB; //(sizei n, uint *buffers)
	extern PFNGLISBUFFERARBPROC glIsBufferARB; //(GLuint buffer)
	extern PFNGLBUFFERDATAARBPROC glBufferDataARB;	//(enum target, sizeiptrARB size, const void *data,	enum usage)
	extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB; //(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)
	extern PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB;	//(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data)
	extern PFNGLMAPBUFFERARBPROC glMapBufferARB;	//(GLenum target, GLenum access)
	extern PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;	//(GLenum target)
	extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB;	//(GLenum target, GLenum pname, GLint *params)
	extern PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB;	//(GLenum target, GLenum pname, GLvoid* *params)

	/************************************************************************/
	/* GL_ARB_vertex_program (core feature in OGL 2.0)                      */
	/************************************************************************/
	extern PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;	//(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
	extern PFNGLBINDPROGRAMARBPROC glBindProgramARB;	//(GLenum target, GLuint program);
	extern PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;	//(GLsizei n, const GLuint *programs);
	extern PFNGLGENPROGRAMSARBPROC glGenProgramsARB;	//(GLsizei n, GLuint *programs);

	/************************************************************************/
	/* GL_ARB_shader_objects [all] (core feature in OGL 2.0                 */
	/************************************************************************/
	extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB; //(GLhandleARB obj);
	extern PFNGLGETHANDLEARBPROC glGetHandleARB; //(GLenum pname);
	extern PFNGLDETACHOBJECTARBPROC glDetachObjectARB; //(GLhandleARB containerObj, GLhandleARB attachedObj);
	extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB; //(GLenum shaderType);
	extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB; //(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
	extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB; //(GLhandleARB shaderObj);
    extern PFNGLGETSHADERIVPROC glGetShaderiv; //(GLuint shader, GLenum pname, GLint *params);
    extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog; // (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB; //(void);
	extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB; //(GLhandleARB containerObj, GLhandleARB obj);
	extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB; //(GLhandleARB programObj);
	extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB; //(GLhandleARB programObj);
	extern PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB; //(GLhandleARB programObj);
    extern PFNGLGETPROGRAMIVPROC glGetProgramiv; // (GLuint program, GLenum pname, GLint *params);
    extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;    // (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	extern PFNGLUNIFORM1FARBPROC glUniform1fARB; //(GLint location, GLfloat v0);
	extern PFNGLUNIFORM2FARBPROC glUniform2fARB; //(GLint location, GLfloat v0, GLfloat v1);
	extern PFNGLUNIFORM3FARBPROC glUniform3fARB; //(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	extern PFNGLUNIFORM4FARBPROC glUniform4fARB; //(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	extern PFNGLUNIFORM1IARBPROC glUniform1iARB; //(GLint location, GLint v0);
	extern PFNGLUNIFORM2IARBPROC glUniform2iARB; //(GLint location, GLint v0, GLint v1);
	extern PFNGLUNIFORM3IARBPROC glUniform3iARB; //(GLint location, GLint v0, GLint v1, GLint v2);
	extern PFNGLUNIFORM4IARBPROC glUniform4iARB; //(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	extern PFNGLUNIFORM1FVARBPROC glUniform1fvARB; //(GLint location, GLsizei count, const GLfloat *value);
	extern PFNGLUNIFORM2FVARBPROC glUniform2fvARB; //(GLint location, GLsizei count, const GLfloat *value);
	extern PFNGLUNIFORM3FVARBPROC glUniform3fvARB; //(GLint location, GLsizei count, const GLfloat *value);
	extern PFNGLUNIFORM4FVARBPROC glUniform4fvARB; //(GLint location, GLsizei count, const GLfloat *value);
	extern PFNGLUNIFORM1IVARBPROC glUniform1ivARB; //(GLint location, GLsizei count, const GLint *value);
	extern PFNGLUNIFORM2IVARBPROC glUniform2ivARB; //(GLint location, GLsizei count, const GLint *value);
	extern PFNGLUNIFORM3IVARBPROC glUniform3ivARB; //(GLint location, GLsizei count, const GLint *value);
	extern PFNGLUNIFORM4IVARBPROC glUniform4ivARB; //(GLint location, GLsizei count, const GLint *value);
	extern PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	extern PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	extern PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	extern PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB; //(GLhandleARB obj, GLenum pname, GLfloat *params);
	extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB; //(GLhandleARB obj, GLenum pname, GLint *params);
	extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB; //(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
	extern PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB; //(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
	extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB; //(GLhandleARB programObj, const GLcharARB *name);
	extern PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB; //(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
	extern PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB; //(GLhandleARB programObj, GLint location, GLfloat *params);
	extern PFNGLGETUNIFORMIVARBPROC glGetUniformivARB; //(GLhandleARB programObj, GLint location, GLint *params);
	extern PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB; //(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

#ifdef __cplusplus
}
#endif

#endif //_GLFUNC_H_
