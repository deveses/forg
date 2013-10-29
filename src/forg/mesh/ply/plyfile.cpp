#include "forg_pch.h"

#include "mesh/ply/plyfile.h"
#include "core/StringTokenizer.hpp"
#include <string>
#undef max
#undef min
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>

namespace forg { namespace mesh { namespace ply {

char *type_names[] = {
    "invalid",
    "char", "short", "int",
    "uchar", "ushort", "uint",
    "float", "double",
};

int ply_type_size[] = {
    0, 1, 2, 4, 1, 2, 4, 4, 8
};

#define NO_OTHER_PROPS  -1

#define DONT_STORE_PROP  0
#define STORE_PROP       1

#define OTHER_PROP       0
#define NAMED_PROP       1


template <class T>
bool from_string(T& t,
                 const std::string& s,
                 std::ios_base& (*f)(std::ios_base&) = std::dec)
{
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
}

/*
template <>
bool from_string<int>(int& t,
                      const std::string& s,
                      std::ios_base& (*f)(std::ios_base&))
{
    t = atoi(s.c_str());

    return true;
}

template <>
bool from_string<uint>(uint& t,
                      const std::string& s,
                      std::ios_base& (*f)(std::ios_base&))
{
    t = atoi(s.c_str());

    return true;
}

template <>
bool from_string<double>(double& t,
                       const std::string& s,
                       std::ios_base& (*f)(std::ios_base&))
{
    t = atof(s.c_str());

    return true;
}
*/

int equal_strings(char *s1, char *s2)
{
    return (strcmp(s1, s2) == 0);
}

class plyElemFind : public std::binary_function<PlyElement, std::string, bool>
{
public:
    result_type operator()(const PlyElement& elem, const std::string& name) const
    {
        return (result_type)(elem.name == name);
    }
};

class plyPropFind : public std::binary_function<PlyProperty, std::string, bool>
{
public:
    result_type operator()(const PlyProperty& prop, const std::string& name) const
    {
        return (result_type)(prop.name == name);
    }
};

/******************************************************************************
Return the type of a property, given the name of the property.

Entry:
name - name of property type

Exit:
returns integer code for property, or 0 if not found
******************************************************************************/
int get_prop_type(const std::string& type_name)
{
    int i;

    for (i = PLY_START_TYPE + 1; i < PLY_END_TYPE; i++)
        if (type_name == type_names[i])
            return (i);

    /* if we get here, we didn't find the type */
    return (0);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

plyfile::plyfile()
{

}

plyfile::~plyfile()
{
    Clean();
}

void plyfile::Clean()
{
    if (m_input.is_open())
        m_input.close();

    m_elements.clear();
    m_comment.clear();
    m_obj_info.clear();
}

//////////////////////////////////////////////////////////////////////////

bool plyfile::Open(const char *filename)
{
    Clean();

    m_input.open(filename, std::ios_base::in|std::ios_base::binary);

    if (! m_input.is_open())
        return false;

    m_filename = filename;

    return ReadHeader();
}

//////////////////////////////////////////////////////////////////////////

int plyfile::GetElementCount(const char* elem_name)
{
    PlyElementVecI iter = std::find_if(m_elements.begin(), m_elements.end(), std::bind2nd(plyElemFind(), elem_name));

    if (iter != m_elements.end())
    {
        return iter->num;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////

bool plyfile::ReadHeader()
{
    if (! m_input.is_open())
        return false;

    /* read and parse the file's header */

    std::string str;
    std::getline(m_input, str);
/*
    m_data_offset = m_input.gcount();
    m_data_offset = m_input.tellg();*/


    if (str != "ply")
        return false;

    m_filetype = PLY_ASCII; //default type

    bool header_end = false;

    while ( m_input )
    {
        core::StringTokenizer<char> tokenizer(str, " \n\0");

        while (tokenizer.HasMoreTokens())
        {
            std::string tok = tokenizer.NextToken();

            if (tok == "format") {
                tok = tokenizer.NextToken();

                if (tok == "ascii")
                    m_filetype = PLY_ASCII;
                else if (tok == "binary_big_endian")
                    m_filetype = PLY_BINARY_BE;
                else if (tok == "binary_little_endian")
                    m_filetype = PLY_BINARY_LE;
                else
                    return false;

                tok = tokenizer.NextToken();

                from_string(m_version, tok, std::dec);
            }
            else if (tok == "element")
            {
                m_elements.push_back(PlyElement());

                PlyElement& elem = m_elements.back();

                elem.name = tokenizer.NextToken();
                from_string(elem.num, tokenizer.NextToken());
                elem.size = 0;
            }
            else if (tok == "property")
            {
                /* create the new property */

                PlyElement& elem = m_elements.back();

                elem.props.push_back(PlyProperty());

                PlyProperty& prop = elem.props.back();

                tok = tokenizer.NextToken();

                if (tok == "list") {       /* is a list */
                    prop.is_list = 1;
                    prop.count_external = get_prop_type (tokenizer.NextToken());
                    prop.external_type = get_prop_type (tokenizer.NextToken());
                    prop.name = tokenizer.NextToken();
                    elem.size += ply_type_size[ prop.external_type ] + ply_type_size[ prop.count_external ];
                }
                else {                                        /* not a list */
                    prop.external_type = get_prop_type (tok);
                    prop.name = tokenizer.NextToken();
                    prop.is_list = 0;
                    elem.size += ply_type_size[ prop.external_type ];
                }

                prop.store = false;
            }
            else if (tok == "comment")
            {
                std::string comment;

                comment = tokenizer.NextToken("\n\0");

                m_comment.push_back( comment );
            }
            else if (tok == "obj_info")
            {
                std::string info;
                while (tokenizer.HasMoreTokens())
                {
                    info += tokenizer.NextToken();
                }

                m_obj_info.push_back( info );
            }
            else if (tok == "end_header")
            {
                header_end = true;
                break;
            }
        }

        if (header_end)
        {
            m_data_offset = m_input.tellg();
            break;
        }

        std::getline(m_input, str);
    }

/*

    / * create tags for each property of each element, to be used * /
    / * later to say whether or not to store each property for the user * /

    for (i = 0; i < plyfile->nelems; i++) {
        elem = plyfile->elems[i];
        elem->store_prop = (char *) malloc (sizeof (char) * elem->nprops);
        for (j = 0; j < elem->nprops; j++)
            elem->store_prop[j] = DONT_STORE_PROP;
        elem->other_offset = NO_OTHER_PROPS; / * no "other" props by default * /
    }
*/

    m_elem_index = 0;
    m_elem_read = 0;

    if (m_filetype != PLY_ASCII)
    {
        m_swap_endian = (m_filetype != PLY_BINARY_LE);
    }

    return true;
}

void plyfile::GetProperty(const char* elem_name, const char* prop_name, int offset, int type)
{
    PlyElementVecI iter = std::find_if(m_elements.begin(), m_elements.end(), std::bind2nd(plyElemFind(), elem_name));

    if (iter != m_elements.end())
    {
        PlyPropertyVecI propi = std::find_if(iter->props.begin(), iter->props.end(), std::bind2nd(plyPropFind(), prop_name));

        if (propi != iter->props.end())
        {
            propi->store = true;
            propi->offset = offset;
            propi->internal_type = type;
        }
    }
}

void GetAsciiItem(char* item_ptr, int in_type, int out_type, std::string& item)
{
    int v_int = 0;
    uint v_uint = 0;
    double v_double = 0.0;

    switch (in_type) {
    case PLY_CHAR:
    case PLY_UCHAR:
    case PLY_SHORT:
    case PLY_USHORT:
    case PLY_INT:
        from_string(v_int, item);
        if (in_type != out_type)
        {
            v_uint = v_int;
            v_double = v_int;
        }
        break;

    case PLY_UINT:
        from_string(v_uint, item);
        if (in_type != out_type)
        {
            v_int = v_uint;
            v_double = v_uint;
        }
        break;

    case PLY_FLOAT:
    case PLY_DOUBLE:
        from_string(v_double, item);
        if (in_type != out_type)
        {
            v_uint = (uint)v_double;
            v_int = (int)v_double;
        }
        break;
    }


    unsigned char *puchar;
    short int *pshort;
    unsigned short int *pushort;
    int *pint;
    unsigned int *puint;
    float *pfloat;
    double *pdouble;

    switch (out_type) {
    case PLY_CHAR:
        *item_ptr = v_int;
        break;
    case PLY_UCHAR:
        puchar = (unsigned char *) item_ptr;
        *puchar = v_int;
        break;
    case PLY_SHORT:
        pshort = (short *) item_ptr;
        *pshort = v_int;
        break;
    case PLY_USHORT:
        pushort = (unsigned short *) item_ptr;
        *pushort = v_int;
        break;
    case PLY_INT:
        pint = (int *) item_ptr;
        *pint = v_int;
        break;
    case PLY_UINT:
        puint = (unsigned int *) item_ptr;
        *puint = v_uint;
        break;
    case PLY_FLOAT:
        pfloat = (float *) item_ptr;
        *pfloat = (float)v_double;
        break;
    case PLY_DOUBLE:
        pdouble = (double *) item_ptr;
        *pdouble = v_double;
        break;
    }
}

void SwapEndian(char* data, int size)
{
    char t = 0;
    int k = size >> 1;

    for (int i=0; i<k; i++)
    {
        t = data[i];
        data[i] = data[size - 1 - i];
        data[size - 1 - i] = t;
    }
}

void GetBinaryItem(char* item_ptr, int in_type, int out_type, bool swap, std::ifstream& item)
{
    int v_int = 0;
    uint v_uint = 0;
    double v_double = 0.0;
    float v_float = 0.0f;
    int size_in = ply_type_size[in_type];

    switch (in_type) {
    case PLY_CHAR:
    case PLY_UCHAR:
    case PLY_SHORT:
    case PLY_USHORT:
    case PLY_INT:
        item.read((char*)&v_int, size_in);
        if (swap) SwapEndian((char*)&v_int, size_in);
        if (in_type != out_type)
        {
            v_uint = v_int;
            v_double = v_int;
        }
        break;

    case PLY_UINT:
        item.read((char*)&v_uint, size_in);
        if (swap) SwapEndian((char*)&v_uint, size_in);
        if (in_type != out_type)
        {
            v_int = v_uint;
            v_double = v_uint;
        }
        break;

    case PLY_FLOAT:
        item.read((char*)&v_float, size_in);
        if (swap) SwapEndian((char*)&v_float, size_in);
        if (in_type != out_type)
        {
            v_double = v_float;
            v_uint = (uint)v_float;
            v_int = (int)v_float;
        }
        break;
    case PLY_DOUBLE:
        item.read((char*)&v_double, size_in);
        if (swap) SwapEndian((char*)&v_double, size_in);
        if (in_type != out_type)
        {
            v_uint = (uint)v_double;
            v_int = (int)v_double;
        }
        break;
    }


    unsigned char *puchar;
    short int *pshort;
    unsigned short int *pushort;
    int *pint;
    unsigned int *puint;
    float *pfloat;
    double *pdouble;

    switch (out_type) {
    case PLY_CHAR:
        *item_ptr = v_int;
        break;
    case PLY_UCHAR:
        puchar = (unsigned char *) item_ptr;
        *puchar = v_int;
        break;
    case PLY_SHORT:
        pshort = (short *) item_ptr;
        *pshort = v_int;
        break;
    case PLY_USHORT:
        pushort = (unsigned short *) item_ptr;
        *pushort = v_int;
        break;
    case PLY_INT:
        pint = (int *) item_ptr;
        *pint = v_int;
        break;
    case PLY_UINT:
        puint = (unsigned int *) item_ptr;
        *puint = v_uint;
        break;
    case PLY_FLOAT:
        pfloat = (float *) item_ptr;
        *pfloat = (float)v_float;
        break;
    case PLY_DOUBLE:
        pdouble = (double *) item_ptr;
        *pdouble = v_double;
        break;
    }
}

void plyfile::GetElement(void *elem_ptr)
{
    if (m_filetype == PLY_ASCII)
        GetElementAscii(elem_ptr);
    else
        GetElementBinary(elem_ptr);
}

void plyfile::GetElementAscii(void *elem_ptr)
{
    if (! m_input.is_open())
        return;

    if (m_elem_index >= m_elements.size())
        return;

    std::string str;
    std::getline(m_input, str);

    PlyElement& elem = m_elements[m_elem_index];

    core::StringTokenizer<char> tokenizer(str, " \n\0");

    for (PlyPropertyVecI propi=elem.props.begin(); propi != elem.props.end(); ++propi)
    {
        std::string tok = tokenizer.NextToken();

        if (! propi->store)
            continue;

        if (propi->is_list)
        {
            int count = 0;
            int item_size = ply_type_size[propi->internal_type];

            from_string(count, tok);

            for (int k=0; k<count; k++)
            {
                tok = tokenizer.NextToken();

                GetAsciiItem((char*)elem_ptr + propi->offset + k*item_size, propi->external_type, propi->internal_type, tok);
            }
        } else
        {
            GetAsciiItem((char*)elem_ptr + propi->offset, propi->external_type, propi->internal_type, tok);
        }
    }

    m_elem_read++;

    if (elem.num <= m_elem_read)
    {
        m_elem_index++;
        m_elem_read = 0;
    }
}

void plyfile::GetElementBinary(void *elem_ptr)
{
    if (! m_input.is_open())
        return;

    if (m_elem_index >= m_elements.size())
        return;

    PlyElement& elem = m_elements[m_elem_index];

    for (PlyPropertyVecI propi=elem.props.begin(); propi != elem.props.end(); ++propi)
    {
        if (propi->is_list)
        {
            int count = 0;
            int item_size = ply_type_size[propi->internal_type];

            GetBinaryItem((char*)&count, propi->count_external, PLY_INT, m_swap_endian, m_input);

            for (int k=0; k<count; k++)
            {
                if (propi->store)
                    GetBinaryItem((char*)elem_ptr + propi->offset + k*item_size, propi->external_type, propi->internal_type, m_swap_endian, m_input);
                else
                    m_input.seekg(item_size, std::ios_base::cur);
            }
        } else
        {
            if (propi->store)
                GetBinaryItem((char*)elem_ptr + propi->offset, propi->external_type, propi->internal_type, m_swap_endian, m_input);
            else
                m_input.seekg(ply_type_size[propi->external_type], std::ios_base::cur);
        }
    }

    m_elem_read++;

    if (elem.num <= m_elem_read)
    {
        m_elem_index++;
        m_elem_read = 0;
    }
}

}}} //namespaces
