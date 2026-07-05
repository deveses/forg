/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

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
