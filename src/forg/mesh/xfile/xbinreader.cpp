#include "forg_pch.h"
#include "mesh/xfile/xbinreader.h"
#include "debug/dbg.h"

namespace forg { namespace xfile { namespace reader {

/*
template <typename T>
int read_value(T& var, std::ifstream& input)
{
    input.read((char*)&var, sizeof(T));

    return input.fail();
}

template <typename T>
int read_value(T* buffer, size_t count, std::ifstream& input)
{
    input.read((char*)buffer, sizeof(T)*count);

    return input.fail();
}
*/

xbinreader::xbinreader(std::ifstream& input, bool doubleFloat)
: m_input(input)
{
    m_bDoubleFloat = doubleFloat;
}

bool xbinreader::read_data(char* buffer, unsigned int count)
{
    m_input.read(buffer, count);

    return m_input.fail();
}

WORD xbinreader::ReadToken()
{
	WORD token = 0;

	if (m_next_tokens.size() == 0)
	{
        if ( read_value(token))
            return 0;

        //m_input.read((char*)&token, sizeof(token));
        //if (m_input.fail())
        //    return 0;

		m_next_tokens.push_back(token);
	}

	token = m_next_tokens.front();
	m_next_tokens.pop_front();
	m_last_tokens.push_back(token);

	return token;
}

int xbinreader::UnreadToken()
{
	if (m_last_tokens.size() == 0)
	{	//nie zostalo jeszcze nic przeczytane
		return 1;
	}

	//wrzucamy token spowrotem do kolejki nastepnych
	WORD token = m_last_tokens.back();
	m_last_tokens.pop_back();
	m_next_tokens.push_front(token);

	return 0;
}

int xbinreader::ReadInteger(int& value)
{

    if (ReadToken() != EToken::TOKEN_INTEGER)
    {
    	UnreadToken();
    	return -1;
    }

    int ret = read_value(value);

    //DBG_MSG("[xbinreader::ReadInteger] value: %d\n", value);

    return ret;
}

int xbinreader::ReadIntegerList(IntegerList& int_list)
{
    if (ReadToken() != EToken::TOKEN_INTEGER_LIST)
    {
    	UnreadToken();
    	return -1;
    }

    DWORD count = 0;

    if (read_value(count))
        return 1;

    //int_list.reserve(count);

    int rval = 0;

    if (count > 0)
    {
        IntegerList iarr(count);

        rval = read_value(iarr.get(), count);

        int_list.swap(iarr);
    }

/*
    DWORD v;
    for(DWORD i=0; i<count; i++)
    {
        if (read_value(v, m_input))
        {
            rval = 1;
            break;
        } else
        {
            int_list.push_back(v);
        }
    }
*/

    //DBG_MSG("[xbinreader::ReadIntegerList] first: %d last: %d\n", int_list.front(), int_list.back());

    return rval;
}

#pragma message(" * dokonczyc ReadFloatList")
int	xbinreader::ReadFloatList(FloatList& float_list)
{
    if (ReadToken() != EToken::TOKEN_FLOAT_LIST)
    {
    	UnreadToken();
    	return -1;
    }

    DWORD count = 0;
    read_value(count);

    int rval = 0;

//     float fv = 0.0f;
//     double dv = 0.0;

    if (count > 0)
    {
        FloatList farr(count);

        rval = read_value(farr.get(), count);

        float_list.swap(farr);
    }

/*
    for (DWORD i = 0; i<count && !rval; i++)
    {
        if (m_bDoubleFloat)
        {
            rval = read_value(dv, m_input);
        } else
        {
            rval = read_value(fv, m_input);
        }

        if (rval == 0)
        {
            if (m_bDoubleFloat)
                float_list.push_back((float)dv);
            else
                float_list.push_back(fv);
        }
    }
*/

    return rval;
}

#pragma message(" * dokonczyc ReadStringList")
int xbinreader::ReadStringList(StringList& string_list)
{
    // string {list_separator string} list_separator

//string_list           : string_list_1 list_separator
//
//string_list_1         : string
//						| string_list_1 list_separator string
//

//list_separator        : comma
//						| semicolon
//
    int rval = 0;
    int list_size = 0;

    do
    {
    	xstring str;

    	rval = ReadString(str);

    	if (rval == 0)	//list_separator check follows
    	{
            string_list.push_back(str);
            list_size++;

            WORD tok = ReadToken();

    		rval = ((tok != EToken::TOKEN_COMMA) && (tok != EToken::TOKEN_SEMICOLON));
            if (rval)
            {
                UnreadToken();
                rval = -1;
            }
    	}

    } while (rval == 0);

    return rval > 0 ? rval : ((list_size == 0) ? rval : 0);
}

int xbinreader::ReadName(xstring& name)
{
	WORD tok = ReadToken();

	if (tok != EToken::TOKEN_NAME)
	{
		name = "";
		UnreadToken();
		return -1;
	}

	DWORD count = 0;
	int rval = 0;


    read_value(count);
    char *charr = new char[count+1];

    if ( read_value(charr, count))
        rval = 1;
    else
    {
        charr[count] = 0;
        name = charr;
    }

    delete [] charr;

	return rval;
}

int xbinreader::ReadString(xstring& str)
{
    WORD tok = ReadToken();

    if (tok != EToken::TOKEN_STRING)
    {
    	str = "";
    	UnreadToken();
    	return -1;
    }

    DWORD count = 0;
    WORD terminator = 0;
    int rval = 0;

    read_value(count);

    char* char_array = new char[count+1];

    if ( read_value(char_array, count) || read_value(terminator))
    {
        rval = 1;
    }

    //terminator &= 0xffff;
    if (terminator != EToken::TOKEN_SEMICOLON && terminator != EToken::TOKEN_COMMA)
    {
    	rval = 1;
    } else
    {
        char_array[(uint)count] = 0;
        str = char_array;
    }

    delete[] char_array;

    return rval;
}

int xbinreader::ReadGUID(xguid& tguid)
{
    if (ReadToken() != EToken::TOKEN_GUID)
    {
    	UnreadToken();
    	return -1;
    }

    read_value(tguid);

    return 0;
}


}}}
