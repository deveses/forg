#ifndef _FORG_NET_COMMAND_H_
#define _FORG_NET_COMMAND_H_

#include <map>
#include <string>

namespace forg::net {

/// A parsed control command: a verb plus raw string parameters.
/**
 * The net module knows nothing about rendering. A request such as
 * GET /camera/orbit?dx=0.1&dy=-0.2 becomes verb "camera.orbit" with
 * params {"dx":"0.1","dy":"-0.2"}. The application maps verbs to actions.
 */
struct Command
{
    std::string verb;
    std::map<std::string, std::string> params;
};

/// Reads a string parameter. Returns false if the key is missing.
bool TryGetString(const Command& cmd, const char* key, std::string& out);

/// Reads a float parameter. Returns false if missing or not a number.
bool TryGetFloat(const Command& cmd, const char* key, float& out);

/// Reads an int parameter. Returns false if missing or not a number.
bool TryGetInt(const Command& cmd, const char* key, int& out);

} // namespace forg::net

#endif //_FORG_NET_COMMAND_H_
