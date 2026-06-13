#include <catch2/catch_test_macros.hpp>

#include "forg/core/BitArray.h"

TEST_CASE("BitArray tracks length and individual bits", "[core][bitarray]")
{
    forg::core::BitArray bits(8, false);
    REQUIRE(bits.ToString().length() == 8);
    REQUIRE_FALSE(bits.Get(0));

    bits.Set(0);
    bits.Set(7);

    REQUIRE(bits.Get(0));
    REQUIRE(bits.Get(7));
    REQUIRE_FALSE(bits.Get(3));
    REQUIRE(bits.ToString() == "10000001");
}

TEST_CASE("BitArray grows when setting past the current length", "[core][bitarray]")
{
    forg::core::BitArray bits(2, false);

    bits.Set(10);

    REQUIRE(bits.ToString().length() == 11);
    REQUIRE(bits.Get(10));
    REQUIRE_FALSE(bits.Get(9));
}

TEST_CASE("BitArray combines values with bitwise operators", "[core][bitarray]")
{
    forg::core::BitArray left(4, false);
    left.Set(0);
    left.Set(2);

    forg::core::BitArray right(4, false);
    right.Set(1);
    right.Set(2);

    forg::core::BitArray intersection = left & right;
    REQUIRE_FALSE(intersection.Get(0));
    REQUIRE_FALSE(intersection.Get(1));
    REQUIRE(intersection.Get(2));
    REQUIRE_FALSE(intersection.Get(3));

    forg::core::BitArray combined = left | right;
    REQUIRE(combined.Get(0));
    REQUIRE(combined.Get(1));
    REQUIRE(combined.Get(2));
    REQUIRE_FALSE(combined.Get(3));

    forg::core::BitArray either = left ^ right;
    REQUIRE(either.Get(0));
    REQUIRE(either.Get(1));
    REQUIRE_FALSE(either.Get(2));
    REQUIRE_FALSE(either.Get(3));
}
