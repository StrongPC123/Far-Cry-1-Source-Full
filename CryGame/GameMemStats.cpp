
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <crysizer.h>
#include "Game.h" 
#include "TagPoint.h"
#include "XSurfaceMgr.h"
#include "EntityClassRegistry.h"
#include "XServer.h"
#include "XServerSlot.h"
#include "XPlayer.h"
#include "XPuppetProxy.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"

#include "ScriptObjectLanguage.h"
#include "ScriptObjectPlayer.h"
#include "ScriptObjectSpectator.h"
#include "ScriptObjectVehicle.h"
#include "ScriptObjectStream.h"
#include "ScriptObjectAI.h"
#include "ScriptObjectBoids.h"

//////////////////////////////////////////////////////////////////////////
void CXGame::GetMemoryStatistics(ICrySizer *pSizer)
{
unsigned size;

	pSizer->AddObject( this, sizeof *this );
	pSizer->AddObject( &m_EntityClassRegistry, m_EntityClassRegistry.MemStats() );
	pSizer->AddObject( &m_XAreaMgr, m_XAreaMgr.MemStat() );
	pSizer->AddObject( &m_XDemoMgr, sizeof(m_XDemoMgr) );

	TagPointMap::iterator tpItr = m_mapTagPoints.begin();
	for(size=0;tpItr!=m_mapTagPoints.end();tpItr++)
	{
		size +=	(tpItr->first).capacity();
		size += (tpItr->second)->MemStats();
	}
	pSizer->AddObject(&m_mapTagPoints, size);
	pSizer->AddObject(&m_XSurfaceMgr, m_XSurfaceMgr.MemStat());
	if(m_pServer)
		pSizer->AddObject(m_pServer, m_pServer->MemStats());
	if(m_pClient)
		pSizer->AddObject(m_pClient, sizeof( *m_pClient ) );

	pSizer->AddObject( m_pScriptObjectGame, sizeof *m_pScriptObjectGame);
	pSizer->AddObject( m_pScriptObjectInput, sizeof *m_pScriptObjectInput);

	pSizer->AddObject( m_pScriptObjectBoids, sizeof *m_pScriptObjectBoids);
	pSizer->AddObject( m_pScriptObjectLanguage, sizeof *m_pScriptObjectLanguage);
	pSizer->AddObject( m_pScriptObjectAI, sizeof *m_pScriptObjectAI);

	size=0;
	for(ActionsEnumMap::iterator aItr=m_mapActionsEnum.begin(); aItr!=m_mapActionsEnum.end(); aItr++)
	{
		size += (aItr->first).capacity();
		size += sizeof( ActionInfo );
		ActionInfo *curA = &(aItr->second);
		size += curA->sDesc.capacity();
		for(unsigned int i=0; i< curA->vecSetToActionMap.size(); i++)
			size += curA->vecSetToActionMap[i].capacity();
	}
	pSizer->AddObject( &m_mapActionsEnum, size);

	if(m_pWeaponSystemEx)
		pSizer->AddObject( m_pWeaponSystemEx, m_pWeaponSystemEx->MemStats());

	size = 0;
	IEntityIt *eItr=m_pSystem->GetIEntitySystem()->GetEntityIterator();
	IEntity* ent;
	while((ent=eItr->Next())!=NULL)
	{
		IEntityContainer *pCnt=ent->GetContainer();
		CPlayer *pPlayer = NULL;

		if(pCnt)
		{
			pCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer);
			if( pPlayer )
				size += pPlayer->MemStats();
		}
	}

	pSizer->AddObject( "players from entSystem", size);
}

//////////////////////////////////////////////////////////////////////////
unsigned	CTagPoint::MemStats()
{
	unsigned memSize = sizeof *this;
	memSize += m_sName.capacity();
	return memSize;
}

//////////////////////////////////////////////////////////////////////////
unsigned CXSurfaceMgr::MemStat()
{
unsigned size = sizeof *this;

	for(PhysicalSurfecesMapItor pmi=m_mapPhysSurfaces.begin(); pmi!=m_mapPhysSurfaces.end(); pmi++)
		size += (pmi->second).capacity() + sizeof(int);	
	for(MaterialsMapItor mmi=m_mapMaterials.begin(); mmi!=m_mapMaterials.end(); mmi++)
		size += (mmi->first).capacity() + sizeof(int);	
	for(MaterialsNamesMapItor mni=m_mapMaterialsNames.begin(); mni!=m_mapMaterialsNames.end(); mni++)
	{
		MatDesc &mdesc = mni->second;
		size += (mni->first).capacity() + mdesc.sScriptFilename.capacity() + sizeof(mdesc);
	}

  return size;
}

//////////////////////////////////////////////////////////////////////////
unsigned CEntityClassRegistry::MemStats()
{
unsigned size = sizeof *this;

	for(EntityClassMapItor itr=m_vEntityClasses.begin(); itr!=m_vEntityClasses.end(); itr++)
	{
	  size += sizeof(EntityClass) + (itr->second).strFullScriptFile.capacity()
						+ (itr->second).strScriptFile.capacity()
						+ (itr->second).strClassName.capacity();
	}
	
	return size;
}

//////////////////////////////////////////////////////////////////////////
unsigned CXServerSlot::MemStats()
{
unsigned size = sizeof *this;
	size += m_strPlayerName.capacity();
	size += m_strPlayerModel.capacity();
	return size;
}

//////////////////////////////////////////////////////////////////////////
unsigned CPlayer::MemStats()
{
	unsigned size = sizeof *this;
	size += m_mapPlayerWeapons.size()*(sizeof(WeaponInfo) + sizeof(int) + sizeof(WeaponInfo*) );
	size += sizeof(CXPuppetProxy);
	return size;
}

