#include "forg_pch.h"

#include "rendering/Color.h"

namespace forg {

Color::Color(uint argb)
{
	*this = argb;
}

 Color::operator uint() const
{
	uint _r = (uint)(r*255.0f);
	uint _g = (uint)(g*255.0f);
	uint _b = (uint)(b*255.0f);
	uint _a = (uint)(a*255.0f);

	return ((_a & 0xff)<<24) | ((_r & 0xff)<<16) | ((_g & 0xff)<<8) | ((_b & 0xff));
}

Color& Color::operator = (const Color& c)
{
	a = c.a;
	r = c.r;
	g = c.g;
	b = c.b;

	return *this;
}

Color& Color::operator = (uint argb)
{
	a = (float)((argb >> 24) & 0xff) / 255.0f;;
	r = (float)((argb >> 16) & 0xff) / 255.0f;
	g = (float)((argb >> 8) & 0xff) / 255.0f;;
	b = (float)(argb & 0xff) / 255.0f;;

	return *this;
}

}

