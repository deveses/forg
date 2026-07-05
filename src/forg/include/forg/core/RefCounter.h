// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "base.h"

#include <atomic>

namespace forg::core {

/// BitArray class
/**
 * BitArray
 * @author eses
 * @version 1.0
 * @date 07-2005
 * @todo
 * @bug
 * @warning
 */
class FORG_API RefCounter
{
    std::atomic<int> m_refCount{1};

  public:
    RefCounter() = default;
    virtual ~RefCounter() = default;

  public:
    /// Brief description
    /**
     * Detailed description
     * @return Number of references.
     */
    virtual int AddRef(void);

    /// Brief description
    /**
     * Detailed description
     * @return Number of references.
     */
    virtual int Release(void);
};

} // namespace forg::core
