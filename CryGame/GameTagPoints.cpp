
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameTagPoints.cpp
//  
//	History:
//	-December 11,2001: created
//	-October	31,2003: split from GameLoading.cpp and other files
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"

#include "UIHud.h"

#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"
#include "TagPoint.h"
//#include "XPath.h"

//////////////////////////////////////////////////////////////////////
// Inserts a tag-point in the list.
ITagPoint *CXGame::CreateTagPoint(const string &name, const Vec3d &pos, const Vec3d &angles)
{
	// if a tag point with this name doesn't exist
	//if (m_mapTagPoints.find(name) == m_mapTagPoints.end() )
	//{
		// create new one
		CTagPoint *pNewPoint = new CTagPoint(this);
		pNewPoint->OverrideName(name.c_str());
		pNewPoint->SetPos(pos);
		pNewPoint->SetAngles(angles);

		// insert it into the map
		m_mapTagPoints.insert(TagPointMap::iterator::value_type(name,pNewPoint));
		return (ITagPoint *) pNewPoint;
	//}
	//else
	//	return 0;
}
//////////////////////////////////////////////////////////////////////
// Remove tag-point.
void CXGame::RemoveTagPoint(ITagPoint *pPoint)
{
	TagPointMap::iterator ti;
	// find and delete tag-point
	for (ti = m_mapTagPoints.begin();ti!=m_mapTagPoints.end();ti++)
	{
		if ( ti->second == pPoint )
		{
			m_mapTagPoints.erase(ti);
			pPoint->Release();
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Retrieve a tag-point by its name
ITagPoint *CXGame::GetTagPoint(const string &name)
{
	TagPointMap::iterator ti;
	// find and return tag-point
	if ((ti = m_mapTagPoints.find(name)) == m_mapTagPoints.end())
		return 0;
	else
		return (ITagPoint *) ti->second;
}

//////////////////////////////////////////////////////////////////////
bool CXGame::RenameTagPoint(const string &oldname, const string &newname)
{
	TagPointMap::iterator ti;
	// find tag-point
	if (( ti = m_mapTagPoints.find(oldname)) != m_mapTagPoints.end())
	{
		// does the new name already exist ?
		if (m_mapTagPoints.find(newname) == m_mapTagPoints.end())
		{
			// change name
			CTagPoint *pPoint = ti->second;
			pPoint->OverrideName(newname);

			m_mapTagPoints.erase(oldname);
			m_mapTagPoints.insert(TagPointMap::iterator::value_type(newname,pPoint));

			return true;
		}
		else 
			return false;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
//! Remove all tag-points from the list
void CXGame::ClearTagPoints()
{
	if (!m_mapTagPoints.empty())
	{
		TagPointMap::iterator ti;
		for (ti=m_mapTagPoints.begin();ti!=m_mapTagPoints.end();ti++)
			(ti->second)->Release();

		m_mapTagPoints.clear();
	}
	if(m_pServer)m_pServer->ClearRespawnPoints();
}

//////////////////////////////////////////////////////////////////////////
void CXGame::AddRespawnPoint(ITagPoint *pPoint)
{
//	m_pServer->m_vRespawnPoints.push_back(pPoint);
	m_pServer->AddRespawnPoint(pPoint);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::RemoveRespawnPoint(ITagPoint *pPoint)
{
	m_pServer->RemoveRespawnPoint(pPoint);
}
