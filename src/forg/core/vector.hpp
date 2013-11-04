#ifndef VECTOR_HPP_INCLUDED
#define VECTOR_HPP_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

namespace forg
{
    namespace core
    {

        template <class _Type>
        class vector
        {
        public:
            typedef _Type element_type;

            typedef unsigned int size_type;

            typedef element_type& reference;

            typedef const element_type& const_reference;

            typedef vector<_Type> this_type;

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

                element_type* operator->() const
                {	// return (reference to) designated object
                    return m_ptr;
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

                iterator operator+(difference_type _Off) const
                {	// return this + integer
                    iterator _Tmp = *this;
                    return (_Tmp += _Off);
                }

                iterator& operator-=(difference_type _Off)
                {	// decrement by integer
                    return (*this += -_Off);
                }

                iterator operator-(difference_type _Off) const
                {	// return this - integer
                    iterator _Tmp = *this;
                    return (_Tmp -= _Off);
                }

                reference operator[](difference_type _Off) const
                {	// subscript
                    return (*(*this + _Off));
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
            vector()
                : m_data(0)
                , m_size(0)
                , m_capacity(0)
            {
            }

            vector(size_type _count)
                : m_data(0)
                , m_size(0)
                , m_capacity(0)
            {
                reserve(_count);
                m_size = _count;
            }


            ~vector()
            {
                destroy();
            }

        private:
            char*  m_data;
            size_type m_size;
            size_type m_capacity;

        public:
            reference operator[](size_type _Off)
            {	// subscript mutable sequence
                return (*(begin() + _Off));
            }

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
                    char* arr = new char[_count*sizeof(_Type)];

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

            void resize(size_type _newsize)
            {
                if (_newsize < m_size)
                {

                    _destruct_n(_newsize, m_size - _newsize);

                } 
                else if (_newsize > m_size)
                {
                    reserve(_newsize);

                    _construct_n(m_size, _newsize - m_size);
                }

                m_size = _newsize;
            }

            reference front()
            {
                return ((_Type*)m_data)[0];
            }

            reference back()
            {
                return ((_Type*)m_data)[m_size-1];
            }

            iterator begin()
            {
                return iterator((_Type*)m_data);
            }

            const_iterator begin() const
            {
                return const_iterator((_Type*)m_data);
            }

            iterator end()
            {
                return iterator(((_Type*)m_data) + m_size);
            }

            const_iterator end() const
            {
                return const_iterator(((_Type*)m_data) + m_size);
            }

            _Type* get() { return (_Type*)m_data; }

            const _Type* get() const { return (_Type*)m_data; }

            void clear()
            {
                _destruct_n(0, m_size);
                m_size = 0;
            }

            void swap(vector& _right)
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

                _construct_n(m_size, 1);
                ((_Type*)m_data)[m_size] = _value;

                m_size++;
            }

            void pop_back()
            {
                if (m_size > 0)
                {
                    m_size--;
                }
            }

            void copy_to(vector& _right, iterator _begin,iterator _end, iterator _where)
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
            void _construct_n(uint _off, uint _count)
            {
                _Type* arr = (_Type*)m_data;

                for (uint i=0; i<_count; i++)
                {
                    new (&arr[_off+i]) _Type;
                }
            }

            void _destruct_n(uint _off, uint _count)
            {
                _Type* arr = (_Type*)m_data;
                for (uint i=0; i<_count; i++)
                {
                    arr[_off+i].~_Type();
                }
            }

            void destroy()
            {
                _destruct_n(0, m_size);
                delete [] m_data;
                m_data = 0;
                m_capacity = m_size = 0;
                //delete [] (release());
            }

        };

    }
}  // namespace

#endif // VECTOR_HPP_INCLUDED
