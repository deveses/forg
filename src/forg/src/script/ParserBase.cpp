#include "forg_pch.h"

#include "forg/script/ParserBase.h"

namespace forg::script {

bool FileParserBase::Open(const char* _filename)
{
    return m_file.Open(_filename);
}

void FileParserBase::Close() { m_file.Close(); }

bool FileParserBase::GetChar(int& _char)
{
    char ch = 0;
    bool result = m_file.ReadT(ch);

    _char = ch;

    return result;
}

bool FileParserBase::ReadLine(std::string& _line)
{
    _line.clear();

    int ch = 0;
    bool read_any = false;

    while (GetChar(ch))
    {
        read_any = true;

        if (ch == '\n')
            return true;

        if (ch != '\r')
            _line.push_back(static_cast<char>(ch));
    }

    return read_any;
}

TokenParserBase::TokenParserBase()
{
    m_state.Reset();
    m_current_token = 0;
}

bool TokenParserBase::HasMoreTokens()
{
    return (m_current_token < m_tokens.size());
}

SToken* TokenParserBase::GetNextToken()
{
    if (HasMoreTokens())
    {
        SToken* token = &m_tokens[m_current_token];
        m_current_token++;
        return token;
    }

    return nullptr;
}

SToken* TokenParserBase::PeekNextToken()
{
    if (HasMoreTokens())
        return &m_tokens[m_current_token];

    return nullptr;
}

bool TokenParserBase::ReadTokens()
{
    m_tokens.clear();
    m_current_token = 0;

    int ch = 0;
    int rt = 0;

    for (; GetChar(ch) && rt >= 0;)
    {
        SToken tok;

        rt = m_lexer.ReadToken(ch, GetSymbol(ch), tok);

        if (rt > 0)
        {
            m_tokens.push_back(tok);
        }
    }

    if (rt >= 0)
    {
        SToken tok;
        rt = m_lexer.Flush(tok);
        if (rt > 0)
        {
            m_tokens.push_back(tok);
        }
    }

    return (m_tokens.size() > 0);
}

TokenBackup::TokenBackup(TokenParserBase* _parser)
{
    m_parser = _parser;
    m_token_index = _parser->GetNextTokenIndex();
}

TokenBackup::~TokenBackup()
{
    if (m_parser)
    {
        m_parser->SetNextTokenIndex(m_token_index);
    }
}

} // namespace forg::script
