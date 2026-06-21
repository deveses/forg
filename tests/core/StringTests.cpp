#include "forg/core/string.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("core string copies and moves null-terminated content", "[core][string]")
{
    forg::core::string original("wood.png");
    forg::core::string copy(original);
    forg::core::string moved(std::move(copy));

    REQUIRE(std::string(original.c_str()) == "wood.png");
    REQUIRE(std::string(moved.c_str()) == "wood.png");
    REQUIRE(std::string(copy.c_str()).empty());
}

TEST_CASE("core string resize and clear preserve a terminator", "[core][string]")
{
    forg::core::string value;
    value.resize(4);
    value[0] = 't';
    value[1] = 'e';
    value[2] = 's';
    value[3] = 't';
    REQUIRE(std::string(value.c_str()) == "test");

    value.clear();
    REQUIRE(std::string(value.c_str()).empty());
}
