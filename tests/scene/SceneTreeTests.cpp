#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#include "forg/io/MemorySerializer.h"
#include "forg/scene/Scene.h"
#include "forg/scene/MeshNode.h"
#include "forg/scene/TreeNode.h"
#include "forg/script/yaml/YAMLSerializer.h"
#include "forg/rendering/reference/SWRenderDevice.h"
#include "forg/ui/gui.h"

namespace {

void RequireSerializedMixedScene(const forg::scene::Scene& target)
{
    REQUIRE(target.NodeCount() == 4);
    REQUIRE(target.Node(0)->Parent() == &target);
    REQUIRE(target.Node(1)->Parent() == target.Node(0));
    REQUIRE(target.Node(2)->Parent() == target.Node(1));
    REQUIRE(target.Node(3)->Parent() == target.Node(0));
    REQUIRE(dynamic_cast<const forg::scene::MeshNode*>(target.Node(0)) ==
            nullptr);
    const forg::scene::MeshNode* loadedMesh =
        dynamic_cast<const forg::scene::MeshNode*>(target.Node(1));
    REQUIRE(loadedMesh != nullptr);
    REQUIRE(dynamic_cast<const forg::scene::MeshNode*>(target.Node(2)) ==
            nullptr);
    const forg::ui::GuiNode* loadedGui =
        dynamic_cast<const forg::ui::GuiNode*>(target.Node(3));
    REQUIRE(loadedGui != nullptr);
    REQUIRE(loadedGui->Id() == 9);
    REQUIRE(loadedGui->ControlType() == forg::ui::GuiControlType::Button);

    REQUIRE(loadedMesh->GetModel().SourcePath() == "assets/triangle.gltf");
    REQUIRE(loadedMesh->GetModel().LoadOptions() == 17);
    REQUIRE_FALSE(loadedMesh->GetModel().IsLoaded());
    REQUIRE(loadedMesh->GetModel().GetTransform().M41 == 2.0f);
    REQUIRE(loadedMesh->GetModel().GetTransform().M42 == 3.0f);
    REQUIRE(loadedMesh->GetModel().GetTransform().M43 == 4.0f);
}

void PopulateSerializedMixedScene(forg::scene::Scene& source)
{
    forg::scene::SceneNode& root = source.CreateNode();
    forg::scene::MeshNode& mesh = source.CreateMeshNode();
    forg::scene::SceneNode& child = source.CreateNode();
    forg::ui::GuiNode& gui = source.CreateGuiNode();
    REQUIRE(root.AddChild(mesh));
    REQUIRE(mesh.AddChild(child));
    REQUIRE(root.AddChild(gui));

    forg::Matrix4 transform = forg::Matrix4::Identity;
    transform.M41 = 2.0f;
    transform.M42 = 3.0f;
    transform.M43 = 4.0f;
    mesh.GetModel().SetSource("assets/triangle.gltf", 17);
    mesh.GetModel().SetTransform(transform);

    gui.SetId(9);
    gui.SetControlType(forg::ui::GuiControlType::Button);
    gui.SetBounds(10, 20, 30, 40);
}

} // namespace

TEST_CASE("TreeNode tracks parent and children", "[scene][tree]")
{
    forg::scene::TreeNode parent;
    forg::scene::TreeNode child;

    REQUIRE(parent.IsRoot());
    REQUIRE(child.IsRoot());
    REQUIRE(parent.AddChild(child));

    REQUIRE_FALSE(child.IsRoot());
    REQUIRE(child.Parent() == &parent);
    REQUIRE(parent.ChildCount() == 1);
    REQUIRE(parent.Child(0) == &child);
    REQUIRE(parent.Child(1) == nullptr);

    REQUIRE(parent.RemoveChild(child));
    REQUIRE(child.IsRoot());
    REQUIRE(parent.ChildCount() == 0);
    REQUIRE_FALSE(parent.RemoveChild(child));
}

TEST_CASE("TreeNode reparents and rejects duplicate or cyclic children",
          "[scene][tree]")
{
    forg::scene::TreeNode parent;
    forg::scene::TreeNode otherParent;
    forg::scene::TreeNode child;
    forg::scene::TreeNode grandchild;

    REQUIRE(parent.AddChild(child));
    REQUIRE_FALSE(parent.AddChild(child));
    REQUIRE_FALSE(child.AddChild(child));

    REQUIRE(otherParent.AddChild(child));
    REQUIRE(parent.ChildCount() == 0);
    REQUIRE(otherParent.ChildCount() == 1);
    REQUIRE(child.Parent() == &otherParent);

    REQUIRE(child.AddChild(grandchild));
    REQUIRE_FALSE(grandchild.AddChild(child));
    REQUIRE(child.Parent() == &otherParent);
    REQUIRE(grandchild.Parent() == &child);
}

TEST_CASE("TreeNode RemoveFromParent and ClearChildren detach links",
          "[scene][tree]")
{
    forg::scene::TreeNode parent;
    forg::scene::TreeNode first;
    forg::scene::TreeNode second;

    REQUIRE(parent.AddChild(first));
    REQUIRE(parent.AddChild(second));

    first.RemoveFromParent();
    REQUIRE(first.Parent() == nullptr);
    REQUIRE(parent.ChildCount() == 1);
    REQUIRE(parent.Child(0) == &second);

    parent.ClearChildren();
    REQUIRE(parent.ChildCount() == 0);
    REQUIRE(second.Parent() == nullptr);
}

TEST_CASE("Scene creates and owns root scene nodes", "[scene][tree]")
{
    forg::scene::Scene scene;

    forg::scene::SceneNode& first = scene.CreateNode();
    forg::scene::SceneNode& second = scene.CreateNode();

    REQUIRE(scene.NodeCount() == 2);
    REQUIRE(scene.Node(0) == &first);
    REQUIRE(scene.Node(1) == &second);
    REQUIRE(scene.Node(2) == nullptr);
    REQUIRE(first.Parent() == &scene);
    REQUIRE(second.Parent() == &scene);
    REQUIRE(scene.ChildCount() == 2);
}

TEST_CASE("Scene creates and owns mesh nodes alongside generic nodes",
          "[scene][tree]")
{
    forg::scene::Scene scene;

    forg::scene::SceneNode& generic = scene.CreateNode();
    forg::scene::MeshNode& mesh = scene.CreateMeshNode();

    REQUIRE(scene.NodeCount() == 2);
    REQUIRE(scene.Node(0) == &generic);
    REQUIRE(scene.Node(1) == &mesh);
    REQUIRE(generic.Parent() == &scene);
    REQUIRE(mesh.Parent() == &scene);
    REQUIRE(dynamic_cast<forg::scene::MeshNode*>(scene.Node(0)) == nullptr);
    REQUIRE(dynamic_cast<forg::scene::MeshNode*>(scene.Node(1)) == &mesh);
}

TEST_CASE("Scene destroys a node and reparents its descendants",
          "[scene][tree]")
{
    forg::scene::Scene scene;
    forg::scene::SceneNode& parent = scene.CreateNode();
    forg::scene::SceneNode& child = scene.CreateNode();
    REQUIRE(parent.AddChild(child));

    REQUIRE(scene.DestroyNode(parent));

    REQUIRE(scene.NodeCount() == 1);
    REQUIRE(scene.Node(0) == &child);
    REQUIRE(child.Parent() == &scene);
    REQUIRE(scene.ChildCount() == 1);
    REQUIRE(scene.Child(0) == &child);
    REQUIRE_FALSE(scene.DestroyNode(parent));
}

TEST_CASE("Scene ClearNodes detaches and releases all owned nodes",
          "[scene][tree]")
{
    forg::scene::Scene scene;
    forg::scene::SceneNode& first = scene.CreateNode();
    forg::scene::SceneNode& second = scene.CreateNode();
    REQUIRE(first.AddChild(second));

    scene.ClearNodes();

    REQUIRE(scene.NodeCount() == 0);
    REQUIRE(scene.ChildCount() == 0);
}

TEST_CASE("MeshNode exposes its embedded Model", "[scene][tree]")
{
    forg::scene::MeshNode node;
    const forg::scene::MeshNode& constNode = node;

    REQUIRE(&node.GetModel() == &constNode.GetModel());
    REQUIRE_FALSE(node.GetModel().IsLoaded());
}

TEST_CASE("Scene round-trips empty scenes through a memory serializer",
          "[scene][serialization]")
{
    forg::scene::Scene source;
    forg::io::MemorySerializer serializer;

    REQUIRE(source.Save(serializer));
    REQUIRE(serializer.ResetReading());

    forg::scene::Scene target;
    REQUIRE(target.Load(serializer));
    REQUIRE(target.NodeCount() == 0);
    REQUIRE(target.ChildCount() == 0);
}

TEST_CASE("Scene round-trips mixed node hierarchies through memory",
          "[scene][serialization]")
{
    forg::scene::Scene source;
    PopulateSerializedMixedScene(source);

    forg::io::MemorySerializer serializer;
    REQUIRE(source.Save(serializer));
    REQUIRE(serializer.ResetReading());

    forg::scene::Scene target;
    REQUIRE(target.Load(serializer));
    RequireSerializedMixedScene(target);
}

TEST_CASE("Scene round-trips mixed node hierarchies through YAML text",
          "[scene][serialization][yaml]")
{
    forg::scene::Scene source;
    PopulateSerializedMixedScene(source);

    forg::io::YAMLSerializer writer;
    REQUIRE(source.Save(writer));

    std::string text;
    REQUIRE(writer.SaveToString(text));
    REQUIRE(text.find("scene:") != std::string::npos);
    REQUIRE(text.find("nodes:") != std::string::npos);
    REQUIRE(text.find("source_path: \"assets/triangle.gltf\"") !=
            std::string::npos);

    forg::io::YAMLSerializer reader;
    REQUIRE(reader.LoadFromString(text));

    forg::scene::Scene target;
    REQUIRE(target.Load(reader));
    RequireSerializedMixedScene(target);
}

TEST_CASE("Scene round-trips through a file-backed YAML serializer",
          "[scene][serialization][yaml]")
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "forg-scene-test.yaml";
    forg::scene::Scene source;
    PopulateSerializedMixedScene(source);

    forg::io::YAMLSerializer writer;
    REQUIRE(writer.OpenWrite(path.string()));
    REQUIRE(source.Save(writer));
    REQUIRE(writer.Flush());

    forg::io::YAMLSerializer reader;
    REQUIRE(reader.OpenRead(path.string()));

    forg::scene::Scene target;
    REQUIRE(target.Load(reader));
    RequireSerializedMixedScene(target);

    std::filesystem::remove(path);
}

TEST_CASE("Scene loads primitive mesh resources from serialized metadata",
          "[scene][serialization]")
{
    forg::scene::Scene source;
    forg::scene::MeshNode& mesh = source.CreateMeshNode();
    mesh.SetCylinder(1.0f, 2.0f, 5.0f, 10, 40);

    forg::io::MemorySerializer serializer;
    REQUIRE(source.Save(serializer));
    REQUIRE(serializer.ResetReading());

    forg::scene::Scene target;
    REQUIRE(target.Load(serializer));

    forg::rendering::reference::SWRenderDevice device(nullptr);
    REQUIRE(target.LoadResources(&device));

    forg::scene::MeshNode* loadedMesh =
        dynamic_cast<forg::scene::MeshNode*>(target.Node(0));
    REQUIRE(loadedMesh != nullptr);
    REQUIRE(loadedMesh->GetModel().IsLoaded());
    REQUIRE(loadedMesh->GetModel().GetMesh() != nullptr);
    REQUIRE(loadedMesh->GetModel().MeshType() ==
            forg::scene::ModelMeshType::Cylinder);
    REQUIRE(loadedMesh->GetModel().GetMesh()->GetNumVertices() > 0);
    REQUIRE(loadedMesh->GetModel().GetMesh()->GetNumFaces() > 0);
}

TEST_CASE("Scene Save rejects unowned scene node children",
          "[scene][serialization]")
{
    forg::scene::Scene scene;
    forg::scene::SceneNode& owned = scene.CreateNode();
    forg::scene::SceneNode unowned;
    REQUIRE(owned.AddChild(unowned));

    forg::io::MemorySerializer serializer;
    REQUIRE_FALSE(scene.Save(serializer));
}

TEST_CASE("Scene Load failure leaves existing scene unchanged",
          "[scene][serialization]")
{
    forg::io::MemorySerializer serializer;
    REQUIRE(serializer.BeginObject("scene"));
    int version = 1;
    REQUIRE(serializer.Value("version", version));
    forg::uint count = 1;
    REQUIRE(serializer.BeginArray("nodes", count));
    REQUIRE(serializer.BeginObject("node"));
    forg::core::string type("SceneNode");
    int invalidParent = 7;
    REQUIRE(serializer.Value("type", type));
    REQUIRE(serializer.Value("parent", invalidParent));
    REQUIRE(serializer.EndObject());
    REQUIRE(serializer.EndArray());
    REQUIRE(serializer.EndObject());
    REQUIRE(serializer.ResetReading());

    forg::scene::Scene scene;
    forg::scene::SceneNode& existing = scene.CreateNode();

    REQUIRE_FALSE(scene.Load(serializer));
    REQUIRE(scene.NodeCount() == 1);
    REQUIRE(scene.Node(0) == &existing);
    REQUIRE(existing.Parent() == &scene);
}
