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

#ifndef SHARED_ARRAY_HPP_INCLUDED
#define SHARED_ARRAY_HPP_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

namespace forg
{
    namespace core
    {

        template <typename T>
        class shared_array
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
                SharedData(T* pData)
                        : m_pRawData(pData)
                        , m_nCounter(1)
                {}

                ~SharedData()
                {
                    ASSERT(m_nCounter == 0);
                }

                T& operator [](unsigned int nIndex)
                {
                    return m_pRawData[nIndex];
                }

                T* get()
                {
                    return m_pRawData;
                }

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
                        delete [] m_pRawData;
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
                    : m_pSharedData(new SharedData(new T[nSize]))
                    , m_nSize(nSize)
            {}

            shared_array(const shared_array& copy)
                    : m_pSharedData(copy.m_pSharedData)
                    , m_nSize(copy.m_nSize)
            {
                m_pSharedData->addRef();
            }

            ~shared_array()
            {
                _destruct();
            }

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
            this_type& operator =(const shared_array& copy)
            {
                _destruct();

                m_pSharedData = copy.m_pSharedData;
                m_nSize = copy.m_nSize;

                m_pSharedData->addRef();

                return *this;
            }

            element_type& operator [](unsigned int nIndex)
            {
                return m_pSharedData->operator[](nIndex);
            }

            //////////////////////////////////////////////////////////////////////
            // Public methods
            //////////////////////////////////////////////////////////////////////
        public:
            element_type* get()
            {
                return m_pSharedData->get();
            }

            size_type size()
            {
                return m_nSize;
            }

            //////////////////////////////////////////////////////////////////////////
            // Helpers
            //////////////////////////////////////////////////////////////////////////
        private:
            void _destruct()
            {
                if (m_pSharedData->release() == 0)
                {
                    delete m_pSharedData;
                    //m_pSharedData = 0;
                }
            }

        };

    }
}  // namespace

#endif // SHARED_ARRAY_HPP_INCLUDED
