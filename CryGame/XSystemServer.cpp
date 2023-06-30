//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSystemServer.cpp
//  Description: Implemetation of the IXSystem interface for the server.
//
//  History:
//  - August 8, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSystemServer.h"

#include "XPlayer.h"
#include "WeaponSystemEx.h"
#include "XVehicleSystem.h"
#include "TagPoint.h"

#include <IAISystem.h>
#include <ISound.h>
#include <I3DEngine.h>
#include <IMovieSystem.h>
#include <ICryPak.h>


//////////////////////////////////////////////////////////////////////
CXSystemServer::CXSystemServer(CXServer *pXServer,CXGame *pGame, ILog *pLog):CXSystemBase(pGame, pLog)
{
	m_pXServer = pXServer;
	m_pEntitySystem->EnableServer(true);
}

//////////////////////////////////////////////////////////////////////
CXSystemServer::~CXSystemServer()
{
	DeleteAllEntities();
}

//////////////////////////////////////////////////////////////////////
void CXSystemServer::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////
bool CXSystemServer::LoadLevel(const char *szLevelDir,const char *szMissionName, bool bEditor)
{
	assert(!m_pGame->m_pServer->m_bIsLoadingLevel);
	m_pGame->m_pServer->m_bIsLoadingLevel=true;

	//////////////////////////////////////////////////////////////////////////
	// MP Stuff.
	//////////////////////////////////////////////////////////////////////////
	m_mapTeams.clear();
	AddTeam("spectators",0);
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	SMissionInfo missionInfo;
	missionInfo.bEditor = bEditor;
	missionInfo.SetLevelFolder(szLevelDir);
	if (szMissionName)
		missionInfo.sMissionName = szMissionName;
	// [anton] make sure physical world has the most recent IsMultiplayer flag before loading
	m_pSystem->GetIPhysicalWorld()->GetPhysVars()->bMultiplayer = m_pGame->IsMultiplayer() ? 1:0;

	StartLoading(bEditor);

	if (m_pGame->IsMultiplayer())
	{
		for (int i = 0; i < 10; i++) m_pXServer->UpdateXServerNetwork(); // Do some network updates.
	}

	if (!LoadLevelCommon(missionInfo))
	{
		EndLoading(bEditor);
		return false;
	}

	EndLoading(bEditor);

	m_pXServer->GetRules()->OnAfterLoad();

	if (m_pGame->IsMultiplayer())
	{
		for (int i = 0; i < 10; i++) m_pXServer->UpdateXServerNetwork(); // Do some network updates.
	}

	m_pSystem->GetINetwork()->OnAfterServerLoadLevel(
				m_pXServer->sv_name->GetString(),
				m_pXServer->sv_maxplayers->GetIVal(),
				m_pXServer->m_ServerInfos.nPort);

	m_pGame->m_bMapLoadedFromCheckpoint=false;
	m_pGame->m_pServer->m_bIsLoadingLevel=false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXSystemServer::OnReadyToLoadLevel( SMissionInfo &missionInfo )
{
	IGameMods *pGameMods=m_pGame->GetModsInterface();

	assert(pGameMods);			// Game Init failed?

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// INIT SERVER GAME RULES.
	m_pXServer->GetRules()->ShutDown();
	m_pXServer->GetRules()->Init(m_pGame, m_pSystem->GetIConsole(), m_pGame->GetScriptSystem(), m_pLog);
	m_pXServer->GetServerInfo();	// refill the server info structure
	m_pXServer->m_ServerInfos.strMap = missionInfo.sLevelName;
	m_pXServer->m_GameContext.strMapFolder = missionInfo.sLevelFolder;
	m_pXServer->m_GameContext.strMission = missionInfo.sMissionName;
	m_pXServer->m_GameContext.strGameType = m_pGame->g_GameType->GetString();
	m_pXServer->m_GameContext.strMod = pGameMods->GetCurrentMod();
	m_pXServer->m_GameContext.bInternetServer = (m_pXServer->m_ServerInfos.nServerFlags&SXServerInfos::FLAG_NET) != 0;
	m_pXServer->m_GameContext.bForceNonDevMode= (m_pXServer->m_ServerInfos.nServerFlags&SXServerInfos::FLAG_CHEATS) == 0;
	m_pXServer->m_GameContext.wLevelDataCheckSum = m_wCheckSum;
	//m_pXServer->m_GameContext.strGameStuffScript = m_pXServer->m_ServerRules.GetGameStuffScript();

	///////////////////////////////////////////////////////////////////////////////////////
	// Invalidate All players slots.
	// Make sure players cannot move anymore.
	///////////////////////////////////////////////////////////////////////////////////////
	{
		CXServer::XSlotMap::iterator itor = m_pXServer->GetSlotsMap().begin();
		while (itor != m_pXServer->GetSlotsMap().end())
		{
			itor->second->SetPlayerID(INVALID_WID);
			++itor;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	m_pLog->Log( "Tell Clients about the mapchange ...." );
	CXServer::XSlotMap::iterator itor = m_pXServer->GetSlotsMap().begin();
	while(itor != m_pXServer->GetSlotsMap().end())
	{
		CXServerSlot *slot=itor->second;

		m_pLog->Log("   name: '%s'",slot->GetName());
	
		slot->ContextSetup();
		slot->CleanupSnapshot();

		++itor;
	}
	if (m_pGame->IsMultiplayer())
	{
		for (int i = 0; i < 10; i++) m_pXServer->UpdateXServerNetwork(); // Do some network updates.
	}
}

//////////////////////////////////////////////////////////////////////
IEntity*	CXSystemServer::SpawnEntity(CEntityDesc &ed)
{
	//ed.id = 0; // force entity system to generate id
	IEntity* entity = m_pEntitySystem->SpawnEntity(ed);
	
	if (entity && m_pGame->IsMultiplayer())
	{
		entity->SetNeedUpdate(true);
	}

	return entity;
}

//////////////////////////////////////////////////////////////////////
void CXSystemServer::RemoveEntity(EntityId wID, bool bRemoveNow)
{
	m_pEntitySystem->RemoveEntity(wID, bRemoveNow);
}

//////////////////////////////////////////////////////////////////////
//!delete all entities
void CXSystemServer::DeleteAllEntities()
{
#if !defined(LINUX)	
		IMovieSystem *pMovieSystem=m_pSystem->GetIMovieSystem();
		if (pMovieSystem)
			pMovieSystem->Reset(false);
#endif
		m_pEntitySystem->Reset();
}

//////////////////////////////////////////////////////////////////////
void CXSystemServer::Disconnected(const char *szCause)
{
	// <<FIXME>> FINISH ME !
	int a=5;
}

void	CXSystemServer::BindEntity(EntityId idParent,EntityId idChild)
{
//	m_pXServer->BindEntity(idParent,idChild);
}

void	CXSystemServer::UnbindEntity(EntityId idParent,EntityId idChild)
{
//	m_pXServer->UnbindEntity(idParent,idChild);
}


//////////////////////////////////////////////////////////////////////
bool CXSystemServer::LoadCharacter(IEntity* pEntity, const char *szModel)
{
	// Timur[8/23/2001] 
	ASSERT(pEntity);
	
	return true;
}

bool CXSystemServer::WriteTeams(CStream &stm)
{
	stm.Write((BYTE)m_mapTeams.size());
	for (TeamsMapItor itor=m_mapTeams.begin();itor!=m_mapTeams.end();++itor)
	{
		itor->second.Write(stm);
	}
	return true;
}


void CXSystemServer::AddRespawnPoint(ITagPoint *tp)
{
	m_pGame->AddRespawnPoint( tp );
//	m_pXServer->m_vRespawnPoints.push_back(tp);
}

void CXSystemServer::OnSetVar(ICVar *pVar)
{
	if (m_pXServer)
		m_pXServer->SyncVariable(pVar);
}