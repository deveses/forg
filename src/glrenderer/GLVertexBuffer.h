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

#ifndef _GL_VERTEX_BUFFER_H_
#define _GL_VERTEX_BUFFER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <forg.h>
#include "GLRenderDevice.h"

namespace forg {

	class GLVertexBuffer : public IVertexBuffer
{
public:
	GLVertexBuffer(GLRenderDevice* device, uint sizeOfBufferInBytes, uint usage, uint pool);
	~GLVertexBuffer(void);

public:
	int Lock(
		uint offsetToLock,
		uint sizeToLock,
		void ** ppbData,
		uint flags
		);

	int Unlock();

	uint m_buffer_id;
private:
	GLRenderDevice* m_device;
	const GLDeviceCaps* m_caps;

	uint m_usage;
	uint m_pool;
	uint m_type;
	bool m_created;

};

}

#endif  // _GL_VERTEX_BUFFER_H_
