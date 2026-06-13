#include "net/Command.h"

#include <cstdlib>

namespace forg { namespace net {

bool TryGetString(const Command& cmd, const char* key, std::string& out)
{
    std::map<std::string, std::string>::const_iterator it = cmd.params.find(key);
    if (it == cmd.params.end())
    {
        return false;
    }
    out = it->second;
    return true;
}

bool TryGetFloat(const Command& cmd, const char* key, float& out)
{
    std::string s;
    if (!TryGetString(cmd, key, s))
    {
        return false;
    }

    const char* begin = s.c_str();
    char* end = 0;
    float value = std::strtof(begin, &end);
    if (end == begin)
    {
        return false; // not a number
    }
    out = value;
    return true;
}

bool TryGetInt(const Command& cmd, const char* key, int& out)
{
    std::string s;
    if (!TryGetString(cmd, key, s))
    {
        return false;
    }

    const char* begin = s.c_str();
    char* end = 0;
    long value = std::strtol(begin, &end, 10);
    if (end == begin)
    {
        return false; // not a number
    }
    out = static_cast<int>(value);
    return true;
}

}}
