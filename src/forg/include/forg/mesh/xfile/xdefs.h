/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2006  Slawomir Strumecki

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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef XFILE_XDEFS_INCLUDED
#define XFILE_XDEFS_INCLUDED

#include <base.h>

#include <string>
#include <vector>
#include <list>

namespace forg { namespace xfile {

    namespace EToken {
	    enum TYPE {
	        TOKEN_UNKNOWN       =   0,

		    TOKEN_NAME          =   1,      //alpha-num, with '_'
		    TOKEN_STRING        =   2,
		    TOKEN_INTEGER       =   3,
		    TOKEN_GUID          =   5,      //<XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX> X: [0-9a-f]
		    TOKEN_INTEGER_LIST  =   6,
		    TOKEN_FLOAT_LIST    =   7,

		    TOKEN_OBRACE        =   10,     //{
		    TOKEN_CBRACE        =   11,     //}
		    TOKEN_OPAREN        =   12,     //(
		    TOKEN_CPAREN        =   13,     //)
		    TOKEN_OBRACKET      =   14,     //[
		    TOKEN_CBRACKET      =   15,     //]
		    TOKEN_OANGLE        =   16,     //<
		    TOKEN_CANGLE        =   17,     //>
		    TOKEN_DOT           =   18,     //.
		    TOKEN_COMMA         =   19,     //,
		    TOKEN_SEMICOLON     =   20,     //;

		    TOKEN_TEMPLATE      =   31,     //template
		    TOKEN_WORD          =   40,     //WORD
		    TOKEN_DWORD         =   41,     //DWORD
		    TOKEN_FLOAT         =   42,     //FLOAT
		    TOKEN_DOUBLE        =   43,     //DOUBLE
		    TOKEN_CHAR          =   44,     //CHAR
		    TOKEN_UCHAR         =   45,     //UCHAR
		    TOKEN_SWORD         =   46,     //SWORD
		    TOKEN_SDWORD        =   47,     //SDWORD
		    TOKEN_VOID          =   48,     //VOID
		    TOKEN_LPSTR         =   49,     //LPSTR
		    TOKEN_UNICODE       =   50,     //UNICODE
		    TOKEN_CSTRING       =   51,     //CSTRING
		    TOKEN_ARRAY         =   52      //array
	    };
    };

    namespace ETemplatePrimitiveType {
        enum TYPE
        {
            Unknown = 0,
            Word = 40,
            Dword,
            Float,
            Double,
            Char,
            UChar,
            SWord,
            SDword,
            Lpstr = 49,
            Unicode,
            CString
        };
    };

    template <class _Type>
    class PrimitiveArray
    {
    public:
        typedef _Type element_type;

        typedef unsigned int size_type;

        typedef element_type& reference;

        typedef const element_type& const_reference;

        typedef PrimitiveArray<_Type> this_type;

        typedef size_type difference_type;

        class iterator
        {
        public:
            iterator()
            {
                m_ptr = 0;
            }

            iterator(_Type* _ptr)
            {
                m_ptr = _ptr;
            }

            reference operator*() const
            {	// return (reference to) designated object
                return *m_ptr;
            }

            bool operator==(const iterator& _Right) const
            {	// test for iterator equality

                return (m_ptr == _Right.m_ptr);
            }

            bool operator!=(const iterator& _Right) const
            {	// test for iterator inequality
                return (m_ptr != _Right.m_ptr);
            }

            difference_type operator-(const iterator& _Right) const
            {	// return difference of iterators
                return (m_ptr - _Right.m_ptr);
            }

            iterator& operator++()
            {	// preincrement
                m_ptr++;

                return (*this);
            }

            iterator operator++(int)
            {	// postincrement
                iterator _Tmp = *this;
                ++*this;
                return (_Tmp);
            }

            iterator& operator+=(difference_type _Off)
            {	// increment by integer
                m_ptr += _Off;
                return (*this);
            }

        private:
            _Type* m_ptr;
        };

        class const_iterator
        {
        public:
            const_iterator()
            {
                m_ptr = 0;
            }

            const_iterator(_Type* _ptr)
            {
                m_ptr = _ptr;
            }

            const_reference operator*() const
            {	// return (reference to) designated object
                return *m_ptr;
            }

            bool operator==(const const_iterator& _Right) const
            {	// test for iterator equality

                return (m_ptr == _Right.m_ptr);
            }

            bool operator!=(const const_iterator& _Right) const
            {	// test for iterator inequality
                return (m_ptr != _Right.m_ptr);
            }

            difference_type operator-(const const_iterator& _Right) const
            {	// return difference of iterators
                return (m_ptr - _Right.m_ptr);
            }

            const_iterator& operator++()
            {	// preincrement
                m_ptr++;

                return (*this);
            }

            const_iterator operator++(int)
            {	// postincrement
                const_iterator _Tmp = *this;
                ++*this;
                return (_Tmp);
            }

            const_iterator& operator+=(difference_type _Off)
            {	// increment by integer
                m_ptr += _Off;
                return (*this);
            }

        private:
            _Type* m_ptr;
        };

    public:
        PrimitiveArray()
            : m_data(0)
            , m_size(0)
            , m_capacity(0)
        {
        }

        PrimitiveArray(size_type _count)
            : m_data(0)
            , m_size(0)
            , m_capacity(0)
        {
            reserve(_count);
            m_size = _count;
        }


        ~PrimitiveArray()
        {
            destroy();
        }

    private:
        element_type*  m_data;
        size_type m_size;
        size_type m_capacity;


    public:
        size_type size() const
        {
            return m_size;
        }

        void reserve(size_type _count)
        {
            // it assure non zero size
            if (_count > m_capacity)
            {
                _Type* arr = new _Type[_count];

                // copy old data
                if (m_size > 0)
                {
                    memcpy(arr, m_data, m_size*sizeof(_Type));
                }

                delete [] m_data;

                m_data = arr;
                m_capacity = _count;
            }
        }

        _Type* release()
        {
            _Type* arr = m_data;

            m_data = 0;
            m_size = m_capacity = 0;

            return arr;
        }

        reference front()
        {
            return m_data[0];
        }

        reference back()
        {
            return m_data[m_size-1];
        }

        iterator begin()
        {
            return iterator(m_data);
        }

        const_iterator begin() const
        {
            return const_iterator(m_data);
        }

        iterator end()
        {
            return iterator(m_data + m_size);
        }

        const_iterator end() const
        {
            return const_iterator(m_data + m_size);
        }

        _Type* get() { return m_data; }

        const _Type* get() const { return m_data; }

        void clear()
        {
            m_size = 0;
        }

        void swap(PrimitiveArray& _right)
        {
            if (&_right != this)
            {
                size_type s = _right.m_size;
                size_type c = _right.m_capacity;
                element_type* arr = _right.m_data;

                _right.m_size = m_size;
                _right.m_capacity = m_capacity;
                _right.m_data = m_data;

                m_size = s;
                m_capacity = c;
                m_data = arr;
            }
        }

        void push_back(const _Type& _value)
        {
            if (m_size == m_capacity)
            {
                reserve((m_size<<1) + 1);
            }

            m_data[m_size] = _value;
            m_size++;
        }

        void copy_to(PrimitiveArray& _right, iterator _begin,iterator _end, iterator _where)
        {
            size_type offset = _begin - begin();
            size_type count = _end - _begin;
            size_type where = _where - _right.begin();

            // if there is something to copy
            if (offset < m_size)
            {
                // check if we have enough elements
                if (offset + count > m_size)
                {
                    count = m_size - offset;
                }

                _right.clear();
                _right.reserve(count + where);

                memcpy(_right.m_data + where, m_data + offset, count * sizeof(_Type));

                if (_right.m_size < count + where)
                {
                    _right.m_size = count + where;
                }
            }
        }

        private:
        void destroy()
        {
            delete [] (release());
        }

    };



    typedef unsigned short WORD;
	typedef unsigned long DWORD;
	typedef unsigned char UCHAR;
	typedef UCHAR BYTE;
	typedef long LONG;

	typedef std::string xstring;

	typedef PrimitiveArray<DWORD> IntegerList;
    typedef IntegerList::iterator IntegerListI;

    typedef PrimitiveArray<float> FloatList;
    typedef FloatList::iterator FloatListI;

    typedef std::list<float> DoubleList;
	typedef std::list<xstring> StringList;

    const char* GetPrimitiveTypeName(int type);

	struct xguid {
		DWORD	Data1;		//data field 1
		WORD	Data2;		//data field 2
		WORD	Data3;		//data field 3
		BYTE	Data4[8];	//data field 4

        const static xguid Empty;

        xstring ToString() const;

        bool IsEmpty() const
        {
            return ! (Data1 || Data2 || Data3 ||
                (Data4[0]|Data4[1]|Data4[2]|Data4[3]|Data4[4]|Data4[5]|Data4[6]|Data4[7])
                );
        }

	};

    bool operator < (const xguid& left, const xguid& right);

    bool operator == (const xguid& left, const xguid& right);

}}

#endif
