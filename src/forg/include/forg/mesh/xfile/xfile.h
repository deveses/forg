// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "mesh/xfile/xdatamgr.h"
#include "mesh/xfile/xdefs.h"
#include "mesh/xfile/xstdtemplates.h"
#include "mesh/xfile/xtemplatesmgr.h"

#include <fstream>

namespace forg::xfile {

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
    // Nested
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

} // namespace forg::xfile
