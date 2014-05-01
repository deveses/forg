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

#ifndef STRINGTOKENIZER_HPP_INCLUDED
#define STRINGTOKENIZER_HPP_INCLUDED

#include "base.h"
#include <string>
//#include "core/string.hpp"

namespace forg { namespace core {

/// StringTokenizer template class
/**
* StringTokenizer
* Java-like class to get tokens from string
* @author eses
* @version 1.0
* @date 07-2005
* @todo
* @bug
* @warning
*/

template <typename T = char>
class StringTokenizer
{
//////////////////////////////////////////////////////////////////////
// Nested
//////////////////////////////////////////////////////////////////////
public:
    typedef std::basic_string<T> string_type;
    typedef typename std::basic_string<T>::size_type size_type;
    //typedef core::basic_string<T> string_type;
    //typedef typename core::basic_string<T>::size_type size_type;

//////////////////////////////////////////////////////////////////////////
// Construction/Deconstruction
//////////////////////////////////////////////////////////////////////////
public:
	/// Constructs a string tokenizer for the specified string.
	/**
	* Constructs a string tokenizer for the specified string.
	* @param str
	* String to be parsed.
	*/
	StringTokenizer(const string_type& str)
		: m_string(str)
		, m_nLastPos(0)
		, m_bReturnDelims(false)
	{
	}

	/// Constructs a string tokenizer for the specified string.
	/**
	* Constructs a string tokenizer for the specified string.
	* The characters in the delim argument are the delimiters for separating tokens.
	* Delimiter characters themselves will not be treated as tokens.
	* @param str
	* String to be parsed.
	* @param delim
	* The delimiters.
	*/
	StringTokenizer(const string_type& str, const string_type& delim)
		: m_string(str)
		, m_delim(delim)
		, m_nLastPos(0)
		, m_bReturnDelims(false)
	{
	}

	/// Constructs a string tokenizer for the specified string.
	/**
	* Constructs a string tokenizer for the specified string.
	* All characters in the delim argument are the delimiters for separating tokens.
	* If the returnDelims flag is true, then the delimiter characters are also returned as tokens.
	* Each delimiter is returned as a string of length one. If the flag is false,
	* the delimiter characters are skipped and only serve as separators between tokens.
	* @param str
	* String to be parsed.
	* @param delim
	* The delimiters.
	* @param returnDelims
	* Flag indicating whether to return the delimiters as tokens.
	*/
	StringTokenizer(const string_type& str, const string_type& delim, bool returnDelims)
		: m_string(str)
		, m_delim(delim)
		, m_nLastPos(0)
		, m_bReturnDelims(returnDelims)
	{
	}

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
private:
	string_type m_string;
	string_type m_delim;
	size_type m_nLastPos;
	bool m_bReturnDelims;

//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////
public:
	/// Calculates the number of times that this tokenizer's nextToken method can be called.
	/**
	* Calculates the number of times that this tokenizer's nextToken method can be called
	* before it generates an exception. The current position is not advanced.
	* @return The number of tokens remaining in the string using the current delimiter set.
	*/
	int	CountTokens()
	{
        TRAP_NOT_IMPLEMENTED();
//#warning "fuction not implemented"
		return 0;
	}

	/// Tests if there are more tokens available from this tokenizer's string.
	/**
	* Tests if there are more tokens available from this tokenizer's string.
	* @return
	* true if and only if there is at least one token in the string after the current position; false otherwise.
	*/
	bool HasMoreTokens()
	{
		return m_nLastPos < m_string.size();
	}

	/// Returns the next token from this string tokenizer.
	/**
	* Returns the next token from this string tokenizer.
	* @return
	*/
	string_type NextToken()
	{
		string_type token;

		size_type pos = m_string.find_first_of(m_delim, m_nLastPos);

        if ((pos != string_type::npos) && (pos >= m_nLastPos))
		{
			token = m_string.substr(m_nLastPos, pos - m_nLastPos);
			m_nLastPos = pos + 1;
		} else
		{
            // there is some substring at the end of string
            if (m_nLastPos < m_string.size())
            {
                token = m_string.substr(m_nLastPos, pos - m_nLastPos);
            }

			m_nLastPos = m_string.length();
		}

		return token;
	}


	/// Returns the next token in this string tokenizer's string.
	/**
	* Returns the next token in this string tokenizer's string.
	* First, the set of characters considered to be delimiters by this StringTokenizer object is changed
	* to be the characters in the string delim. Then the next token in the string after the current position
	* is returned. The current position is advanced beyond the recognized token. The new delimiter set remains
	* the default after this call.
	* @param delim
	* The new delimiters.
	* @return The next token, after switching to the new delimiter set.
	*/
	string_type NextToken(const string_type& delim)
	{
        string_type token;

        size_type pos = m_string.find_first_of(delim, m_nLastPos);

        if ((pos != string_type::npos) && (pos >= m_nLastPos))
        {
            token = m_string.substr(m_nLastPos, pos - m_nLastPos);
            m_nLastPos = pos + 1;
        } else
        {
            // there is some substring at the end of string
            if (m_nLastPos < m_string.size())
            {
                token = m_string.substr(m_nLastPos, pos - m_nLastPos);
            }

            m_nLastPos = m_string.length();
        }

        return token;
	}
};

}}  // namespace

#endif // STRINGTOKENIZER_HPP_INCLUDED
