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

#ifdef _WIN32
/************************************************************************/
/* WGL_EXT_swap_control [all]                                           */
/************************************************************************/
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0; //(int interval);
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = 0; //(void);
#endif

/************************************************************************/
/* GL_ARB_vertex_buffer_object [all] (standard in OGL 1.5)              */
/************************************************************************/
PFNGLBINDBUFFERARBPROC glBindBufferARB = 0;	//(enum target, uint buffer)
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = 0; //(sizei n, const uint *buffers)
PFNGLGENBUFFERSARBPROC glGenBuffersARB = 0; //(sizei n, uint *buffers)
PFNGLISBUFFERARBPROC glIsBufferARB = 0; //(GLuint buffer)
PFNGLBUFFERDATAARBPROC glBufferDataARB = 0;	//(enum target, sizeiptrARB size, const void *data,	enum usage)
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = 0; //(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB = 0;	//(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data)
PFNGLMAPBUFFERARBPROC glMapBufferARB = 0;	//(GLenum target, GLenum access)
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = 0;	//(GLenum target)
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = 0;	//(GLenum target, GLenum pname, GLint *params)
PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB = 0;	//(GLenum target, GLenum pname, GLvoid* *params)

/************************************************************************/
/* GL_ARB_vertex_program (standard in OGL 2.0)                          */
/************************************************************************/
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = 0;	//(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
PFNGLBINDPROGRAMARBPROC glBindProgramARB = 0;	//(GLenum target, GLuint program);
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = 0;	//(GLsizei n, const GLuint *programs);
PFNGLGENPROGRAMSARBPROC glGenProgramsARB = 0;	//(GLsizei n, GLuint *programs);

/************************************************************************/
/* GL_ARB_shader_objects                                                */
/************************************************************************/
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = 0; //(GLhandleARB obj);
PFNGLGETHANDLEARBPROC glGetHandleARB = 0; //(GLenum pname);
PFNGLDETACHOBJECTARBPROC glDetachObjectARB = 0; //(GLhandleARB containerObj, GLhandleARB attachedObj);
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = 0; //(GLenum shaderType);
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = 0; //(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = 0; //(GLhandleARB shaderObj);
PFNGLGETSHADERIVPROC glGetShaderiv = 0; //(GLuint shader, GLenum pname, GLint *params);
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0; // (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = 0; //(void);
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = 0; //(GLhandleARB containerObj, GLhandleARB obj);
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = 0; //(GLhandleARB programObj);
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0; // (GLuint program, GLenum pname, GLint *params);
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;    // (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = 0; //(GLhandleARB programObj);
PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB = 0; //(GLhandleARB programObj);
PFNGLUNIFORM1FARBPROC glUniform1fARB = 0; //(GLint location, GLfloat v0);
PFNGLUNIFORM2FARBPROC glUniform2fARB = 0; //(GLint location, GLfloat v0, GLfloat v1);
PFNGLUNIFORM3FARBPROC glUniform3fARB = 0; //(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
PFNGLUNIFORM4FARBPROC glUniform4fARB = 0; //(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
PFNGLUNIFORM1IARBPROC glUniform1iARB = 0; //(GLint location, GLint v0);
PFNGLUNIFORM2IARBPROC glUniform2iARB = 0; //(GLint location, GLint v0, GLint v1);
PFNGLUNIFORM3IARBPROC glUniform3iARB = 0; //(GLint location, GLint v0, GLint v1, GLint v2);
PFNGLUNIFORM4IARBPROC glUniform4iARB = 0; //(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
PFNGLUNIFORM1FVARBPROC glUniform1fvARB = 0; //(GLint location, GLsizei count, const GLfloat *value);
PFNGLUNIFORM2FVARBPROC glUniform2fvARB = 0; //(GLint location, GLsizei count, const GLfloat *value);
PFNGLUNIFORM3FVARBPROC glUniform3fvARB = 0; //(GLint location, GLsizei count, const GLfloat *value);
PFNGLUNIFORM4FVARBPROC glUniform4fvARB = 0; //(GLint location, GLsizei count, const GLfloat *value);
PFNGLUNIFORM1IVARBPROC glUniform1ivARB = 0; //(GLint location, GLsizei count, const GLint *value);
PFNGLUNIFORM2IVARBPROC glUniform2ivARB = 0; //(GLint location, GLsizei count, const GLint *value);
PFNGLUNIFORM3IVARBPROC glUniform3ivARB = 0; //(GLint location, GLsizei count, const GLint *value);
PFNGLUNIFORM4IVARBPROC glUniform4ivARB = 0; //(GLint location, GLsizei count, const GLint *value);
PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB = 0; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB = 0; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = 0; //(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB = 0; //(GLhandleARB obj, GLenum pname, GLfloat *params);
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = 0; //(GLhandleARB obj, GLenum pname, GLint *params);
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = 0; //(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB = 0; //(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = 0; //(GLhandleARB programObj, const GLcharARB *name);
PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB = 0; //(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB = 0; //(GLhandleARB programObj, GLint location, GLfloat *params);
PFNGLGETUNIFORMIVARBPROC glGetUniformivARB = 0; //(GLhandleARB programObj, GLint location, GLint *params);
PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB = 0; //(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

#ifdef __cplusplus
}
#endif

