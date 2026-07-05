// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "base.h"

namespace forg::os {

class ILibrary
{
  public:
    virtual ~ILibrary() {};

    virtual int Open(LPCTSTR szName, int nFlags) = 0;
    virtual void Close() = 0;

    virtual void* Address(LPCSTR szName) = 0;
};

} // namespace forg::os
