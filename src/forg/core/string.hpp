#ifndef STRING_HPP_INCLUDED
#define STRING_HPP_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

namespace forg { namespace core {

    template <class T>
    class basic_string
    {
    public:
        typedef unsigned int size_type;

        enum {npos = 0xffffffff};

    private:
        T* m_buffer;
        size_type m_length;
        size_type m_capacity;

    public:
        basic_string()
        {
            m_buffer = 0;
            m_length = 0;
            m_capacity = 0;
        }

        basic_string(const T* _str)
        {
            size_type l = slen(_str);

            reserve(l+1);

            memcpy(m_buffer, _str, l*sizeof(T));

            m_length = l;
        }

        basic_string(const basic_string& _str)
        {
            m_buffer = allocate(_str.m_capacity);
            m_length = _str.m_length;
            m_capacity = _str.m_capacity;

            memcpy(m_buffer, _str.m_buffer, m_length*sizeof(T));
        }

        const char& operator[] ( size_type pos ) const
        {
            return m_buffer[pos];
        }
      
        char& operator[] ( size_type pos )
        {
            return m_buffer[pos];
        }

        basic_string& operator = (const basic_string& rhs)
        {
            return *this;
        }

        basic_string operator + (const basic_string& rhs)
        {
            basic_string sum(*this);

            sum += rhs;

            return sum;
        }

        basic_string& operator += (const basic_string& rhs)
        {
            return append(rhs);
        }

        bool operator == (const basic_string& rhs) const
        {
            return false;
        }

        const T* c_str() const { return m_buffer; }

        size_type size() const { return m_length; }

        size_type length() const { return m_length; }

        void clear()
        {
            m_length = 0;
        }

        basic_string& append(const basic_string& _str)
        {
            reserve(m_length + _str.m_length + 1);

            memcpy(m_buffer + m_length, _str.m_buffer, _str.m_length*sizeof(T));

            m_length += _str.m_length;

            return *this;
        }

        basic_string& erase( size_type pos = 0, size_type n = npos )
        {
            return *this;
        }

        basic_string substr( size_type pos = 0, size_type n = npos ) const
        {
            return "";
        }

        size_type find_last_of( char c, size_type pos = npos ) const
        {
            return npos;
        }

        size_type find_first_of( char c, size_type pos = npos ) const
        {
            return npos;
        }

        void resize(size_type _size)
        {
        }

        void reserve(size_type _size)
        {
            if (_size > m_capacity)
            {
                _size = _size * 809 / 500;
                T* nb = allocate(_size);

                if (m_buffer)
                {
                    memcpy(nb, m_buffer, m_length*sizeof(T));
                    deallocate(m_buffer);
                }
                
                m_capacity = _size;
                m_buffer = nb;
            }
        }

    private:
        T* allocate(size_type _size)
        {
            return new T[_size];
        }

        void deallocate(T* _mem)
        {
            delete [] _mem;
        }

        size_type slen(const T* _str)
        {
            size_type l = 0;
       
            while (*_str)
            {
                l++;
                _str++;
            }
            
            return l;
        }
    };

    typedef basic_string<char> string;
    
}}
#endif  // STRING_HPP_INCLUDED
