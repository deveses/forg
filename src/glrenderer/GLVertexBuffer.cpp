#include "GLVertexBuffer.h"

#include "GLFunc.h"


namespace forg {

GLVertexBuffer::GLVertexBuffer(GLRenderDevice* device, uint sizeOfBufferInBytes, uint usage, uint pool)
: m_device(0)
, m_caps(0)
, m_created(false)
{
	m_caps = device->get_DeviceCaps();

	m_usage = usage;
	m_pool = pool;

	GLV(glGenBuffersARB(1, &m_buffer_id));
	GLV(glBindBufferARB(GL_ARRAY_BUFFER, m_buffer_id));
	GLV(glBufferDataARB(GL_ARRAY_BUFFER, sizeOfBufferInBytes, NULL, GL_STATIC_DRAW));

}

GLVertexBuffer::~GLVertexBuffer(void)
{
	GLV(glDeleteBuffersARB(1, &m_buffer_id));
}

int GLVertexBuffer::Lock(
		 uint offsetToLock,
		 uint sizeToLock,
		 void ** ppbData,
		 uint flags
		 )
{
    GLenum access = GL_READ_WRITE;

    if (flags & LockFlags::ReadOnly)
        access = GL_READ_ONLY;

    GLV(glBindBufferARB(GL_ARRAY_BUFFER, m_buffer_id));

	GLV_RETURNV( (*ppbData = glMapBufferARB(GL_ARRAY_BUFFER, access)), 1 );

    GLV(glBindBufferARB(GL_ARRAY_BUFFER, 0));

	return 0;
}

int GLVertexBuffer::Unlock()
{
    GLV(glBindBufferARB(GL_ARRAY_BUFFER, m_buffer_id));

	GLV(glUnmapBufferARB(GL_ARRAY_BUFFER)); //if false device is lost?

    GLV(glBindBufferARB(GL_ARRAY_BUFFER, 0));

	return 0;
}

}
