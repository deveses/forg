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

#ifndef _FORG_CORE_AUTOPTR_H_
#define _FORG_CORE_AUTOPTR_H_

#if _MSC_VER > 1000
#pragma once
#endif


#include "base.h"

/*
	smart pointers.

	to do:
	reference counting,
	reference linking (smart pointers as double-linked list
	with pointers to previous and next smart pointer,
	deleting object if both sides are null),
*/

namespace forg { namespace core {

template <typename T>
struct PointerOwner
{
    void acquire(T* p)
    {
    }

    void release(T* p)
    {
    }

    void destroy(T* p)
    {
        delete p;
    }
};

template <typename T>
struct RefCountOwner
{
    void acquire(T* p)
    {
        if (p)
        {
            //p->AddRef();
        }
    }

    void release(T* p)
    {
        if (p)
        {
            //p->Release();
        }
    }

    void destroy(T* p)
    {
        if (p)
        {
            p->Release();
        }
    }
};

template<typename T>
struct auto_ptr_ref
{
    T* m_ptr;

    explicit
    auto_ptr_ref(T* p): m_ptr(p) 
    { 
    }
};


/// auto_ptr template
/**
* Basic smart pointer with destructive copy.
* @author eses
* @version 1.0
* @date 10-2005
* @todo
* @bug
* @warning
*/
template<typename T, typename OwnPolicy = PointerOwner<T>>
class auto_ptr
{
	private:
		T* m_ptr;

        OwnPolicy own_policy;

		typedef auto_ptr<T, OwnPolicy> this_type;

	public:

		typedef T element_type;

		//////////////////////////////////////////////////////////////////////////
		// Construction / Destruction
		//////////////////////////////////////////////////////////////////////////
		auto_ptr(): m_ptr(0) // never throws
		{
		}

		//destructive copy
		auto_ptr(auto_ptr& aptr)
			: m_ptr( aptr.release() )
		{
		}

        template<typename U>
		auto_ptr(auto_ptr<U>& aptr)
			: m_ptr( aptr.release() )
		{
            own_policy.acquire(ptr);
		}


		explicit auto_ptr(element_type* p): m_ptr(p) // never throws
		{
		}

		~auto_ptr() // never throws
		{
			//boost::checked_delete(ptr);
            own_policy.destroy(m_ptr);
			m_ptr = 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// Operators
		//////////////////////////////////////////////////////////////////////////

		//destructive copy
		auto_ptr& operator = (auto_ptr& b)
		{
			reset(b.release());
			return (*this);
		}

        template<typename U>
		auto_ptr& operator = (auto_ptr<U>& b)
		{
			reset(b.release());
			return (*this);
		}

        // assigment implies an implicit conversion
        /*
		auto_ptr& operator = (T* ptr)
		{
			Reset(ptr);
			return (*this);
		}
		*/

		element_type& operator*() const // never throws
		{
			return *m_ptr;
		}

		element_type* operator->() const // never throws
		{
			return m_ptr;
		}

		// implicit conversion to "bool"
		// commented because unwanted conversion to integer in some situations
		//operator bool () const
		//{
		//	return ptr != 0;
		//}

		bool operator !() const // never throws
		{
			return m_ptr == 0;
		}

		//almost never applies because its templated version
		//except "if (sp == 0)" for example
		inline friend bool operator ==(const auto_ptr& lhs, const T* rhs)
		{
			return lhs.m_ptr == rhs;
		}

		inline friend bool operator ==(const T* lhs, const auto_ptr& rhs)
		{
			return lhs == rhs->m_ptr;
		}

		inline friend bool operator !=(const auto_ptr& lhs, const T* rhs)
		{
			return lhs.m_ptr != rhs;
		}

		inline friend bool operator !=(const T* lhs, const auto_ptr& rhs)
		{
			return lhs != rhs->m_ptr;
		}

		//templated versions

		template <class U>
		inline friend bool operator ==(const auto_ptr& lhs, const U* rhs)
		{
			return lhs.m_ptr == rhs;
		}

		template <class U>
		inline friend bool operator ==(const U* lhs, const auto_ptr& rhs)
		{
			return lhs == rhs->m_ptr;
		}

		template <class U>
		inline friend bool operator !=(const auto_ptr& lhs, const U* rhs)
		{
			return lhs.m_ptr != rhs;
		}

		template <class U>
		inline friend bool operator !=(const U* lhs, const auto_ptr& rhs)
		{
			return lhs != rhs->ptr;
		}

		//operators for distinct smart pointer types

		template <class U>
		bool operator ==(const auto_ptr<U>& rhs)
		{
			return m_ptr == rhs.m_ptr;
		}

		template <class U>
		bool operator !=(const auto_ptr<U>& rhs)
		{
			return m_ptr != rhs.m_ptr;
		}

		//////////////////////////////////////////////////////////////////////////
		// Public Methods
		//////////////////////////////////////////////////////////////////////////

        bool is_null() const
        {
            return (m_ptr == 0);
        }

		element_type* release()
		{
			T* tmp = m_ptr;

			m_ptr = 0;

            own_policy.release(tmp);

			return (tmp);
		}

		void reset(element_type* p = 0) // never throws
		{
			this_type(p).swap(*this);
		}

		element_type* get() const // never throws
		{
			return m_ptr;
		}

		void swap(auto_ptr& b) // never throws
		{
			T* tmp = b.m_ptr;
			b.m_ptr = m_ptr;
			m_ptr = tmp;
		}

		// automatic conversion

	    auto_ptr(auto_ptr_ref<element_type> ref)
        : m_ptr(ref.m_ptr) { }

        auto_ptr& operator = (auto_ptr_ref<element_type> ref)
        {
            if (ref.m_ptr != this->get())
            {
                own_policy.destroy(m_ptr);

                m_ptr = ref.m_ptr;
            }

            return *this;
        }

        template<typename U>
        operator auto_ptr_ref<U>()
        {
            return auto_ptr_ref<U>(this->release());
        }

        template<typename U>
        operator auto_ptr<U>()
        {
            return auto_ptr<U>(this->release());
        }
	};

	template<class T> inline void swap(auto_ptr<T>& a, auto_ptr<T>& b) // never throws
	{
		a.Swap(b);
	}

	// get_pointer(p) is a generic way to say p.get()

	template<class T> inline T* get_pointer(const auto_ptr<T>& p)
	{
		return p.Get();
	}


}}

#endif //_FORG_CORE_AUTOPTR_H_

