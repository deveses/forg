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

TEST_CASE("BitArray grows when setting past the current length",
          "[core][bitarray]")
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

TEST_CASE("BitArray copy and assignment are independent", "[core][bitarray]")
{
    forg::core::BitArray original(4, false);
    original.Set(1);

    forg::core::BitArray copied(original);
    forg::core::BitArray assigned;
    assigned = original;

    original.Set(1, false);
    original.Set(3);

    REQUIRE(copied.Get(1));
    REQUIRE_FALSE(copied.Get(3));
    REQUIRE(assigned.Get(1));
    REQUIRE_FALSE(assigned.Get(3));
}

TEST_CASE("BitArray clears individual and all bits", "[core][bitarray]")
{
    forg::core::BitArray bits(6, true);

    bits.Set(2, false);

    REQUIRE(bits.Get(0));
    REQUIRE_FALSE(bits.Get(2));
    REQUIRE(bits.Get(5));

    bits.SetAll(false);

    for (forg::uint i = 0; i < bits.get_Length(); ++i)
        REQUIRE_FALSE(bits.Get(i));
}

TEST_CASE("BitArray Not flips only logical bits", "[core][bitarray]")
{
    forg::core::BitArray bits(5, false);
    bits.Set(1);
    bits.Set(4);

    forg::core::BitArray inverted = bits.Not();

    REQUIRE(inverted.Get(0));
    REQUIRE_FALSE(inverted.Get(1));
    REQUIRE(inverted.Get(2));
    REQUIRE(inverted.Get(3));
    REQUIRE_FALSE(inverted.Get(4));
    REQUIRE(inverted.ToString() == "01101");
}

TEST_CASE("BitArray compares equal across lengths when stored bits match",
          "[core][bitarray]")
{
    forg::core::BitArray shortBits(1, false);
    forg::core::BitArray longBits(2, false);

    REQUIRE(shortBits == longBits);

    longBits.Set(1);

    REQUIRE(shortBits != longBits);
}

TEST_CASE("BitArray growth initializes intermediate bits to false",
          "[core][bitarray]")
{
    forg::core::BitArray bits(1, false);

    bits.Set(40);

    REQUIRE(bits.get_Length() == 41);
    REQUIRE(bits.Get(40));

    for (forg::uint i = 1; i < 40; ++i)
        REQUIRE_FALSE(bits.Get(i));
}
