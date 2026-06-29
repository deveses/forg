#include <catch2/catch_test_macros.hpp>

#include "forg/scene/Scene.h"
#include "forg/script/yaml/YAMLSerializer.h"
#include "forg/ui/gui.h"

#include <string>

namespace {

std::string DataPath(const char* filename)
{
    return std::string(FORG_TEST_REPO_DATA_DIR) + "/" + filename;
}

forg::ui::GuiNode* GuiRoot(forg::scene::Scene& scene)
{
    return dynamic_cast<forg::ui::GuiNode*>(scene.Node(0));
}

} // namespace

TEST_CASE("GuiNode scene loads controls from dialog YAML", "[ui][yaml]")
{
    forg::io::YAMLSerializer serializer;
    REQUIRE(serializer.OpenRead(DataPath("ui/dialog.yml")));

    forg::scene::Scene scene;
    REQUIRE(scene.Load(serializer));

    REQUIRE(scene.NodeCount() == 5);
    forg::ui::GuiNode* root = GuiRoot(scene);
    REQUIRE(root != nullptr);
    REQUIRE(root->ControlType() == forg::ui::GuiControlType::Container);
    REQUIRE(root->TexturePath() == "data/ui/debug_texture2.dds");

    REQUIRE(root->FindById(0) != nullptr);
    REQUIRE(root->FindById(1) != nullptr);
    forg::ui::GuiNode* knob = root->FindById(2);
    REQUIRE(knob != nullptr);
    REQUIRE(knob->ControlType() == forg::ui::GuiControlType::Knob);
    REQUIRE(root->FindById(3) != nullptr);
}

TEST_CASE("GuiNode bounds are parent-relative", "[ui][scene]")
{
    forg::scene::Scene scene;
    forg::ui::GuiNode& root = scene.CreateGuiNode();
    root.SetBounds(10, 20, 100, 100);

    forg::ui::GuiNode& child = scene.CreateGuiNode();
    child.SetBounds(5, 6, 30, 40);
    REQUIRE(root.AddChild(child));

    const forg::Rectangle bounds = child.AbsoluteBounds();
    REQUIRE(bounds.left == 15);
    REQUIRE(bounds.top == 26);
    REQUIRE(bounds.right == 45);
    REQUIRE(bounds.bottom == 66);
}

TEST_CASE("GuiNode hit testing returns deepest matching child", "[ui]")
{
    forg::scene::Scene scene;
    forg::ui::GuiNode& root = scene.CreateGuiNode();
    root.SetId(1);
    root.SetBounds(10, 20, 100, 100);

    forg::ui::GuiNode& child = scene.CreateGuiNode();
    child.SetId(2);
    child.SetBounds(5, 6, 30, 40);
    REQUIRE(root.AddChild(child));

    REQUIRE(root.FindAtPoint({15, 26}) == &child);
    REQUIRE(root.FindAtPoint({45, 66}) == &child);
    REQUIRE(root.FindAtPoint({11, 21}) == &root);
    REQUIRE(root.FindAtPoint({9, 30}) == nullptr);
    REQUIRE(root.FindAtPoint({20, 19}) == nullptr);
}

TEST_CASE("GuiNode round-trips through scene YAML", "[ui][yaml]")
{
    forg::scene::Scene source;
    forg::ui::GuiNode& root = source.CreateGuiNode();
    root.SetId(-1);
    root.SetControlType(forg::ui::GuiControlType::Container);
    root.SetTexturePath("data/ui/debug_texture2.dds");

    forg::ui::GuiNode& knob = source.CreateGuiNode();
    knob.SetId(2);
    knob.SetControlType(forg::ui::GuiControlType::Knob);
    knob.SetBounds(3, 4, 30, 30);
    knob.SetRange(10, 20);
    knob.SetValue(17);
    REQUIRE(root.AddChild(knob));

    forg::io::YAMLSerializer writer;
    REQUIRE(source.Save(writer));

    std::string text;
    REQUIRE(writer.SaveToString(text));
    REQUIRE(text.find("type: \"GuiNode\"") != std::string::npos);
    REQUIRE(text.find("control_type: \"knob\"") != std::string::npos);

    forg::io::YAMLSerializer reader;
    REQUIRE(reader.LoadFromString(text));

    forg::scene::Scene target;
    REQUIRE(target.Load(reader));

    forg::ui::GuiNode* loadedRoot = GuiRoot(target);
    REQUIRE(loadedRoot != nullptr);
    forg::ui::GuiNode* loadedKnob = loadedRoot->FindById(2);
    REQUIRE(loadedKnob != nullptr);
    REQUIRE(loadedKnob->ControlType() == forg::ui::GuiControlType::Knob);
    REQUIRE(loadedKnob->Value() == 17);
    REQUIRE(loadedKnob->Min() == 10);
    REQUIRE(loadedKnob->Max() == 20);
}
