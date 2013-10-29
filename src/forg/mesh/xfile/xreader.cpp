#include "forg_pch.h"

#include "mesh/xfile/xreader.h"
#include "debug/dbg.h"

namespace forg { namespace xfile { namespace reader {

    int xreader::ReadPrimitiveType(ETemplatePrimitiveType::TYPE& primitive_type)
    {
        WORD tok = ReadToken();

        if (tok == 0) return 1;

        WORD ptype = 0;

        switch(tok) {
                case EToken::TOKEN_WORD:
                case EToken::TOKEN_DWORD:
                case EToken::TOKEN_FLOAT:
                case EToken::TOKEN_DOUBLE:
                case EToken::TOKEN_CHAR:
                case EToken::TOKEN_UCHAR:
                case EToken::TOKEN_SWORD:
                case EToken::TOKEN_SDWORD:
                case EToken::TOKEN_LPSTR:
                case EToken::TOKEN_UNICODE:
                case EToken::TOKEN_CSTRING:
                    ptype = tok;
                    break;
        }

        if (ptype == 0)
        {
            UnreadToken();
            return -1;
        }

        primitive_type = (ETemplatePrimitiveType::TYPE)ptype;

        return 0;
    }


    int xreader::ReadIntegers(IntegerList& int_list, DWORD count)
    {
        int rval = 0;

        if (m_Integers.size() == 0 || m_IntBegin == m_Integers.end())
        {
            m_Integers.clear();
            rval = ReadIntegerList(m_Integers);
            m_IntBegin = m_Integers.begin();
        }

        size_t icount = m_Integers.end() - m_IntBegin;

        if (rval == 0)
        {
            if (count > 1)
            {
                if (count <= icount)
                {
                    IntegerList::iterator last = m_IntBegin;

                    if (count == icount)
                    {
                        last = m_Integers.end();
                    }
                    else
                    {
                        last += count;
                    }

                    if (count == m_Integers.size())
                    {
                        m_Integers.swap(int_list);
                    } else
                    {
                        m_Integers.copy_to(int_list, m_IntBegin, last, int_list.begin());
                    }

                    m_IntBegin = last;
                }
                else
                {
                    DBG_TRACE_ERR("Wrong array size", 1);
                    rval = 1;
                }
            } else
            {
                int_list.push_back( *m_IntBegin );
                ++m_IntBegin;
            }
        }

        return rval;
    }

    int xreader::ReadFloats(FloatList& float_list, DWORD count)
    {
        int rval = 0;

        if (m_Floats.size() == 0 || m_FloatBegin == m_Floats.end())
        {
            m_Floats.clear();
            rval = ReadFloatList(m_Floats);
            m_FloatBegin = m_Floats.begin();
        }

        size_t fcount = m_Floats.end() - m_FloatBegin;

        if (rval == 0)
        {
            if (count > 1)
            {
                if (count <= fcount)
                {
                    FloatList::iterator last = m_FloatBegin;

                    if (count == fcount)
                    {
                        last = m_Floats.end();
                    }
                    else
                    {
                        last += count;
                    }

                    if (count == m_Floats.size())
                    {
                        m_Floats.swap(float_list);
                    } else
                    {
                        m_Floats.copy_to(float_list, m_FloatBegin, last, float_list.begin());
                    }

                    m_FloatBegin = last;

                }
                else
                {
                    DBG_TRACE_ERR("Wrong array size", 1);
                    rval = 1;
                }
            } else
            {
                float_list.push_back( *m_FloatBegin );
                ++m_FloatBegin;
            }
        }

        return rval;
    }
}}}
