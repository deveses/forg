/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2008  Slawomir Strumecki

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

#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/IRenderDevice.h"

namespace forg {

// TODO: move somewhere
struct Rectangle
{
    int left;
    int top;
    int right;
    int bottom;

    void Offset(int dx, int dy) { left += dx; right += dx; top += dy; bottom += dy; }
    int Height() const { return bottom - top; }
    int Width() const { return right - left; }
};

struct Point
{
    int x;
    int y;
};

/// Sprite
/**
* Sprite
* @author eses
* @version 1.0
* @date 04-2008
* @todo
* @bug
* @warning
*/
class Sprite
{
    // 'structors
    private:
    Sprite();

    public:
    FORG_API static Sprite* CreateSprite(IRenderDevice* device);

    FORG_API ~Sprite();

    // Attributes
    private:
    IRenderDevice* m_device;
    Matrix4 m_transform;
    uint m_Flags;

    // Public methods
    public:
    /// Prepares a device for drawing sprites.
    FORG_API int Begin(uint Flags);

    /// Adds a sprite to the list of batched sprites.
    FORG_API int Flush();

    /// Calls Flush and restores the device state to how it was before Begin was called.
    FORG_API int End();

    /// Adds a sprite to the list of batched sprites.
    FORG_API int Draw(
        ITexture* srcTexture,
        const Rectangle* srcRectangle,
        const Vector3* center,
        const Vector3* position,
        Color4b color
        );

    FORG_API int SetTransform(const Matrix4* transform);

    FORG_API int SetWorldViewLH(const Matrix4* world, const Matrix4* view);

    FORG_API int SetWorldViewRH(const Matrix4* world, const Matrix4* view);

};

}

#endif // SPRITE_H_INCLUDED
