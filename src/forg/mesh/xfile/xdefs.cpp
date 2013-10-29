#include "forg_pch.h"

#include "mesh/xfile/xdefs.h"

namespace forg { namespace xfile {

    const xguid xguid::Empty = {
        0,
        0,
        0,
        {0,0,0,0,0,0,0,0}
    };

const char* GetPrimitiveTypeName(int type)
{
    switch(type) {
    case EToken::TOKEN_WORD:
    return "WORD";
    case EToken::TOKEN_DWORD:
    return "DWORD";
    case EToken::TOKEN_FLOAT:
    return "FLOAT";
    case EToken::TOKEN_DOUBLE:
    return "DOUBLE";
    case EToken::TOKEN_CHAR:
    return "CHAR";
    case EToken::TOKEN_UCHAR:
    return "UCHAR";
    case EToken::TOKEN_SWORD:
    return "SWORD";
    case EToken::TOKEN_SDWORD:
    return "SDWORD";
    case EToken::TOKEN_LPSTR:
    return "LPSTR";
    case EToken::TOKEN_UNICODE:
    return "UNICODE";
    case EToken::TOKEN_CSTRING:
    return "CSTRING";
    }

    return "UNKNOWN_TYPE";
}

xstring xguid::ToString() const
{
    char buff[64];

    sprintf(buff, "%lx-%x-%x-%x%x-%x%x%x%x%x%x", Data1, Data2, Data3,
        Data4[0], Data4[1],
        Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]
    );


    return buff;
}

bool operator == (const xguid& left, const xguid& right)
{
    return (0 == memcmp(&left, &right, sizeof(xguid)));
}

bool operator < (const xguid& left, const xguid& right)
{

    if (left.Data1 != right.Data1)
        return left.Data1 < right.Data1;

    if (left.Data2 != right.Data2)
        return left.Data2 < right.Data2;

    if (left.Data3 != right.Data3)
        return left.Data3 < right.Data3;

    for (int i=0; i<8; i++)
    {
        if (left.Data4[i] != right.Data4[i])
            return left.Data4[i] < right.Data4[i];
    }

    return false;
}

}}
