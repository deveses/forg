// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/base.h"

namespace forg::os {

class FORG_API File
{
  public:
    File();
    ~File();

    bool Open(const char* _filename);
    void Close();

    bool GetSize(u32& _out_size);

    u32 Read(void* _buffer, u32 _size);

    template <class T> bool ReadT(T& _v)
    {
        return (Read(&_v, sizeof(T)) == sizeof(T));
    }

  private:
    void* m_handle = nullptr;
    u32 m_size = 0;
};

} // namespace forg::os
