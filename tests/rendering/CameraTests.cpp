#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "forg/math/Matrix4.h"
#include "forg/math/Vector3.h"
#include "forg/rendering/Camera.h"

using Catch::Approx;
using forg::Camera;
using forg::math::Matrix4;
using forg::math::Vector3;

namespace {
void RequireMatricesEqual(const Matrix4& a, const Matrix4& b)
{
    const float* pa = (const float*)a;
    const float* pb = (const float*)b;
    for (int i = 0; i < 16; ++i)
    {
        REQUIRE(pa[i] == Approx(pb[i]));
    }
}
} // namespace

TEST_CASE("Camera computes its view matrix at construction",
          "[rendering][camera]")
{
    // Regression: the cached view matrix used to be left uninitialized until
    // the first movement op ran, so the first rendered frames used garbage.
    // After construction it must already equal the default look-at transform.
    Camera camera;

    Matrix4 view;
    camera.GetViewMatrix(view);

    Matrix4 expected;
    Matrix4::LookAtRH(expected, Vector3(0.0f, 0.0f, 5.0f),
                      Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

    RequireMatricesEqual(view, expected);
}

TEST_CASE("Camera computes its projection matrix at construction",
          "[rendering][camera]")
{
    Camera camera;

    Matrix4 proj;
    camera.GetProjectionMatrix(proj);

    const float* p = (const float*)proj;
    for (int i = 0; i < 16; ++i)
    {
        REQUIRE(std::isfinite(p[i]));
    }

    Matrix4 expected;
    Matrix4::PerspectiveFovRH(expected, camera.get_FOV(), camera.get_Aspect(),
                              camera.get_NearRange(), camera.get_FarRange());

    RequireMatricesEqual(proj, expected);
}

TEST_CASE("Camera supports orthographic projection mode", "[rendering][camera]")
{
    Camera camera;
    camera.set_View(forg::Orthogonal);

    Matrix4 proj;
    camera.GetProjectionMatrix(proj);

    const float distance =
        (camera.get_Target() - camera.get_Position()).Length();
    const float height = 2.0f * distance * std::tan(camera.get_FOV() / 2.0f);

    Matrix4 expected;
    Matrix4::OrthoRH(expected, height * camera.get_Aspect(), height,
                     camera.get_NearRange(), camera.get_FarRange());

    RequireMatricesEqual(proj, expected);
}
