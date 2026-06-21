/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2024  Slawomir Strumecki

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

// Metal headers must precede forg's base.h: base.h defines null/IN/OUT macros
// that break the system headers if they are seen first. (MRC build: no ARC.)
#import <Metal/Metal.h>

#include "MetalBuffers.h"

namespace forg
{

/////////////////////////////////////////////////////////////////////////////////////
// MetalVertexBuffer
/////////////////////////////////////////////////////////////////////////////////////
MetalVertexBuffer::MetalVertexBuffer() : m_buffer(0), m_length(0) {}

MetalVertexBuffer::~MetalVertexBuffer()
{
    if (m_buffer)
    {
        [(id<MTLBuffer>)m_buffer release];
        m_buffer = 0;
    }
}

int MetalVertexBuffer::Create(void* mtlDevice, uint length)
{
    id<MTLDevice> dev = (id<MTLDevice>)mtlDevice;
    id<MTLBuffer> buf = [dev newBufferWithLength:length
                                         options:MTLResourceStorageModeShared];

    m_buffer =
        (void*)buf; // newBuffer... is +1 owned; released in the destructor
    m_length = length;

    return (buf != nil) ? FORG_OK : FORG_INVALID_CALL;
}

int MetalVertexBuffer::Lock(uint offsetToLock, uint /*sizeToLock*/,
                            void** ppbData, uint /*flags*/)
{
    if (m_buffer == 0 || ppbData == 0)
        return FORG_INVALID_CALL;

    *ppbData = (char*)[(id<MTLBuffer>)m_buffer contents] + offsetToLock;
    return FORG_OK;
}

int MetalVertexBuffer::Unlock()
{
    // Shared storage: writes through the contents pointer are already visible.
    return FORG_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
// MetalIndexBuffer
/////////////////////////////////////////////////////////////////////////////////////
MetalIndexBuffer::MetalIndexBuffer() : m_buffer(0), m_length(0), m_short(true)
{
}

MetalIndexBuffer::~MetalIndexBuffer()
{
    if (m_buffer)
    {
        [(id<MTLBuffer>)m_buffer release];
        m_buffer = 0;
    }
}

int MetalIndexBuffer::Create(void* mtlDevice, uint length,
                             bool sixteenBitIndices)
{
    id<MTLDevice> dev = (id<MTLDevice>)mtlDevice;
    id<MTLBuffer> buf = [dev newBufferWithLength:length
                                         options:MTLResourceStorageModeShared];

    m_buffer = (void*)buf;
    m_length = length;
    m_short = sixteenBitIndices;

    return (buf != nil) ? FORG_OK : FORG_INVALID_CALL;
}

int MetalIndexBuffer::Lock(uint offsetToLock, uint /*sizeToLock*/,
                           void** ppbData, uint /*flags*/)
{
    if (m_buffer == 0 || ppbData == 0)
        return FORG_INVALID_CALL;

    *ppbData = (char*)[(id<MTLBuffer>)m_buffer contents] + offsetToLock;
    return FORG_OK;
}

int MetalIndexBuffer::Unlock() { return FORG_OK; }

} // namespace forg
