/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef _XFILE_H_
#define _XFILE_H_

#if _MSC_VER > 1000
#pragma once
#endif


#include "mesh/xfile/xdefs.h"
#include "mesh/xfile/xtemplatesmgr.h"
#include "mesh/xfile/xdatamgr.h"
#include "mesh/xfile/xstdtemplates.h"

namespace forg { namespace xfile {

typedef struct
{
	DWORD magic_number;
	DWORD version;
	DWORD type;
	DWORD float_size;
} xfile_header;

/// DirectX file loader
class XFile
{
    //Nested
public:
    typedef std::vector<const IData*> XDataPtrVec;
    typedef XDataPtrVec::iterator XDataPtrVecI;
    typedef XDataPtrVec::iterator XDataPtrVecCI;

public:
	XFile();
	~XFile();

private:
    std::string m_filename;
    std::ifstream m_input;

	xfile_header m_header;
    XTemplatesMgr m_tmpls_mgr;
    XDataMgr m_data_mgr;

	bool m_bDoubleFloat;

public:
	bool Open(const char* filename);
	void PrintDescription();

    /**
    * Gets objects' instances. IData object has hierarchical structure.
    * @param data_vec [out] container for top level objects
    */
    void GetDataObjects(XDataPtrVec& data_vec);

private:
	int ReadHeader();
	int ReadData(reader::xreader& reader);
};

}}

#endif //_XFILE_H_
