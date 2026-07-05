// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
// #include "base.h"
#include "forg/core/string.hpp"

#include <vector>

namespace forg::core {

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
    typedef std::vector<int> IntArray;
    typedef forg::core::string string;

    // Constructors
  public:
    //		explicit BitArray(const array<bool>& values);
    //		explicit BitArray(const array<byte>& values);
    //		explicit BitArray(const array<int>& values);

    /// Sets the number of elements contained in the BitArray.
    /**
     * Initializes a new instance of the BitArray class that can hold the
     * specified number of bit values, which are initially set to false.
     * @param length
     * The number of bit values in the new BitArray
     */
    explicit BitArray(u32 length = 32);
    BitArray(u32 length, bool defaultValue);
    BitArray(const BitArray& bits);

    // Destructor
  public:
    ~BitArray(void);

    // Operators
    BitArray& operator=(const BitArray& bits);
    bool operator[](u32 index) const;
    BitArray operator&(const BitArray& arg) const;
    BitArray operator|(const BitArray& arg) const;
    BitArray operator^(const BitArray& arg) const;
    BitArray operator~() const;
    BitArray& operator&=(const BitArray& arg);
    BitArray& operator|=(const BitArray& arg);
    BitArray& operator^=(const BitArray& arg);
    friend bool operator==(const BitArray& b1, const BitArray& b2);
    friend bool operator!=(const BitArray& b1, const BitArray& b2);

    // Attributes
  private:
    IntArray m_array;
    u32 m_length;

    // Attributes Properties
  public:
    /// Gets the number of elements contained in the BitArray.
    /**
     * Gets the number of elements contained in the BitArray.
     * @return Number of elements.
     */
    u32 get_Count() const;

    /// Gets the number of elements contained in the BitArray.
    /**
     * Gets the number of elements contained in the BitArray.
     * @return Number of elements.
     */
    u32 get_Length() const;

    /// Sets the number of elements contained in the BitArray.
    /**
     * Sets the number of elements contained in the BitArray.
     * @param value
     * Number of elements
     */
    void set_Length(u32 value);

    // Class Methods (public)
  public:
    /// Performs the bitwise AND operation.
    /**
     * Performs the bitwise AND operation on the elements in the current
     * BitArray against the corresponding elements in the specified BitArray.
     * @param value
     * The BitArray with which to perform the bitwise AND operation.
     * @return A BitArray containing the result of the bitwise AND operation
     * on the elements in the current BitArray against the corresponding
     * elements in the specified BitArray.
     */
    BitArray And(const BitArray& value) const;

    /// Performs the bitwise OR operation.
    /**
     * Performs the bitwise OR operation on the elements in the current BitArray
     * against the corresponding elements in the specified BitArray.
     * @param value
     * The BitArray with which to perform the bitwise OR operation.
     * @return A BitArray containing the result of the bitwise OR operation
     * on the elements in the current BitArray against the corresponding
     * elements in the specified BitArray.
     */
    BitArray Or(const BitArray& value) const;

    /// Performs the bitwise XOR operation.
    /**
     * Performs the bitwise XOR operation on the elements in the current
     * BitArray against the corresponding elements in the specified BitArray.
     * @param value
     * The BitArray with which to perform the bitwise XOR operation.
     * @return A BitArray containing the result of the bitwise XOR operation
     * on the elements in the current BitArray against the corresponding
     * elements in the specified BitArray.
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
    bool Get(u32 index) const;

    /// Sets the bit at a specific position in the BitArray to the specified
    /// value.
    /**
     * Sets the bit at a specific position in the BitArray to the specified
     * value.
     * @param index
     * The zero-based index of the bit to set.
     * @param value
     * The Boolean value to assign to the bit.
     */
    void Set(u32 index, bool value = true);

    // CORELIB_API void Set(int index, int value); //do zrobienia

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

} // namespace forg::core
