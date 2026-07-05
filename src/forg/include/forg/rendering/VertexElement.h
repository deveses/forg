// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
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
