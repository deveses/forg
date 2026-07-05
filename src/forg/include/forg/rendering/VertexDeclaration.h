// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "rendering/VertexElement.h"

namespace forg {

/// VertexDeclaration class
/**
 * VertexDeclaration
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class FORG_API VertexDeclaration
{
  public:
    VertexDeclaration(const VertexElement* pDecl);
    virtual ~VertexDeclaration(void);

  private:
    VertexElement elements[256];
    u32 m_nElementsCount;
    u32 m_nVertexSize;

  public:
    const VertexElement* GetDeclaration() const;
    u32 GetElementsCount() const;
    u32 GetVertexSize() const;
};

using LPVERTEXDECLARATION = VertexDeclaration*;

} // namespace forg
