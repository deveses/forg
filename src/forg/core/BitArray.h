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

#ifndef _FORG_CORE_BITARRAY_H_
#define _FORG_CORE_BITARRAY_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/vector.hpp"
#include "core/string.hpp"

namespace forg { namespace core {

/// BitArray class
/**
* BitArray
* @author eses
* @version 1.0
* @date 07-2005
* @todo
* @bug
* @warning
*/
class FORG_API BitArray
{
	// Nested
	public:
    typedef forg::core::vector<int> IntArray;
    typedef forg::core::string string;

	// Constructors
	public:
//		explicit BitArray(const array<bool>& values);
//		explicit BitArray(const array<byte>& values);
//		explicit BitArray(const array<int>& values);

		/// Sets the number of elements contained in the BitArray.
		/**
		* Initializes a new instance of the BitArray class that can hold the specified
		* number of bit values, which are initially set to false.
		* @param length
		* The number of bit values in the new BitArray
		*/
		explicit BitArray(uint length = 32);
		BitArray(uint length, bool defaultValue);
		BitArray(const BitArray& bits);


	// Destructor
	public:
		~BitArray(void);

	// Operators
		BitArray& operator = (const BitArray& bits);
		bool operator[](uint index) const;
		BitArray operator &(const BitArray& arg) const;
		BitArray operator |(const BitArray& arg) const;
		 BitArray operator ^(const BitArray& arg) const;
		BitArray operator ~() const;
		BitArray& operator &=(const BitArray& arg);
		BitArray& operator |=(const BitArray& arg);
		BitArray& operator ^=(const BitArray& arg);
		friend bool operator == (const BitArray& b1, const BitArray& b2);
		friend bool operator != (const BitArray& b1, const BitArray& b2);


	// Attributes
	private:
		IntArray m_array;
		uint m_length;

	// Attributes Properties
	public:
		/// Gets the number of elements contained in the BitArray.
		/**
		* Gets the number of elements contained in the BitArray.
		* @return Number of elements.
		*/
		uint get_Count() const;

		/// Gets the number of elements contained in the BitArray.
		/**
		* Gets the number of elements contained in the BitArray.
		* @return Number of elements.
		*/
		uint get_Length() const;

		/// Sets the number of elements contained in the BitArray.
		/**
		* Sets the number of elements contained in the BitArray.
		* @param value
		* Number of elements
		*/
		void set_Length(uint value);

	// Class Methods (public)
	public:
		/// Performs the bitwise AND operation.
		/**
		* Performs the bitwise AND operation on the elements in the current BitArray
		* against the corresponding elements in the specified BitArray.
		* @param value
		* The BitArray with which to perform the bitwise AND operation.
		* @return A BitArray containing the result of the bitwise AND operation
		* on the elements in the current BitArray against the corresponding elements
		* in the specified BitArray.
		*/
		BitArray And(const BitArray& value) const;

		/// Performs the bitwise OR operation.
		/**
		* Performs the bitwise OR operation on the elements in the current BitArray
		* against the corresponding elements in the specified BitArray.
		* @param value
		* The BitArray with which to perform the bitwise OR operation.
		* @return A BitArray containing the result of the bitwise OR operation
		* on the elements in the current BitArray against the corresponding elements
		* in the specified BitArray.
		*/
		BitArray Or(const BitArray& value) const;

		/// Performs the bitwise XOR operation.
		/**
		* Performs the bitwise XOR operation on the elements in the current BitArray
		* against the corresponding elements in the specified BitArray.
		* @param value
		* The BitArray with which to perform the bitwise XOR operation.
		* @return A BitArray containing the result of the bitwise XOR operation
		* on the elements in the current BitArray against the corresponding elements
		* in the specified BitArray.
		*/
		BitArray Xor(const BitArray& value) const;

		/// Inverts all the bit values in the current BitArray.
		/**
		* Inverts all the bit values in the current BitArray,
		* so that elements set to true are changed to false, and elements
		* set to false are changed to true.
		* @return The current instance with inverted bit values.
		*/
		BitArray Not() const;

		/// Gets the value of the bit at a specific position in the BitArray.
		/**
		* Gets the value of the bit at a specific position in the BitArray.
		* @param index
		* The zero-based index of the value to get.
		* @return The value of the bit at position index.
		*/
		bool Get(uint index) const;

		/// Sets the bit at a specific position in the BitArray to the specified value.
		/**
		* Sets the bit at a specific position in the BitArray to the specified value.
		* @param index
		* The zero-based index of the bit to set.
		* @param value
		* The Boolean value to assign to the bit.
		*/
		void Set(uint index, bool value = true);

		//CORELIB_API void Set(int index, int value); //do zrobienia

		/// Sets all bits in the BitArray to the specified value.
		/**
		* Sets all bits in the BitArray to the specified value.
		* @param value
		* The Boolean value to assign to all bits.
		*/
		void SetAll(bool value = true);

		/// Returns a String that represents the current Object.
		/**
		* Returns a String that represents the current Object.
		* @return A String that represents the current Object.
		*/
		string ToString();

	// Helpers
	private:

};

}}

#endif  //  _FORG_CORE_BITARRAY_H_
