#include <catch2/catch_test_macros.hpp>

#include <string>

#include "forg/script/xml/XMLParser.h"

namespace
{
std::string TestDataPath(const char* filename)
{
    return std::string(FORG_TEST_DATA_DIR) + "/" + filename;
}
}

TEST_CASE("XMLParser reads elements and attributes from a file", "[script][xml]")
{
    forg::script::xml::XMLParser parser;
    const std::string path = TestDataPath("simple.xml");

    REQUIRE(parser.Open(path.c_str()));
    forg::script::xml::XMLDocument* document = parser.Parse();
    REQUIRE(document != nullptr);

    forg::script::xml::XMLNode* config = document->FindNode("config");
    REQUIRE(config != nullptr);

    forg::script::xml::XMLNode* renderer = document->FindNode("renderer");
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->FindAttribute("driver") != nullptr);
    REQUIRE(renderer->FindAttribute("driver")->GetContent() == "libswrenderer.dylib");

    forg::script::xml::XMLNode* window = document->FindNode("window");
    REQUIRE(window != nullptr);
    REQUIRE(window->FindAttribute("width") != nullptr);
    REQUIRE(window->FindAttribute("width")->GetContent() == "800");

    parser.Close();
}

TEST_CASE("XMLParser rejects missing files", "[script][xml]")
{
    forg::script::xml::XMLParser parser;
    const std::string path = TestDataPath("missing.xml");

    REQUIRE_FALSE(parser.Open(path.c_str()));
}
