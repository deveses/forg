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

#ifndef _FORG_CORE_REFCOUNTER_H_
#define _FORG_CORE_REFCOUNTER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"

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
class FORG_API RefCounter
{
    int m_refCount;

public:
    RefCounter() { m_refCount = 1; }
    virtual ~RefCounter() {}

public:
	/// Brief description
	/**
	* Detailed description
	* @return Number of references.
	*/
	virtual int AddRef(void);

	/// Brief description
	/**
	* Detailed description
	* @return Number of references.
	*/
	virtual int Release(void);
};

}}

#endif  //  _FORG_CORE_REFCOUNTER_H_
