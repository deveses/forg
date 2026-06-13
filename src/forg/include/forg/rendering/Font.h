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

#ifndef _FORG_FONT_H_
#define _FORG_FONT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "rendering/IRenderDevice.h"
#include "rendering/Sprite.h"

namespace forg {

#define MAX_FACENAME     32

struct FontDescription
{
    /// Height, in logical units, of the font's character cell or character.
    int Height;
    /// Width, in logical units, of characters in the font.
    uint Width;
    /// Weight of the font in the range from 0 through 1000.
    uint Weight;
    /// Number of mip levels requested. If this value is zero, a complete mipmap chain is created.
    /// If the value is 1, the texture space is mapped identically to the screen space.
    uint MipLevels;
    /// Set to TRUE for an Italic font.
    bool Italic;
    /// Character set.
    byte CharSet;
    /// Output precision. The output precision defines how closely the output must match the requested font height,
    /// width, character orientation, escapement, pitch, and font type.
    byte OutputPrecision;
    /// Output quality.
    byte Quality;
    /// Pitch and family of the font.
    byte PitchAndFamily;
    /// A null-terminated string or characters that specifies the typeface name of the font.
    /// If FaceName is an empty string, the first font that matches the other specified attributes will be used.
    char FaceName[MAX_FACENAME];

    char FontPath[MAX_PATH];
};

/**
* Encapsulates the textures and resources needed to render a specific font.
* Uses Freetype library (http://www.freetype.org)
* @todo complete implementation, large texture managment
*/
class Font
{
    //////////////////////////////////////////////////////////////////////////
    // Nested
    //////////////////////////////////////////////////////////////////////////
private:
    struct CharMetrics
    {
        /// offset in buffer
        int offset;

        int width;

        /// height
        int rows;

        /// bearing x
        int left;

        /// bearing y
        int top;

        int advance;

        CharMetrics() : offset(-1), width(0), rows(0), left(0), top(0)
        {
        }
    };

    enum
    {
        DTFMT_SINGLELINE = 0,
        DTFMT_CALCRECT,
        DTFMT_CENTER,
		DTFMT_VCENTER,
        DTFMT_LEFT,
        DTFMT_RIGHT,
        DTFMT_TOP,
        DTFMT_BOTTOM,
        DTFMT_NOCLIP,
        DTFMT_WORDBREAK
    };

    //////////////////////////////////////////////////////////////////////////
    // 'structors
    //////////////////////////////////////////////////////////////////////////
public:
    Font();
    FORG_API ~Font();

private:
    IRenderDevice* m_device;

private:
    Sprite* m_sprite;
    LPTEXTURE m_texture;
    uint m_tex_width;
    uint m_tex_height;
    char* m_bitmap;
    uint m_size;

    uint m_CharWidth;
    uint m_CharHeight;

    CharMetrics m_metrics[256];

public:
    FORG_API static Font* CreateIndirect(IRenderDevice* device, FontDescription* fontDesc);

    /// Draws formatted text
    /**
    * Draws formatted text
    * @param pString
    * Pointer to a string to draw.If the Count parameter is -1, the string must be null-terminated.
    * @param count
    * Specifies the number of characters in the string. If Count is -1, then the pString parameter is assumed to be a pointer
    * to a null-terminated string and ID3DXFont::DrawText computes the character count automatically.
    * @param color
    * Color of the text.
    */
    FORG_API int DrawText2(
        //Sprite* pSprite
        LPCTSTR pString,
        int count,
        Rectangle*
        pRect,
        uint format,
        Color4b color );
};

}


#endif //_FORG_FONT_H_
