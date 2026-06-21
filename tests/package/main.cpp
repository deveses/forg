#include <forg/rendering/Color.h>

int main()
{
    constexpr forg::Color color(1.0f, 0.0f, 0.0f, 1.0f);
    return color.r == 1.0f ? 0 : 1;
}
