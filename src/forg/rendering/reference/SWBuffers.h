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

#ifndef _FORG_REFERENCE_SWBUFFERS_H_
#define _FORG_REFERENCE_SWBUFFERS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/ITexture.h"
#include "rendering/IVertexBuffer.h"
#include "rendering/IIndexBuffer.h"

namespace forg { namespace rendering { namespace reference {

    /////////////////////////////////////////////////////////////////////////////////////
    // SWTexture
    /////////////////////////////////////////////////////////////////////////////////////

    class FORG_API SWTexture : public ITexture
    {
        int m_refCount;
        char* m_data;
        uint m_Width;
        uint m_Height;
        uint m_Levels;
        uint m_Usage;
        uint m_Format;
        uint m_Pool;
    public:
        SWTexture();
        virtual ~SWTexture();

        int Create(uint Width, uint Height, uint Levels, uint Usage, uint Format, uint Pool);

        uint Sample(float u, float v);

    // ITexture implementation
    public:
        uint GetLevelCount();

        int GetLevelDesc(uint Level, SurfaceDescription* Description) const;

        void* LockRect(uint Level, uint Flags);

        int UnlockRect(uint Level);
    };


    /////////////////////////////////////////////////////////////////////////////////////
    // SWVertexBuffer
    /////////////////////////////////////////////////////////////////////////////////////
    class FORG_API SWVertexBuffer : public IVertexBuffer
    {
        char* m_data;

        uint m_length;
        uint m_usage;
        uint m_pool;

    public:
        SWVertexBuffer();
        virtual ~SWVertexBuffer();

        char* GetData() { return m_data; }

        int Create(uint length, uint usage, uint pool);

    public:
	    virtual int Lock(uint offsetToLock, uint sizeToLock, void ** ppbData, uint flags);

	    virtual int Unlock();
    };



    /////////////////////////////////////////////////////////////////////////////////////
    // SWIndexBuffer
    /////////////////////////////////////////////////////////////////////////////////////
    class FORG_API SWIndexBuffer : public IIndexBuffer
    {
        char* m_data;

        uint m_length;
        uint m_usage;
        bool m_short;
        uint m_pool;

	public:
        SWIndexBuffer();
		virtual ~SWIndexBuffer();

        char* GetData() { return m_data; }

        uint GetLength() const { return m_length; }

        int GetIndexSize() const { return (m_short ? 2 : 4); }

        bool IsIndexShort() const { return m_short; }

        int Create(uint length, uint usage, bool sixteenBitIndices, uint pool);

	public:
		virtual int Lock(uint offsetToLock, uint sizeToLock, void ** ppbData, uint flags);

		virtual int Unlock();
    };
}}} // forg::rendering::reference

#endif  //_FORG_REFERENCE_SWBUFFERS_H_