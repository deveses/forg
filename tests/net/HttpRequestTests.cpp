#include <catch2/catch_test_macros.hpp>

#include "forg/net/HttpRequest.h"

using forg::net::Command;
using forg::net::CommandFromRequest;
using forg::net::ParseQuery;
using forg::net::ParseRequestLine;

TEST_CASE("ParseRequestLine splits method, path and query", "[net][http]")
{
    std::string method, path, query;
    bool ok = ParseRequestLine("GET /camera/orbit?dx=0.1&dy=-0.2 HTTP/1.1",
                               method, path, query);

    REQUIRE(ok);
    REQUIRE(method == "GET");
    REQUIRE(path == "/camera/orbit");
    REQUIRE(query == "dx=0.1&dy=-0.2");
}

TEST_CASE("ParseRequestLine handles a target with no query", "[net][http]")
{
    std::string method, path, query;
    bool ok = ParseRequestLine("POST /state HTTP/1.1", method, path, query);

    REQUIRE(ok);
    REQUIRE(method == "POST");
    REQUIRE(path == "/state");
    REQUIRE(query.empty());
}

TEST_CASE("ParseRequestLine rejects a malformed line", "[net][http]")
{
    std::string method, path, query;
    REQUIRE_FALSE(ParseRequestLine("garbage", method, path, query));
}

TEST_CASE("ParseQuery decodes key/value pairs", "[net][http]")
{
    std::map<std::string, std::string> q = ParseQuery("dx=0.1&dy=-0.2");

    REQUIRE(q.size() == 2);
    REQUIRE(q["dx"] == "0.1");
    REQUIRE(q["dy"] == "-0.2");
}

TEST_CASE("ParseQuery percent- and plus-decodes values", "[net][http]")
{
    std::map<std::string, std::string> q =
        ParseQuery("path=a%2Fb.ply&name=hello+world");

    REQUIRE(q["path"] == "a/b.ply");
    REQUIRE(q["name"] == "hello world");
}

TEST_CASE("ParseQuery on an empty string yields no params", "[net][http]")
{
    REQUIRE(ParseQuery("").empty());
}

TEST_CASE("CommandFromRequest maps the path to a dotted verb", "[net][http]")
{
    Command cmd = CommandFromRequest("/camera/orbit", "dx=0.1&dy=-0.2");

    REQUIRE(cmd.verb == "camera.orbit");
    REQUIRE(cmd.params["dx"] == "0.1");
    REQUIRE(cmd.params["dy"] == "-0.2");
}

TEST_CASE("CommandFromRequest handles a single-segment path", "[net][http]")
{
    Command cmd = CommandFromRequest("/state", "");

    REQUIRE(cmd.verb == "state");
    REQUIRE(cmd.params.empty());
}

TEST_CASE("Command param helpers read typed values", "[net][command]")
{
    Command cmd =
        CommandFromRequest("/mesh/box", "w=1.5&slices=24&path=cube.ply");

    float w = 0.0f;
    REQUIRE(forg::net::TryGetFloat(cmd, "w", w));
    REQUIRE(w == 1.5f);

    int slices = 0;
    REQUIRE(forg::net::TryGetInt(cmd, "slices", slices));
    REQUIRE(slices == 24);

    std::string path;
    REQUIRE(forg::net::TryGetString(cmd, "path", path));
    REQUIRE(path == "cube.ply");
}

TEST_CASE("Command param helpers report a missing key", "[net][command]")
{
    Command cmd = CommandFromRequest("/camera/orbit", "dx=0.1");

    float dy = 99.0f;
    REQUIRE_FALSE(forg::net::TryGetFloat(cmd, "dy", dy));
    REQUIRE(dy == 99.0f); // left untouched
}
