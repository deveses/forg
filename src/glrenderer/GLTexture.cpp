#include "GLTexture.h"

#include "GLFunc.h"
#include "GLRenderDevice.h"

namespace forg {

static unsigned int bit_log2(unsigned int v)
{
    unsigned int r = 0;
    unsigned int shift = 0;

    shift = ((v & 0xFFFF0000) != 0) << 4;
    v >>= shift;
    r |= shift;
    shift = ((v & 0xFF00) != 0) << 3;
    v >>= shift;
    r |= shift;
    shift = ((v & 0xF0) != 0) << 2;
    v >>= shift;
    r |= shift;
    shift = ((v & 0xC) != 0) << 1;
    v >>= shift;
    r |= shift;
    shift = ((v & 0x2) != 0) << 0;
    v >>= shift;
    r |= shift;

    return r;
}

static int bit_max(int x, int y) { return (x - ((x - y) & -(x < y))); }

static u32 get_internal_format(u32 format)
{
    switch (format)
    {
    case forg::Format::A8L8:
        return GL_LUMINANCE8_ALPHA8;
    case forg::Format::A8R8G8B8:
        return GL_RGBA;
    }

    return GL_RGBA;
}

static u32 get_pixel_format(u32 format)
{
    switch (format)
    {
    case forg::Format::A8L8:
        return GL_LUMINANCE_ALPHA;
    case forg::Format::A8R8G8B8:
        return GL_BGRA;
    }

    return GL_BGRA;
}

static u32 get_bpp(u32 format)
{
    switch (format)
    {
    case forg::Format::A8L8:
        return 2;
    case forg::Format::A8R8G8B8:
        return 4;
        ;
    }

    return 4;
}

static u32 compute_data_size(u32 width, u32 height, u32 format)
{
    return width * height * get_bpp(format);
}

static u32 get_mipmap_size(u32 mmap, u32 value)
{
    value >>= mmap;

    if (value == 0)
        value = 1;

    return value;
}

// =============================================================================
// ITextureGLImpl
// =============================================================================
ITextureGLImpl::~ITextureGLImpl() {}

ITextureGLImpl* ITextureGLImpl::Create(IRenderDevice* device, u32 width,
                                       u32 height, u32 numLevels, u32 usage,
                                       u32 format, u32 pool)
{
    ITextureGLImpl* timpl = new ITextureGLImpl();

    timpl->m_texture =
        new GLTexture(device, width, height, numLevels, usage, format, pool);
    timpl->m_refCount = 1;

    return timpl;
}

// =============================================================================
// GLTexture
// =============================================================================

GLTexture::GLTexture(IRenderDevice* device, u32 width, u32 height,
                     u32 numLevels, u32 usage, u32 format, u32 pool)
{
    m_device = device;
    m_width = width;
    m_height = height;
    m_levels = numLevels;
    m_usage = usage;
    m_format = format;
    m_pool = pool;

    if (numLevels == 0)
    {
        int num_w = bit_log2(width);
        int num_h = bit_log2(height);

        m_levels = bit_max(num_w, num_h) + 1;
    }

    if (static_cast<GLRenderDevice*>(device)->get_DeviceCaps()->HasCapability(
            GLCaps_PixelBufferObject))
    {
        Create = &GLTexture::CreatePBO;
        Release = &GLTexture::ReleasePBO;
        LockRectInternal = &GLTexture::LockRectPBO;
        UnlockRectInternal = &GLTexture::UnlockRectPBO;
    }
    else
    {
        Create = &GLTexture::CreateSysMem;
        Release = &GLTexture::ReleaseSysMem;
        LockRectInternal = &GLTexture::LockRectSysMem;
        UnlockRectInternal = &GLTexture::UnlockRectSysMem;
    }

    (this->*Create)();
}

GLTexture::~GLTexture(void) { (this->*Release)(); }

void* GLTexture::LockRect(u32 Level, u32 Flags)
{
    return (this->*LockRectInternal)(Level, Flags);
}

int GLTexture::UnlockRect(u32 Level)
{
    return (this->*UnlockRectInternal)(Level);
}

int GLTexture::GetLevelDesc(u32 Level, SurfaceDescription* Description) const
{
    if (Description)
    {
        Description->Width = get_mipmap_size(Level, m_width);
        Description->Height = get_mipmap_size(Level, m_height);
        Description->Format = m_format;
    }

    return FORG_OK;
}

// =============================================================================
// System Memory (GL 1.1)
// =============================================================================

void GLTexture::CreateSysMem()
{
    GLV(glGenTextures(1, &m_id));
    GLV(glBindTexture(GL_TEXTURE_2D, m_id));

    u32 internal_format = get_internal_format(m_format);
    u32 pixel_format = get_pixel_format(m_format);

    u32 data_size = compute_data_size(m_width, m_height, m_format);

    m_data = new byte*[m_levels];

    u32 width = m_width;
    u32 height = m_height;
    for (u32 i = 0; i < m_levels; i++)
    {
        m_data[i] = new byte[data_size];

        GLV(glTexImage2D(GL_TEXTURE_2D,
                         i,               // mipmap level
                         internal_format, // format
                         width, height,   // size
                         0,               // border
                         pixel_format,    // pixel format
                         GL_UNSIGNED_BYTE, m_data[i]));

        if (width > 1)
        {
            width >>= 1;
            data_size >>= 1;
        }

        if (height > 1)
        {
            height >>= 1;
            data_size >>= 1;
        }
    }

    GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
    GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));

    if (m_levels == 1)
    {
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    else
    {
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR));
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLV(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLTexture::ReleaseSysMem()
{
    for (u32 i = 0; i < m_levels; i++)
    {
        delete[] m_data[i];
    }

    delete[] m_data;

    GLV(glDeleteTextures(1, &m_id));
}

void* GLTexture::LockRectSysMem(u32 Level, u32 Flags)
{
    return m_data[Level];
}

int GLTexture::UnlockRectSysMem(u32 Level)
{
    GLV(glBindTexture(GL_TEXTURE_2D, m_id));
    GLV(glTexSubImage2D(GL_TEXTURE_2D,
                        Level, // mipmap level
                        0, 0,  // offset
                        get_mipmap_size(Level, m_width),
                        get_mipmap_size(Level, m_height),
                        get_pixel_format(m_format), // pixel format
                        GL_UNSIGNED_BYTE, m_data[Level]));
    GLV(glBindTexture(GL_TEXTURE_2D, 0));

    return FORG_OK;
}

// =============================================================================
// Pixel Buffer Object (GL 2.1)
// =============================================================================

void GLTexture::CreatePBO()
{
    GLV(glGenTextures(1, &m_id));

    u32 internal_format = get_internal_format(m_format);
    u32 pixel_format = get_pixel_format(m_format);

    u32 data_size = compute_data_size(m_width, m_height, m_format);

    u32 width = m_width;
    u32 height = m_height;

    // m_levels = 1;
    m_buffers = new u32[m_levels];

    GLV(glGenBuffersARB(m_levels, m_buffers));
    GLV(glBindTexture(GL_TEXTURE_2D, m_id));

    // GL_GENERATE_MIPMAP available only if the GL version is 1.4 or greater
    if ((m_usage & Usage::AutoGenerateMipMap) == Usage::AutoGenerateMipMap)
    {
        m_levels = 1;
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }

    for (u32 i = 0; i < m_levels; i++)
    {
        GLV(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_buffers[i]));
        GLV(glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0,
                            GL_STREAM_DRAW_ARB));

        GLV(glTexImage2D(GL_TEXTURE_2D,
                         i,               // mipmap level
                         internal_format, // format
                         width, height,   // size
                         0,               // border
                         pixel_format,    // pixel format
                         GL_UNSIGNED_BYTE, 0));

        if (width > 1)
        {
            width >>= 1;
            data_size >>= 1;
        }

        if (height > 1)
        {
            height >>= 1;
            data_size >>= 1;
        }
    }

    GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
    GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));

    if (m_levels == 1)
    {
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    else
    {
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR));
        GLV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLV(glBindTexture(GL_TEXTURE_2D, 0));
    GLV(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0));
}

void GLTexture::ReleasePBO()
{
    GLV(glDeleteTextures(1, &m_id));
    GLV(glDeleteBuffersARB(m_levels, m_buffers));

    delete[] m_buffers;
}

void* GLTexture::LockRectPBO(u32 Level, u32 Flags)
{
    GLenum glFlags = GL_WRITE_ONLY_ARB;

    if (Flags == LockFlags::ReadOnly)
    {
        glFlags = GL_READ_ONLY_ARB;
    }

    GLV(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_buffers[Level]));
    // u32 data_size = compute_data_size(m_width, m_height, m_format);
    // GLV(glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0,
    // GL_STREAM_DRAW_ARB));
    GLubyte* ptr =
        (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, glFlags);

    return ptr;
}

int GLTexture::UnlockRectPBO(u32 Level)
{
    GLV(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_buffers[Level]));
    GLV(glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB));

    GLV(glBindTexture(GL_TEXTURE_2D, m_id));
    GLV(glTexSubImage2D(GL_TEXTURE_2D, Level, 0, 0,
                        get_mipmap_size(Level, m_width),
                        get_mipmap_size(Level, m_height),
                        get_pixel_format(m_format), GL_UNSIGNED_BYTE, 0));
    GLV(glBindTexture(GL_TEXTURE_2D, 0));

    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    return FORG_OK;
}

} // namespace forg
