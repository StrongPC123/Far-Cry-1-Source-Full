#ifndef _VERTEX_LIST_
#define _VERTEX_LIST_

#include <iagent.h>

class CCryFile;

class CVertexList
{

	Obstacles m_vList;

public:
	CVertexList(void);
	~CVertexList(void);
	int AddVertex(const ObstacleData & od);

	const ObstacleData GetVertex(int index);
	ObstacleData &ModifyVertex(int index);
	int FindVertex(const ObstacleData & od);

	void WriteToFile( CCryFile& file );
	void ReadFromFile( class CCryFile &file );

	void Clear() {m_vList.clear();}
	int GetSize() {return m_vList.size();}
};

#endif // #ifndef _VERTEX_LIST_