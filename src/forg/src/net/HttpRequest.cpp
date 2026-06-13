#include "net/HttpRequest.h"

#include <sstream>

namespace forg { namespace net {

static int hexDigit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string urlDecode(const std::string& s)
{
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        if (c == '+')
        {
            out += ' ';
        }
        else if (c == '%' && i + 2 < s.size())
        {
            int hi = hexDigit(s[i + 1]);
            int lo = hexDigit(s[i + 2]);
            if (hi >= 0 && lo >= 0)
            {
                out += static_cast<char>((hi << 4) | lo);
                i += 2;
            }
            else
            {
                out += c;
            }
        }
        else
        {
            out += c;
        }
    }

    return out;
}

bool ParseRequestLine(const std::string& line,
                      std::string& method,
                      std::string& path,
                      std::string& query)
{
    std::istringstream iss(line);
    std::string target;
    if (!(iss >> method >> target))
    {
        return false;
    }

    size_t q = target.find('?');
    if (q == std::string::npos)
    {
        path = target;
        query.clear();
    }
    else
    {
        path = target.substr(0, q);
        query = target.substr(q + 1);
    }

    return true;
}

std::map<std::string, std::string> ParseQuery(const std::string& query)
{
    std::map<std::string, std::string> out;

    size_t start = 0;
    while (start < query.size())
    {
        size_t amp = query.find('&', start);
        std::string pair = (amp == std::string::npos)
                         ? query.substr(start)
                         : query.substr(start, amp - start);

        if (!pair.empty())
        {
            size_t eq = pair.find('=');
            if (eq == std::string::npos)
            {
                out[urlDecode(pair)] = "";
            }
            else
            {
                out[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
            }
        }

        if (amp == std::string::npos)
        {
            break;
        }
        start = amp + 1;
    }

    return out;
}

Command CommandFromRequest(const std::string& path, const std::string& query)
{
    Command cmd;

    std::string verb = path;
    if (!verb.empty() && verb[0] == '/')
    {
        verb.erase(0, 1);
    }
    for (size_t i = 0; i < verb.size(); ++i)
    {
        if (verb[i] == '/')
        {
            verb[i] = '.';
        }
    }

    cmd.verb = verb;
    cmd.params = ParseQuery(query);
    return cmd;
}

}}
