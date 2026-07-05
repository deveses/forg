// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2007 Slawomir Strumecki

#pragma once
#include <fstream>
#include <iostream>
#include <vector>

namespace forg::mesh::ply {

#define PLY_ASCII 1     /* ascii PLY file */
#define PLY_BINARY_BE 2 /* binary PLY file, big endian */
#define PLY_BINARY_LE 3 /* binary PLY file, little endian */

#define PLY_OKAY 0   /* ply routine worked okay */
#define PLY_ERROR -1 /* error in ply routine */
/* scalar data types supported by PLY format */
#define PLY_START_TYPE 0
#define PLY_CHAR 1
#define PLY_SHORT 2
#define PLY_INT 3
#define PLY_UCHAR 4
#define PLY_USHORT 5
#define PLY_UINT 6
#define PLY_FLOAT 7
#define PLY_DOUBLE 8
#define PLY_END_TYPE 9

#define PLY_SCALAR 0
#define PLY_LIST 1

struct PlyProperty
{ /* description of a property */

    std::string name;   /* property name */
    int external_type;  /* file's data type */
    int count_external; /* file's count type */
    int is_list;        /* 1 = list, 0 = scalar */
    bool store;

    int internal_type;  /* program's data type */
    int offset;         /* offset bytes of prop in a struct */
    int count_internal; /* program's count type */
    int count_offset;   /* offset byte for list count */
};

struct PlyElement
{                     /* description of an element */
    std::string name; /* element name */
    int num;          /* number of elements in this object */
    int size;         /* size of element (bytes) or -1 if variable */

    std::vector<PlyProperty> props; /* list of properties in the file */
};

/** Polygon File (ply) reader/writer
 * Reads/writes geometry from/to ply file format
 * @todo writing
 */
class plyfile
{
    //////////////////////////////////////////////////////////////////////////
    // Nested
    //////////////////////////////////////////////////////////////////////////
  public:
    typedef std::vector<PlyElement> PlyElementVec;
    typedef PlyElementVec::iterator PlyElementVecI;

    typedef std::vector<PlyProperty> PlyPropertyVec;
    typedef PlyPropertyVec::iterator PlyPropertyVecI;

    //////////////////////////////////////////////////////////////////////////
    // 'structors
    //////////////////////////////////////////////////////////////////////////
  public:
    plyfile();
    virtual ~plyfile();

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    //////////////////////////////////////////////////////////////////////////
  private:
    std::string m_filename;
    std::ifstream m_input;
    size_t m_data_offset;

    int m_filetype;
    double m_version;
    PlyElementVec m_elements;
    std::vector<std::string> m_comment;
    std::vector<std::string> m_obj_info;

    bool m_swap_endian;
    size_t m_elem_index;
    int m_elem_read;

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    //////////////////////////////////////////////////////////////////////////
  public:
    bool Open(const char* filename);

    size_t GetNumElements() { return m_elements.size(); };
    const char* GetElementName(size_t index)
    {
        return m_elements[index].name.c_str();
    }
    int GetElementCount(const char* elem_name);

    void GetProperty(const char* elem_name, const char* prop_name, int offset,
                     int type);
    void GetElement(void* elem_ptr);
    //////////////////////////////////////////////////////////////////////////
    // Helpers
    //////////////////////////////////////////////////////////////////////////
  private:
    void Clean();
    bool ReadHeader();
    void GetElementAscii(void* elem_ptr);
    void GetElementBinary(void* elem_ptr);
};

} // namespace forg::mesh::ply
