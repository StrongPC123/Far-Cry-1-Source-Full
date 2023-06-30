//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSystemClient.cpp
//  Description: Implemetation of the IXSystem interface for the client.
//
//  History:
//  - August 8, 2001: Created by Alberto Demichelis
//  - September 24,2001: Modified by Petar Kotevski
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSystemClient.h"
//#include "TeamMgr.h"

#include "XPlayer.h"
#include "I3DEngine.h"
#include "UIHud.h"
#include <ISound.h>

#include <IMovieSystem.h>
#include <IConsole.h>
#include <ICryPak.h>

///////////////////////////////////////////////
CXSystemClient::CXSystemClient(CXGame *pGame,ILog *pLog):CXSystemBase(pGame,pLog)
{
	m_pEntitySystem->EnableClient(true);
}

///////////////////////////////////////////////
CXSystemClient::~CXSystemClient()
{
	DeleteAllEntities();
}

///////////////////////////////////////////////
void CXSystemClient::Release()
{
	delete this;
}

///////////////////////////////////////////////
bool CXSystemClient::LoadLevel(const char *szLevelDir,const char *szMissionName, bool bEditor)
{
	SMissionInfo missionInfo;
	missionInfo.bEditor = bEditor;
	missionInfo.SetLevelFolder(szLevelDir);
	if (szMissionName)
		missionInfo.sMissionName = szMissionName;

	StartLoading(bEditor);

	if (!LoadLevelCommon(missionInfo))
	{
		EndLoading(bEditor);
		return false;
	}

	EndLoading(bEditor);
	m_pGame->m_bMapLoadedFromCheckpoint=false;

	return (true);
}

///////////////////////////////////////////////
IEntity*	CXSystemClient::SpawnEntity(CEntityDesc &ed)
{
	IEntity *pE=m_pEntitySystem->SpawnEntity(ed);
	//reset the pos to 0 0 0 because the server doesn't send the pos into the description
	//pE->SetPos( Vec3d(0.0f, 0.0f, 0.0f) );
	if (pE && pE->GetPhysics())
	{
		pe_params_flags pf;
		pf.flagsOR = pef_monitor_impulses;
		pE->GetPhysics()->SetParams(&pf);
	}
		

	return pE;
}

///////////////////////////////////////////////
void CXSystemClient::RemoveEntity(EntityId wID, bool bRemoveNow)
{
	m_pEntitySystem->RemoveEntity(wID, bRemoveNow);
}

///////////////////////////////////////////////
void CXSystemClient::DeleteAllEntities()
{
#if !defined(LINUX)	
	IMovieSystem *pMovieSystem=m_pSystem->GetIMovieSystem();
	if (pMovieSystem)
		pMovieSystem->Reset(false);
#endif
	m_pEntitySystem->Reset();
}

///////////////////////////////////////////////
void CXSystemClient::Disconnected(const char *szCause)
{
	if(!szCause) szCause = "NULL ERROR";
	TRACE("Client Disconnected");
	TRACE(szCause);
//	m_pLog->LogToConsole("Client Disconnected %s",GetEnglish(szCause)->c_str());
	m_pLog->LogToConsole("Client Disconnected %s",szCause);
}

void CXSystemClient::SetVariable(const char *sName,const char *sValue)
{
	ICVar *pCVar=m_pConsole->GetCVar(sName);
	if(pCVar){
		m_pLog->LogToConsole("SETTING %s=%s",sName,sValue);
		pCVar->Set(sValue);
	}
}
