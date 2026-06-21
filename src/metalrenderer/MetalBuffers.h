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

#ifndef _FORG_METAL_BUFFERS_H_
#define _FORG_METAL_BUFFERS_H_

#include "base.h"
#include "rendering/IIndexBuffer.h"
#include "rendering/IVertexBuffer.h"

namespace forg
{

// Both buffers wrap a single MTLBuffer (storageModeShared) so the CPU pointer
// returned by Lock() points straight at GPU-visible memory; Unlock() is a
// no-op. The MTLBuffer handle is stored as void* so this header stays
// includable from plain C++ (the factory in MetalRenderer.cpp never sees an
// ObjC type).

class MetalVertexBuffer : public IVertexBuffer
{
  public:
    MetalVertexBuffer();
    virtual ~MetalVertexBuffer();

    // mtlDevice is an id<MTLDevice>; allocates a `length`-byte shared buffer.
    int Create(void* mtlDevice, uint length);

    void* GetMTLBuffer() const { return m_buffer; }
    uint GetLength() const { return m_length; }

    virtual int Lock(uint offsetToLock, uint sizeToLock, void** ppbData,
                     uint flags);
    virtual int Unlock();

  private:
    void* m_buffer; // id<MTLBuffer>, retained
    uint m_length;
};

class MetalIndexBuffer : public IIndexBuffer
{
  public:
    MetalIndexBuffer();
    virtual ~MetalIndexBuffer();

    int Create(void* mtlDevice, uint length, bool sixteenBitIndices);

    void* GetMTLBuffer() const { return m_buffer; }
    uint GetLength() const { return m_length; }
    bool IsIndexShort() const { return m_short; }

    virtual int Lock(uint offsetToLock, uint sizeToLock, void** ppbData,
                     uint flags);
    virtual int Unlock();

  private:
    void* m_buffer; // id<MTLBuffer>, retained
    uint m_length;
    bool m_short;
};

} // namespace forg

#endif //_FORG_METAL_BUFFERS_H_
