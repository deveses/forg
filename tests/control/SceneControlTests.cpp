#include <catch2/catch_test_macros.hpp>

#include "forg/control/SceneControl.h"
#include "forg/Input.h"
#include "forg/net/HttpRequest.h"
#include "forg/rendering/reference/SWRenderDevice.h"

using forg::control::DispatchCommand;
using forg::control::SceneControlContext;
using forg::net::CommandFromRequest;

namespace {

// A mostly device-free scene: non-mesh commands leave device null, while mesh
// commands opt into a reference render device.
struct Scene
{
    forg::Camera camera;
    forg::Light light;
    forg::Color clearColor;
    forg::scene::Model model;
    forg::InputEvent lastInput = {forg::InputEventType::Scroll,
                                  forg::InputButton::None, 0.0f, 0.0f, 0.0f};
    int inputCount = 0;

    Scene() : light(), clearColor(0.0f, 0.0f, 0.0f) {}

    static bool RecordInput(const forg::InputEvent& event, void* userData)
    {
        Scene* scene = static_cast<Scene*>(userData);
        scene->lastInput = event;
        ++scene->inputCount;
        return true;
    }

    SceneControlContext context(forg::IRenderDevice* device = 0)
    {
        SceneControlContext ctx;
        ctx.camera = &camera;
        ctx.model = &model;
        ctx.light = &light;
        ctx.clearColor = &clearColor;
        ctx.device = device;
        ctx.inputHandler = &RecordInput;
        ctx.inputUserData = this;
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

TEST_CASE("input.drag dispatches a pointer drag input event",
          "[control][input]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r = DispatchCommand(
        ctx, CommandFromRequest("/input/drag", "button=left&dx=3&dy=-4"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.inputCount == 1);
    REQUIRE(scene.lastInput.Type == forg::InputEventType::PointerDrag);
    REQUIRE(scene.lastInput.Button == forg::InputButton::Left);
    REQUIRE(scene.lastInput.DeltaX == 3.0f);
    REQUIRE(scene.lastInput.DeltaY == -4.0f);
    REQUIRE(scene.lastInput.ScrollDelta == 0.0f);
}

TEST_CASE("input.scroll dispatches a scroll input event", "[control][input]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r =
        DispatchCommand(ctx, CommandFromRequest("/input/scroll", "delta=2"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.inputCount == 1);
    REQUIRE(scene.lastInput.Type == forg::InputEventType::Scroll);
    REQUIRE(scene.lastInput.Button == forg::InputButton::None);
    REQUIRE(scene.lastInput.DeltaX == 0.0f);
    REQUIRE(scene.lastInput.DeltaY == 0.0f);
    REQUIRE(scene.lastInput.ScrollDelta == 2.0f);
}

TEST_CASE("input commands reject missing or invalid parameters",
          "[control][input]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    REQUIRE(DispatchCommand(
                ctx, CommandFromRequest("/input/drag", "button=left&dx=3")) ==
            "{\"ok\":false,\"error\":\"badparam\"}");
    REQUIRE(DispatchCommand(ctx, CommandFromRequest("/input/drag",
                                                    "button=nope&dx=3&dy=4")) ==
            "{\"ok\":false,\"error\":\"badparam\"}");
    REQUIRE(DispatchCommand(ctx, CommandFromRequest("/input/scroll", "")) ==
            "{\"ok\":false,\"error\":\"badparam\"}");
    REQUIRE(scene.inputCount == 0);
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

TEST_CASE("mesh primitive commands create serializable model metadata",
          "[control]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    Scene scene;
    SceneControlContext ctx = scene.context(&device);

    std::string r = DispatchCommand(
        ctx, CommandFromRequest("/mesh/cylinder",
                                "r1=1&r2=2&length=5&slices=10&stacks=40"));

    REQUIRE(r == "{\"ok\":true}");
    REQUIRE(scene.model.MeshType() == forg::scene::ModelMeshType::Cylinder);
    REQUIRE(scene.model.MeshParams().Cylinder.Radius1 == 1.0f);
    REQUIRE(scene.model.MeshParams().Cylinder.Radius2 == 2.0f);
    REQUIRE(scene.model.MeshParams().Cylinder.Length == 5.0f);
    REQUIRE(scene.model.MeshParams().Cylinder.Slices == 10);
    REQUIRE(scene.model.MeshParams().Cylinder.Stacks == 40);
    REQUIRE(scene.model.SourcePath().length() == 0);
    REQUIRE(scene.model.IsLoaded());
    REQUIRE(scene.model.GetMesh() != nullptr);
    REQUIRE(scene.model.GetMesh()->GetNumVertices() > 0);
}

TEST_CASE("mesh primitive command fails without a render device", "[control]")
{
    Scene scene;
    SceneControlContext ctx = scene.context();

    std::string r = DispatchCommand(ctx, CommandFromRequest("/mesh/box", ""));

    REQUIRE(r == "{\"ok\":false,\"error\":\"loadfailed\"}");
    REQUIRE(scene.model.MeshType() == forg::scene::ModelMeshType::None);
    REQUIRE_FALSE(scene.model.IsLoaded());
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
