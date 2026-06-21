#include <catch2/catch_test_macros.hpp>

#include "forg/control/SceneControl.h"
#include "forg/net/HttpRequest.h"

using forg::control::DispatchCommand;
using forg::control::SceneControlContext;
using forg::net::CommandFromRequest;

namespace {

// A device-free scene: enough to exercise every non-mesh-building command.
// Mesh creation/loading needs a real IRenderDevice and is covered by the
// app-level curl tests instead.
struct Scene
{
    forg::Camera camera;
    forg::Light light;
    forg::Color clearColor;
    forg::geometry::Mesh::MeshPtr mesh; // null
    forg::Matrix4 meshTm;

    Scene()
        : light(), clearColor(0.0f, 0.0f, 0.0f), meshTm(forg::Matrix4::Identity)
    {
    }

    SceneControlContext context()
    {
        SceneControlContext ctx;
        ctx.camera = &camera;
        ctx.mesh = &mesh;
        ctx.meshTm = &meshTm;
        ctx.light = &light;
        ctx.clearColor = &clearColor;
        ctx.device = 0;
        return ctx;
    }
};

} // namespace

TEST_CASE("camera.place sets absolute position and target", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r = DispatchCommand(
        ctx,
        CommandFromRequest("/camera/place", "px=1&py=2&pz=3&tx=0&ty=0&tz=0"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.camera.get_Position().X == 1.0f);
    REQUIRE(scene.camera.get_Position().Y == 2.0f);
    REQUIRE(scene.camera.get_Position().Z == 3.0f);
    REQUIRE(scene.camera.get_Target().X == 0.0f);
}

TEST_CASE("clear.color updates the clear color", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r = DispatchCommand(
        ctx, CommandFromRequest("/clear/color", "r=0.25&g=0.5&b=0.75"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.clearColor.r == 0.25f);
    REQUIRE(scene.clearColor.g == 0.5f);
    REQUIRE(scene.clearColor.b == 0.75f);
}

TEST_CASE("light.set updates only the supplied fields", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r = DispatchCommand(
        ctx, CommandFromRequest("/light/set", "px=3&dr=0.5&range=42"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.light.Position.X == 3.0f);
    REQUIRE(scene.light.Diffuse.r == 0.5f);
    REQUIRE(scene.light.Range == 42.0f);
}

TEST_CASE("state reports the current scene as JSON", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();
    DispatchCommand(ctx, CommandFromRequest("/camera/place",
                                            "px=1&py=2&pz=3&tx=0&ty=0&tz=0"));

    std::string state = DispatchCommand(ctx, CommandFromRequest("/state", ""));

    REQUIRE(state.find("\"ok\":true") != std::string::npos);
    REQUIRE(state.find("\"position\":[1,2,3]") != std::string::npos);
    REQUIRE(state.find("\"vertices\":0") != std::string::npos); // null mesh
}

TEST_CASE("an unknown verb is rejected", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r =
        DispatchCommand(ctx, CommandFromRequest("/does/not/exist", ""));
    REQUIRE(r == "{\"ok\":false,\"error\":\"unknown\"}");
}

TEST_CASE("camera.zoom without fov reports a bad parameter", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r =
        DispatchCommand(ctx, CommandFromRequest("/camera/zoom", ""));
    REQUIRE(r == "{\"ok\":false,\"error\":\"badparam\"}");
}
