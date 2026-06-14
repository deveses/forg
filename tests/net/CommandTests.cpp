#include <catch2/catch_test_macros.hpp>

#include "forg/net/Command.h"

using forg::net::Command;
using forg::net::TryGetFloat;
using forg::net::TryGetInt;

namespace
{
Command withParam(const char* key, const char* value)
{
    Command cmd;
    cmd.params[key] = value;
    return cmd;
}
}

TEST_CASE("TryGetInt parses a plain integer", "[net][command]")
{
    int out = 0;
    REQUIRE(TryGetInt(withParam("n", "42"), "n", out));
    REQUIRE(out == 42);
}

TEST_CASE("TryGetInt rejects values outside int range", "[net][command]")
{
    int out = 123;
    REQUIRE_FALSE(TryGetInt(withParam("n", "99999999999"), "n", out));
    REQUIRE(out == 123); // left untouched on rejection
}

TEST_CASE("TryGetInt rejects non-numeric input", "[net][command]")
{
    int out = 7;
    REQUIRE_FALSE(TryGetInt(withParam("n", "abc"), "n", out));
    REQUIRE(out == 7);
}

TEST_CASE("TryGetFloat parses a finite float", "[net][command]")
{
    float out = 0.0f;
    REQUIRE(TryGetFloat(withParam("x", "1.5"), "x", out));
    REQUIRE(out == 1.5f);
}

TEST_CASE("TryGetFloat rejects nan and inf literals", "[net][command]")
{
    float out = 7.0f;
    REQUIRE_FALSE(TryGetFloat(withParam("x", "nan"), "x", out));
    REQUIRE_FALSE(TryGetFloat(withParam("x", "inf"), "x", out));
    REQUIRE(out == 7.0f); // left untouched on rejection
}

TEST_CASE("TryGetFloat rejects overflow to infinity", "[net][command]")
{
    float out = 7.0f;
    REQUIRE_FALSE(TryGetFloat(withParam("x", "1e400"), "x", out));
    REQUIRE(out == 7.0f);
}
