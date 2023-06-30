//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: IDGenerator.cpp
//  Description: ID Generator class.
//
//  History:
//  - 08/06/2001: Created by Alberto Demichelis
//  - 12/03/2003: Martin Mittring made distinction between static and dynamic ids
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDGenerator.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

///////////////////////////////////////////////
CIDGenerator::CIDGenerator()
{
	m_vUsedIDs.resize(0xFFFF+1,false);
	Reset();
}

///////////////////////////////////////////////
CIDGenerator::~CIDGenerator()
{
}

///////////////////////////////////////////////
void CIDGenerator::Reset()
{
	BitVectorItor itor=m_vUsedIDs.begin();
	int nSize=m_vUsedIDs.size();
	int n=0;
	while(n<nSize)
	{
		m_vUsedIDs[n] = false;
		n++;
	}
	//memset(m_vUsedIDs,0,sizeof(m_vUsedIDs));
	m_vUsedIDs[0] = true;
}



///////////////////////////////////////////////
WORD CIDGenerator::GetNewDynamic()
{
	WORD i = 0xffff;		// to be able to have different ID range for dynamically generated entities
	
	while (m_vUsedIDs[i])
		--i;										// WORD is cycling through the whole buffer

	m_vUsedIDs[i] = true;
	return i;
}

bool CIDGenerator::IsDynamicEntityId( WORD nID )
{
	return (nID&0x8000)!=0;
}

///////////////////////////////////////////////
WORD CIDGenerator::GetNewStatic()
{
	WORD i = 0;		// to be able to have different ID range for dynamically generated entities
	
	while(m_vUsedIDs[i])
		++i;										// WORD is cycling through the whole buffer

	m_vUsedIDs[i] = true;
	return i;
}

///////////////////////////////////////////////
void CIDGenerator::Remove(WORD nID)
{	
	assert(m_vUsedIDs[nID]==true);
	m_vUsedIDs[nID] = false;
}

////////////////////////////////////////////
void CIDGenerator::Mark(WORD nID)
{
	assert(m_vUsedIDs[nID]==false);
	m_vUsedIDs[nID] = true;
}

bool CIDGenerator::IsUsed(WORD nID)
{
	return m_vUsedIDs[nID];
}
