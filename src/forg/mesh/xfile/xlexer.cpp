#include "forg_pch.h"

#include "mesh/xfile/xlexer.h"
#include "mesh/xfile/xdefs.h"

#include <iostream>
#include <fstream>

/*
keywords:
ARRAY
DWORD UCHAR FLOAT ULONGLONG SDWORD CHAR STRING WORD CSTRING SWORD DOUBLE
BINARY BINARY_RESOURCE
UNICODE
TEMPLATE

#  This is a comment.
// This is another comment.

WORD 16 bits
DWORD 32 bits
FLOAT IEEE float
DOUBLE 64 bits
CHAR 8 bits
UCHAR 8 bits
BYTE 8 bits
STRING NULL terminated string
CSTRING Formatted C string (not supported)
UNICODE Unicode string (not supported)

keywords' letters: abcdefghilmnoprstuwy
*/

#define CAST_INT(x) ((int)x)

namespace forg { namespace xfile {

namespace Symbols {
enum TYPE
{
    OTHER = 0,
    DIGIT,
    LET_A, LET_B, LET_C, LET_D, LET_E, LET_F, LET_G, LET_H, LET_I, LET_L, LET_M,
    LET_N, LET_O, LET_P, LET_R, LET_S, LET_T, LET_U, LET_W, LET_Y, LET_OTHER,
    SYM_OBRACE, SYM_CBRACE, SYM_OPAREN, SYM_CPAREN, SYM_OBRACKET, SYM_CBRACKET, SYM_OANGLE, SYM_CANGLE,
    SYM_DOT, SYM_COMMA, SYM_SEMICOLON, SYM_MINUS, SYM_UNDERSCORE, SYM_QUOTATION,
    _COUNT
};
};

static Symbols::TYPE sym_tab[256] = { Symbols::OTHER };

static const unsigned short transition[][Symbols::_COUNT] =
{
//    ?   D   a  b  c  d  e  f  g  h  i  l  m  n  o  p  r  s  t  u  w  y  X    {  }  (  )  [  ]  <  >    .  ,  ;  -  _  "
    { 0, 42, 23, 1, 1,32, 1,37, 1, 1, 1, 1, 1, 1, 1, 1, 1,51, 2, 1,28, 1, 1,  10,11,12,13,14,15,16,17,  18,19,20,45, 0,49},

    // token_name (1)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},

    // token_template (2-9)
    { 0,  1,  1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},

    // one character tokens (10-20)
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    // token_oangle (16)
    { 0, 21, 21,21,21,21,21,21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_guid (21-22) - simplified, without format check
    { 0, 21, 21,21,21,21,21,21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,22,   0, 0, 0,21, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_array (23-27)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,24, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,25, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1, 26, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,27, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 0, 0},

//    ?   D   a  b  c  d  e  f  g  h  i  l  m  n  o  p  r  s  t  u  w  y  X    {  }  (  )  [  ]  <  >    .  ,  ;  -  _  "

    // token_word (28-31)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,29, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,30, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1,31, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_dword (32-36)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,33, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,34, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,35, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1,36, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_float (37-41)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1,38, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,39, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1, 40, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,41, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},

    // token_integer (42)
    { 0, 42,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,  46,43, 0, 0, 0, 0},

    // token_integer_list (43-44)
    {43, 43,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0,43,44, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_float_list (45-48)
    { 0, 45,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,  46, 0, 0, 0, 0, 0},
    {46, 46,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0,47,48, 0, 0, 0},
    {47, 45,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0,45, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

//    ?   D   a  b  c  d  e  f  g  h  i  l  m  n  o  p  r  s  t  u  w  y  X    {  }  (  )  [  ]  <  >    .  ,  ;  -  _  "

    // token_string (49-50)
    {49, 49, 49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,  49,49,49,49,49,49,49,49,  49,49,49,49,49,50},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},

    // token_lpstr (51-56)
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,52, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,53, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1,54, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,55, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  1,  1, 1, 1, 1, 1, 1,56, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0},
};

// only for orientation
#define MAX_STATE 47

/*
static const int ending_states[] =
{
    0,1,1,1,1, 1,1,1,1,1,
    1,1,1,1,1, 1,1,1,1,1,
    1,0,1,1,1, 1,1,1,
};
*/

static const EToken::TYPE state_token_map[] =
{
    EToken::TOKEN_UNKNOWN,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_TEMPLATE,     //9
    EToken::TOKEN_OBRACE,       //10
    EToken::TOKEN_CBRACE,       //11
    EToken::TOKEN_OPAREN,       //12
    EToken::TOKEN_CPAREN,
    EToken::TOKEN_OBRACKET,
    EToken::TOKEN_CBRACKET,
    EToken::TOKEN_OANGLE,
    EToken::TOKEN_CANGLE,
    EToken::TOKEN_DOT,
    EToken::TOKEN_COMMA,
    EToken::TOKEN_SEMICOLON,    //20
    EToken::TOKEN_GUID,         //21
    EToken::TOKEN_GUID,         //22
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_ARRAY,        //27
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_WORD,         //31
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_DWORD,        //36
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_FLOAT,        //41
    EToken::TOKEN_INTEGER,      //42
    EToken::TOKEN_INTEGER_LIST,
    EToken::TOKEN_INTEGER_LIST, //44
    EToken::TOKEN_FLOAT_LIST,
    EToken::TOKEN_FLOAT_LIST,
    EToken::TOKEN_FLOAT_LIST,
    EToken::TOKEN_FLOAT_LIST,      //48
    EToken::TOKEN_STRING,
    EToken::TOKEN_STRING,          //50
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_NAME,
    EToken::TOKEN_LPSTR,
};

XLexer::XLexer()
{
    m_initialized = false;
    m_state.Reset();
}

void XLexer::Initialize()
{
    m_state.yyline = 1;

    for (int i='0'; i<='9'; i++) sym_tab[i] = Symbols::DIGIT;
    for (int i='a'; i<='z'; i++) sym_tab[i] = Symbols::LET_OTHER;
    for (int i='A'; i<='Z'; i++) sym_tab[i] = Symbols::LET_OTHER;

    sym_tab[CAST_INT('a')] = sym_tab[CAST_INT('A')] = Symbols::LET_A;
    sym_tab[CAST_INT('b')] = sym_tab[CAST_INT('B')] = Symbols::LET_B;
    sym_tab[CAST_INT('c')] = sym_tab[CAST_INT('C')] = Symbols::LET_C;
    sym_tab[CAST_INT('d')] = sym_tab[CAST_INT('D')] = Symbols::LET_D;
    sym_tab[CAST_INT('e')] = sym_tab[CAST_INT('E')] = Symbols::LET_E;
    sym_tab[CAST_INT('f')] = sym_tab[CAST_INT('F')] = Symbols::LET_F;
    sym_tab[CAST_INT('g')] = sym_tab[CAST_INT('G')] = Symbols::LET_G;
    sym_tab[CAST_INT('h')] = sym_tab[CAST_INT('H')] = Symbols::LET_H;
    sym_tab[CAST_INT('i')] = sym_tab[CAST_INT('I')] = Symbols::LET_I;
    sym_tab[CAST_INT('l')] = sym_tab[CAST_INT('L')] = Symbols::LET_L;
    sym_tab[CAST_INT('m')] = sym_tab[CAST_INT('M')] = Symbols::LET_M;
    sym_tab[CAST_INT('n')] = sym_tab[CAST_INT('N')] = Symbols::LET_N;
    sym_tab[CAST_INT('o')] = sym_tab[CAST_INT('O')] = Symbols::LET_O;
    sym_tab[CAST_INT('p')] = sym_tab[CAST_INT('P')] = Symbols::LET_P;
    sym_tab[CAST_INT('r')] = sym_tab[CAST_INT('R')] = Symbols::LET_R;
    sym_tab[CAST_INT('s')] = sym_tab[CAST_INT('S')] = Symbols::LET_S;
    sym_tab[CAST_INT('t')] = sym_tab[CAST_INT('T')] = Symbols::LET_T;
    sym_tab[CAST_INT('u')] = sym_tab[CAST_INT('U')] = Symbols::LET_U;
    sym_tab[CAST_INT('w')] = sym_tab[CAST_INT('W')] = Symbols::LET_W;
    sym_tab[CAST_INT('y')] = sym_tab[CAST_INT('Y')] = Symbols::LET_Y;

    sym_tab[CAST_INT('{')] = Symbols::SYM_OBRACE;
    sym_tab[CAST_INT('}')] = Symbols::SYM_CBRACE;
    sym_tab[CAST_INT('(')] = Symbols::SYM_OPAREN;
    sym_tab[CAST_INT(')')] = Symbols::SYM_CPAREN;
    sym_tab[CAST_INT('[')] = Symbols::SYM_OBRACKET;
    sym_tab[CAST_INT(']')] = Symbols::SYM_CBRACKET;
    sym_tab[CAST_INT('<')] = Symbols::SYM_OANGLE;
    sym_tab[CAST_INT('>')] = Symbols::SYM_CANGLE;
    sym_tab[CAST_INT('.')] = Symbols::SYM_DOT;
    sym_tab[CAST_INT(',')] = Symbols::SYM_COMMA;
    sym_tab[CAST_INT(';')] = Symbols::SYM_SEMICOLON;
    sym_tab[CAST_INT('-')] = Symbols::SYM_MINUS;
    sym_tab[CAST_INT('_')] = Symbols::SYM_UNDERSCORE;
    sym_tab[CAST_INT('\"')] = Symbols::SYM_QUOTATION;

    m_initialized = true;
}

int XLexer::GetToken(ScannerToken& stok)
{
    if (! m_initialized)
        Initialize();

    // update cursor position
    if (m_state.yylexem.size() > 0)
    {
        m_state.yycol += (int)m_state.yylexem.size();
    }

    // omit white characters and comments
    bool comment = false;
    do
    {
        m_state.yyinput->get(m_state.yychar);
        if (m_state.yyinput->fail())
            return 0;

        m_state.yycol++;

        if ( m_state.yychar == '#' || (m_state.yychar == '/' && m_state.yyinput->peek() == '/') )
            comment = true;

        if ( m_state.yychar == '\n' )
        {
            m_state.yyline++;
            m_state.yycol = 0;
            comment = false;
            continue;
        }

    } while ( isspace(m_state.yychar) || comment );
    m_state.yyinput->putback( m_state.yychar );
    m_state.yycol--;


    // read character and shift state
    bool process = true;
    m_state.yylexem.clear();
    int state = 0;
    int next_state = 0;
    EToken::TYPE token = EToken::TOKEN_UNKNOWN;
    EToken::TYPE next_token = EToken::TOKEN_UNKNOWN;
    do
    {
        // get next character
        m_state.yyinput->get( m_state.yychar );
        if ( m_state.yyinput->fail() )
            return 0;

        // could be in float or integer list
        if ( m_state.yychar == '\n' )
        {
            m_state.yyline++;
        }

        // append to current lexem (ignore control characters)
        if (m_state.yychar > 0x1f)
            m_state.yylexem.push_back(m_state.yychar);

        // get new state
        state = transition[state][ sym_tab[ CAST_INT(m_state.yychar) ] ];

        // get next state
        int nextch = m_state.yyinput->peek();
        next_state = transition[state][ sym_tab[ nextch ] ];

        token = state_token_map[state];
        next_token = state_token_map[next_state];
        if (token == EToken::TOKEN_UNKNOWN || next_token == EToken::TOKEN_UNKNOWN)
            process = false;
        else
        {
            if (token != next_token)
            {
//                if (ending_states[next_state])
//                    continue;
//                process = false;
            }
        }
    } while ( process );

    stok.token = token;
    stok.lexem = m_state.yylexem;
    stok.line = m_state.yyline;
    stok.col = m_state.yycol;

    // evaluate token on demand

    return stok.token;
}


}}
