#include <catch2/catch_test_macros.hpp>

#include <string>

#include "forg/script/xml/XMLParser.h"

namespace {
std::string TestDataPath(const char* filename)
{
    return std::string(FORG_TEST_DATA_DIR) + "/" + filename;
}
} // namespace

TEST_CASE("XMLParser reads elements and attributes from a file",
          "[script][xml]")
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
    REQUIRE(renderer->FindAttribute("driver")->GetContent() ==
            "libswrenderer.dylib");

    forg::script::xml::XMLNode* window = document->FindNode("window");
    REQUIRE(window != nullptr);
    REQUIRE(window->FindAttribute("width") != nullptr);
    REQUIRE(window->FindAttribute("width")->GetContent() == "800");

    parser.Close();
}

TEST_CASE("XMLParser releases its document tree on destruction",
          "[script][xml]")
{
    // Parse a tree with nested elements, siblings, attributes and an empty
    // element, then let the parser fall out of scope. Under AddressSanitizer
    // this exercises the recursive node cleanup with no double-free / invalid
    // free (and no leak where LeakSanitizer is available).
    {
        forg::script::xml::XMLParser parser;
        const std::string path = TestDataPath("nested.xml");

        REQUIRE(parser.Open(path.c_str()));
        forg::script::xml::XMLDocument* document = parser.Parse();
        REQUIRE(document != nullptr);
        REQUIRE(document->FindNode("device") != nullptr);
        parser.Close();
        // `document` is owned by the parser and freed when it goes out of
        // scope.
    }
}

TEST_CASE("XMLParser rejects missing files", "[script][xml]")
{
    forg::script::xml::XMLParser parser;
    const std::string path = TestDataPath("missing.xml");

    REQUIRE_FALSE(parser.Open(path.c_str()));
}

TEST_CASE("XMLParser finds nested and sibling elements", "[script][xml]")
{
    forg::script::xml::XMLParser parser;
    const std::string path = TestDataPath("nested.xml");

    REQUIRE(parser.Open(path.c_str()));
    forg::script::xml::XMLDocument* document = parser.Parse();
    REQUIRE(document != nullptr);

    forg::script::xml::XMLNode* graphics = document->FindNode("graphics");
    REQUIRE(graphics != nullptr);

    forg::script::xml::XMLNode* renderer = document->FindNode("renderer");
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->FindAttribute("driver") != nullptr);
    REQUIRE(renderer->FindAttribute("driver")->GetContent() == "metal");

    forg::script::xml::XMLNode* audio = document->FindNode("audio");
    REQUIRE(audio != nullptr);

    forg::script::xml::XMLNode* device = document->FindNode("device");
    REQUIRE(device != nullptr);
    REQUIRE(device->FindAttribute("name") != nullptr);
    REQUIRE(device->FindAttribute("name")->GetContent() == "builtin");

    forg::script::xml::XMLNode* empty = document->FindNode("empty");
    REQUIRE(empty != nullptr);

    REQUIRE(document->FindNode("network") == nullptr);

    parser.Close();
}
