#ifndef _FORG_NET_HTTPREQUEST_H_
#define _FORG_NET_HTTPREQUEST_H_

#include <string>
#include <map>

#include "net/Command.h"

namespace forg { namespace net {

/// Parses an HTTP request line, e.g. "GET /camera/orbit?dx=0.1 HTTP/1.1".
/**
* Splits the request target into path and query. Returns false if the line
* does not contain at least a method and a target.
*/
bool ParseRequestLine(const std::string& line,
                      std::string& method,
                      std::string& path,
                      std::string& query);

/// Parses a query string ("a=1&b=2") into a key/value map.
/** Percent escapes (%XX) and '+' are decoded. */
std::map<std::string, std::string> ParseQuery(const std::string& query);

/// Builds a Command from a request path and query.
/** "/camera/orbit" becomes verb "camera.orbit"; the query becomes params. */
Command CommandFromRequest(const std::string& path, const std::string& query);

}}

#endif //_FORG_NET_HTTPREQUEST_H_
