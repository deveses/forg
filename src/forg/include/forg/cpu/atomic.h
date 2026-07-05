// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include <atomic>

namespace forg::cpu {

inline int AtomicIncrement(std::atomic<int>* value) noexcept
{
    return ++(*value);
}
inline int AtomicDecrement(std::atomic<int>* value) noexcept
{
    return --(*value);
}

} // namespace forg::cpu
