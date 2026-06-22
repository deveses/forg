#include "forg/rendering/reference/SWRenderDevice.h"

#include <catch2/catch_test_macros.hpp>

namespace {

unsigned int BufferChecksum(const forg::uint* buffer, forg::uint count)
{
    unsigned int checksum = 2166136261u;
    for (forg::uint i = 0; i < count; ++i)
    {
        checksum ^= buffer[i];
        checksum *= 16777619u;
    }
    return checksum;
}

} // namespace

TEST_CASE("Reference renderer produces stable headless triangle output",
          "[rendering][reference]")
{
    forg::rendering::reference::SWRenderDevice device(nullptr);
    REQUIRE(device.Initialize(16, 16) == FORG_OK);
    REQUIRE(device.SetViewport(0, 0, 16, 16) == FORG_OK);
    REQUIRE(device.Clear(forg::ClearFlags_Target | forg::ClearFlags_ZBuffer,
                         forg::Color(0.0f, 0.0f, 0.0f, 1.0f), 1.0f,
                         0) == FORG_OK);

    const forg::Vector3 triangle[] = {
        forg::Vector3(2.0f, 2.0f, 0.0f),
        forg::Vector3(2.0f, 13.0f, 0.0f),
        forg::Vector3(13.0f, 2.0f, 0.0f),
    };
    device.DrawTriangle(triangle);

    const forg::uint* buffer = device.GetBuffer();
    REQUIRE(buffer[0] == 0xff000000u);
    REQUIRE(buffer[8 * 16 + 4] == 0xff00007fu);
    REQUIRE(buffer[13 * 16 + 13] == 0xff000000u);
    REQUIRE(BufferChecksum(buffer, 16 * 16) == 0x8aca7a79u);
}
