#include "forg_pch.h"

#include "XMLParser.h"
#include "core/vector.hpp"
#include "core/auto_ptr.hpp"

namespace forg { namespace script { namespace xml {

namespace EToken 
{ 
    enum TYPE
    {
        Unknown,
        WhiteSpace,
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

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

bool ParserBase::HasMoreTokens()
{
    if (m_current_token < m_tokens.size())
        return true;

    return false;
}

SToken* ParserBase::GetNextToken()
{
    if (HasMoreTokens())
    {
        SToken* t = &m_tokens[m_current_token];
        
        m_current_token++;

        return t;
    }

    return 0;
}

SToken* ParserBase::PeekNextToken()
{
    if (HasMoreTokens())
    {
        return &m_tokens[m_current_token];        
    }

    return 0;
}

bool ParserBase::ReadTokens()
{
    int ch = 0;
    int rt=0;

    for (; GetChar(ch) && rt>=0;)
    {
        SToken tok;
        
        rt = m_lexer.ReadToken(ch, GetSymbol(ch), tok);
        
        if (rt > 0)
        {
            m_tokens.push_back(tok);
        }
    }

    if (rt>=0)
    {
        SToken tok;
        rt = m_lexer.Flush(tok);
        if (rt > 0)
        {
            m_tokens.push_back(tok);
        }

    }

    return (rt>=0);
}

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
XMLNode::XMLNode(int _type)
{
    m_parent = 0;
    m_next = 0;
    m_children = 0;
    m_attributes = 0;
    m_type = _type;
}

XMLNode::~XMLNode()
{
}

XMLNode* XMLNode::FindAttribute(const core::string& _name)
{
    XMLNode* n = m_attributes;

    while (n)
    {
        if (n->GetName() == _name)
        {
            return n;
        }

        n = n->GetNext();
    }

    return 0;
}
///////////////////////////////////////////////////////////////////////////

XMLDocument::XMLDocument()
    : m_root(NULL)
{
}

XMLDocument::~XMLDocument()
{
}

XMLNode* XMLDocument::FindNode(const core::string& _name)
{
    core::vector<XMLNode*> stack;

    stack.push_back(m_root);

    while(stack.size()>0)
    {
        XMLNode* n = stack.back();
        stack.pop_back();

        if (n->GetName() == _name)
        {
            return n;
        }

        n = n->GetChildren();
        while (n)
        {
            stack.push_back(n);
            n = n->GetNext();
        }
    }

    return 0;
}
///////////////////////////////////////////////////////////////////////////
class TokenBackup
{
    XMLParser* m_parser;
    uint m_token_index;

public:
    TokenBackup(XMLParser* _parser)
    {
        m_parser = _parser;
        m_token_index = _parser->GetNextTokenIndex();
    }

    ~TokenBackup()
    {
        if (m_parser)
        {
            m_parser->SetNextTokenIndex(m_token_index);
        }
    }

    void Reset()
    {
        m_parser = 0;
    }
};
///////////////////////////////////////////////////////////////////////////
namespace ParserError
{
    enum TYPE
    {
        NoErrors = 0,
        UnexpectedToken
    };
}

///////////////////////////////////////////////////////////////////////////
XMLParser::XMLParser()
{
    m_doc = 0;
}

XMLParser::~XMLParser()
{
    if (m_doc)
    {
        delete m_doc;
    }
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

bool XMLParser::GetChar(int& _char)
{
    char ch = 0;
    bool r = m_file.ReadT(ch);

    _char = ch;

    return r;
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

void XMLParser::InitTokens()
{
    SFAState* start = m_lexer.GetStartNode();

    start->AddLoopback(ESymbol::WhiteSpace);
    start->AddLoopback(ESymbol::NewLine);
    start->Connect(ESymbol::OpenAngle, EToken::Start, true);

    SFAState* name = start->Connect(ESymbol::Letter, EToken::Name, true);
    name->AddLoopback(ESymbol::Letter);
    //name->Add(ESymbol::WhiteSpace, EToken::Name, true);
    
    start->Connect(ESymbol::Slash, EToken::Slash, true);
    start->Connect(ESymbol::CloseAngle, EToken::Close, true);
    start->Connect(ESymbol::Equal, EToken::Equal, true);

    SFAState* att_value = start->Connect(ESymbol::Quotation, EToken::Unknown, false);
    att_value->AddLoopback(ESymbol::Digit);
    att_value->AddLoopback(ESymbol::Letter);
    att_value->Connect(ESymbol::Quotation, EToken::Value, true);
    //m_lex_start.Add(ESymbol::CloseAngle, EToken::Close);
    //m_lex_start.AddEmpty();

    // TODO: make it working - implement regexp
    m_lexer.DefineToken(EToken::WhiteSpace, " \n\t\r");   
    m_lexer.DefineToken(EToken::Name, "[A-Za-z0-9]+");   
    m_lexer.DefineToken(EToken::Slash, "/");   
    m_lexer.DefineToken(EToken::Close, "<");
    m_lexer.DefineToken(EToken::Equal, "=");
    m_lexer.DefineToken(EToken::Value, "\"(.*)\"");
    m_lexer.DefineToken(EToken::Data, "[^<]*");
    ///
}

// BASIC XML FORMAT DRAFT  
// ELEMENT:
//  START-TAG
//  CONTENT
//  END-TAG
// CONTENT: 
//  DATA or ELEMENT

XMLDocument* XMLParser::Parse()
{
/*    SLexerState lex_start;
    InitTokens(&lex_start);

    if (! ReadTokens(lex_start))
        return false;
        */
    InitTokens();

    if (! ReadTokens())
        return false;

    m_current_token = 0;
    m_error_code = 0;
    if (m_doc)
    {
        delete m_doc;
        m_doc = 0;
    }

    XMLDocument* doc = new XMLDocument();

    if (ReadDocument(doc))
    {
        m_doc = doc;
    } else
    {
        delete doc;
    }

    return m_doc;
}

bool XMLParser::ReadDocument(XMLDocument* _doc)
{
    XMLNode* root_node = new XMLNode(EXMLNodeType::Root);

    if (ReadContent(root_node))
    {
        _doc->SetRootNode(root_node);

        return true;
    } 

    delete root_node;

    return false;
}

bool XMLParser::ReadContent(XMLNode* _node)
{
    while (HasMoreTokens())
    {
        XMLNode* n = ReadElement(_node);

        if (n!=0)
        {
            _node->AddChild(n);
            continue;
        }

        if (m_error_code != ParserError::NoErrors)
            return false;

        if (!_node->IsRoot() && ReadEndTag(_node))
        {
            break;
        }
    }

    return true;
}

XMLNode* XMLParser::ReadElement(XMLNode* _parent)
{
    bool empty_element = false;

    TokenBackup backup(this);
    SToken* tok = GetNextToken();

    if (tok->token_id != EToken::Start)
        return 0;

    tok = GetNextToken();

    if (tok->token_id != EToken::Name)
        return 0;
    
    core::auto_ptr<XMLNode> node(new XMLNode(EXMLNodeType::Element));
    
    node->SetName(tok->text);

    while (ReadAttribute(node.get()))
    {
    }

    tok = GetNextToken();

    if (tok->token_id == EToken::Slash && PeekNextToken()->token_id == EToken::Close)
    {
        empty_element = true;

        tok = GetNextToken();
    }

    if (tok->token_id != EToken::Close)
    {
        SetErrorCode(ParserError::UnexpectedToken);
        return 0;
    }

    backup.Reset();

    if (!empty_element)
    {
        ReadContent(node.get());
    }

    return node.release();
}

bool XMLParser::ReadEndTag(XMLNode* _node)
{
    TokenBackup backup(this);
    
    SToken* tok = GetNextToken();

    if (tok->token_id != EToken::Start)
        return false;

    tok = GetNextToken();
    if (tok->token_id != EToken::Slash)
        return false;

    SToken* tok_name = GetNextToken();
    if (tok_name->token_id != EToken::Name)
        return false;

    tok = GetNextToken();
    if (tok->token_id != EToken::Close)
        return false;

    // TODO: case insensitive comparision
    if (tok_name->text != _node->GetName())
    {
        return false;
    }

    backup.Reset();

    return true;
}

// TODO: attribute data without quotation marks
bool XMLParser::ReadAttribute(XMLNode* _node)
{
    TokenBackup backup(this);
    SToken* tok_name = GetNextToken();

    if (tok_name && tok_name->token_id != EToken::Name)
        return false;

    SToken* tok_value = GetNextToken();
    if (tok_value && tok_value->token_id != EToken::Equal)
        return false;

    tok_value = GetNextToken();
    if (tok_value && tok_value->token_id != EToken::Value)
        return false;

    backup.Reset();

    XMLNode* att = new XMLNode(EXMLNodeType::Attribute);

    core::string vtext = tok_value->text.substr(1, tok_value->text.length()-2);

    att->SetName(tok_name->text);
    att->SetContent(vtext);

    _node->AddAttribute(att);

    return true;
}



}}}