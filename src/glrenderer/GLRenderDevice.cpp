#include "GLRenderDevice.h"
#include "rendering/Vertex.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"
#include "GLTexture.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/gl.h>
//#include <GL/GLU.h>

#include "GLFunc.h"

#ifndef WIN32
#include <GL/glx.h>

//#define wglGetCurrentContext glXGetCurrentContext
//#define wglMakeCurrent glXMakeCurrent
//#define wglDeleteContext glXDestroyContext
#endif

namespace forg {

#ifdef _DEBUG
	LPCSTR GLGetErrorString(int err)
	{
		switch(err) {
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			return "Unknown Error";
		}

		return 0;
	}

	int GLErrorCheck(int line, LPCSTR file, LPCSTR func)
	{
		int glerr = glGetError();
		if(glerr != GL_NO_ERROR)
		{
			TCHAR dbuf[512];

			sprintf(
				dbuf,
				_T("*** Unexpected error encountered! ***\nFile: %s\nLine: %d\nError Code: %s (0x%x)\nCalling: %s\n\n"),
				file, line, GLGetErrorString(glerr), glerr, func
				);

			forg::debug::DbgOutputString(dbuf);
		}

		return glerr;
	}
#endif

#define RAD2DEG 57.295779513082320876798154814105

enum InternalDeclarationIndices
{
	IntDecl_Position = 0,
	IntDecl_Color,
	IntDecl_Normal,
	IntDecl_TextureCoordinate
};

GLenum PTtoGLEnumMap[] =
{
	0,
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN
};

GLenum BlendToGL[] =
{
    0,
    GL_ZERO,
    GL_ONE ,
    0, // D3DBLEND_SRCCOLOR = 3,
    0, // D3DBLEND_INVSRCCOLOR = 4,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA_SATURATE,
    0, // D3DBLEND_BOTHSRCALPHA = 12,
    0, // D3DBLEND_BOTHINVSRCALPHA = 13,
    0, // D3DBLEND_BLENDFACTOR = 14,
    0, // D3DBLEND_INVBLENDFACTOR = 15,
    0, // D3DBLEND_SRCCOLOR2 = 16,
    0, // D3DBLEND_INVSRCCOLOR2 = 17,
};

//=============================================================================
// GLVertexShader
//=============================================================================

class GLVertexShader : public IVertexShader
{
private:
    GLuint m_program;

public:
    GLVertexShader(GLuint prog)
    {
        m_program = prog;
    }

public:
    GLuint GetProgramObject() { return m_program; }
};

class GLPixelShader : public IVertexShader
{
private:
    GLuint m_program;

public:
    GLPixelShader(GLuint prog)
    {
        m_program = prog;
    }

public:
    GLuint GetProgramObject() { return m_program; }
};

class GLConstantTable
{
    GLuint m_shader;
public:
    GLConstantTable(GLuint _shader)
    {
        m_shader = _shader;
    }
};

forg::auto_ptr<GLVertexShader> g_vs;

GLint CheckCompilationStatus(GLuint _shader)
{
    GLint res = GL_FALSE;
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE)
    {
        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &res);
        if (res>0)
        {
            GLchar info_log[512];
            if (res > sizeof(info_log))
            {
                char* buf = new char[res];
                glGetShaderInfoLog(_shader, res, &res, buf);
                DBG_MSG("COMPILATION ERROR, shader info log:\n%s\n", buf);
                delete [] buf;
            } else
            {
                glGetShaderInfoLog(_shader, sizeof(info_log), &res, info_log);
                DBG_MSG("COMPILATION ERROR, shader info log:\n%s\n", info_log);
            }
        }

        return GL_FALSE;
    }

    return GL_TRUE;
}

GLint CheckLinkStatus(GLuint _program)
{
    GLint res = GL_FALSE;
    glGetProgramiv(_program, GL_LINK_STATUS, &res);
    if (res == GL_FALSE)
    {
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &res);
        if (res>0)
        {
            GLchar info_log[512];
            if (res > sizeof(info_log))
            {
                char* buf = new char[res];
                glGetProgramInfoLog(_program, res, &res, buf);
                DBG_MSG("LINK ERROR, program info log:\n%s\n", buf);
                delete [] buf;
            } else
            {
                glGetProgramInfoLog(_program, sizeof(info_log), &res, info_log);
                DBG_MSG("LINK ERROR, program info log:\n%s\n", info_log);
            }
        }

        return GL_FALSE;
    }

    return GL_TRUE;
}

int CompileShader(const char* srcData, const char* profile)
{
    GLuint shtype = ((profile[0] == 'v') ?  GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB);

    GLuint shader = glCreateShaderObjectARB(shtype);
    
    GLV(glShaderSourceARB(shader, 1, &srcData, NULL));
    GLV(glCompileShaderARB(shader));

    if (GL_FALSE == CheckCompilationStatus(shader))
    {
        DBG_MSG("[CompileShader] Failed to compile shader (%s)!\n", profile);
        return -1;
    }
    
    if (g_vs.is_null())
    {
        GLuint program_obj = glCreateProgramObjectARB();
        g_vs = forg::auto_ptr<GLVertexShader>(new GLVertexShader(program_obj));
    }

    GLV( glAttachObjectARB(g_vs->GetProgramObject(), shader) );
    GLV( glLinkProgramARB(g_vs->GetProgramObject()) );
    if (GL_FALSE == CheckLinkStatus(g_vs->GetProgramObject()))
    {
        DBG_MSG("[LinkProgram] Failed to link program!\n");
        return -1;
    }

    return FORG_OK;
}

int CompileShaderFromFile(const char* shader_path, const char* profile)
{
    int ret = -1;
    FILE* fp = fopen(shader_path, "r");
    
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* srcdata = new char[file_size+1];
        fread(srcdata, 1, file_size, fp);
        srcdata[file_size] = 0;
        ret = CompileShader(srcdata, profile);
        delete [] srcdata;

        fclose(fp);
    }

    return ret;
}

//=============================================================================
// GLRenderDevice
//=============================================================================

GLRenderDevice::GLRenderDevice(void)
: m_hWnd(0)
, m_hDC(0)
, m_hRC(0)
, m_vertex_buffer(0)
, m_index_buffer(0)
, m_pVertexDeclaration(0)
, m_refCount(1)
{
	m_caps.ReadExtensions();

	//wglMakeCurrent((HDC)m_hDC, (HGLRC)m_hRC);
    m_render_states.SourceBlend = GL_ONE;
    m_render_states.DestinationBlend = GL_ZERO;

    m_use_shaders = false;
/*
    m_use_shaders = true;
    
    if (FORG_OK == CompileShaderFromFile("../bin/data/shaders/pass.vs", "vs"))
    {
        CompileShaderFromFile("../bin/data/shaders/pass.ps", "ps");
    }
*/
}


GLRenderDevice::~GLRenderDevice(void)
{
  	if (wglGetCurrentContext() == m_hRC)
	{
		if (FALSE == wglMakeCurrent(0, 0))
        {
            DBG_MSG("wglMakeCurrent failed!\n");
        }
	}

    if (m_hDC != NULL)
        ReleaseDC((HWND)m_hWnd, (HDC)m_hDC);

	if (m_hRC != NULL)
	{
		wglDeleteContext((HGLRC)m_hRC);
		m_hRC = 0;
	}

//    if (m_hDC != NULL)
//        ReleaseDC((HWND)m_hWnd, (HDC)m_hDC);

	m_hDC = 0;
	m_hWnd = 0;
}

/************************************************************************/
/* IRenderDevice methods                                                */
/************************************************************************/
int GLRenderDevice::Clear(uint flags, Color color, float zdepth, int stencil)
{
	GLbitfield mask = 0;

	if ((flags & ClearFlags_Target) == ClearFlags_Target)
	{
		mask |= GL_COLOR_BUFFER_BIT;
		glClearColor(color.r, color.g, color.b, color.a);
	}

	if ((flags & ClearFlags_Stencil) == ClearFlags_Stencil)
	{
		mask |= GL_STENCIL_BUFFER_BIT;
		glClearStencil(stencil);
	}

	if ((flags & ClearFlags_ZBuffer) == ClearFlags_ZBuffer)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
		glClearDepth(zdepth);
	}

	GLV(glClear(mask));

	return 0;
}

int GLRenderDevice::Present()
{
	glFlush();
	//glFinish();

#ifdef WIN32
	return (! SwapBuffers( (HDC)m_hDC ));
#else
    return 0;
#endif
}

int GLRenderDevice::Reset()
{
	return 0;
}

int GLRenderDevice::BeginScene()
{
	//wglMakeCurrent((HDC)m_hDC, (HGLRC)m_hRC);	//bardzo wolne, nie wywolywac przy kazdej klatce

	//GLV(glPushMatrix());

	return 0;
}

int GLRenderDevice::EndScene()
{
	return 0;
}

LPVERTEXDECLARATION GLRenderDevice::CreateVertexDeclaration(const VertexElement* pVertexElements)
{
	VertexDeclaration* decl = new VertexDeclaration(pVertexElements);

	return decl;
}

LPVERTEXBUFFER GLRenderDevice::CreateVertexBuffer(
								  uint length,
								  uint usage,
								  uint pool
								  )
{
	return (new GLVertexBuffer(this, length, usage, pool));
}

LPINDEXBUFFER GLRenderDevice::CreateIndexBuffer(
										uint length,
										uint usage,
										bool sixteenBitIndices,
										uint pool
										)
{
	return (new GLIndexBuffer(this, length, usage, pool, sixteenBitIndices));
}

/*
In D3DX9 Vertex shader is a container for compiled shader code.
Code is compiled by
a function such as D3DXCompileShader to create the array from a HLSL shader. 
a function like D3DXAssembleShader to create the token array from an assembly language shader. 
a function like ID3DXEffectCompiler::CompileShader to create the array from an effect. 
During rendering scene shader is set by Device::SetVertexShader.
Compunication through constant table created during compilation.
Variables setting example: 
    g_pConstantTable->SetMatrix( DXUTGetD3D9Device(), "mWorldViewProj", &mWorldViewProj );
    g_pConstantTable->SetFloat( DXUTGetD3D9Device(), "fTime", ( float )fTime );
------------
In OpenGL 2, vertex shader is subtype of shader object created by glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB).
Shader code is then assigned by glShaderSourceARB and compiled by glCompileShaderARB.
Compiled shader is attached by glAttachObjectARB to program object created by glCreateProgramObjectARB.
Then program object is linked to opengl by glLinkProgramARB.
it is turned on by glUseProgramObjectARB.
Variables are set by glGetUniformLocationARB and methods like this:
	void SetInt(GLint variable, int newValue)								{ glUniform1iARB(variable, newValue);		}
	void SetFloat(GLint variable, float newValue)							{ glUniform1fARB(variable, newValue);		}
	void SetFloat2(GLint variable, float v0, float v1)						{ glUniform2fARB(variable, v0, v1);			}
	void SetFloat3(GLint variable, float v0, float v1, float v2)			{ glUniform3fARB(variable, v0, v1, v2);		}
	void SetFloat4(GLint variable, float v0, float v1, float v2, float v3)	{ glUniform4fARB(variable, v0, v1, v2, v3);	}

*/
LPVERTEXSHADER GLRenderDevice::CreateVertexShader(const void *pFunction)
{
    return 0;
}

int GLRenderDevice::SetVertexShader(IVertexShader* shader)
{
    if (shader != 0)
    {
        GLVertexShader* glshader = (GLVertexShader*)shader;

        GLV( glUseProgramObjectARB(glshader->GetProgramObject()) );
    } else
    {
        glUseProgramObjectARB(0);
    }

    return FORG_OK;
}

LPPIXELSHADER GLRenderDevice::CreatePixelShader(const void *pFunction)
{
    return 0;
}

int GLRenderDevice::SetPixelShader(IPixelShader* shader)
{
    if (shader != 0)
    {
        GLVertexShader* glshader = (GLVertexShader*)shader;

        GLV( glUseProgramObjectARB(glshader->GetProgramObject()) );
    } else
    {
        glUseProgramObjectARB(0);
    }

    return FORG_OK;
}

LPTEXTURE GLRenderDevice::CreateTexture(
						uint Width,
						uint Height,
						uint Levels,
						uint Usage,
						uint Format,
						uint Pool
						)
{
    return (ITextureGLImpl::Create(this, Width, Height, Levels, Usage, Format, Pool));
}

LPTEXTURE GLRenderDevice::CreateTextureFromFile(
								const char* filename,
								uint Width,
								uint Height,
								uint Levels,
								uint Usage,
								uint Format,
								uint Pool
								)
{
	return 0;
}

int GLRenderDevice::DrawIndexedUserPrimitives_Slow(PrimitiveType primitiveType,
                                              uint minVertexIndex,
                                              uint numVertexIndices,
                                              uint primitiveCount,
                                              const void* indexData,
                                              bool sixteenBitIndices,
                                              const void* vertexStreamZeroData,
                                              uint VertexStreamZeroStride)
{
    GLenum mode = PTtoGLEnumMap[ primitiveType ];
    uint stride = m_pVertexDeclaration->GetVertexSize();

    glBegin(mode);
    for (uint idx = minVertexIndex; idx<numVertexIndices; idx++)
    {
        uint vindex = 0;

        if (sixteenBitIndices)
            vindex = *((ushort*)((char*)indexData + idx * 2));
        else
            vindex = *((uint*)((char*)indexData + idx * 4));

        if (m_internal_decl[IntDecl_Color].Type != DeclarationType_Unused)
        {
            byte *col = (byte*)((char*)vertexStreamZeroData + stride*vindex + m_internal_decl[IntDecl_Color].Offset);
            glColor4ubv(col);
            //glColor3f(1.0f,0.0f,0.0f);
        }

        if (m_internal_decl[IntDecl_Position].Type != DeclarationType_Unused)
        {
            float *vert = (float*)((char*)vertexStreamZeroData + stride*vindex + m_internal_decl[IntDecl_Position].Offset);
            glVertex3fv(vert);
        }


    }
    glEnd();

    return 0;
}

int GLRenderDevice::DrawIndexedUserPrimitives(PrimitiveType primitiveType,
											  uint minVertexIndex,
											  uint numVertexIndices,
											  uint primitiveCount,
											  const void* indexData,
											  bool sixteenBitIndices,
											  const void* vertexStreamZeroData,
											  uint VertexStreamZeroStride)
{

/*
    return DrawIndexedUserPrimitives_Slow(
        primitiveType,
        minVertexIndex,
        numVertexIndices,
        primitiveCount,
        indexData,
        sixteenBitIndices,
        vertexStreamZeroData,
        VertexStreamZeroStride);*/

	GLenum mode = PTtoGLEnumMap[ primitiveType ];
	uint stride = m_pVertexDeclaration->GetVertexSize();

	if (m_internal_decl[IntDecl_Position].Type != DeclarationType_Unused)
	{
        glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_Position].Type), GL_FLOAT, stride, (byte*)vertexStreamZeroData+m_internal_decl[IntDecl_Position].Offset);
	}

	if (m_internal_decl[IntDecl_Color].Type != DeclarationType_Unused)
	{
        glEnableClientState( GL_COLOR_ARRAY );
		glColorPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_Color].Type), GL_UNSIGNED_BYTE, stride, (byte*)vertexStreamZeroData+m_internal_decl[IntDecl_Color].Offset);
	}

	if (m_internal_decl[IntDecl_Normal].Type != DeclarationType_Unused)
	{
		glNormalPointer(GL_FLOAT, stride, (byte*)vertexStreamZeroData+m_internal_decl[IntDecl_Normal].Offset);
		glEnableClientState( GL_NORMAL_ARRAY );
	}

	if (m_internal_decl[IntDecl_TextureCoordinate].Type != DeclarationType_Unused)
	{
		//glClientActiveTextureARB(GL_TEXTURE0 + m_internal_decl[IntDecl_TextureCoordinate].UsageIndex);
		glTexCoordPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_TextureCoordinate].Type), GL_FLOAT, stride, (byte*)vertexStreamZeroData + m_internal_decl[IntDecl_TextureCoordinate].Offset);
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}

    uint num_indices = primitiveCount;

    switch(primitiveType)
    {
    case PrimitiveType_PointList:
        num_indices = primitiveCount;
        break;
    case PrimitiveType_LineList:
        num_indices <<= 1;
        break;
    case PrimitiveType_LineStrip:
        num_indices = primitiveCount + 1;
        break;
    case PrimitiveType_TriangleList:
        num_indices *= 3;
        break;
    case PrimitiveType_TriangleStrip:
        num_indices = 2 + primitiveCount;
        break;
    case PrimitiveType_TriangleFan:
        break;
    }

    //SetStreamSource(0, 0, 0, 0);
    //SetIndices(0);
	GLV(glDrawElements(mode, num_indices, sixteenBitIndices ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indexData));	//ogl 1.1
    //glDrawArrays(mode, 0, 1);

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	return FORG_OK;
}

int GLRenderDevice::DrawIndexedPrimitive(PrimitiveType primitiveType,
										 int baseVertex,
										 int minVertexIndex,
										 int numVertices,
										 int startIndex,
										 int primCount)
{
	GLenum mode = PTtoGLEnumMap[ primitiveType ];
	uint stride = m_pVertexDeclaration->GetVertexSize();

    // ======================================================================
    // Setup vertex components order
    // ======================================================================

	if (m_internal_decl[IntDecl_Position].Type != DeclarationType_Unused)
	{
        // While a non-zero buffer object is bound to the
        // GL_ARRAY_BUFFER target, the vertex array pointer parameter that is traditionally
        // interpreted as a pointer to client-side memory is instead interpreted as an offset within the
        // buffer object measured in basic machine units.
		glVertexPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_Position].Type), GL_FLOAT, stride, (byte*)0 + m_internal_decl[IntDecl_Position].Offset);
		glEnableClientState( GL_VERTEX_ARRAY );
	}

	if (m_internal_decl[IntDecl_Color].Type != DeclarationType_Unused)
	{
		glColorPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_Color].Type), GL_UNSIGNED_BYTE, stride, (byte*)0 + m_internal_decl[IntDecl_Color].Offset);
		glEnableClientState( GL_COLOR_ARRAY );
	}

	if (m_internal_decl[IntDecl_Normal].Type != DeclarationType_Unused)
	{
		glNormalPointer(GL_FLOAT, stride, (byte*)0 + m_internal_decl[IntDecl_Normal].Offset);
		glEnableClientState( GL_NORMAL_ARRAY );
	}

	if (m_internal_decl[IntDecl_TextureCoordinate].Type != DeclarationType_Unused)
	{
		//glClientActiveTextureARB(GL_TEXTURE0 + m_internal_decl[IntDecl_TextureCoordinate].UsageIndex);
		GLV(glTexCoordPointer(VertexElement::GetTypeCount(m_internal_decl[IntDecl_TextureCoordinate].Type), GL_FLOAT, stride, (byte*)0 + m_internal_decl[IntDecl_TextureCoordinate].Offset));
		GLV(glEnableClientState( GL_TEXTURE_COORD_ARRAY ));
	}

    // ======================================================================
    // Setup number of indices
    // ======================================================================

	bool sixteenBitIndices = ((GLIndexBuffer*)m_index_buffer)->m_sixteen;
	uint numVertexIndices = primCount;

    switch(primitiveType)
    {
    case PrimitiveType_PointList:
        numVertexIndices = primCount;
        break;
    case PrimitiveType_LineList:
        numVertexIndices <<= 1;
        break;
    case PrimitiveType_LineStrip:
        numVertexIndices = primCount + 1;
        break;
    case PrimitiveType_TriangleList:
        numVertexIndices *= 3;
        break;
    case PrimitiveType_TriangleStrip:
        numVertexIndices = primCount + 1;
        break;
    case PrimitiveType_TriangleFan:
        break;
    }

    // While a non-zero buffer object is bound to the GL_ARRAY_ELEMENT_BUFFER target,
    // the indices parameter of glDrawElements,
    // glDrawRangeElements, or
    // glMultiDrawElements that is traditionally
    // interpreted as a pointer to client-side memory is instead interpreted as an offset within the
    // buffer object measured in basic machine units.

    uint realStart = (sixteenBitIndices ? startIndex << 1 : startIndex << 2);
    GLV(glDrawElements(mode, numVertexIndices, sixteenBitIndices ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (byte*)realStart));	//ogl 1.1

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	GLV(glDisableClientState( GL_TEXTURE_COORD_ARRAY ));

	return 0;
}



int GLRenderDevice::SetRenderState(uint state, uint value)
{
	switch(state) {
	case RenderStates_CullMode:
		return SetRenderState_CullMode(value);
		break;
	case RenderStates_ShadeMode:
		return SetRenderState_ShadeMode(value);
		break;
	case RenderStates_NormalizeNormals:
		if (value)
			glEnable(GL_NORMALIZE);
		else
			glDisable(GL_NORMALIZE);
		break;
	case RenderStates_FillMode:
		return SetRenderState_FillMode(value);
		break;
	case RenderStates_Lighting:
		if (value)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
		break;
    case RenderStates_AlphaTestEnable:
        if (value)
            glEnable(GL_ALPHA_TEST);
        else
            glDisable(GL_ALPHA_TEST);
        break;
    case RenderStates_AlphaBlendEnable:
        if (value)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            // disable writing  to depth buffer to allow blend other transparent/translucent objects
            glDepthMask (GL_FALSE);
        }
        else
        {
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        break;
    case RenderStates_SourceBlend:
        m_render_states.SourceBlend = BlendToGL[value];
        glBlendFunc (m_render_states.SourceBlend, m_render_states.DestinationBlend);
        break;
    case RenderStates_DestinationBlend:
        m_render_states.DestinationBlend = BlendToGL[value];
        glBlendFunc (m_render_states.SourceBlend, m_render_states.DestinationBlend);
        break;
	}

	return FORG_OK;
}

void GLRenderDevice::SetTransform(TransformType state, const Matrix4& matrix)
{
	switch(state) {
	case TransformType_Projection:
		glMatrixMode(GL_PROJECTION);
        m_matProjection = matrix;
        glLoadMatrixf(matrix);
		break;
	case TransformType_View:
        m_matView = matrix;
		break;
	case TransformType_World:
		glMatrixMode(GL_MODELVIEW);
        m_matWorld = matrix;
        Matrix4::Multiply(m_matModelView, m_matWorld, m_matView);
        glLoadMatrixf(m_matModelView);

        break;

    case TransformType_Texture0:
	case TransformType_Texture1:
	case TransformType_Texture2:
	case TransformType_Texture3:
	case TransformType_Texture4:
	case TransformType_Texture5:
	case TransformType_Texture6:
	case TransformType_Texture7:
	case TransformType_World1:
	case TransformType_World2:
	case TransformType_World3:
        break;
	}

	//glLoadIdentity();
	//glMultMatrixf((float*)&matrix);

}

void GLRenderDevice::GetTransform(TransformType state, Matrix4& matrix)
{
    switch(state) {
	case TransformType_Projection:
        matrix = m_matProjection;
		break;
	case TransformType_View:
        matrix = m_matView;
		break;
	case TransformType_World:
        matrix = m_matWorld;
        break;

    case TransformType_Texture0:
	case TransformType_Texture1:
	case TransformType_Texture2:
	case TransformType_Texture3:
	case TransformType_Texture4:
	case TransformType_Texture5:
	case TransformType_Texture6:
	case TransformType_Texture7:
	case TransformType_World1:
	case TransformType_World2:
	case TransformType_World3:
        break;
	}
}

int GLRenderDevice::SetVertexDeclaration(const VertexDeclaration* pDecl)
{
	m_pVertexDeclaration = pDecl;

	const VertexElement* decl = m_pVertexDeclaration->GetDeclaration();
	uint count = m_pVertexDeclaration->GetElementsCount();
	//uint stride = m_pVertexDeclaration->GetVertexSize();  //unused

	m_internal_decl[IntDecl_Position].Type = DeclarationType_Unused;
	m_internal_decl[IntDecl_Color].Type = DeclarationType_Unused;
	m_internal_decl[IntDecl_Normal].Type = DeclarationType_Unused;
	m_internal_decl[IntDecl_TextureCoordinate].Type = DeclarationType_Unused;

	for (uint i=0; i<count; i++)
	{
		switch(decl[i].Usage) {
		case DeclarationUsage_Position:
			m_internal_decl[IntDecl_Position] = decl[i];
			break;
		case DeclarationUsage_Color:
			m_internal_decl[IntDecl_Color] = decl[i];
			break;
		case DeclarationUsage_Normal:
			m_internal_decl[IntDecl_Normal] = decl[i];
			break;
		case DeclarationUsage_TextureCoordinate:
			m_internal_decl[IntDecl_TextureCoordinate] = decl[i];
			break;
		}
	}

    if (m_use_shaders)
    {
        SetVertexShader(g_vs.get());
    }

	return 0;
}

int GLRenderDevice::SetStreamSource(
					int streamNumber,
					IVertexBuffer* streamData,
					int offsetInBytes,
					int stride)
{
	GLVertexBuffer *vb = static_cast<GLVertexBuffer*>(streamData);

	m_vertex_buffer = vb;

    // glBindBuffer is available only if the GL version is 1.5 or greater.
    GLV(glBindBufferARB(GL_ARRAY_BUFFER, vb ? vb->m_buffer_id : 0));

	return 0;
}

int GLRenderDevice::SetIndices(IIndexBuffer* pIndexData)
{
	GLIndexBuffer *ib = static_cast<GLIndexBuffer*>(pIndexData);

	m_index_buffer = ib;

    // glBindBuffer is available only if the GL version is 1.5 or greater.
    GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ib ? ib->m_buffer_id : 0));

	return 0;
}

int GLRenderDevice::SetViewport(uint X, uint Y, uint Width, uint Height, float MinZ, float MaxZ)
{
	GLV(glViewport(X, Y, Width, Height));
	GLV(glDepthRange(MinZ, MaxZ));
	return 0;
}

int GLRenderDevice::GetViewport(Viewport* viewport)
{
    if (viewport)
    {
        glGetIntegerv(GL_VIEWPORT, (GLint*)viewport);
        glGetFloatv(GL_DEPTH_RANGE, (GLfloat*)&viewport->MinZ);
    }

    return FORG_OK;
}

// TODO: store texture, add ref and release
int GLRenderDevice::SetTexture(
			   uint Sampler,
			   ITexture* pTexture
			   )
{
	ITextureGLImpl* texture = static_cast<ITextureGLImpl*>(pTexture);

    GLV(glBindTexture(GL_TEXTURE_2D, texture ? texture->get_Texture()->get_TextureID() : 0));

	return FORG_OK;
}

int GLRenderDevice::SetLight( uint Index, const Light* pLight )
{
    glPushMatrix();
    glLoadIdentity();

    glLightfv(GL_LIGHT0, GL_AMBIENT, (const float*)&pLight->Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (const float*)&pLight->Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, (const float*)&pLight->Specular);

    glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, (const float*)&pLight->Attenuation0);
    glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, (const float*)&pLight->Attenuation1);
    glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (const float*)&pLight->Attenuation2);

/*
    glLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, (const float*)&pLight->Falloff);
    float cut_off = pLight->Phi * RAD2DEG / 2.0f;
    glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, (const float*)&cut_off);*/


    glLightfv(GL_LIGHT0, GL_POSITION, pLight->Position);

    glPopMatrix();

    return FORG_OK;
}

int GLRenderDevice::LightEnable( uint LightIndex, bool bEnable )
{
    glEnable(GL_LIGHT0 + LightIndex);

    return FORG_OK;
}

int GLRenderDevice::SetMaterial(const Material* pMaterial)
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, (float*)&pMaterial->Ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, (float*)&pMaterial->Diffuse);
    glMaterialfv(GL_FRONT, GL_EMISSION, (float*)&pMaterial->Emissive);
    glMaterialfv(GL_FRONT, GL_SPECULAR, (float*)&pMaterial->Specular);

    glMaterialf(GL_FRONT, GL_SHININESS, pMaterial->Power);

    return FORG_OK;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

const GLDeviceCaps* GLRenderDevice::get_DeviceCaps() const
{
	return &m_caps;
}

/************************************************************************/
/* Helpers                                                              */
/************************************************************************/

inline int GLRenderDevice::SetRenderState_CullMode(uint value)
{
	switch(value) {
	case Cull_None:
		glDisable(GL_CULL_FACE);
		break;
	case Cull_Clockwise:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		break;
	case Cull_CounterClockwise:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		break;
	}

	return 0;
}

inline int GLRenderDevice::SetRenderState_ShadeMode(uint value)
{
	switch(value) {
	case ShadeMode_Flat:
		glShadeModel(GL_FLAT);
		break;
	case ShadeMode_Gouraud:
		glShadeModel(GL_SMOOTH);
		break;
	}

	return 0;
}

inline int GLRenderDevice::SetRenderState_FillMode(uint value)
{
	switch(value) {
	case FillMode_Point:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case FillMode_WireFrame:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case FillMode_Solid:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}

	return 0;
}

}
