#include "forg_pch.h"

#include "lexer.h"

namespace forg { namespace script {

// RegExp
// 1. boolean or
// 2. grouping by parentheses
// 3. quantification: ? (0 or 1), * (0 or more), + (1 or more)
// . - any character
// [] - matches single character or range, e.g. [abc], [abc-f]
// [^ ] - matches single or range of character not contained in brackets
// ^ - starting position in string or line
// $ - ending position in string or line
// () - grouping or subexpression
// \n - matches nth subexpression matched
// * - matches 0 or more
// {m, n} - matches m to n times

///////////////////////////////////////////////////////////////////////////////
// State machine (Finite Automata)
///////////////////////////////////////////////////////////////////////////////

SFAState* SFAState::GetState(int _input)
{
    for (uint i=0; i<connections.size(); i++)
    {
        if (connections[i].input == _input)
        {
            return connections[i].state;
        }
    }

    return 0;
}

SFAState* SFAState::Connect(int _input, int _output, bool _accept)
{
    for (uint i=0; i<connections.size(); i++)
    {
        if (connections[i].input == _input)
        {
            return connections[i].state;
        }
    }

    SFAStateConnection sc;
    sc.input = _input;
    sc.state = new SFAState(_output, _accept);

    connections.push_back(sc);

    return sc.state;
}

SFAState* SFAState::AddLoopback(int _input)
{
    for (uint i=0; i<connections.size(); i++)
    {
        if (connections[i].input == _input)
        {
            return connections[i].state;
        }
    }

    SFAStateConnection sc;
    sc.input = _input;
    sc.state = this;

    connections.push_back(sc);

    return sc.state;
}

StateMachine::StateMachine()
{
    m_current = &m_start;
}

StateMachine::~StateMachine()
{
    for (StateVec::iterator it=m_states.begin(); it!=m_states.end(); ++it)
    {
        delete *it;
    }
}

SFAState* StateMachine::CreateState()
{
    m_states.push_back(new SFAState());

    return m_states.back();
}

void StateMachine::DeleteState(SFAState* _state)
{
    delete _state;
}

bool StateMachine::Transition(int _input, int& _output)
{
    SFAState* next_state = m_current->GetState(_input);

    if (m_current!=next_state && m_current->accept && m_current != &m_start && (next_state == 0 || !next_state->accept))
    {
        // we have terminated token
        _output = m_current->output;
        m_current = &m_start;
        return true;
    } 
    
    if (next_state != 0)
    {
        // we have token, but not terminated
        m_current = next_state;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Regular Expression
///////////////////////////////////////////////////////////////////////////////
void RegularExpression::Set(const core::string& _re)
{
    m_def = _re;
}

bool RegularExpression::Match(const core::string& _text)
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Lexer
///////////////////////////////////////////////////////////////////////////////

Lexer::Lexer()
{
    m_state.Reset();

    m_text_lenght = 0;
}

Lexer::~Lexer()
{
}

void Lexer::DefineToken(int _token, const core::string _def)
{
    m_token_defs.push_back(STokenDefinition());

    m_token_defs.back().definition = _def;
    m_token_defs.back().identifier = _token;
    m_token_defs.back().rexp.Set(_def);
}

int Lexer::Flush(SToken& _token)
{
    int result = 0;

    int out = 0;
    if (m_machine.Transition(0, out))
    {
        m_text[m_text_lenght] = 0;
        m_text_lenght = 0;

        _token.token_id = out;
        _token.text = m_text;

        result = 1;
    }

    return result;
}

int Lexer::ReadToken(int _char, int _symbol, SToken& _token)
{
    int result = 0;

    // update lexer state
    m_state.char_read++;
    m_state.column++;
    m_state.symbol = _symbol;

    if (_char=='\n')
    {
        m_state.line++;
        m_state.column=0;
    }

    // update state machine
    int out = 0;
    if (m_machine.Transition(m_state.symbol, out))
    {
        m_text[m_text_lenght] = 0;
        m_text_lenght = 0;

        _token.token_id = out;
        _token.text = m_text;

        m_machine.Transition(m_state.symbol, out);

        result = 1;
    }

    m_text[m_text_lenght] = (core::string::char_type)_char;
    m_text_lenght++;

    if (m_machine.GetCurrentState() == m_machine.GetStart() && m_text_lenght>0)
    {
        m_text_lenght=0;
    } 

    return result;
}


}}