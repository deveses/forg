#include "GLIndexBuffer.h"
#include "GLFunc.h"
#include "GLRenderDevice.h"

namespace forg {

GLIndexBuffer::GLIndexBuffer(GLRenderDevice* device, uint sizeOfBufferInBytes, uint usage, uint pool, bool sixteenBitIndices)
: m_device(0)
, m_caps(0)
{
	m_caps = device->get_DeviceCaps();

	m_size = sizeOfBufferInBytes;
	m_usage = usage;
	m_pool = pool;
    m_sixteen = sixteenBitIndices;

	GLV(glGenBuffersARB(1, &m_buffer_id));
	GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id));
	GLV(glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeOfBufferInBytes, NULL, GL_STATIC_DRAW));
}

GLIndexBuffer::~GLIndexBuffer(void)
{
	GLV(glDeleteBuffersARB(1, &m_buffer_id));
}


int GLIndexBuffer::Lock(
		 uint offsetToLock,
		 uint sizeToLock,
		 void ** ppbData,
		 uint flags
		 )
{
    GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id));

	GLV_RETURNV( (*ppbData = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)), 1);

    GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0));

	return 0;
}

int GLIndexBuffer::Unlock()
{
    GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id));

	GLV(glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER));

    GLV(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0));

	return 0;
}

}
