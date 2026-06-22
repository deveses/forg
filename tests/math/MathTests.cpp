#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <limits>

#include "forg/math/Math.h"
#include "forg/math/Matrix4.h"
#include "forg/math/Quaternion.h"
#include "forg/math/Vector2.h"
#include "forg/math/Vector3.h"
#include "forg/math/Vector4.h"

using Catch::Approx;

TEST_CASE("Vector constructors initialize components", "[math][vector]")
{
    forg::math::Vector2 v2(1.0f, 2.0f);
    REQUIRE(v2.X == Approx(1.0f));
    REQUIRE(v2.Y == Approx(2.0f));

    forg::math::Vector3 v3(3.0f, 4.0f, 5.0f);
    REQUIRE(v3.X == Approx(3.0f));
    REQUIRE(v3.Y == Approx(4.0f));
    REQUIRE(v3.Z == Approx(5.0f));

    forg::math::Vector4 v4(6.0f, 7.0f, 8.0f, 9.0f);
    REQUIRE(v4.X == Approx(6.0f));
    REQUIRE(v4.Y == Approx(7.0f));
    REQUIRE(v4.Z == Approx(8.0f));
    REQUIRE(v4.W == Approx(9.0f));
}

TEST_CASE("Vector3 arithmetic computes dot, cross, and normalization",
          "[math][vector]")
{
    const forg::math::Vector3 a(1.0f, 2.0f, 3.0f);
    const forg::math::Vector3 b(4.0f, -5.0f, 6.0f);

    REQUIRE((a + b).X == Approx(5.0f));
    REQUIRE((a + b).Y == Approx(-3.0f));
    REQUIRE((a + b).Z == Approx(9.0f));
    REQUIRE(forg::math::Vector3::Dot(a, b) == Approx(12.0f));

    forg::math::Vector3 cross;
    forg::math::Vector3::Cross(cross, a, b);
    REQUIRE(cross.X == Approx(27.0f));
    REQUIRE(cross.Y == Approx(6.0f));
    REQUIRE(cross.Z == Approx(-13.0f));

    forg::math::Vector3 normalized;
    forg::math::Vector3::Normalize(normalized,
                                   forg::math::Vector3(0.0f, 3.0f, 4.0f));
    REQUIRE(normalized.X == Approx(0.0f));
    REQUIRE(normalized.Y == Approx(0.6f));
    REQUIRE(normalized.Z == Approx(0.8f));
}

TEST_CASE("Matrix4 identity and translation transform coordinates",
          "[math][matrix]")
{
    forg::math::Matrix4 identity;
    forg::math::Vector3 source(1.0f, 2.0f, 3.0f);
    forg::math::Vector3 transformed;

    forg::math::Vector3::TransformCoordinate(transformed, source, identity);
    REQUIRE(transformed.X == Approx(1.0f));
    REQUIRE(transformed.Y == Approx(2.0f));
    REQUIRE(transformed.Z == Approx(3.0f));

    forg::math::Matrix4 translation;
    forg::math::Matrix4::Translation(translation, 10.0f, 20.0f, 30.0f);
    forg::math::Vector3::TransformCoordinate(transformed, source, translation);
    REQUIRE(transformed.X == Approx(11.0f));
    REQUIRE(transformed.Y == Approx(22.0f));
    REQUIRE(transformed.Z == Approx(33.0f));
}

TEST_CASE("Quaternion normalization produces a unit quaternion",
          "[math][quaternion]")
{
    forg::math::Quaternion q(0.0f, 0.0f, 0.0f, 2.0f);
    forg::math::Quaternion normalized;

    forg::math::Quaternion::Normalize(normalized, q);

    REQUIRE(normalized.v.X == Approx(0.0f));
    REQUIRE(normalized.v.Y == Approx(0.0f));
    REQUIRE(normalized.v.Z == Approx(0.0f));
    REQUIRE(normalized.s == Approx(1.0f));
    REQUIRE(forg::math::Quaternion::Length(normalized) == Approx(1.0f));
}

TEST_CASE("Math bit helpers preserve boundary behavior", "[math][bit]")
{
    using forg::math::Math;

    STATIC_REQUIRE(Math::bit_sign(-1) == -1);
    STATIC_REQUIRE(Math::bit_sign(0) == 1);
    STATIC_REQUIRE(Math::bit_min(std::numeric_limits<int>::min(), 4) ==
                   std::numeric_limits<int>::min());
    STATIC_REQUIRE(Math::bit_max(std::numeric_limits<int>::max(), -4) ==
                   std::numeric_limits<int>::max());
    STATIC_REQUIRE(Math::bit_avarage(std::numeric_limits<int>::min(),
                                     std::numeric_limits<int>::max()) == 0);
    STATIC_REQUIRE_FALSE(Math::is_pow2(0));
    STATIC_REQUIRE(Math::is_pow2(1024));
    STATIC_REQUIRE(Math::count_bits_set(0xf0f0U) == 8);
    STATIC_REQUIRE(Math::bit_log2(0) == 0);
    STATIC_REQUIRE(Math::bit_log2(1024) == 10);
    STATIC_REQUIRE(Math::count_zeros_trail(0) == 32);
    STATIC_REQUIRE(Math::first_bit_num(0x40U) == 6);
    STATIC_REQUIRE(Math::next_pow2(0) == 0);
    STATIC_REQUIRE(Math::next_pow2(17) == 32);
}
