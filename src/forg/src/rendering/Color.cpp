#include "forg_pch.h"

#include "rendering/Color.h"

namespace forg {

static u32 color_component_to_byte(float value) noexcept
{
    // NaN fails both comparisons inside clamp(), so catch it here with
    // negatives.
    if (!(value > 0.0f))
        return 0;

    return static_cast<u32>(clamp(value, 0.0f, 1.0f) * 255.0f);
}

Color::Color(u32 argb) { *this = argb; }

Color::operator u32() const
{
    u32 _r = color_component_to_byte(r);
    u32 _g = color_component_to_byte(g);
    u32 _b = color_component_to_byte(b);
    u32 _a = color_component_to_byte(a);

    return ((_a & 0xff) << 24) | ((_r & 0xff) << 16) | ((_g & 0xff) << 8) |
           ((_b & 0xff));
}

Color& Color::operator=(u32 argb)
{
    a = static_cast<float>((argb >> 24) & 0xff) / 255.0f;
    r = static_cast<float>((argb >> 16) & 0xff) / 255.0f;
    g = static_cast<float>((argb >> 8) & 0xff) / 255.0f;
    b = static_cast<float>(argb & 0xff) / 255.0f;

    return *this;
}

} // namespace forg
