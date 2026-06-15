/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
*******************************************************************************/

#ifndef _FORG_SCRIPT_PARSERBASE_H_
#define _FORG_SCRIPT_PARSERBASE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>
#include <vector>

#include "forg/base.h"
#include "forg/os/File.h"
#include "forg/script/lexer.h"

namespace forg { namespace script {

    class FORG_API FileParserBase
    {
    protected:
        forg::os::File m_file;

    public:
        virtual ~FileParserBase() {}

        bool Open(const char* _filename);
        void Close();

    protected:
        virtual bool GetChar(int& _ch);
        bool ReadLine(std::string& _line);
    };

    class FORG_API TokenParserBase : public FileParserBase
    {
    protected:
        typedef std::vector<SToken> TokenVec;

        Lexer m_lexer;
        TokenVec m_tokens;
        SParserState m_state;
        uint m_current_token;

    public:
        TokenParserBase();
        virtual ~TokenParserBase() {}

        bool ReadTokens();

        bool HasMoreTokens();
        SToken* GetNextToken();
        SToken* PeekNextToken();

        uint GetNextTokenIndex() const { return m_current_token; }
        void SetNextTokenIndex(uint _index) { m_current_token = _index; }

        virtual int GetSymbol(int _ch) = 0;
    };

    class FORG_API TokenBackup
    {
        TokenParserBase* m_parser;
        uint m_token_index;

    public:
        TokenBackup(TokenParserBase* _parser);
        ~TokenBackup();

        void Reset() { m_parser = nullptr; }
    };

}}

#endif
