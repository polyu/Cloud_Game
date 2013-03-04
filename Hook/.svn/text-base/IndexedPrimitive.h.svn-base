#pragma once

//-----------------------------------------------------------------------------------
//	class:	IndexedPrimitive
//	--------------------------------------
//	Description:	A simple class used for storing the data regarding a single 
//					indexed primitive.
//-----------------------------------------------------------------------------------
class IndexedPrimitive
{
public:
	IndexedPrimitive(void);
	IndexedPrimitive(int BaseVertexIndex,unsigned int MinVertexIndex,unsigned int NumVertices,
					unsigned int startIndex,unsigned int primCount);
	~IndexedPrimitive(void);

	void Set(int BaseVertexIndex,unsigned int MinVertexIndex,unsigned int NumVertices,
				unsigned int startIndex,unsigned int primCount);

	bool operator<(const IndexedPrimitive &ip) const{
		return m_uNumVertices < ip.m_uNumVertices;
	}


private:
	int m_iBaseVertexIndex;
	unsigned int m_uMinVertexIndex;
	unsigned int m_uNumVertices;
	unsigned int m_ustartIndex;
	unsigned int m_uprimCount;
};
