#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <limits>
#include <type_traits>

#include "forg/enums.h"
#include "forg/rendering/Color.h"
#include "forg/rendering/VertexDeclaration.h"
#include "forg/rendering/VertexElement.h"

using Catch::Approx;

TEST_CASE("Rendering value layouts remain stable", "[rendering][layout]")
{
    STATIC_REQUIRE(std::is_standard_layout_v<forg::Color3f>);
    STATIC_REQUIRE(sizeof(forg::Color3f) == sizeof(float) * 3);
    STATIC_REQUIRE(offsetof(forg::Color3f, r) == 0);
    STATIC_REQUIRE(offsetof(forg::Color3f, g) == sizeof(float));
    STATIC_REQUIRE(offsetof(forg::Color3f, b) == sizeof(float) * 2);

    STATIC_REQUIRE(std::is_standard_layout_v<forg::Color>);
    STATIC_REQUIRE(sizeof(forg::Color) == sizeof(float) * 4);
    STATIC_REQUIRE(offsetof(forg::Color, r) == 0);
    STATIC_REQUIRE(offsetof(forg::Color, a) == sizeof(float) * 3);

    STATIC_REQUIRE(std::is_standard_layout_v<forg::VertexElement>);
    STATIC_REQUIRE(sizeof(forg::VertexElement) == 8);
    STATIC_REQUIRE(offsetof(forg::VertexElement, Stream) == 0);
    STATIC_REQUIRE(offsetof(forg::VertexElement, Offset) ==
                   sizeof(forg::ushort));
    STATIC_REQUIRE(offsetof(forg::VertexElement, Type) ==
                   sizeof(forg::ushort) * 2);
    STATIC_REQUIRE(offsetof(forg::VertexElement, UsageIndex) ==
                   sizeof(forg::ushort) * 2 + sizeof(forg::byte) * 2);

    STATIC_REQUIRE_FALSE(std::is_standard_layout_v<forg::VertexDeclaration>);
    STATIC_REQUIRE(sizeof(forg::VertexDeclaration) >=
                   sizeof(forg::VertexElement) * 256 + sizeof(forg::uint) * 2);
}

TEST_CASE("Color converts between ARGB integers and float channels",
          "[rendering][color]")
{
    forg::Color color(0x80402010u);

    REQUIRE(color.a == Approx(128.0f / 255.0f));
    REQUIRE(color.r == Approx(64.0f / 255.0f));
    REQUIRE(color.g == Approx(32.0f / 255.0f));
    REQUIRE(color.b == Approx(16.0f / 255.0f));
    REQUIRE(static_cast<forg::uint>(color) == 0x80402010u);
}

TEST_CASE("Color conversion clamps invalid channels", "[rendering][color]")
{
    const forg::Color color(-1.0f, 2.0f,
                            std::numeric_limits<float>::quiet_NaN(), 1.0f);

    REQUIRE(static_cast<forg::uint>(color) == 0xff00ff00U);
}

TEST_CASE("Color component values are constexpr and const-correct",
          "[rendering][color]")
{
    constexpr forg::Color3f color(0.25f, 0.5f, 0.75f);
    constexpr forg::Color3f doubled = color * 2.0f;

    STATIC_REQUIRE(doubled.r == 0.5f);
    STATIC_REQUIRE(doubled.g == 1.0f);
    STATIC_REQUIRE(doubled.b == 1.5f);
}

TEST_CASE("VertexElement reports declaration type sizes and component counts",
          "[rendering][vertex]")
{
    REQUIRE(forg::VertexElement::GetTypeSize(forg::DeclarationType_Float1) ==
            4);
    REQUIRE(forg::VertexElement::GetTypeSize(forg::DeclarationType_Float3) ==
            12);
    REQUIRE(forg::VertexElement::GetTypeSize(forg::DeclarationType_Float4) ==
            16);

    REQUIRE(forg::VertexElement::GetTypeCount(forg::DeclarationType_Float1) ==
            1);
    REQUIRE(forg::VertexElement::GetTypeCount(forg::DeclarationType_Float3) ==
            3);
    REQUIRE(forg::VertexElement::GetTypeCount(forg::DeclarationType_Float4) ==
            4);
}

TEST_CASE("VertexDeclaration counts elements and computes vertex size",
          "[rendering][vertex]")
{
    const forg::VertexElement declaration[] = {
        {0,  0, forg::DeclarationType_Float3,forg::DeclarationUsage_Position,
         0                                          },
        {0, 12, forg::DeclarationType_Float2,
         forg::DeclarationUsage_TextureCoordinate, 0},
        forg::VertexElement::VertexDeclarationEnd,
    };

    forg::VertexDeclaration vertex_declaration(declaration);

    REQUIRE(vertex_declaration.GetElementsCount() == 2);
    REQUIRE(vertex_declaration.GetVertexSize() == 20);
    REQUIRE(vertex_declaration.GetDeclaration()[0].Usage ==
            forg::DeclarationUsage_Position);
    REQUIRE(vertex_declaration.GetDeclaration()[1].Usage ==
            forg::DeclarationUsage_TextureCoordinate);
}
