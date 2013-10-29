#include "forg_pch.h"

#include "mesh/xfile/xtexreader.h"
#include "debug//dbg.h"

namespace forg { namespace xfile { namespace reader {

xtexreader::xtexreader(std::ifstream& input, bool doubleFloat)
: m_input(input)
{
    m_bDoubleFloat = doubleFloat;
    m_lexer.SetInput(&input);
}

WORD xtexreader::ReadToken()
{
    ScannerToken scan;

	if (m_next_tokens.size() == 0)
	{
        if ( m_lexer.GetToken( scan ) == 0 )
        {
            if (! m_input.eof())
            {
                DBG_MSG("[xtexreader::ReadToken] Unknown token (lexem <%s> at line %d, column %d)\n", scan.lexem.c_str(), scan.line, scan.col);
            }

            return 0;
        }

        //DBG_MSG("[xtexreader::ReadToken] lexem <%s> at line %d, column %d\n", scan.lexem.c_str(), scan.line, scan.col);

        m_last_tokens.push_back(scan);
	} else
    {
        scan = m_next_tokens.front();
        m_next_tokens.pop_front();

        m_last_tokens.push_back(scan);
    }

	return scan.token;
}

int xtexreader::UnreadToken()
{
	if (m_last_tokens.size() == 0)
	{	//nie zostalo jeszcze nic przeczytane
		return 1;
	}

	//wrzucamy token spowrotem do kolejki nastepnych
	ScannerToken token = m_last_tokens.back();
	m_last_tokens.pop_back();

	m_next_tokens.push_front(token);

	return 0;
}

int xtexreader::EvalToken(xstring& value)
{
    if (m_last_tokens.size())
    {
        value = m_last_tokens.back().lexem;

        if (m_last_tokens.back().token == EToken::TOKEN_STRING)
        {
            size_t off = 0;
            size_t len = value.size();

            if (value[0] == '\"')
            {
                off++;
                len--;
            }

            if (value[off + len - 1] == '\"')
            {
                len--;
            }

            value = value.substr(off, len);
        }

        return 0;
    }

    return -1;
}

int xtexreader::EvalToken(xguid& value)
{
    if (m_last_tokens.size())
    {
        int d1 = 0;
        int d2 = 0;
        int d3 = 0;

        char p1[5] = {0};

        sscanf(m_last_tokens.back().lexem.c_str(), "<%x-%hx-%hx-%hx-%4s%x>", &value.Data1, &value.Data2, &value.Data3, &d1, p1, &d3);
        sscanf(p1, "%x", &d2);

        value.Data4[0] = (d1 & 0xff00) >> 8;
        value.Data4[1] = (d1 & 0xff);
        value.Data4[2] = (d2 & 0xff00) >> 8;
        value.Data4[3] = (d2 & 0xff);

        value.Data4[4] = (d3 & 0xff000000) >> 24;
        value.Data4[5] = (d3 & 0xff0000) >> 16;
        value.Data4[6] = (d3 & 0xff00) >> 8;
        value.Data4[7] = (d3 & 0xff);



        return 0;
    }

    return -1;
}

int xtexreader::EvalToken(int& value)
{
    if (m_last_tokens.size())
    {
        sscanf(m_last_tokens.back().lexem.c_str(), "%d", &value);

        return 0;
    }

    return -1;
}

int xtexreader::EvalToken(IntegerList& value)
{
    if (m_last_tokens.size())
    {
        size_t tlen = m_last_tokens.back().lexem.size();
        char* text = new char[tlen+1];

        m_last_tokens.back().lexem.copy(text, tlen);
        text[tlen] = 0;

        char* token = strtok(text, ",;");

        while (token != NULL)
        {
            int i = 0;
            sscanf(token, "%d", &i);

            value.push_back(i);

            token = strtok(NULL, ",;");
        }

        delete [] text;

        return 0;
    }

    return -1;
}

int xtexreader::EvalToken(FloatList& value)
{
    if (m_last_tokens.size())
    {
        size_t tlen = m_last_tokens.back().lexem.size();
        char* text = new char[tlen+1];

        m_last_tokens.back().lexem.copy(text, tlen);
        text[tlen] = 0;

        char* token = strtok(text, ",;");

        while (token != NULL)
        {
            float f = 0.0f;
            sscanf(token, "%f", &f);

            value.push_back(f);

            token = strtok(NULL, ",;");
        }

        delete [] text;

        return 0;
    }

    return -1;
}

int xtexreader::ReadInteger(int& value)
{

    if (ReadToken() != EToken::TOKEN_INTEGER)
    {
    	UnreadToken();
    	return -1;
    }

    return EvalToken(value);
}

int xtexreader::ReadIntegerList(IntegerList& int_list)
{
    int tok = ReadToken();

    if (tok != EToken::TOKEN_INTEGER_LIST && tok != EToken::TOKEN_INTEGER)
    {
    	UnreadToken();
    	return -1;
    }

    return EvalToken(int_list);
}

int	xtexreader::ReadFloatList(FloatList& float_list)
{
    if (ReadToken() != EToken::TOKEN_FLOAT_LIST)
    {
    	UnreadToken();
    	return -1;
    }

    int rval = EvalToken(float_list);

    return rval;
}

int xtexreader::ReadStringList(StringList& string_list)
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

int xtexreader::ReadName(xstring& name)
{
	WORD tok = ReadToken();

	if (tok != EToken::TOKEN_NAME)
	{
		name = "";
		UnreadToken();
		return -1;
	}

    return EvalToken(name);
}

int xtexreader::ReadString(xstring& str)
{
    WORD tok = ReadToken();

    if (tok != EToken::TOKEN_STRING)
    {
    	str = "";
    	UnreadToken();
    	return -1;
    }

    return EvalToken(str);
}

int xtexreader::ReadGUID(xguid& tguid)
{
    if (ReadToken() != EToken::TOKEN_GUID)
    {
    	UnreadToken();
    	return -1;
    }

    return EvalToken(tguid);
}


}}}
