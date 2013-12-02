#ifndef STRING_HPP_INCLUDED
#define STRING_HPP_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "debug/dbg.h"

namespace forg { namespace core {

    template <class T>
    class basic_string
    {
    public:
        typedef unsigned int size_type;
		typedef T char_type;

        enum {npos = 0xffffffff};

    private:
        char_type* m_buffer;
        size_type m_length;
        size_type m_capacity;

    public:
        basic_string()
        {
            m_buffer = 0;
            m_length = 0;
            m_capacity = 0;
        }

		basic_string(const char_type* _str)
        {
			m_buffer = 0;
			m_length = 0;
			m_capacity = 0;

            size_type l = slen(_str);

            reserve(l+1);

			memcpy(m_buffer, _str, l*sizeof(char_type));

			m_buffer[l] = 0;

            m_length = l;
        }

        basic_string(size_type _size, char_type _char_fill)
        {
            m_buffer = 0;
            m_length = 0;
            m_capacity = 0;

            size_type l = _size;

            reserve(l + 1);

            for (size_type i = 0; i < l; i++)
            {
                m_buffer[i] = _char_fill;
            }

            m_buffer[l] = 0;

            m_length = l;
        }

        basic_string(const basic_string& _str)
        {
            m_buffer = allocate(_str.m_capacity);
            m_length = _str.m_length;
            m_capacity = _str.m_capacity;

            copy(m_buffer, _str.m_buffer, m_length*sizeof(T));
        }

        ~basic_string()
        {
            deallocate(m_buffer);
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
            
            size_type l = rhs.length();

            reserve(l+1);

            copy(m_buffer, rhs.m_buffer, l*sizeof(T));

            m_length = l;

            m_buffer[l] = 0;

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
            return (0 == compare(*this, rhs));
        }

        bool operator != (const basic_string& rhs) const
        {
            return (compare(rhs) != 0);
        }

        int compare(const basic_string& rhs) const
        {
            for (size_type i=0; i<length() && i<rhs.length(); i++)
            {
                if (m_buffer[i] != rhs.m_buffer[i])
                {
                    return m_buffer[i] - rhs.m_buffer[i];
                }
            }

            return 0;
        }

        const T* c_str() const { return m_buffer; }

        size_type size() const { return m_length; }

        size_type length() const { return m_length; }

        void clear()
        {
            m_length = 0;
        }

        basic_string& append(const basic_string& _str, size_type _pos = 0, size_type _num = npos)
        {
            if (_pos >= _str.m_length) _pos = 0;
            if (_num == npos) _num = _str.m_length - _pos;

            if (_num > 0 && (_num + _pos <= _str.m_length) )
            {
                reserve(m_length + _num + 1);

                copy(m_buffer + m_length, _str.m_buffer + _pos, _num*sizeof(char_type));

                m_length += _num;

                m_buffer[m_length] = 0;
            }

            return *this;
        }

        basic_string& erase( size_type pos = 0, size_type n = npos )
        {
            if (pos < m_length)
            {
                if (n != npos)
                {
                    basic_string tail;
                    tail.append(*this, pos + n, npos);

                    m_buffer[pos] = 0;
                    m_length = pos;

                    append(tail);
                }
                else
                {
                    // cut the tail
                    m_buffer[pos] = 0;
                    m_length = pos;
                }
            }

			return *this;
        }

        basic_string substr( size_type pos = 0, size_type n = npos ) const
        {
            if (pos < m_length)
            {                
                basic_string sub;
                sub.append(*this, pos, n);
                return sub;
            }

            return "";
        }

        size_type find_last_of( char_type _c, size_type _pos = npos ) const
        {
            for (size_type i = 0; i < m_length; i++)
            {
                size_type p = m_length - 1 - i;
                if (m_buffer[p] == _c)
                {
                    return p;
                }
            }
			
			return npos;
        }

        size_type find_first_of( char_type c, size_type pos = npos ) const
        {
			TRAP_NOT_IMPLEMENTED();
			
			return npos;
        }

        void resize(size_type _size)
        {
            reserve(_size);
            m_length = _size;
        }

        void reserve(size_type _size)
        {
            if (_size > m_capacity)
            {
                _size = _size * 809 / 500;
                T* nb = allocate(_size);

                if (m_buffer)
                {
                    copy(nb, m_buffer, m_length*sizeof(T));
                    deallocate(m_buffer);
                }
                
                m_capacity = _size;
                m_buffer = nb;
            }
        }

        static int compare(const basic_string& _lhs, const basic_string& _rhs)
        {
            int r = _lhs.length() - _rhs.length();
            basic_string::size_type p = 0;
            
            while (r == 0 && p < _lhs.length() && p < _rhs.length())
            {
                r = _lhs[p] - _rhs[p];
                p++;
            }

            return r;
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

        void copy(T* _dst, const T* _src, size_type _bytes)
        {
            memcpy(_dst, _src, _bytes);
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
