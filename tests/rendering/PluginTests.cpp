#include "forg/rendering/IRenderer.h"

#include <catch2/catch_test_macros.hpp>

#ifdef FORG_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

class TestRenderer : public forg::IRenderer
{
  public:
    forg::IRenderDevice* CreateDevice(forg::HWIN,
                                      forg::RENDER_PARAMETERS*) override
    {
        return nullptr;
    }

    forg::LPCTSTR get_Name() override { return _T("test renderer"); }
};

forg::IRenderer* CreateTestRenderer() { return new TestRenderer(); }

forg::IRenderer* CreateNullRenderer() { return nullptr; }

int DestroyTestRenderer(forg::IRenderer* renderer)
{
    delete renderer;
    return FORG_OK;
}

int FailDestroyTestRenderer(forg::IRenderer* renderer)
{
    delete renderer;
    return FORG_INVALID_CALL;
}

void CheckPlugin(const char* path)
{
#ifdef FORG_PLATFORM_WINDOWS
    HMODULE module = LoadLibraryA(path);
    REQUIRE(module != nullptr);
    auto getDescriptor = reinterpret_cast<forg::PFGETRENDERERPLUGINDESCRIPTOR>(
        GetProcAddress(module, "forgGetRendererPluginDescriptor"));
    auto legacyCreateRenderer = reinterpret_cast<forg::PFCREATERENDERER>(
        GetProcAddress(module, "forgCreateRenderer"));
#else
    void* module = dlopen(path, RTLD_NOW);
    REQUIRE(module != nullptr);
    auto getDescriptor = reinterpret_cast<forg::PFGETRENDERERPLUGINDESCRIPTOR>(
        dlsym(module, "forgGetRendererPluginDescriptor"));
    auto legacyCreateRenderer = reinterpret_cast<forg::PFCREATERENDERER>(
        dlsym(module, "forgCreateRenderer"));
#endif

    forg::RendererPluginBinding binding;
    REQUIRE(forg::ProbeRendererPlugin(getDescriptor, legacyCreateRenderer,
                                      binding) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(binding.ApiVersion == forg::RendererPluginApiVersion2);
    REQUIRE(binding.UsesPluginDestroy);
    REQUIRE(binding.Name != nullptr);
    REQUIRE(binding.Name[0] != '\0');

    forg::IRenderer* renderer = nullptr;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->get_Name() != nullptr);
    REQUIRE(forg::DestroyRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);

#ifdef FORG_PLATFORM_WINDOWS
    REQUIRE(FreeLibrary(module) != 0);
#else
    REQUIRE(dlclose(module) == 0);
#endif
}

} // namespace

TEST_CASE("Renderer plugin descriptors reject incompatible contracts",
          "[rendering][plugin]")
{
    forg::RendererPluginDescriptor descriptor{
        sizeof(forg::RendererPluginDescriptor),
        forg::RendererPluginApiVersion + 1, nullptr, nullptr, "Test Renderer"};
    REQUIRE_FALSE(forg::IsRendererPluginCompatible(&descriptor));

    descriptor.ApiVersion = forg::RendererPluginApiVersion;
    REQUIRE_FALSE(forg::IsRendererPluginCompatible(&descriptor));

    descriptor.CreateRenderer = &CreateTestRenderer;
    REQUIRE_FALSE(forg::IsRendererPluginCompatible(&descriptor));
}

TEST_CASE("Renderer plugin boundary supports v1 v2 and legacy factories",
          "[rendering][plugin]")
{
    forg::RendererPluginBinding binding;

    const forg::RendererPluginDescriptor descriptor_v2{
        sizeof(forg::RendererPluginDescriptor),
        forg::RendererPluginApiVersion2,
        &CreateTestRenderer,
        &DestroyTestRenderer,
        "Test Renderer",
    };
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor_v2, binding) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(binding.ApiVersion == forg::RendererPluginApiVersion2);
    REQUIRE(binding.UsesPluginDestroy);
    REQUIRE_FALSE(binding.UsesLegacyFactory);
    REQUIRE(binding.Name != nullptr);
    REQUIRE(binding.Name[0] != '\0');

    forg::IRenderer* renderer = nullptr;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(renderer != nullptr);
    REQUIRE(forg::DestroyRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);

    const forg::RendererPluginDescriptorV1 descriptor_v1{
        sizeof(forg::RendererPluginDescriptorV1),
        forg::RendererPluginApiVersion1,
        &CreateTestRenderer,
    };
    const auto* descriptor_v1_as_current =
        reinterpret_cast<const forg::RendererPluginDescriptor*>(&descriptor_v1);
    REQUIRE(
        forg::BindRendererPluginDescriptor(descriptor_v1_as_current, binding) ==
        forg::RendererPluginStatus::Ok);
    REQUIRE(binding.ApiVersion == forg::RendererPluginApiVersion1);
    REQUIRE_FALSE(binding.UsesPluginDestroy);

    renderer = nullptr;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(forg::DestroyRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);

    REQUIRE(forg::ProbeRendererPlugin(nullptr, &CreateTestRenderer, binding) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(binding.UsesLegacyFactory);
    REQUIRE_FALSE(binding.UsesPluginDestroy);

    renderer = nullptr;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(forg::DestroyRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
}

TEST_CASE("Renderer plugin boundary reports distinct failures",
          "[rendering][plugin]")
{
    forg::RendererPluginBinding binding;

    REQUIRE(forg::ProbeRendererPlugin(nullptr, nullptr, binding) ==
            forg::RendererPluginStatus::MissingSymbols);
    REQUIRE(forg::BindRendererPluginDescriptor(nullptr, binding) ==
            forg::RendererPluginStatus::NullDescriptor);

    forg::RendererPluginDescriptor descriptor{
        sizeof(forg::RendererPluginDescriptorV1),
        forg::RendererPluginApiVersion2,
        &CreateTestRenderer,
        &DestroyTestRenderer,
        "Test Renderer",
    };
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::TruncatedDescriptor);

    descriptor.Size = sizeof(forg::RendererPluginDescriptor);
    descriptor.ApiVersion = forg::RendererPluginApiVersion2 + 1;
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::UnsupportedVersion);

    descriptor.ApiVersion = forg::RendererPluginApiVersion2;
    descriptor.Name = nullptr;
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::MissingName);

    descriptor.Name = "";
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::MissingName);

    descriptor.Name = "Test Renderer";
    descriptor.CreateRenderer = nullptr;
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::MissingCreate);

    descriptor.CreateRenderer = &CreateTestRenderer;
    descriptor.DestroyRenderer = nullptr;
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::MissingDestroy);

    descriptor.DestroyRenderer = &DestroyTestRenderer;
    REQUIRE(forg::BindRendererPluginDescriptor(&descriptor, binding) ==
            forg::RendererPluginStatus::Ok);

    binding.CreateRenderer = &CreateNullRenderer;
    forg::IRenderer* renderer = nullptr;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::FactoryFailed);

    binding.CreateRenderer = &CreateTestRenderer;
    binding.DestroyRenderer = &FailDestroyTestRenderer;
    REQUIRE(forg::CreateRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::Ok);
    REQUIRE(forg::DestroyRendererFromPlugin(binding, renderer) ==
            forg::RendererPluginStatus::DestroyFailed);
}

#ifdef FORG_TEST_SWRENDERER_PATH
TEST_CASE("Software renderer plugin has a compatible lifecycle",
          "[rendering][plugin]")
{
    CheckPlugin(FORG_TEST_SWRENDERER_PATH);
}
#endif

#ifdef FORG_TEST_METALRENDERER_PATH
TEST_CASE("Metal renderer plugin has a compatible lifecycle",
          "[rendering][plugin]")
{
    CheckPlugin(FORG_TEST_METALRENDERER_PATH);
}
#endif

#ifdef FORG_TEST_GLRENDERER_PATH
TEST_CASE("OpenGL renderer plugin has a compatible lifecycle",
          "[rendering][plugin]")
{
    CheckPlugin(FORG_TEST_GLRENDERER_PATH);
}
#endif
