#include "forg_pch.h"

#include "rendering/VertexDeclaration.h"

namespace forg{

VertexDeclaration::VertexDeclaration(const VertexElement* pDecl)
{
	m_nElementsCount = 0;
	m_nVertexSize = 0;

	for (;m_nElementsCount<256 && pDecl[m_nElementsCount] != VertexElement::VertexDeclarationEnd; m_nElementsCount++)
	{
		elements[m_nElementsCount] = pDecl[m_nElementsCount];
	}

	if (m_nElementsCount)
		m_nVertexSize = elements[m_nElementsCount - 1].Offset + VertexElement::GetTypeSize(elements[m_nElementsCount - 1].Type);

	elements[m_nElementsCount] = VertexElement::VertexDeclarationEnd;
}

VertexDeclaration::~VertexDeclaration(void)
{
}

const VertexElement* VertexDeclaration::GetDeclaration() const
{
	return elements;
}

uint VertexDeclaration::GetElementsCount() const
{
	return m_nElementsCount;
}

uint VertexDeclaration::GetVertexSize() const
{
	return m_nVertexSize;
}

}
