#include <catch2/catch_test_macros.hpp>

#include <string>

#include "forg/script/yaml/YAMLParser.h"

namespace
{
std::string TestDataPath(const char* filename)
{
    return std::string(FORG_TEST_DATA_DIR) + "/" + filename;
}
}

TEST_CASE("YAMLParser reads config maps as XML-like nodes and attributes", "[script][yaml]")
{
    forg::script::yaml::YAMLParser parser;
    const std::string path = TestDataPath("simple.yaml");

    REQUIRE(parser.Open(path.c_str()));
    forg::script::yaml::YAMLDocument* document = parser.Parse();
    REQUIRE(document != nullptr);

    forg::script::yaml::YAMLNode* config = document->FindNode("config");
    REQUIRE(config != nullptr);

    forg::script::yaml::YAMLNode* renderer = document->FindNode("renderer");
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->FindAttribute("driver") != nullptr);
    REQUIRE(renderer->FindAttribute("driver")->GetContent() == "libmetalrenderer.dylib");

    forg::script::yaml::YAMLNode* window = document->FindNode("window");
    REQUIRE(window != nullptr);
    REQUIRE(window->FindAttribute("width") != nullptr);
    REQUIRE(window->FindAttribute("width")->GetContent() == "800");

    forg::script::yaml::YAMLNode* controlserver = document->FindNode("controlserver");
    REQUIRE(controlserver != nullptr);
    REQUIRE(controlserver->FindAttribute("enabled") != nullptr);
    REQUIRE(controlserver->FindAttribute("enabled")->GetContent() == "true");

    parser.Close();
}

TEST_CASE("YAMLParser releases its document tree on destruction", "[script][yaml]")
{
    {
        forg::script::yaml::YAMLParser parser;
        const std::string path = TestDataPath("nested.yaml");

        REQUIRE(parser.Open(path.c_str()));
        forg::script::yaml::YAMLDocument* document = parser.Parse();
        REQUIRE(document != nullptr);
        REQUIRE(document->FindNode("device") != nullptr);
        parser.Close();
    }
}

TEST_CASE("YAMLParser rejects missing files", "[script][yaml]")
{
    forg::script::yaml::YAMLParser parser;
    const std::string path = TestDataPath("missing.yaml");

    REQUIRE_FALSE(parser.Open(path.c_str()));
}

TEST_CASE("YAMLParser finds nested and sibling mappings", "[script][yaml]")
{
    forg::script::yaml::YAMLParser parser;
    const std::string path = TestDataPath("nested.yaml");

    REQUIRE(parser.Open(path.c_str()));
    forg::script::yaml::YAMLDocument* document = parser.Parse();
    REQUIRE(document != nullptr);

    forg::script::yaml::YAMLNode* graphics = document->FindNode("graphics");
    REQUIRE(graphics != nullptr);

    forg::script::yaml::YAMLNode* renderer = document->FindNode("renderer");
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->FindAttribute("driver") != nullptr);
    REQUIRE(renderer->FindAttribute("driver")->GetContent() == "metal");

    forg::script::yaml::YAMLNode* audio = document->FindNode("audio");
    REQUIRE(audio != nullptr);

    forg::script::yaml::YAMLNode* device = document->FindNode("device");
    REQUIRE(device != nullptr);
    REQUIRE(device->FindAttribute("name") != nullptr);
    REQUIRE(device->FindAttribute("name")->GetContent() == "builtin");
    REQUIRE(device->FindAttribute("description") != nullptr);
    REQUIRE(device->FindAttribute("description")->GetContent() == "Built-in output #0");

    forg::script::yaml::YAMLNode* empty = document->FindNode("empty");
    REQUIRE(empty != nullptr);

    REQUIRE(document->FindNode("network") == nullptr);

    parser.Close();
}

TEST_CASE("YAMLParser rejects unsupported YAML subset syntax", "[script][yaml]")
{
    forg::script::yaml::YAMLParser parser;
    const std::string path = TestDataPath("invalid.yaml");

    REQUIRE(parser.Open(path.c_str()));
    REQUIRE(parser.Parse() == nullptr);

    parser.Close();
}
