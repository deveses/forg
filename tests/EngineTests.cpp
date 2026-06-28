#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "forg/Engine.h"
#include "forg/Input.h"
#include "forg/math/Vector3.h"
#include "forg/rendering/Camera.h"
#include "forg/scene/Scene.h"
#include "forg/scene/SceneNode.h"

#include <filesystem>
#include <fstream>
#include <string>

using Catch::Approx;

namespace {

class CountingSceneNode : public forg::scene::SceneNode
{
  public:
    int updates = 0;
    double lastDelta = 0.0;

    void Update(double deltaSeconds) override
    {
        ++updates;
        lastDelta = deltaSeconds;
        SceneNode::Update(deltaSeconds);
    }
};

std::filesystem::path TestConfigPath(const char* name)
{
    return std::filesystem::temp_directory_path() / name;
}

void WriteText(const std::filesystem::path& path, const char* text)
{
    std::ofstream file(path);
    file << text;
}

} // namespace

TEST_CASE("Engine starts empty and owns an empty scene", "[engine]")
{
    forg::Engine engine;

    REQUIRE(engine.Device() == nullptr);
    REQUIRE(engine.Renderer() == nullptr);
    REQUIRE(engine.Scene().NodeCount() == 0);
    REQUIRE(engine.Config().RendererDriver.empty());
    REQUIRE(engine.Config().BackBufferWidth == 100);
    REQUIRE(engine.Config().BackBufferHeight == 100);
    REQUIRE(std::string(engine.LastError()).empty());
}

TEST_CASE("Engine owns a stable active camera", "[engine]")
{
    forg::Engine engine;

    forg::Camera& camera = engine.Camera();
    camera.set_ScreenSize(320.0f, 200.0f);

    REQUIRE(&engine.Camera() == &camera);
    REQUIRE(engine.Camera().get_ScreenWidth() == 320.0f);
    REQUIRE(engine.Camera().get_ScreenHeight() == 200.0f);
}

TEST_CASE("Engine handles left-drag input as camera orbit", "[engine][input]")
{
    forg::Engine engine;

    const forg::math::Vector3 before = engine.Camera().get_Position();

    REQUIRE(engine.HandleInput({forg::InputEventType::PointerDrag,
                                forg::InputButton::Left, 10.0f, 0.0f,
                                0.0f}));

    REQUIRE(engine.Camera().get_Position().X != Approx(before.X));
    REQUIRE(engine.Camera().get_Target().X == Approx(0.0f));
    REQUIRE(std::string(engine.LastError()).empty());
}

TEST_CASE("Engine handles right-drag input as camera truck", "[engine][input]")
{
    forg::Engine engine;

    REQUIRE(engine.HandleInput({forg::InputEventType::PointerDrag,
                                forg::InputButton::Right, 10.0f, 20.0f,
                                0.0f}));

    REQUIRE(engine.Camera().get_Position().X == Approx(-0.1f));
    REQUIRE(engine.Camera().get_Position().Y == Approx(0.2f));
    REQUIRE(engine.Camera().get_Target().X == Approx(-0.1f));
    REQUIRE(engine.Camera().get_Target().Y == Approx(0.2f));
    REQUIRE(std::string(engine.LastError()).empty());
}

TEST_CASE("Engine handles scroll input as camera zoom", "[engine][input]")
{
    forg::Engine engine;

    REQUIRE(engine.HandleInput({forg::InputEventType::Scroll,
                                forg::InputButton::None, 0.0f, 0.0f, 1.0f}));

    REQUIRE(engine.Camera().get_Position().Z == Approx(4.7f));
    REQUIRE(engine.Camera().get_Target().Z == Approx(0.0f));
    REQUIRE(std::string(engine.LastError()).empty());
}

TEST_CASE("Engine rejects unsupported input combinations", "[engine][input]")
{
    forg::Engine engine;

    REQUIRE_FALSE(engine.HandleInput({forg::InputEventType::PointerDrag,
                                      forg::InputButton::Middle, 1.0f, 1.0f,
                                      0.0f}));

    REQUIRE(std::string(engine.LastError()).find("Unsupported input") !=
            std::string::npos);
}

TEST_CASE("Engine LoadConfig reads renderer driver and window size", "[engine]")
{
    const std::filesystem::path path = TestConfigPath("forg-engine-valid.yml");
    WriteText(path, "config:\n"
                    "  renderer:\n"
                    "    driver: libtestrenderer.dylib\n"
                    "  window:\n"
                    "    width: 640\n"
                    "    height: 480\n");

    forg::Engine engine;
    REQUIRE(engine.LoadConfig(path.string().c_str()));

    REQUIRE(engine.Config().RendererDriver == "libtestrenderer.dylib");
    REQUIRE(engine.Config().BackBufferWidth == 640);
    REQUIRE(engine.Config().BackBufferHeight == 480);
    REQUIRE(std::string(engine.LastError()).empty());

    std::filesystem::remove(path);
}

TEST_CASE("Engine frame methods require initialization", "[engine]")
{
    bool callbackCalled = false;

    forg::Engine engine;
    engine.SetUpdateCallback(
        [](forg::Engine&, double, void* userData)
        {
            *static_cast<bool*>(userData) = true;
            return true;
        },
        &callbackCalled);

    REQUIRE_FALSE(engine.Update(1.0 / 60.0));
    REQUIRE_FALSE(callbackCalled);
    REQUIRE(std::string(engine.LastError()).find("initialized") !=
            std::string::npos);

    REQUIRE_FALSE(engine.Render());
    REQUIRE(std::string(engine.LastError()).find("initialized") !=
            std::string::npos);

    REQUIRE_FALSE(engine.Frame());
    REQUIRE(std::string(engine.LastError()).find("initialized") !=
            std::string::npos);

    engine.Resize(640, 480);
    REQUIRE(std::string(engine.LastError()).find("initialized") !=
            std::string::npos);
    REQUIRE(engine.FrameStats().FrameIndex == 0);
}

TEST_CASE("Scene update traverses scene nodes", "[engine][scene]")
{
    forg::scene::Scene scene;
    CountingSceneNode parent;
    CountingSceneNode child;

    REQUIRE(scene.AddChild(parent));
    REQUIRE(parent.AddChild(child));

    scene.Update(0.25);

    REQUIRE(parent.updates == 1);
    REQUIRE(parent.lastDelta == 0.25);
    REQUIRE(child.updates == 1);
    REQUIRE(child.lastDelta == 0.25);
}

TEST_CASE("Engine LoadConfig reports missing and invalid configs", "[engine]")
{
    forg::Engine engine;

    REQUIRE_FALSE(engine.LoadConfig("missing-engine-config.yml"));
    REQUIRE_FALSE(std::string(engine.LastError()).empty());

    const std::filesystem::path path =
        TestConfigPath("forg-engine-invalid.yml");
    WriteText(path, "config:\n"
                    "  renderer:\n"
                    "    driver: libtestrenderer.dylib\n"
                    "  window:\n"
                    "    width: nope\n"
                    "    height: 480\n");

    REQUIRE_FALSE(engine.LoadConfig(path.string().c_str()));
    REQUIRE(std::string(engine.LastError()).find("window.width") !=
            std::string::npos);

    std::filesystem::remove(path);
}

TEST_CASE("Engine Initialize requires config and cleans up missing plugin",
          "[engine]")
{
    forg::Engine engine;

    REQUIRE_FALSE(engine.Initialize(nullptr));
    REQUIRE(engine.Device() == nullptr);
    REQUIRE(engine.Renderer() == nullptr);
    REQUIRE(std::string(engine.LastError()).find("config") !=
            std::string::npos);

    const std::filesystem::path path =
        TestConfigPath("forg-engine-missing-plugin.yml");
    WriteText(path, "config:\n"
                    "  renderer:\n"
                    "    driver: missing-renderer-plugin.dylib\n"
                    "  window:\n"
                    "    width: 320\n"
                    "    height: 200\n");

    REQUIRE(engine.LoadConfig(path.string().c_str()));
    REQUIRE_FALSE(engine.Initialize(nullptr));
    REQUIRE(engine.Device() == nullptr);
    REQUIRE(engine.Renderer() == nullptr);
    REQUIRE(std::string(engine.LastError()).find("Unable to load renderer") !=
            std::string::npos);

    std::filesystem::remove(path);
}
