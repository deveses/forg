#include <catch2/catch_test_macros.hpp>

#include "forg/scene/Scene.h"
#include "forg/scene/MeshNode.h"
#include "forg/scene/TreeNode.h"

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
