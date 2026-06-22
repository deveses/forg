#include <forg/rendering/Color.h>
#include <forg/rendering/IRenderer.h>
#include <forg/rendering/Material.h>
#include <forg/rendering/Mesh.h>
#include <forg/rendering/VertexElement.h>
#include <forg/enums.h>

#include <memory>
#include <type_traits>
#include <vector>

namespace {

forg::IRenderer* CreateNoRenderer() { return nullptr; }

} // namespace

int main()
{
    constexpr forg::Color color(1.0f, 0.0f, 0.0f, 1.0f);
    static_assert(std::is_standard_layout_v<forg::Color>);
    static_assert(std::is_same_v<forg::geometry::Mesh::MeshPtr,
                                 std::unique_ptr<forg::geometry::Mesh>>);
    static_assert(std::is_same_v<forg::geometry::Mesh::ExtendedMaterialVec,
                                 std::vector<forg::ExtendedMaterial>>);

    const forg::VertexElement declaration[] = {
        {0, 0, forg::DeclarationType_Float3, forg::DeclarationUsage_Position,
         0},
        forg::VertexElement::VertexDeclarationEnd,
    };
    if (declaration[0].Offset != 0)
    {
        return 1;
    }

    const forg::RendererPluginDescriptor descriptor{
        sizeof(forg::RendererPluginDescriptor),
        forg::RendererPluginApiVersion,
        &CreateNoRenderer,
    };
    if (!forg::IsRendererPluginCompatible(&descriptor))
    {
        return 2;
    }

    const forg::Material material{};
    if (material.Diffuse.a != 1.0f)
    {
        return 3;
    }

    return color.r == 1.0f ? 0 : 4;
}
