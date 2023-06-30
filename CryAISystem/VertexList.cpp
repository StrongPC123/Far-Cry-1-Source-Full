#include "stdafx.h"
#include "vertexlist.h"
#include <ISystem.h>
#include <CryFile.h>

CVertexList::CVertexList(void)
{
	m_vList.clear();
}

CVertexList::~CVertexList(void)
{
}

int CVertexList::AddVertex(const ObstacleData & od)
{

	int index=FindVertex(od);
	if (index<0)
	{
		m_vList.push_back(od);
		index = (int)m_vList.size()-1;
	}

	return index;
}


int CVertexList::FindVertex(const ObstacleData & od)
{

	Obstacles::iterator oi,oiend = m_vList.end();
	int index=0;

	for (oi=m_vList.begin();oi!=oiend;++oi,index++)
	{
		if ( (*oi) == od )
			return index;
	}

	return -1;
}

const ObstacleData CVertexList::GetVertex(int index)
{
	if ((index<0) || (index >= (int)(m_vList.size())))
		CryError("[AISYSTEM] Tried to retrieve a non existing vertex from vertex list.Please regenerate the triangulation and re-export the map.");

	return m_vList[index];
}

ObstacleData &CVertexList::ModifyVertex(int index)
{
	if ((index<0) || (index >= (int)(m_vList.size())))
		CryError("[AISYSTEM] Tried to retrieve a non existing vertex from vertex list.Please regenerate the triangulation and re-export the map.");

	return m_vList[index];
}


void CVertexList::WriteToFile( CCryFile& file )
{
	int iNumber = (int)m_vList.size();
	file.Write( &iNumber, sizeof( int ) );
	if (!iNumber)
		return;
	file.Write( &m_vList[ 0 ], iNumber * sizeof( ObstacleData ) );
}

void CVertexList::ReadFromFile( CCryFile &file )
{
	int iNumber;
	file.Read( &iNumber, sizeof(int) );

	if (iNumber>0) 
	{
		m_vList.resize(iNumber);
		file.Read( &m_vList[0], iNumber*sizeof(ObstacleData) );
	}

}
