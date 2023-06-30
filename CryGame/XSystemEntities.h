
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef XSYSTEMENTITIES_H
#define	XSYSTEMENTITIES_H

// stores data needed to spawn/remove the entities at runtime
//////////////////////////////////////////////////////////////////////////
class CEntityStreamData
{
public:
	CEntityStreamData();
	~CEntityStreamData();

	XDOM::IXMLDOMNodePtr m_pNode;
	bool	m_bSpawn;
}; 

typedef std::list<CEntityStreamData> CEntityStreamDataList;
typedef CEntityStreamDataList::iterator CEntityStreamDataListIt;

#endif