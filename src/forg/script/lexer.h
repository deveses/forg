/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

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

#ifndef _FORG_LEXER_H_
#define _FORG_LEXER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "base.h"
#include "core/string.hpp"
#include "core/vector.hpp"

namespace forg { namespace script { 

    // ========================================================================
    // State machine
    // ========================================================================

    struct SFAState;

    struct SFAStateConnection
    {
        int input;
        SFAState* state;
    };

    struct SFAState
    {
        core::vector<SFAStateConnection> connections;

        int output;

        bool accept;

        SFAState()
        {
            accept = true;
            output = 0;
        }

        SFAState(int _output, bool _accept)
        {
            accept = _accept;
            output = _output;
        }

        SFAState* GetState(int _input);

        SFAState* Connect(int _input, int _output, bool _accept);

        SFAState* AddLoopback(int _input);
    };

    class StateMachine
    {
        typedef core::vector<SFAState*> StateVec;

        StateVec m_states;
        SFAState m_start;
        SFAState* m_current;

    public:
        StateMachine();
        ~StateMachine();

        SFAState* GetStart() { return &m_start; }
        SFAState* GetNextState(int _input) { return m_current->GetState(_input); }
        SFAState* GetCurrentState() { return m_current; }
        bool Transition(int _input, int& _output);

        SFAState* CreateState();
        void DeleteState(SFAState* _state);
    };

    // ========================================================================
    // Regular expression
    // ========================================================================
    class RegularExpression
    {
        core::string m_def;

    public:
        void Set(const core::string& _re);

        bool Match(const core::string& _text);
    };

    // ========================================================================
    // Lexer analyzer
    // ========================================================================

    struct SToken
    {
        int token_id;

        core::string text;
    };

    struct STokenDefinition
    {
        core::string definition;
        int identifier;

        RegularExpression rexp;
    };

    struct SParserState
    {
        int char_read;
        int symbol;
        int line;
        int column;

        void Reset()
        {
            char_read = 0;
            line = 0;
            column = 0;
        }
    };

    enum {MAX_TOKEN_LENGTH = 1024};

    class FORG_API Lexer
    {
        typedef core::vector<STokenDefinition> TokenDefVec;

        // scanner state
        SParserState m_state;

        // state machine
        StateMachine m_machine;

        TokenDefVec m_token_defs;

        char m_text[MAX_TOKEN_LENGTH];
        uint m_text_lenght;

    public:
        Lexer();
        ~Lexer();

        void DefineToken(int _token, const core::string _def);
        //SLexerState* GetStartNode() { return &m_start_state; };
        SFAState* GetStartNode() { return m_machine.GetStart(); };
        int ReadToken(int _char, int _symbol, SToken& _token);
        int Flush(SToken& _token);
    };

}}

#endif