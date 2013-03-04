#include "IndexedPrimitive.h"

IndexedPrimitive::IndexedPrimitive(void):
m_iBaseVertexIndex(0),
m_uMinVertexIndex(0),
m_uNumVertices(0),
m_ustartIndex(0),
m_uprimCount(0)
{
}

IndexedPrimitive::IndexedPrimitive(int BaseVertexIndex,
								   unsigned int MinVertexIndex,
								   unsigned int NumVertices,
								   unsigned int startIndex,
								   unsigned int primCount):
m_iBaseVertexIndex(BaseVertexIndex),
m_uMinVertexIndex(MinVertexIndex),
m_uNumVertices(NumVertices),
m_ustartIndex(startIndex),
m_uprimCount(primCount)
{
}

IndexedPrimitive::~IndexedPrimitive(void)
{
}


void IndexedPrimitive::Set(int BaseVertexIndex,unsigned int MinVertexIndex,unsigned int NumVertices, unsigned int startIndex,unsigned int primCount)
{
	m_iBaseVertexIndex = BaseVertexIndex;
	m_uMinVertexIndex = MinVertexIndex;
	m_uNumVertices = NumVertices;
	m_uprimCount = primCount;
	m_ustartIndex = startIndex;
}
