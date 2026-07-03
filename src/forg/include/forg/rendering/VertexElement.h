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

#ifndef FORG_RENDERING_VERTEXELEMENT_H
#define FORG_RENDERING_VERTEXELEMENT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <cstddef>
#include <type_traits>

#include "base.h"

namespace forg {

// #define VertexDeclarationEnd() {0xFF, 0, DeclarationType_Unused, 0, 0}

/// VertexElement structure
/**
 * VertexElement
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
struct FORG_API VertexElement
{
    u16 Stream;
    u16 Offset;
    byte Type;
    byte Usage;
    byte UsageIndex;

    static const VertexElement VertexDeclarationEnd;

    // IVertexElement(u16 stream, u16 offset, DeclarationType declType,
    // DeclarationUsage declUsage, byte usageIndex);

    bool operator!=(const VertexElement elem) const;
    static u32 GetTypeSize(byte type);
    static u32 GetTypeCount(byte type);
};

static_assert(std::is_standard_layout_v<VertexElement>);
static_assert(sizeof(VertexElement) == 8);
static_assert(alignof(VertexElement) == alignof(u16));
static_assert(offsetof(VertexElement, Offset) == sizeof(u16));
static_assert(offsetof(VertexElement, Type) == sizeof(u16) * 2);
static_assert(offsetof(VertexElement, Usage) == sizeof(u16) * 2 + sizeof(byte));
static_assert(offsetof(VertexElement, UsageIndex) ==
              sizeof(u16) * 2 + sizeof(byte) * 2);

} // namespace forg

#endif // FORG_RENDERING_VERTEXELEMENT_H
