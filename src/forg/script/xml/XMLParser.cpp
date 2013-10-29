#include "forg_pch.h"

#include "XMLParser.h"
#include "core/vector.hpp"

namespace forg { namespace script { namespace xml {

namespace EToken 
{ 
    enum TYPE
    {
        Unknown,
        Start,
        Slash,
        Close,
        Data,
        Name,
        Equal,
        Quotation,
        Attribute,
        Value,
    }; 
}

namespace ESymbol
{
    enum TYPE
    {
        Unknown,
        OpenAngle,      // <
        CloseAngle,     // >
        Slash,          // /
        Quotation,      // "
        Equal,          // =
        NewLine,        // \n
        WhiteSpace,     // \t\x20
        Digit,
        Letter,
    };
}

struct SLexerState;

struct SStateConnection
{
    int symbol;
    SLexerState* state;
};

struct SLexerState
{
    typedef EToken::TYPE TokenType;

    core::vector<SStateConnection> connections;

    TokenType token;
    bool start;
    bool end;   // terminator


    SLexerState()
    {
        token = (TokenType)0;
        start = true;
    }

    SLexerState(TokenType _tok, bool _end)
    {
        token = _tok;
        start = false;
        end = _end;
    }

    SLexerState* GetState(int symbol)
    {
        for (uint i=0; i<connections.size(); i++)
        {
            if (connections[i].symbol == symbol)
            {
                return connections[i].state;
            }
        }

        return 0;
    }

    SLexerState* AddEmpty(int symbol)
    {
        for (uint i=0; i<connections.size(); i++)
        {
            if (connections[i].symbol == symbol)
            {
                return connections[i].state;
            }
        }

        SStateConnection sc;

        sc.symbol = symbol;
        sc.state = this;

        connections.push_back(sc);

        return sc.state;
    }

    SLexerState* Add(int symbol, TokenType token, bool _end)
    {
        for (uint i=0; i<connections.size(); i++)
        {
            if (connections[i].symbol == symbol)
            {
                return connections[i].state;
            }
        }

        SStateConnection sc;

        sc.symbol = symbol;
        sc.state = new SLexerState(token, _end);

        connections.push_back(sc);

        return sc.state;
    }

};

///////////////////////////////////////////////////////////////////////////////

XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
}

bool XMLParser::Open(const char* _filename)
{
    if (m_file.Open(_filename))
    {
        return true;
    }

    return false;
}

void XMLParser::Close()
{
    m_file.Close();
}

int XMLParser::GetSymbol(int _char)
{
    switch (_char)
    {
    case '<': return ESymbol::OpenAngle;
    case '>': return ESymbol::CloseAngle;
    case '=': return ESymbol::Equal;
    case '/': return ESymbol::Slash;
    case '\"': return ESymbol::Quotation;
    case '\n': return ESymbol::NewLine;
    case ' ': return ESymbol::WhiteSpace;
    case '\t': return ESymbol::WhiteSpace;
    case '\r': return ESymbol::WhiteSpace;
    case '_' : return ESymbol::Letter;  //temp
    case '.' : return ESymbol::Letter;  //temp
    }

    if (_char >= '0' && _char <= '9')
        return ESymbol::Digit;

    if (_char >= 'a' && _char <= 'z')
        return ESymbol::Letter;

    if (_char >= 'A' && _char <= 'Z')
        return ESymbol::Letter;

    return ESymbol::Unknown;
}

bool XMLParser::Parse()
{
    if (! ReadTokens())
        return false;

    return true;
}

bool XMLParser::ReadTokens()
{
    char ch = 0;

    ///
    SLexerState m_lex_start;
    m_lex_start.AddEmpty(ESymbol::WhiteSpace);
    m_lex_start.Add(ESymbol::OpenAngle, EToken::Start, true);
    m_lex_start.Add(ESymbol::Letter, EToken::Name, false)->AddEmpty(ESymbol::Letter);
    m_lex_start.Add(ESymbol::Slash, EToken::Slash, true);
    m_lex_start.Add(ESymbol::CloseAngle, EToken::Close, true);
    m_lex_start.Add(ESymbol::Equal, EToken::Equal, true);
    SLexerState* att_value = m_lex_start.Add(ESymbol::Quotation, EToken::Unknown, false);
    att_value->AddEmpty(ESymbol::Digit);
    att_value->AddEmpty(ESymbol::Letter);
    att_value->Add(ESymbol::Quotation, EToken::Value, true);
    //m_lex_start.Add(ESymbol::CloseAngle, EToken::Close);
    //m_lex_start.AddEmpty();
    ///

    m_state.Reset();

    SLexerState* lex_state = &m_lex_start;
    while(m_file.ReadT(ch))
    {
        m_state.char_read++;
        m_state.column++;

        m_state.symbol = GetSymbol(ch);

        if (ch=='\n')
        {
            m_state.line++;
            m_state.column=0;
        }
        else if (m_state.symbol == ESymbol::WhiteSpace)
        {
        }
        else
        {
            SLexerState* next_state = lex_state->GetState(m_state.symbol);

            if (next_state == 0 && !lex_state->start)
            {
                SToken tok;
                tok.token_id = lex_state->token;
                m_tokens.push_back(tok);

                lex_state = &m_lex_start;;
                next_state = lex_state->GetState(m_state.symbol);
            }

            if (next_state == 0)
            {
                //error
                return false;
            } 
            else if (next_state->end)
            {
                SToken tok;
                tok.token_id = next_state->token;
                m_tokens.push_back(tok);

                // we have terminated token
                lex_state = &m_lex_start;
            } else
            {
                // we have token, but not terminated
                lex_state = next_state;
            }
        }
    }

    return true;
}


}}}