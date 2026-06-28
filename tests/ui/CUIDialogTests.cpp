#include <catch2/catch_test_macros.hpp>

#include "forg/ui/gui.h"

#include <string>

namespace {

std::string DataPath(const char* filename)
{
    return std::string(FORG_TEST_REPO_DATA_DIR) + "/" + filename;
}

} // namespace

TEST_CASE("CUIDialog loads controls from dialog YAML", "[ui][yaml]")
{
    forg::ui::CUIDialog dialog;

    REQUIRE(dialog.Load(DataPath("ui/dialog.yml").c_str()));

    REQUIRE(dialog.GetControl(0) != nullptr);
    REQUIRE(dialog.GetControl(1) != nullptr);
    REQUIRE(dialog.GetControl(2) != nullptr);
    REQUIRE(dialog.GetControl(3) != nullptr);
    REQUIRE(dialog.GetKnob(2) != nullptr);
}

TEST_CASE("CUIControl hit testing respects both bounds axes", "[ui]")
{
    forg::ui::CUIDialog dialog;
    REQUIRE(dialog.AddButton(7, 10, 20, 30, 40) == FORG_OK);

    REQUIRE(dialog.GetControlAtPoint({10, 20}) == dialog.GetControl(7));
    REQUIRE(dialog.GetControlAtPoint({40, 60}) == dialog.GetControl(7));
    REQUIRE(dialog.GetControlAtPoint({9, 30}) == nullptr);
    REQUIRE(dialog.GetControlAtPoint({20, 19}) == nullptr);
    REQUIRE(dialog.GetControlAtPoint({41, 30}) == nullptr);
    REQUIRE(dialog.GetControlAtPoint({20, 61}) == nullptr);
}
