#include "forg_pch.h"

#include "rendering/Vertex.h"
#include "enums.h"
#include "math/Vector3.h"

namespace forg { namespace geometry {

/************************************************************************/
/* PositionOnly                                                         */
/************************************************************************/

const int PositionOnly::StrideSize = sizeof(PositionOnly);

VertexElement PositionOnly::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
		VertexElement::VertexDeclarationEnd
};

PositionOnly::PositionOnly(const Vector3& value)
{
	this->X = value.X;
	this->Y = value.Y;
	this->Z = value.Z;
}

PositionOnly::PositionOnly(float xvalue, float yvalue, float zvalue)
{
	this->X = xvalue;
	this->Y = yvalue;
	this->Z = zvalue;
}

/************************************************************************/
/* PositionTextured                                                        */
/************************************************************************/

const int PositionTextured::StrideSize = sizeof(PositionTextured);

VertexElement PositionTextured::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
	{0, 12, DeclarationType_Float2, DeclarationUsage_TextureCoordinate, 0},
	VertexElement::VertexDeclarationEnd
};

PositionTextured::PositionTextured(const Vector3& pos, float u, float v)
{
	this->X = pos.X;
	this->Y = pos.Y;
	this->Z = pos.Z;
	this->Tu = u;
	this->Tv = v;
}

PositionTextured::PositionTextured(float xvalue, float yvalue, float zvalue, float u, float v)
{
	this->X = xvalue;
	this->Y = yvalue;
	this->Z = zvalue;
	this->Tu = u;
	this->Tv = v;
}

/************************************************************************/
/* PositionColored                                                      */
/************************************************************************/

const int PositionColored::StrideSize = sizeof(PositionColored);

VertexElement PositionColored::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
	{0, 12, DeclarationType_Color, DeclarationUsage_Color, 0},
	VertexElement::VertexDeclarationEnd
};

PositionColored::PositionColored(const Vector3& value, int c)
{
	this->X = value.X;
	this->Y = value.Y;
	this->Z = value.Z;
	this->Color = c;
}

PositionColored::PositionColored(float xvalue, float yvalue, float zvalue, int c)
{
	this->X = xvalue;
	this->Y = yvalue;
	this->Z = zvalue;
	this->Color = c;
}

void PositionColored::set_Position(const Vector3& value)
{
    this->X = value.X;
    this->Y = value.Y;
    this->Z = value.Z;
}

void PositionColored::set_Position(float fX, float fY, float fZ)
{
    this->X = fX;
   	this->Y = fY;
	this->Z = fZ;
}

void PositionColored::set_Color(int iColor)
{
    this->Color = iColor;
}
/************************************************************************/
/* PositionColoredTextured                                              */
/************************************************************************/

const int PositionColoredTextured::StrideSize = sizeof(PositionColoredTextured);

VertexElement PositionColoredTextured::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
	{0, 12, DeclarationType_Color, DeclarationUsage_Color, 0},
	{0, 16, DeclarationType_Float2, DeclarationUsage_TextureCoordinate, 0},
	VertexElement::VertexDeclarationEnd
};

/*
PositionColoredTextured::PositionColoredTextured(const Vector3& value, int c, float u, float v)
{
	this->X = value.X;
	this->Y = value.Y;
	this->Z = value.Z;
	this->Color = c;
	this->Tu = u;
	this->Tv = v;
}

PositionColoredTextured::PositionColoredTextured(float xvalue, float yvalue, float zvalue, int c, float u, float v)
{
	this->X = xvalue;
	this->Y = yvalue;
	this->Z = zvalue;
	this->Color = c;
	this->Tu = u;
	this->Tv = v;
}
*/

/************************************************************************/
/* PositionNormal                                                       */
/************************************************************************/

const int PositionNormal::StrideSize = sizeof(PositionNormal);

VertexElement PositionNormal::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
	{0, 12, DeclarationType_Float3, DeclarationUsage_Normal, 0},
	VertexElement::VertexDeclarationEnd
};

PositionNormal::PositionNormal(const Vector3& pos, const Vector3& nor)
{
    Position = pos;
    Normal = nor;
/*
	this->X = pos.X;
	this->Y = pos.Y;
	this->Z = pos.Z;
	this->Nx = nor.X;
	this->Ny = nor.Y;
	this->Nz = nor.Z;
*/
}

PositionNormal::PositionNormal(float xvalue, float yvalue, float zvalue, float nxvalue, float nyvalue, float nzvalue)
{
	Position.X = xvalue;
	Position.Y = yvalue;
	Position.Z = zvalue;
	Normal.X = nxvalue;
	Normal.Y = nyvalue;
	Normal.Z = nzvalue;
}

void PositionNormal::set_Position(const Vector3& value)
{
    Position = value;
/*
	this->X = value.X;
	this->Y = value.Y;
	this->Z = value.Z;
*/
}

void PositionNormal::set_Normal(const Vector3& value)
{
    Normal = value;
/*
	this->Nx = value.X;
	this->Ny = value.Y;
	this->Nz = value.Z;
*/
}

/************************************************************************/
/* PositionNormalTextured                                               */
/************************************************************************/

const int PositionNormalTextured::StrideSize = sizeof(PositionNormalTextured);

VertexElement PositionNormalTextured::Declaration[] =
{
	{0, 0, DeclarationType_Float3, DeclarationUsage_Position, 0},
	{0, 12, DeclarationType_Float3, DeclarationUsage_Normal, 0},
	{0, 24, DeclarationType_Float2, DeclarationUsage_TextureCoordinate, 0},
	VertexElement::VertexDeclarationEnd
};

PositionNormalTextured::PositionNormalTextured(const Vector3& pos, const Vector3& nor, float u, float v)
{
    Position = pos;
    Normal = nor;

	this->Tu = u;
	this->Tv = v;
}

PositionNormalTextured::PositionNormalTextured(float xvalue, float yvalue, float zvalue, float nxvalue, float nyvalue, float nzvalue, float u, float v)
{
	Position.X = xvalue;
	Position.Y = yvalue;
	Position.Z = zvalue;
	Normal.X = nxvalue;
	Normal.Y = nyvalue;
	Normal.Z = nzvalue;
	this->Tu = u;
	this->Tv = v;
}

void PositionNormalTextured::set_Position(const Vector3& value)
{
    Position = value;
}

void PositionNormalTextured::set_Normal(const Vector3& value)
{
    Normal = value;
}

void PositionNormalTextured::set_Texel(const Vector2& value)
{
	this->Tu = value.X;
	this->Tv = value.Y;
}

void PositionNormalTextured::set_Texel(float u, float v)
{
	this->Tu = u;
	this->Tv = v;
}

}}
