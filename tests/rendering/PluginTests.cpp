#include "forg/rendering/IRenderer.h"

#include <catch2/catch_test_macros.hpp>

#ifdef FORG_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

void CheckPlugin(const char* path)
{
#ifdef FORG_PLATFORM_WINDOWS
    HMODULE module = LoadLibraryA(path);
    REQUIRE(module != nullptr);
    auto getDescriptor = reinterpret_cast<forg::PFGETRENDERERPLUGINDESCRIPTOR>(
        GetProcAddress(module, "forgGetRendererPluginDescriptor"));
#else
    void* module = dlopen(path, RTLD_NOW);
    REQUIRE(module != nullptr);
    auto getDescriptor = reinterpret_cast<forg::PFGETRENDERERPLUGINDESCRIPTOR>(
        dlsym(module, "forgGetRendererPluginDescriptor"));
#endif

    REQUIRE(getDescriptor != nullptr);
    const forg::RendererPluginDescriptor* descriptor = getDescriptor();
    REQUIRE(forg::IsRendererPluginCompatible(descriptor));

    forg::IRenderer* renderer = descriptor->CreateRenderer();
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->get_Name() != nullptr);
    delete renderer;

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
        forg::RendererPluginApiVersion + 1, nullptr};
    REQUIRE_FALSE(forg::IsRendererPluginCompatible(&descriptor));

    descriptor.ApiVersion = forg::RendererPluginApiVersion;
    REQUIRE_FALSE(forg::IsRendererPluginCompatible(&descriptor));
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
