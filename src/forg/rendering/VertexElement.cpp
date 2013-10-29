#include "forg_pch.h"

#include "rendering/VertexElement.h"
#include "enums.h"

namespace forg{
	const VertexElement VertexElement::VertexDeclarationEnd =	{0xff, 0, DeclarationType_Unused, DeclarationUsage_Position, 0};

	static uint _type_sizes[] = {
		4,	//DeclarationType_Float1
		8,	//DeclarationType_Float2
		12, //DeclarationType_Float3
		16, //DeclarationType_Float4
		4,	//DeclarationType_Color
		4,	//DeclarationType_Ubyte4
		4,	//DeclarationType_Short2
		8,	//DeclarationType_Short4
		4,	//DeclarationType_Ubyte4N
		3,  //DeclarationType_UDec3 = 13,
		3,  //DeclarationType_Dec3N = 14,
		4,  //DeclarationType_Float16Two = 15,
		8,  //DeclarationType_Float16Four = 0x10,
		0,  //DeclarationType_Unused = 0x11
	};

	static uint _type_counts[] = {
		1,	//DeclarationType_Float1 = 0,
		2,	//DeclarationType_Float2 = 1,
		3,	//DeclarationType_Float3 = 2,
		4,	//DeclarationType_Float4 = 3,
		4,	//DeclarationType_Color = 4,
		1,	//DeclarationType_Ubyte4 = 5,
		2,	//DeclarationType_Short2 = 6,
		4,	//DeclarationType_Short4 = 7,
		1,	//DeclarationType_Ubyte4N = 8,
		2,	//DeclarationType_Short2N = 9,
		4,	//DeclarationType_Short4N = 10,
		2,	//DeclarationType_UShort2N = 11,
		4,	//DeclarationType_UShort4N = 12,
		3,	//DeclarationType_UDec3 = 13,
		3,	//DeclarationType_Dec3N = 14,
		2,	//DeclarationType_Float16Two = 15,
		4,	//DeclarationType_Float16Four = 0x10,
		0	//DeclarationType_Unused = 0x11
	};

	//IVertexElement::IVertexElement(ushort stream, ushort offset, DeclarationType declType, DeclarationUsage declUsage, byte usageIndex)
	//{
	//	Stream = stream;
	//	Offset = offset;
	//	Type = declType;
	//	Usage = declUsage;
	//	UsageIndex = usageIndex;
	//}


	bool VertexElement::operator != (VertexElement elem) const
	{
		return ( Stream != elem.Stream
			|| Offset != elem.Offset
			|| Type != elem.Type
			|| Usage != elem.Usage
			|| UsageIndex != elem.UsageIndex );
	}

	uint VertexElement::GetTypeSize(byte type)
	{
		return _type_sizes[type];
	}

	uint VertexElement::GetTypeCount(byte type)
	{
		return _type_counts[type];
	}

}
