// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include "forg/debug/dbg.h"

#include <cstdlib>

namespace forg::core {

template <typename T> class shared_array
{
    //////////////////////////////////////////////////////////////////////
    // Nested
    //////////////////////////////////////////////////////////////////////
  public:
    typedef T element_type;

    typedef unsigned int size_type;

  private:
    typedef shared_array<T> this_type;

    class SharedData
    {
        T* m_pRawData;
        long m_nCounter;

      public:
        SharedData(T* pData) : m_pRawData(pData), m_nCounter(1) {}

        ~SharedData() { ASSERT(m_nCounter == 0); }

        T& operator[](unsigned int nIndex) { return m_pRawData[nIndex]; }

        T* get() { return m_pRawData; }

        long addRef()
        {
            ++m_nCounter;

            return m_nCounter;
        }

        long release()
        {
            ASSERT(m_nCounter > 0);
            --m_nCounter;

            if (m_nCounter == 0)
            {
                delete[] m_pRawData;
                m_pRawData = 0;
            }

            return m_nCounter;
        }
    };

    //////////////////////////////////////////////////////////////////////
    // 'structors
    //////////////////////////////////////////////////////////////////////
  public:
    shared_array(size_type nSize = 0)
        : m_pSharedData(new SharedData(new T[nSize])), m_nSize(nSize)
    {
    }

    shared_array(const shared_array& copy)
        : m_pSharedData(copy.m_pSharedData), m_nSize(copy.m_nSize)
    {
        m_pSharedData->addRef();
    }

    ~shared_array() { _destruct(); }

    //////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////
  private:
    SharedData* m_pSharedData;
    size_type m_nSize;

    //////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////
  public:
    this_type& operator=(const shared_array& copy)
    {
        _destruct();

        m_pSharedData = copy.m_pSharedData;
        m_nSize = copy.m_nSize;

        m_pSharedData->addRef();

        return *this;
    }

    element_type& operator[](unsigned int nIndex)
    {
        return m_pSharedData->operator[](nIndex);
    }

    //////////////////////////////////////////////////////////////////////
    // Public methods
    //////////////////////////////////////////////////////////////////////
  public:
    element_type* get() { return m_pSharedData->get(); }

    size_type size() { return m_nSize; }

    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
  private:
    void _destruct()
    {
        if (m_pSharedData->release() == 0)
        {
            delete m_pSharedData;
            // m_pSharedData = 0;
        }
    }
};

} // namespace forg::core
