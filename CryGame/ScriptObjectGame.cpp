
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//
// ScriptObjectGame.cpp: implementation of the CScriptObjectGame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#if defined LINUX
#include <sys/io.h>
#else
#include <io.h>
#endif
#include "Game.h"
#include "XServer.h"
#include "ScriptObjectGame.h"
#include "UIHud.h"
#include "ScriptObjectRenderer.h"
#include "PlayerSystem.h"
#include "Xplayer.h"
#include "Spectator.h"
#include "IngameDialog.h"
#include "XSystemBase.h"
#include "Flock.h"

#include <IEntitySystem.h>
#include "ScriptObjectVector.h"
#include "ScriptTimerMgr.h"
#include <ISound.h>

#include "WeaponClass.h"
#include "WeaponSystemEx.h"

// for the definition of AIObjectPlayer
#include <IAISystem.h>
#include <IAgent.h>
#include <ICryPak.h>

#include "GameMods.h"

#if !defined(LINUX)
#	include <direct.h>
#	pragma comment (lib, "version.lib")
#else
	#include <sys/stat.h>
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectGame)

CScriptObjectGame::CScriptObjectGame()
{
	m_pGame=NULL;

}

CScriptObjectGame::~CScriptObjectGame()
{
	while(!m_vRenderersObjs.empty())
	{
		CScriptObjectRenderer *p=m_vRenderersObjs.back();
		m_vRenderersObjs.pop_back();
		delete p;
	}
	if(m_psoNavigationPoint)
		m_psoNavigationPoint->Release();
	if(m_psoVector)
		m_psoVector->Release();
	
	//! Release players pool.
	for (unsigned int i = 0; i < m_pPlayersPool.size(); i++)
	{
		if (m_pPlayersPool[i])
			m_pPlayersPool[i]->Release();
	}
}

void CScriptObjectGame::Init(IScriptSystem *pScriptSystem,CXGame *pGame)
{
	m_pGame=pGame;
	m_pSystem=pGame->GetSystem();
	m_pConsole=m_pSystem->GetIConsole();
	m_pRenderer=m_pSystem->GetIRenderer();
	m_p3DEngine=m_pSystem->GetI3DEngine();
	m_pEntitySystem=m_pSystem->GetIEntitySystem();
	m_pPhysicalWorld=m_pSystem->GetIPhysicalWorld();
	m_pInput=m_pSystem->GetIInput();
	InitGlobal(pScriptSystem,"Game",this);
	m_psoNavigationPoint=pScriptSystem->CreateObject();
	m_psoVector=pScriptSystem->CreateObject();
	m_pScriptSystem->SetGlobalValue("PICK_SELONLY", PICK_SELONLY);
	m_pScriptSystem->SetGlobalValue("PICK_SELADD", PICK_SELADD);
	m_pScriptSystem->SetGlobalValue("PICK_SELSUB", PICK_SELSUB);

	m_pScriptSystem->SetGlobalValue("VF_REQUIRE_NET_SYNC", VF_REQUIRE_NET_SYNC);

	// player's vehicle states
	m_pScriptSystem->SetGlobalValue("PVS_OUT", CPlayer::PVS_OUT );
	m_pScriptSystem->SetGlobalValue("PVS_DRIVER", CPlayer::PVS_DRIVER );
	m_pScriptSystem->SetGlobalValue("PVS_GUNNER", CPlayer::PVS_GUNNER );
	m_pScriptSystem->SetGlobalValue("PVS_PASSENGER", CPlayer::PVS_PASSENGER );

	// entity classes
	m_pScriptSystem->SetGlobalValue("SPECTATOR_CLASS_ID",SPECTATOR_CLASS_ID);
	m_pScriptSystem->SetGlobalValue("ADVCAMSYSTEM_CLASS_ID",ADVCAMSYSTEM_CLASS_ID);
	m_pScriptSystem->SetGlobalValue("PLAYER_CLASS_ID",PLAYER_CLASS_ID);
	m_pScriptSystem->SetGlobalValue("SYNCHED2DTABLE_CLASS_ID",SYNCHED2DTABLE_CLASS_ID);	

	m_pGetTagPoint.Create( pScriptSystem );
}


void CScriptObjectGame::Reset()
{
	if(m_vRenderersObjs.size())
	{
		while(!m_vRenderersObjs.empty())
		{
			CScriptObjectRenderer *p=m_vRenderersObjs.back();
			m_vRenderersObjs.pop_back();
			delete p;
		}
	}
}

//------------------------------------------------------------------------------------------------- 
void CScriptObjectGame::OnNETServerFound(CIPAddress &ip, SXServerInfos &pServerInfo)
{
	_SmartScriptObject pServer(m_pScriptSystem);

	pServer->SetValue("Name", pServerInfo.strName.c_str());
	pServer->SetValue("Map", pServerInfo.strMap.c_str());
	pServer->SetValue("Players", (int)pServerInfo.nPlayers);
	pServer->SetValue("MaxPlayers", (int)pServerInfo.nMaxPlayers);
	pServer->SetValue("GameType", pServerInfo.strGameType.c_str());
	pServer->SetValue("Mod", pServerInfo.strMod.c_str());
	pServer->SetValue("Ping", (int)pServerInfo.nPing);
	pServer->SetValue("IP", pServerInfo.IP.GetAsString(true));
	pServer->SetValue("Password", (int)((pServerInfo.nServerFlags&SXServerInfos::FLAG_PASSWORD) ? 1 : 0));
	pServer->SetValue("CheatsEnabled", (int)((pServerInfo.nServerFlags&SXServerInfos::FLAG_CHEATS) ? 1 : 0));
	char str[80];
	pServerInfo.VersionInfo.ToString(str);
	pServer->SetValue("GameVersion", str);
	pServer->SetValue("InternetServer", (int)((pServerInfo.nServerFlags&SXServerInfos::FLAG_NET) ? 1 : 0));
	pServer->SetValue("ComputerType", (int)pServerInfo.nComputerType);
	pServer->SetValue("PunkBuster", (int)((pServerInfo.nServerFlags & SXServerInfos::FLAG_PUNKBUSTER) ? 1 : 0) );

	HSCRIPTFUNCTION pfOnNETServerFound = 0;

	if (m_pScriptThis->GetValue("OnNETServerFound", pfOnNETServerFound))
	{
		m_pScriptSystem->BeginCall(pfOnNETServerFound);
		m_pScriptSystem->PushFuncParam(GetScriptObject());
		m_pScriptSystem->PushFuncParam(pServer);
		m_pScriptSystem->EndCall();
	}
	m_pScriptSystem->ReleaseFunc(pfOnNETServerFound);
}

//------------------------------------------------------------------------------------------------- 
void CScriptObjectGame::OnNETServerTimeout(CIPAddress &ip)
{
	_SmartScriptObject pServer(m_pScriptSystem);

	pServer->SetValue("Name", "");
	pServer->SetValue("Map", "");
	pServer->SetValue("Players", 0);
	pServer->SetValue("MaxPlayers", 0);
	pServer->SetValue("GameType", "");
	pServer->SetValue("Mod", "");
	pServer->SetValue("Ping", 0);
	pServer->SetValue("IP", ip.GetAsString(true));
	pServer->SetValue("Password", 0);
	pServer->SetValue("CheatsEnabled", 0);
	pServer->SetValue("GameVersion", "");
	pServer->SetValue("InternetServer", 0);
	pServer->SetValue("ComputerType", 0);
	pServer->SetValue("PunkBuster", 0);

	HSCRIPTFUNCTION pfOnNETServerTimeout = 0;

	if (m_pScriptThis->GetValue("OnNETServerTimeout", pfOnNETServerTimeout))
	{
		m_pScriptSystem->BeginCall(pfOnNETServerTimeout);
		m_pScriptSystem->PushFuncParam(GetScriptObject());
		m_pScriptSystem->PushFuncParam(pServer);
		m_pScriptSystem->EndCall();
	}
	m_pScriptSystem->ReleaseFunc(pfOnNETServerTimeout);
}

//------------------------------------------------------------------------------------------------- 
void CScriptObjectGame::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectGame>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectGame,GetCDPath);
	REG_FUNC(CScriptObjectGame,GetUserName);
	REG_FUNC(CScriptObjectGame,Load);
	REG_FUNC(CScriptObjectGame,GetPlayers);
	REG_FUNC(CScriptObjectGame,SetHUDFont);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,WriteHudNumber);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,WriteHudString);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,WriteHudStringFixed);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,GetHudStringSize);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,GetServerList);
	REG_FUNC(CScriptObjectGame,GetMaterialIDByName);
	REG_FUNC(CScriptObjectGame,ReloadMaterialPhysics);
	REG_FUNC(CScriptObjectGame,GetActions);
	REG_FUNC(CScriptObjectGame,IsPlayer);
	REG_FUNC(CScriptObjectGame,GetEntitiesScreenSpace);
	REG_FUNC(CScriptObjectGame,GetPlayerEntitiesInRadius);
	REG_FUNC(CScriptObjectGame,DrawRadar);
	REG_FUNC(CScriptObjectGame,DrawHalfCircleGauge);
	REG_FUNC(CScriptObjectGame,ShowIngameDialog);
	REG_FUNC(CScriptObjectGame,HideIngameDialog);
	REG_FUNC(CScriptObjectGame,EnableUIOverlay);
	REG_FUNC(CScriptObjectGame,IsUIOverlay);
	REG_FUNC(CScriptObjectGame,GetEntityTeam);
	REG_FUNC(CScriptObjectGame,GetTeamScore);
	REG_FUNC(CScriptObjectGame,GetTeamFlags);
	REG_FUNC(CScriptObjectGame,Connect);
	REG_FUNC(CScriptObjectGame,Reconnect);
	REG_FUNC(CScriptObjectGame,Disconnect);
	REG_FUNC(CScriptObjectGame,GetLevelList);
	REG_FUNC(CScriptObjectGame,LoadLevel);
	REG_FUNC(CScriptObjectGame,GetLevelName);
	REG_FUNC(CScriptObjectGame,LoadLevelListen);
	REG_FUNC(CScriptObjectGame,LoadLevelMPServer);
	REG_FUNC(CScriptObjectGame,GetVersion);
	REG_FUNC(CScriptObjectGame,GetVersionString);
	REG_FUNC(CScriptObjectGame,CreateVariable);
	REG_FUNC(CScriptObjectGame,RemoveVariable);
	REG_FUNC(CScriptObjectGame,SetVariable);
	REG_FUNC(CScriptObjectGame,GetVariable);
	REG_FUNC(CScriptObjectGame,Save);
	REG_FUNC(CScriptObjectGame,Quit);
	REG_FUNC(CScriptObjectGame,IsPointInWater);
	REG_FUNC(CScriptObjectGame,GetWaterHeight);
	REG_FUNC(CScriptObjectGame,RefreshServerList);//<<FIXME>> move to Client
	REG_FUNC(CScriptObjectGame,ClearServerInfo);
	REG_FUNC(CScriptObjectGame,GetServerInfo);
	REG_FUNC(CScriptObjectGame,GetServerListInfo);
	REG_FUNC(CScriptObjectGame,ExecuteRConCommand);
	REG_FUNC(CScriptObjectGame,IsServer);
	REG_FUNC(CScriptObjectGame,IsClient);
	REG_FUNC(CScriptObjectGame,IsMultiplayer);
	REG_FUNC(CScriptObjectGame,SetTimer);
	REG_FUNC(CScriptObjectGame,KillTimer);
	REG_FUNC(CScriptObjectGame,StartRecord);
	REG_FUNC(CScriptObjectGame,StopRecord);
	REG_FUNC(CScriptObjectGame,StartDemoPlay);
	REG_FUNC(CScriptObjectGame,StopDemoPlay);
	REG_FUNC(CScriptObjectGame,DisplayNetworkStats);
	REG_FUNC(CScriptObjectGame,ForceScoreBoard);
	REG_FUNC(CScriptObjectGame,ReloadMaterials);
	REG_FUNC(CScriptObjectGame,GetTagPoint);
	REG_FUNC(CScriptObjectGame,GetMaterialBySurfaceID);
	REG_FUNC(CScriptObjectGame,ReloadWeaponScripts);
	REG_FUNC(CScriptObjectGame,AddWeapon);
	REG_FUNC(CScriptObjectGame,GetWeaponClassIDByName);
	REG_FUNC(CScriptObjectGame,SetThirdPerson);
	REG_FUNC(CScriptObjectGame,SetViewAngles);
	REG_FUNC(CScriptObjectGame,DumpEntities);
	REG_FUNC(CScriptObjectGame,TouchCheckPoint);
	REG_FUNC(CScriptObjectGame,LoadLatestCheckPoint);
	REG_FUNC(CScriptObjectGame,ShowSaveGameMenu);
	REG_FUNC(CScriptObjectGame,GetSaveGameList);
	REG_FUNC(CScriptObjectGame,ToggleMenu);
	REG_FUNC(CScriptObjectGame,ShowMenu);
	REG_FUNC(CScriptObjectGame,HideMenu);
	REG_FUNC(CScriptObjectGame,IsInMenu);
//	REG_FUNC(CScriptObjectGame,TraceGrenade);
	REG_FUNC(CScriptObjectGame,SendMessage);
	REG_FUNC(CScriptObjectGame,GetEntityClassIDByClassName);
	REG_FUNC(CScriptObjectGame,SetCameraFov);
	REG_FUNC(CScriptObjectGame,GetCameraFov);
	REG_FUNC(CScriptObjectGame,ApplyStormToEnvironment);
	REG_FUNC(CScriptObjectGame,CreateExplosion);
	REG_FUNC(CScriptObjectGame,DrawLabel);
	REG_FUNC(CScriptObjectGame,GetInstantHit);
	REG_FUNC(CScriptObjectGame,GetMeleeHit);
	REG_FUNC(CScriptObjectGame,SaveConfiguration);
	REG_FUNC(CScriptObjectGame,LoadConfiguration);
	REG_FUNC(CScriptObjectGame,LoadConfigurationEx);
	REG_FUNC(CScriptObjectGame,RemoveConfiguration);	
	REG_FUNC(CScriptObjectGame,DrawHealthBar);
	REG_FUNC(CScriptObjectGame,__RespawnEntity);
	REG_FUNC(CScriptObjectGame,ListPlayers);
	REG_FUNC(CScriptObjectGame,LoadScript);
	REG_FUNC(CScriptObjectGame,ForceEntitiesToSleep);
	REG_FUNC(CScriptObjectGame,CreateRenderer);
	REG_FUNC(CScriptObjectGame,SoundEvent);
	REG_FUNC(CScriptObjectGame,CheckMap);
	REG_FUNC(CScriptObjectGame,GetMapDefaultMission);
	REG_FUNC(CScriptObjectGame,CleanUpLevel);
	REG_FUNC(CScriptObjectGame,SavePlayerPos);
	REG_FUNC(CScriptObjectGame,LoadPlayerPos);
	REG_FUNC(CScriptObjectGame,PlaySubtitle);
	//REG_FUNC(CScriptObjectGame,SetListener);
	REG_FUNC(CScriptObjectGame,GetModsList);
	REG_FUNC(CScriptObjectGame,LoadMOD);
	REG_FUNC(CScriptObjectGame,GetCurrentModName);
	REG_FUNC(CScriptObjectGame,AddCommand);
	REG_FUNC(CScriptObjectGame,EnableQuicksave);
}

int CScriptObjectGame::GetCDPath(IFunctionHandler *pH)
{
	string szCDPath;
	
	if (m_pGame->GetCDPath(szCDPath))
	{
		return pH->EndFunction(szCDPath.c_str());
	}

	return pH->EndFunctionNull();
}

int CScriptObjectGame::GetUserName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pSystem->GetUserName());
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::ForceEntitiesToSleep(IFunctionHandler *pH)
{
	if (!m_pGame->IsDevModeEnable())
		return (pH->EndFunction());

	IEntity *pLocal=m_pGame->GetMyPlayer();
	IEntityItPtr pEntities=m_pSystem->GetIEntitySystem()->GetEntityIterator();
	pEntities->MoveFirst();
	IEntity *pEnt=NULL;
	while((pEnt=pEntities->Next())!=NULL)
	{
		//EntityClass *pClass=pECR->GetByClass(pEnt->GetEntityClassName());
		if (pEnt==pLocal)
			continue; // do not put to sleep ourselves

		IAIObject *pAI=pEnt->GetAI();
		if (!pAI || (pAI->GetType()==AIOBJECT_PLAYER))
			continue;
		
		pAI->Event(AIEVENT_SLEEP,0);
		pEnt->SetSleep(true);
		
		//Vec3 pos = pEnt->GetPos();
		//m_pSystem->GetILog()->Log("ENTITY class=%s/%d ent=%s/%d pos=(%.1f,%.1f,%.1f)", pClass->strClassName.c_str(), pClass->cTypeID, pEnt->GetName(), pEnt->GetId(), pos.x, pos.y, pos.z);
	}

	return (pH->EndFunction());
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectGame::__RespawnEntity(IFunctionHandler *pH)
{
	int id=0;
	pH->GetParam(1,id);
	if(id)
	{
		IEntity *pEnt=m_pEntitySystem->GetEntity(id);
		if(pEnt){
			CEntityDesc ed;
			pEnt->GetEntityDesc(ed);
			m_pEntitySystem->RemoveEntity(id,true);
			m_pEntitySystem->SpawnEntity(ed);
		}

	}
	return pH->EndFunction();
}

int CScriptObjectGame::ListPlayers(IFunctionHandler *pH)
{
	if (!m_pGame->GetXSystem() || !m_pGame->m_pServer)
	{
		return pH->EndFunctionNull();
	}

	CXServer::XSlotMap &Slots = m_pGame->m_pServer->GetSlotsMap();

	m_pConsole->PrintLine("\tid      name");

	for(CXServer::XSlotMap::iterator it = Slots.begin(); it != Slots.end(); ++it)
	{
		CXServerSlot *Slot = it->second;

		if(Slot->GetPlayerId() != INVALID_WID)
		{
			char szLine[512] = {0};
			sprintf(szLine, "\t%d   %s", Slot->GetID(), Slot->GetName());

			m_pConsole->PrintLine(szLine);
		};
	}

	return pH->EndFunction();
}

//! ReloadWeaponScripts
int CScriptObjectGame::ReloadWeaponScripts(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pGame->ReloadWeaponScripts();

	return pH->EndFunction();
}

//! Make sure the weapon is loaded
int CScriptObjectGame::AddWeapon(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *pszDesc;
	pH->GetParam(1, pszDesc);

	if(pszDesc)
	{	
		m_pGame->GetWeaponSystemEx()->AddWeapon(pszDesc);
	}
	else m_pScriptSystem->RaiseError("Game:AddWeapon parameter is invalid"); 

	return pH->EndFunction();
}

//! ReloadWeaponScripts
int CScriptObjectGame::GetWeaponClassIDByName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *name;
	pH->GetParam(1, name);

	if(name)
	{
		int val = m_pGame->GetWeaponSystemEx()->GetWeaponClassIDByName(name);
		if (val >= 0)
			return pH->EndFunction(val);
		else
			return pH->EndFunctionNull();
	}
	
	m_pScriptSystem->RaiseError("Game:GetWeaponClassIDByName parameter is invalid"); 

	return pH->EndFunctionNull();
}

int CScriptObjectGame::ReloadMaterials(IFunctionHandler *pH)
{
	m_pGame->m_XSurfaceMgr.LoadMaterials("scripts/materials",true);
	return pH->EndFunction();
}

/*! set a timer callback
	@param table the object that will receive the OnEvent with ScriptEvent_Timer as eventid
	@param fMilliseconds duration on the timer
	@param pParam optional table that will be passed back by the callback
	@return the timer id
*/
int CScriptObjectGame::SetTimer(IFunctionHandler *pH)
{
	if(pH->GetParamCount()<2)
	{
		m_pScriptSystem->RaiseError("Game.SetTimer wrong number of arguments"); 
		return pH->EndFunctionNull();
	}
	float fMilliseconds;
	IScriptObject *pParam=m_pScriptSystem->CreateEmptyObject();
	IScriptObject *pTable=m_pScriptSystem->CreateEmptyObject();
	pH->GetParam(1,pTable);
	pH->GetParam(2,fMilliseconds);
	if(!pH->GetParam(3,pParam))
	{
		pParam->Release();
		pParam=NULL;
	}
	ITimer *pTimer=m_pGame->GetSystem()->GetITimer();
	return pH->EndFunction(m_pGame->m_pScriptTimerMgr->AddTimer(pTable,(unsigned int)(pTimer->GetCurrTime()*1000),(unsigned int)(fMilliseconds),pParam,true));
}

/*! snooze a timer event
	@param nTimerID the timer id returned by Game.SetTimer
*/
int CScriptObjectGame::KillTimer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nTimerID;
	pH->GetParam(1,nTimerID);
	m_pGame->m_pScriptTimerMgr->RemoveTimer(nTimerID);
	return pH->EndFunction();
}

//!refresh the server list from the LAN network
int CScriptObjectGame::RefreshServerList(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pGame->RefreshServerList();
	
	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::ClearServerInfo(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	INETServerSnooper *pSnooper = m_pGame->GetNETSnooper();
	assert(pSnooper);

	pSnooper->ClearList();

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::GetServerInfo(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	INETServerSnooper *pSnooper = m_pGame->GetNETSnooper();
	assert(pSnooper);

	int		iPort;
	char	*szIP = 0;

	if (!pH->GetParam(1, szIP) || !pH->GetParam(2, iPort))
		return pH->EndFunctionNull();

	CIPAddress ip(iPort, szIP);
	pSnooper->AddServer(ip);

	return pH->EndFunction(1);
}


//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::ExecuteRConCommand(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	IRConSystem *pRCon = m_pGame->GetIRConSystem();
	assert(pRCon);

	char *szCommand = 0;

	if(!pH->GetParam(1, szCommand))
		return pH->EndFunctionNull();

	pRCon->ExecuteRConCommand(szCommand);

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::GetServerListInfo(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	INETServerSnooper *pSnooper = m_pGame->GetNETSnooper();
	assert(pSnooper);

	if (pH->GetParamType(1) != svtObject)
		return pH->EndFunctionNull();

	std::vector<CIPAddress> vIP;

	IScriptObject *pServerList = m_pScriptSystem->CreateEmptyObject();
	pH->GetParam(1, pServerList);

	pServerList->BeginIteration();
	while(pServerList->MoveNext())
	{
		int		iPort = 0;
		char	*szIP = 0;

		IScriptObject *pServer = m_pScriptSystem->CreateEmptyObject();
		pServer->GetValue("IP", (const char* &)szIP);
		pServer->GetValue("Port", iPort);
		pSnooper->Release();

		if (!szIP || !iPort)
		{
			continue;
		}

		vIP.push_back(CIPAddress(iPort, szIP));
	}
	pServerList->EndIteration();
	pServerList->Release();

	pSnooper->AddServerList(vIP);

	return pH->EndFunction(1);
}

/*!return all player entity in game
	@return table filled with all player entity in game
*/
int CScriptObjectGame::GetPlayers(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	_SmartScriptObject pObj(m_pScriptSystem);

	if(!m_pGame->GetXSystem())
	{
		// no multiplayer system
		return pH->EndFunction(*pObj);
	}

	IEntityItPtr	pItor = m_pGame->GetXSystem()->GetEntities();
	IEntity			*pEntity = 0;

	int k = 1;

	while (pEntity = pItor->Next())
	{	
		if (m_pGame->GetXSystem()->GetEntityTeam(pEntity->GetId()) < 0)
		{
			continue;
		}

		pObj->SetAt(k, pEntity->GetScriptObject());

		k++;
	}
	pObj->SetNullAt(k);

	return pH->EndFunction(*pObj);
}

/*! set the font used by the functions WriteHudStrings and WriteHudNumber
	@param fontname string enumeration the font name
	@param effectname string enumerationg the font shader
*/
int CScriptObjectGame::SetHUDFont(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CUIHud *pHud = m_pGame->GetHud();
	const char *pFontname;
	const char *pEffectname;
	pH->GetParam(1,pFontname);
	pH->GetParam(2,pEffectname);
	pHud->SetFont(pFontname, pEffectname);

	IFFont *pFont = pHud->Getfont();
	if (pFont)
	{
		pFont->Reset();
	}

	return pH->EndFunction();
}

void SetFont(char *pszFontName, char *pszEffectName);

int CScriptObjectGame::GetHudStringSize(IFunctionHandler *pH)
{
	float xsize = 10.0f, ysize = 10.0f, fWrapWidth=0;

	pH->GetParam(2,xsize);
	pH->GetParam(3,ysize);

	if (pH->GetParamCount() > 3)
	{
		pH->GetParam(4, fWrapWidth);
	}

	wstring swString;
	const char *sStringKey = 0;

	if (pH->GetParam(1,sStringKey))
	{
		m_pGame->m_StringTableMgr.Localize( sStringKey, swString );
	}

	IFFont *pFont = m_pGame->GetHud()->Getfont();
	assert(pFont);

	pFont->Reset();
	pFont->SetSize(vector2f(xsize, ysize));

	vector2f l;

	if (fWrapWidth > 0)
	{
		l = pFont->GetWrappedTextSizeW(swString.c_str(), fWrapWidth);
	}
	else
	{
		l = pFont->GetTextSizeW(swString.c_str());
	}

	float x = l.x/m_pRenderer->ScaleCoordX(1);
	float y = l.y/m_pRenderer->ScaleCoordY(1);
	return pH->EndFunction(x,y);
	
	//Timur Old not efficient way.
	//_SmartScriptObject vec(m_pScriptSystem);  
	//vec->SetValue("x", x);
  //vec->SetValue("y", y);
	//return pH->EndFunction(vec);
}

/*! print a string into the Hud
	@param px x coordinate into the screen(the screen is always normalized to 800x600)
	@param py y coordinate into the screen(the screen is always normalized to 800x600)
	@param number number to print
	@param r red component of the color used to print the number
	@param g green component of the color used to print the number
	@param b blue component of the color used to print the number
	@param bxsize witdh of a single character
	@param ysize height of a single character
*/
int CScriptObjectGame::WriteHudNumber(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(9);
	CUIHud *pHud = m_pGame->GetHud();
	int px,py,number;
	float r,g,b,a,bxsize,ysize;
	pH->GetParam(1,px);
	pH->GetParam(2,py);
	pH->GetParam(3,number);
	pH->GetParam(4,r);
	pH->GetParam(5,g);
	pH->GetParam(6,b);
	pH->GetParam(7,a);
	pH->GetParam(8,bxsize);
	pH->GetParam(9,ysize);
	pHud->WriteNumber(px,py,number,r,g,b,a,bxsize,ysize);
	return pH->EndFunction();
}

/*! print a string into the Hud with variable size fonts(a letter 'm' is wider than 'i')
	@param px x coordinate into the screen(the screen is always normalized to 800x600)
	@param py y coordinate into the screen(the screen is always normalized to 800x600)
	@param string string to print
	@param r red component of the color used to print the number
	@param g green component of the color used to print the number
	@param b blue component of the color used to print the number
	@param a alpha component of the color used to print the number
	@param bxsize witdh of a single character
	@param ysize height of a single character 
	@param bCenter center the message on screen (returns the starting pos if center was true)
	
	*/
int CScriptObjectGame::WriteHudString(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(8);
	if(pH->GetParamCount()<9)
	{
		m_pScriptSystem->RaiseError("CScriptObjectGame::WriteHudString wrong number of arguments"); 
		return pH->EndFunction();
	}

	CUIHud *pHud = m_pGame->GetHud();
	int px,py;
	float r,g,b,a,bxsize,ysize;
	//const char *pszStr;
	bool bCenter=false;
	float fWrapWidth=0;

	pH->GetParam(1,px);
	pH->GetParam(2,py);

	wstring swString;
	//pH->GetParam(3,pszStr);
	//m_pGame->m_StringTableMgr.GetString(pH,3,sString,swString);

	const char *sStringKey = 0;
	if (pH->GetParam(3,sStringKey))
	{
		m_pGame->m_StringTableMgr.Localize( sStringKey,swString );
	}


	pH->GetParam(4,r);
	pH->GetParam(5,g);
	pH->GetParam(6,b);
	pH->GetParam(7,a);
	pH->GetParam(8,bxsize);
	pH->GetParam(9,ysize);
	
	IFFont *pFont=pHud->Getfont();
	pFont->Reset();

	if(pH->GetParamCount()>=10)
	{
		pH->GetParam(10,bCenter);

		if (pH->GetParamCount() >= 11)
		{
			pH->GetParam(11, fWrapWidth);
		}		

		if (bCenter)
		{
			vector2f vLen;
			if (fWrapWidth > 0)
			{
				vLen=pFont->GetWrappedTextSizeW(swString.c_str(), fWrapWidth);
			}
			else
			{
				vLen=pFont->GetTextSizeW(swString.c_str());
			}
			
			vLen.x*=(m_pSystem->GetIRenderer()->GetWidth()/800.0f);
			px=800/2-(int)(vLen.x/2);
		}
	}
	
	pHud->WriteString(px,py,swString.c_str(),r,g,b,a,bxsize,ysize, fWrapWidth);
	return pH->EndFunction(px);
} 

//////////////////////////////////////////////////////////////////////
/*! print a string into the Hud with fixed size(both letter 'm' and 'i' have the same width)
	@param px x coordinate into the screen(the screen is always normalized to 800x600)
	@param py y coordinate into the screen(the screen is always normalized to 800x600)
	@param string string to print
	@param r red component of the color used to print the number
	@param g green component of the color used to print the number
	@param b blue component of the color used to print the number
	@param bxsize witdh of a single character
	@param ysize height of a single character
*/ 
int CScriptObjectGame::WriteHudStringFixed(IFunctionHandler *pH)
{
//#ifdef _XBOX
//  CHECK_PARAMETERS(9);
//#else
  CHECK_PARAMETERS(10);
//#endif
	CUIHud *pHud = m_pGame->GetHud();
	int px,py;
	float r,g,b,a,bxsize,ysize,fWidthScale;
	//const char *pszStr;
  /*
#ifdef _XBOX
	pH->GetParam(1,px);
	pH->GetParam(2,py);
	pH->GetParam(3,pszStr);
	pH->GetParam(4,r);
	pH->GetParam(5,g);
	pH->GetParam(6,b);
	pH->GetParam(7,bxsize);
	pH->GetParam(8,ysize);
	pH->GetParam(9,fWidthScale);
	pHud->WriteStringFixed(px,py,(char*)pszStr,r,g,b,1.0,bxsize,ysize,fWidthScale);
#else
  */
  pH->GetParam(1,px);
  pH->GetParam(2,py);

	wstring swString;
	//pH->GetParam(3,pszStr);
	//m_pGame->m_StringTableMgr.GetString(pH,3,sString,swString);
	
	const char *sStringKey = 0;
	if (pH->GetParam(3,sStringKey))
	{
		m_pGame->m_StringTableMgr.Localize( sStringKey,swString );
	}

	IFFont *pFont=pHud->Getfont();
	pFont->Reset();
  
  pH->GetParam(4,r);
  pH->GetParam(5,g);
  pH->GetParam(6,b);
  pH->GetParam(7,a);
  pH->GetParam(8,bxsize);
  pH->GetParam(9,ysize);
  pH->GetParam(10,fWidthScale);	
	pHud->WriteStringFixed(px,py,swString.c_str(),r,g,b,a,bxsize,ysize,fWidthScale);
//#endif
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
/*!under development ...right now doesn't do anything
*/
int CScriptObjectGame::DisplayNetworkStats(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IFFont *pFont=pFont=m_pSystem->GetICryFont()->GetFont("Default");
	pFont->SetCharWidthScale(1.0f);
	vector2f hsize (16,16);		
	pFont->SetSize(hsize);
	pFont->SetSameSize(false);
	color4f hcolor(1,1,1,1.0f);				
	pFont->SetColor(hcolor);
	pFont->DrawString(10,100,"Network stats");
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
/*!Get the list of server on the network with related infos
	@return a table with the server infos
*/
int CScriptObjectGame::GetServerList(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	_SmartScriptObject pObj(m_pScriptSystem);
	int k=1;

	for (ServerInfosVecItor	i=m_pGame->m_ServersInfos.begin();i!=m_pGame->m_ServersInfos.end();i++)
	{

		SXServerInfos tServInfo=(*i).second;

	    _SmartScriptObject pInner(m_pScriptSystem);

			pInner->SetValue("Name", tServInfo.strName.c_str());
			pInner->SetValue("Map", tServInfo.strMap.c_str());
			pInner->SetValue("Players", (int)tServInfo.nPlayers);
			pInner->SetValue("MaxPlayers", (int)tServInfo.nMaxPlayers);
			pInner->SetValue("Mod", tServInfo.strMod.c_str());
			pInner->SetValue("GameType", tServInfo.strGameType.c_str());
			pInner->SetValue("Ping", (int)tServInfo.nPing);
			pInner->SetValue("IP", tServInfo.IP.GetAsString(true));
			pInner->SetValue("Password", (int)((tServInfo.nServerFlags&SXServerInfos::FLAG_PASSWORD) ? 1 : 0));
			pInner->SetValue("CheatsEnabled", (int)((tServInfo.nServerFlags&SXServerInfos::FLAG_CHEATS) ? 1 : 0));
			char str[80];
			tServInfo.VersionInfo.ToString(str);
			pInner->SetValue("GameVersion", str);
			pInner->SetValue("InternetServer", (int)((tServInfo.nServerFlags&SXServerInfos::FLAG_NET) ? 1 : 0));
			pInner->SetValue("ComputerType", (int)tServInfo.nComputerType);
			pInner->SetValue("PunkBuster", (int)((tServInfo.nServerFlags&SXServerInfos::FLAG_PUNKBUSTER) ? 1 : 0) );

		pObj->SetAt(k, *pInner);k++;				
	} //i
	
	return pH->EndFunction(*pObj);
}



/*!create a local client and connect it to a server
	@param sServer string containing the server name or ip number
	@param bShowConsole (optional, default true)
*/
int CScriptObjectGame::Connect(IFunctionHandler *pH)
{
	bool bDoLateSwitch=false, bDoCDAuthorization=false;
	//if a local server exist shutdown it
	//m_pGame->ShutdownServer();
	
	const char *sServer=NULL;

	if(pH->GetParamCount()!=0)
		pH->GetParam(1,sServer);

	if(pH->GetParamCount()>1)
		pH->GetParam(2,bDoLateSwitch);

	if(pH->GetParamCount()>2)
		pH->GetParam(3,bDoCDAuthorization);

	m_pGame->ShutdownClient();
	m_pGame->ShutdownServer();

	if(sServer==NULL)
	{
		if(!m_pGame->m_pServer)
			return pH->EndFunction();

		sServer = "127.0.0.1";
	}
	bool bReturn = true;

	if (!m_pGame->m_bEditor)
	{
		HSCRIPTFUNCTION pfnOnConnectBegin = m_pScriptSystem->GetFunctionPtr("Game", "OnConnectBegin");

		if (pfnOnConnectBegin)
		{
			m_pScriptSystem->BeginCall(pfnOnConnectBegin);
			m_pScriptSystem->PushFuncParam(m_pGame->GetScriptObject());
			m_pScriptSystem->PushFuncParam(sServer);
			m_pScriptSystem->EndCall(bReturn);

			m_pScriptSystem->ReleaseFunc(pfnOnConnectBegin);
		}
	}

	if (!bReturn)
	{
		return pH->EndFunctionNull();
	}

	m_pGame->StartupClient();

	// if bDoLateSwitch is true then it does the 3 lines below but after the connection is completed.
	m_pGame->m_pClient->XConnect((char *)sServer, bDoLateSwitch, bDoCDAuthorization);

	return pH->EndFunction();
}

/*!create a local client and reconnect to the last server
@param sServer string containing the server name or ip number
@param bShowConsole (optional, default true)
*/
int CScriptObjectGame::Reconnect(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (!m_pGame->m_szLastAddress.empty())
	{
		m_pScriptSystem->BeginCall("Game", "Connect");
		m_pScriptSystem->PushFuncParam(this->GetScriptObject());
		m_pScriptSystem->PushFuncParam(m_pGame->m_szLastAddress.c_str());
		m_pScriptSystem->PushFuncParam((int)(m_pGame->m_bLastDoLateSwitch ? 1 : 0));
		m_pScriptSystem->PushFuncParam((int)(m_pGame->m_bLastCDAuthentication ? 1 : 0));
		m_pScriptSystem->EndCall();
	}
	else
	{
		m_pConsole->PrintLine("No previous connect command.");
	}

	return pH->EndFunction();
}


/*!disconnect the current connection to a remote server
@param sCause string describing the cause of the disconnection[optional]
*/
int CScriptObjectGame::Disconnect(IFunctionHandler *pH)
{
	int iCount=pH->GetParamCount();

	if(iCount>1)
	{
		m_pScriptSystem->RaiseError("Game.Disconnect too many parameters");
		return pH->EndFunction();
	}

	const char *sCause=0;

	if(iCount>0)
		pH->GetParam(1,sCause);			// sCause might get 0 when LUA passed a nil value

	if(!sCause)
		sCause="@UserDisconnected";

//	if(m_pGame->m_pClient)

//	m_pGame->m_pClient->XDisconnect(sCause);
	m_pGame->ShutdownClient();
	m_pGame->ShutdownServer();

	return pH->EndFunction();
}

// Scans the given vector of strings for presence of szString
// case-insensitive
// returns true when the string is present in the array
bool HasStringI (const std::vector<string> &arrStrings, const char* szString)
{
	for (std::vector<string>::const_iterator it = arrStrings.begin(); it != arrStrings.end(); ++it)
		if (!stricmp(it->c_str(), szString))
			return true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
// Returns the table - list of levels with the given mission in them
int CScriptObjectGame::GetLevelList (IFunctionHandler* pH)
{
	// the first and only parameter is the name of the mission
	// empty name means all levels will be returned
	const char* pszMissionFilter = NULL; 

	if (pH->GetParamCount()>=1 && !pH->GetParam(1,pszMissionFilter))
	{
		m_pScriptSystem->RaiseError (  "CScriptObjectGame::GetLevelList : 1st (%s) of %d arguments couldn't be resolved as mission name string.", ScriptVarTypeAsCStr(pH->GetParamType(1)), pH->GetParamCount());
		return pH->EndFunctionNull();
	}

	_SmartScriptObject pObj(m_pScriptSystem);
	int nLevel = 1;
 
	string sLevelsFolder = m_pGame->GetLevelsFolder();
	//struct __finddata64_t fd;
	//intptr_t	hFind;

	struct _finddata_t c_file;
  intptr_t hFile;

	ICryPak *pIPak = m_pSystem->GetIPak();

	if ((hFile = pIPak->FindFirst((sLevelsFolder+string("/*.*")).c_str(),&c_file)) == -1L )					
		return (pH->EndFunction(*pObj));

	//if ((hFind = _findfirst64( (sLevelsFolder+"/*.*").c_str(), &fd )) == -1L)
		//return (pH->EndFunction(*pObj));

	// warning: this should be empty here
	std::vector<string> arrMissions;

	do {
		arrMissions.clear();

		if ((strncmp(c_file.name, ".",1)!=0) &&
			(c_file.attrib & _A_SUBDIR)	&& m_pGame->GetLevelMissions( (sLevelsFolder + "/" +c_file.name).c_str(), arrMissions) &&
      (!pszMissionFilter || HasStringI(arrMissions, pszMissionFilter)))
		{
			_SmartScriptObject pLevelObj(m_pScriptSystem);

			pLevelObj->SetValue("Name", c_file.name);

			_SmartScriptObject pMissionObj(m_pScriptSystem);

			for(int i = 0; i < (int)arrMissions.size(); i++)
			{
				static char szIndex[4];
				pMissionObj->SetValue(itoa(i+1, szIndex, 10), arrMissions[i].c_str());
			}

			pLevelObj->SetValue("MissionList", pMissionObj);
			
			// we found a level
			pObj->SetAt (nLevel, pLevelObj);

			++nLevel;
		}
	//} while(_findnext64(hFind, &fd)==0);
	//_findclose(hFind);

	} while(pIPak->FindNext( hFile, &c_file ) == 0);
	pIPak->FindClose( hFile );

	return pH->EndFunction(*pObj);
}

/*!Load a level ,start a local client and connect it to the local server, no external connections (sp game)
	@param sMapName the name of the map to load
	@param sMissionName the name of the mission[optional]
*/
int CScriptObjectGame::LoadLevel(IFunctionHandler *pH)
{	
	const char *szMapName;
	const char *szMissionName = "";

	if(m_pGame->m_bDedicatedServer)
		return pH->EndFunction();

	pH->GetParam(1,szMapName);
	if(pH->GetParamCount()==2) pH->GetParam(2,szMissionName);

	ICVar* pVar = m_pSystem->GetIConsole()->GetCVar("g_GameType");
	pVar->Set("Default");

	m_pGame->m_tPlayerPersistentData.m_bDataSaved=false;
	m_pGame->LoadLevelCS(false, szMapName, szMissionName, false);
	
	return pH->EndFunction();
}

/*!Load a level, start a local client and connect it to the local server, and allow external connections
	@param sMapName the name of the map to load
	@param sMissionName the name of the mission[optional]
*/
int CScriptObjectGame::LoadLevelListen(IFunctionHandler *pH)
{	
	const char *szMapName;
	const char *szMissionName = "";

	pH->GetParam(1,szMapName);
	if(pH->GetParamCount()==2) pH->GetParam(2,szMissionName);


	m_pGame->LoadLevelCS(false, szMapName, szMissionName, true);
	
	return pH->EndFunction();
}



/*!Load a level on a mp server, keeping the current clients connected to the current server
	@param sMapName the name of the map to load
	@param sMissionName the name of the mission[optional]
*/
int CScriptObjectGame::LoadLevelMPServer(IFunctionHandler *pH)
{	
	const char *szMapName;
	const char *szMissionName = "";

	pH->GetParam(1,szMapName);
	if(pH->GetParamCount()==2) pH->GetParam(2,szMissionName);

	m_pGame->LoadLevelCS(true, szMapName, szMissionName, true);
	
	return pH->EndFunction();
}

/*! Get the game version as a string
*/
int CScriptObjectGame::GetVersion(IFunctionHandler *pH)
{
	unsigned int		dwVersion = GAME_VERSION;
	unsigned char		bPrimary = (GAME_VERSION & 0xff000000) >> 24;
	unsigned char		bSecond = (GAME_VERSION & 0xff0000) >> 16;
	unsigned short	wBuild = (GAME_VERSION & 0xffff);

	char szVersionString[128] = {0};

	if (pH->GetParamCount() > 0)
	{
		char *szFormat = 0;

		pH->GetParam(1, szFormat);

		if (szFormat)
		{
			sprintf(szVersionString, szFormat, bPrimary, bSecond, wBuild);

			return pH->EndFunction(szVersionString);
		}
	}

	sprintf(szVersionString, "%d.%02d.%03d", bPrimary, bSecond, wBuild);

	return pH->EndFunction(szVersionString);
}

int CScriptObjectGame::GetVersionString(IFunctionHandler *pH)
{
#if !defined(LINUX)
	char szDate[128] = __DATE__;
	
	char *szMonth = szDate;
	char *szDay = szDate + 4;
	char *szYear = szDate + 9;

	unsigned int dwMonth = 0;
	switch(*((int *)szMonth))
	{
	case ' naJ':
		dwMonth = 1;
		break;
	case ' beF':
		dwMonth = 2;
		break;
	case ' raM':
		dwMonth = 3;
		break;
	case ' rpA':
		dwMonth = 4;
		break;
	case ' yaM':
		dwMonth = 5;
		break;
	case ' nuJ':
		dwMonth = 6;
		break;
	case ' luJ':
		dwMonth = 7;
		break;
	case ' guA':
		dwMonth = 8;
		break;
	case ' peS':
		dwMonth = 9;
		break;
	case ' tcO':
		dwMonth = 10;
		break;
	case ' voN':
		dwMonth = 11;
		break;
	case ' ceD':
		dwMonth = 12;
		break;
	default:
		assert(0);
	}

  szDay[2] = 0;

	int iBuild = 0;
	m_pSystem->GetFileVersion();

	// get the .exe file name
	char *szLocalCmdLine = GetCommandLine();
	char *p = szLocalCmdLine+1;
	
	while (*p && *p != '"') ++p;

	string szFileName(szLocalCmdLine+1, p);

	unsigned int dwVersionSize = GetFileVersionInfoSize((char *)szFileName.c_str(), 0);

	if (dwVersionSize)
	{
		unsigned char *pData = new unsigned char [dwVersionSize+1];

		if (!pData)
		{
			assert(0);
		}
		else
		{
			char							szQueryInfoString[256] = {0};
			char							*szFileVersionInfo = 0;
			unsigned int			dwSize;
			PUINT pVersionBuffer = NULL;

			if (GetFileVersionInfo((char *)szFileName.c_str(), 0, dwVersionSize, pData))
			{
				strcpy(szQueryInfoString, "\\VarFileInfo\\Translation");

				if (VerQueryValue(pData, szQueryInfoString, (LPVOID*)&pVersionBuffer, &dwSize) && dwSize == 4)
				{
					UINT dwLang = *pVersionBuffer;
					
					sprintf(szQueryInfoString, "\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion", (dwLang & 0xff00) >> 8, dwLang & 0xff,(dwLang & 0xff000000) >> 24, (dwLang & 0xff0000) >> 16);
				}
				else
				{
					sprintf(szQueryInfoString, "\\StringFileInfo\\%04X04B0\\FileVersion", GetUserDefaultLangID());
				}

				if (VerQueryValue(pData, szQueryInfoString, (void **)&szFileVersionInfo, &dwSize) && szFileVersionInfo)
				{
					char *szTok = strtok(szFileVersionInfo, ".");
					szTok = strtok(0, ".");

					if (szTok)
					{
						szTok = strtok(0, ".");

						if (szTok)
						{
							szTok = strtok(0, ".");

							if (szTok)
							{
								iBuild = atoi(szTok);
							}
						}
					}
					else
					{
						char *szTok = strtok(szFileVersionInfo, ",");
						szTok = strtok(0, ",");

						if (szTok)
						{
							szTok = strtok(0, ",");

							if (szTok)
							{
								szTok = strtok(0, ",");

								if (szTok)
								{
									iBuild = atoi(szTok);
								}
							}
						}
					}
				}
			}

			delete[] pData;
		}
	}

	char szVersionString[128];
	sprintf(szVersionString, "FC%s%d%s%s%d", szDay, dwMonth, szYear, GAME_VERSION_SUFIX, iBuild);
#else
	char szVersionString[ 128 ];
	sprintf( szVersionString, "FC %s %d (LINUX)", __DATE__, GAME_VERSION_SUFIX);
#endif
	return pH->EndFunction(szVersionString);
}

/*!create a console variable
	@param sName name of the console variable
	@param sDefault the default value[optional]
*/
int CScriptObjectGame::CreateVariable(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1);
	int nPCount=pH->GetParamCount();
	const char *sName;
	const char *sDefault;
	const char *sflags;
	int iflags=0;
  pH->GetParam(1,sName);
	if(nPCount>1)
	{
		pH->GetParam(2,sDefault);
		if (nPCount>2)
		{
			switch(pH->GetParamType(3))
			{
			case svtString:
				if (pH->GetParam(3,sflags))
				{
					if (strcmp(sflags, "NetSynch")==0)
						iflags=VF_REQUIRE_NET_SYNC;
				}
				else
					m_pSystem->GetILog()->LogWarning("Game:CreateVariable can't get the 3rd parameter (string)");
				break;
			case svtNumber:
				if (pH->GetParam(3, iflags))
				{
					// do nothing, the flags must be the VF_* flags
				}
				else
					m_pSystem->GetILog()->LogWarning("Game:CreateVariable can't get the 3rd parameter (number)");
				break;
			default:
				m_pSystem->GetILog()->LogWarning("Game:CreateVariable unexpected 3rd (flags) parameter type (%s)", ScriptVarTypeAsCStr(pH->GetParamType(3)));
				break;
			}
			
		}
	}	
	else
	{
		sDefault="0";
	}


	m_pConsole->CreateVariable( sName,sDefault,iflags);
//	TRACE("CreateVariable %s=%s flags=0x%x",sName,sDefault,iflags);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::AddCommand(IFunctionHandler *pH)
{
	int nPCount=pH->GetParamCount();
	const char *sName;
	const char *sCommand;
	const char *sHelp=NULL;
	int iflags=0;
	pH->GetParam(1,sName);
	if (nPCount>1)
	{
		pH->GetParam(2,sCommand);
		if (nPCount>2)
		{
			if (!pH->GetParam(3,sHelp))
				sHelp=NULL;			
		}
		if (sHelp)
			m_pConsole->AddCommand(sName,sCommand,0,sHelp);
		else
			m_pConsole->AddCommand(sName,sCommand,VF_NOHELP,"");
	}
	
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::GetLevelName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pGame->GetLevelName());
}



int CScriptObjectGame::GetVariable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sName;
	if (!pH->GetParam(1,sName))
	{
		m_pScriptSystem->RaiseError ("GetVariable invalid parameter type %s, string expected", ScriptVarTypeAsCStr(pH->GetParamType(1)));
		return pH->EndFunctionNull();
	}
	
	ICVar* pVar = m_pSystem->GetIConsole()->GetCVar(sName);
	if (!pVar)
	{
		m_pScriptSystem->RaiseError("GetVariable invalid variable name \"%s\": no such variable found", sName);
		return pH->EndFunctionNull();
	}

	switch (pVar->GetType())
	{
	case CVAR_INT:
		return pH->EndFunction(pVar->GetIVal());
	case CVAR_FLOAT:
		return pH->EndFunction(pVar->GetFVal());
	case CVAR_STRING:
		return pH->EndFunction(pVar->GetString());
	default:
		return pH->EndFunctionNull();
	}
}

int CScriptObjectGame::SetVariable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *sName;
	if (!pH->GetParam(1,sName))
	{
		m_pScriptSystem->RaiseError ("SetVariable invalid parameter type %s, string expected", ScriptVarTypeAsCStr(pH->GetParamType(1)));
		return pH->EndFunctionNull();
	}

	ICVar* pVar = m_pSystem->GetIConsole()->GetCVar(sName);
	if (!pVar)
	{
		m_pScriptSystem->RaiseError("SetVariable invalid variable name \"%s\": no such variable found", sName);
		return pH->EndFunctionNull();
	}

	if ((pVar->GetFlags() & VF_CHEAT) && (!m_pGame->IsDevModeEnable()))
	{		
		// [martin] hide this message because we don't want to help hacking
		// m_pSystem->GetILog()->LogWarning ("\001 Variable %s is cheat protected.", sName);		
		return pH->EndFunctionNull();
	}

	switch (pH->GetParamType(2))
	{
	case svtNull:
		pVar->Set("");
		break;
	case svtString:
		{
			const char *pVal;
			if (!pH->GetParam(2, pVal))
			{
				m_pScriptSystem->RaiseError("SetVariable cannot retrieve the variable %s string value", sName);
				return pH->EndFunctionNull();
			}
			pVar->Set(pVal);
		}
		break;
	case svtNumber:
		{
			int nVal;
			float fVal;
			if (pH->GetParam(2, fVal))
				pVar->Set(fVal);
			else
			if (pH->GetParam(2, nVal))
				pVar->Set(nVal);
			else
			{
				m_pScriptSystem->RaiseError("SetVariable cannot retrieve the variable %s numeric value", sName);
				return pH->EndFunctionNull();
			}
		}
		break;

	default:
		m_pScriptSystem->RaiseError ("SetVariable cannot set the variable %s value: unsupported type %s", sName, ScriptVarTypeAsCStr(pH->GetParamType(2)));
		break;
	}
	return pH->EndFunction();
}


/*!save the game on a file
	@param sFileName the name of the target file[optional] the default "is farcry_save.sav"
*/
int CScriptObjectGame::Save(IFunctionHandler *pH)
{
	const char *sFileName="";
	if(pH->GetParamCount()) pH->GetParam(1,sFileName);
	m_pGame->Save(sFileName, NULL, NULL);
	return pH->EndFunction();
}

/*!load the game from a file
	@param sFileName the name of the target file[optional] the default "is farcry_save.sav"
*/
int CScriptObjectGame::Load(IFunctionHandler *pH)
{
	const char *sFileName="";
	if(pH->GetParamCount()) pH->GetParam(1,sFileName);
	m_pGame->Load(sFileName);
	return pH->EndFunction();
}

int CScriptObjectGame::ShowSaveGameMenu(IFunctionHandler *pH)
{
	return pH->EndFunction(!m_pGame->m_bEditor && !m_pGame->m_strLastSaveGame.empty() && !m_pGame->m_bMenuOverlay);
}

int CScriptObjectGame::LoadLatestCheckPoint(IFunctionHandler *pH)
{
	m_pGame->LoadLatest();
	return pH->EndFunction();
}

// call with the number of the checkpoint to cause a game to be saved for it

int CScriptObjectGame::TouchCheckPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	int cp = 0;
	_VERIFY(pH->GetParam(1, cp)); 
    _SmartScriptObject tpos(m_pScriptSystem, true);
    _VERIFY(pH->GetParam(2, tpos));
    _SmartScriptObject tangles(m_pScriptSystem, true);
    _VERIFY(pH->GetParam(3, tangles));
    float x, y, z, xa, ya, za;
    _VERIFY(tpos->GetValue("x", x));
    _VERIFY(tpos->GetValue("y", y));
    _VERIFY(tpos->GetValue("z", z));
    _VERIFY(tangles->GetValue("x", xa));
    _VERIFY(tangles->GetValue("y", ya));
    _VERIFY(tangles->GetValue("z", za));
    Vec3 pos(x, y, z);
    Vec3 angles(xa, ya, za);
	char buf[1024];
	if (m_pGame->m_pServer)
	{
		//[kirill]
		//this is moved here from bool CXGame::LoadFromStream(CStream &stm, bool isdemo)
		//instedad of overriding player's health on loading let's save desired value 
		//this change needen for quickload to use restore health 
		int playerCurHealth;
		IEntity *pLocalEnt=m_pGame->GetMyPlayer();
		CPlayer *pPlayer=NULL;
		IEntityContainer *pLocalCnt=pLocalEnt->GetContainer();
		if (pLocalCnt && pLocalCnt->QueryContainerInterface(CIT_IPLAYER, (void **)&pPlayer))
		{
			playerCurHealth = pPlayer->m_stats.health;
			if(m_pGame->p_restorehalfhealth->GetIVal())
			{
				pPlayer->m_stats.health = 255;	// [marco] danger! this should be set by
												// gamerules but it gets overwritten by the save checkpoint
				// [kirill]
				// this was requested by UBI. It's expected here that current health value is the maximum 
				//Everytime Jack dies he should respawn with half of his hit points instead of full health. 
				//Same mechanics for Val, she should get half her hit points everytime Jack respawns.
				pPlayer->m_stats.health/=2;
			}
			if (pPlayer->m_stats.health<128)
				pPlayer->m_stats.health = 128;
		}
		//this is moved is over

		sprintf(buf, "checkpoint_%s_%s_%d", m_pGame->g_LevelName->GetString(), m_pGame->m_pServer->m_GameContext.strMission.c_str(), cp);
		m_pGame->Save(buf, &pos, &angles);

		//[kirill]
		//restore player's health to current value (used some forced value for "checkpointRestoreHealth" trick)
		if( pPlayer )
			pPlayer->m_stats.health = playerCurHealth;
	}
	return pH->EndFunction();
}

/*!remove a console variable
	@param sName the name of the variable
*/
int CScriptObjectGame::RemoveVariable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sName;
	pH->GetParam(1,sName);
	m_pConsole->UnregisterVariable(sName,true);
	return pH->EndFunction();
}

/*!quit the game
*/
int CScriptObjectGame::Quit(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	//m_pConsole->Exit(NULL);
	m_pSystem->Quit();
	return pH->EndFunction();
}

/*!return !=nil if the specified coordinate is under the water level
	@param vPosition a table with the x,y,z fields containing the position that has to be tested
	@return !=nil(true) nil(false)
*/
int CScriptObjectGame::IsPointInWater(IFunctionHandler *pH)
{
	CScriptObjectVector vPosition(m_pScriptSystem,true);

	CHECK_PARAMETERS(1);

	pH->GetParam(1, *vPosition);

	if (m_pSystem->GetI3DEngine()->IsPointInWater(vPosition.Get()))
		return pH->EndFunction(true);
	else
		return pH->EndFunction(false);
}
/*!return the height of the water level
	@return the z value of the water level
*/
int CScriptObjectGame::GetWaterHeight(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(0);

	if (pH->GetParamCount()>0) 
	{
		CScriptObjectVector vPosition(m_pScriptSystem,true);
		pH->GetParam(1, *vPosition);		
		return pH->EndFunction(m_pSystem->GetI3DEngine()->GetWaterLevel(&vPosition.Get()));
	}

	return pH->EndFunction(m_pSystem->GetI3DEngine()->GetWaterLevel());
}

/*!
*/
int CScriptObjectGame::GetActions(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	_SmartScriptObject pObj(m_pScriptSystem);
	ActionsEnumMap &ActionsMap=m_pGame->GetActionsEnumMap();
	ActionsEnumMapItor It=ActionsMap.begin();
	int i=1;
	while (It!=ActionsMap.end())
	{
		_SmartScriptObject pVal(m_pScriptSystem);
		_SmartScriptObject pStrTbl(m_pScriptSystem);
		pVal->SetValue("name", It->first.c_str());
		pVal->SetValue("id", It->second.nId);
		pVal->SetValue("desc", It->second.sDesc.c_str());
		pVal->SetValue("configurable", It->second.bConfigurable);
		switch(It->second.nType)
		{
		case ACTIONTYPE_MOVEMENT:
			pVal->SetValue("type", "movement");
			break;
		case ACTIONTYPE_COMBAT:
			pVal->SetValue("type", "combat");
			break;
		case ACTIONTYPE_GAME:
			pVal->SetValue("type", "game");
			break;
		case ACTIONTYPE_MULTIPLAYER:
			pVal->SetValue("type", "multiplayer");
			break;
		case ACTIONTYPE_DEBUG:
			pVal->SetValue("type", "debug");
			break;
		default:
			pVal->SetValue("type", "misc");
		}

		int j=1;
		for (Vec2StrIt StrIt=It->second.vecSetToActionMap.begin();StrIt!=It->second.vecSetToActionMap.end();++StrIt)
		{
			pStrTbl->SetAt(j, (*StrIt).c_str());
			j++;
		}
		pVal->SetValue("actionmaps", pStrTbl);
		pObj->SetAt(i, pVal);
		i++;
		++It;
	}
	return pH->EndFunction(pObj);
}

/*!return true if a certain entity is a player
	@param the entity id
	@return !=nil(true) nil(false)
*/
int CScriptObjectGame::IsPlayer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nEntityId;
	pH->GetParam(1, nEntityId);
	IEntity *pEnt=m_pSystem->GetIEntitySystem()->GetEntity(nEntityId);
	if (!pEnt)
		return pH->EndFunctionNull();
	return pH->EndFunction(m_pGame->GetPlayerSystem()->IsPlayerClass(pEnt->GetClassId()));
}

/*!return true if the local host is a server
	@return !=nil(true) nil(false)
*/
int CScriptObjectGame::IsServer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pGame->IsServer());
}

/*!return true if the local host is a client
	@return !=nil(true) nil(false)
*/
int CScriptObjectGame::IsClient(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pGame->IsClient());
}

/*!return true if we are in multiplayer mode (being either a server or a client)
	@return !=nil(true) nil(false)
*/
int CScriptObjectGame::IsMultiplayer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pGame->IsMultiplayer());
}



int CScriptObjectGame::GetEntitiesScreenSpace(IFunctionHandler *pH)
{
	const char *pszBoneName=NULL;
	if (pH->GetParamCount()>=1)
		pH->GetParam(1, pszBoneName);
	_SmartScriptObject pTable(m_pScriptSystem);
	IEntityItPtr It=m_pSystem->GetIEntitySystem()->GetEntityInFrustrumIterator();
	CCamera Cam=m_pSystem->GetViewCamera();
	Vec3 CamVec=Cam.GetAngles();
	CamVec=ConvertToRadAngles(CamVec);
	int i=1;
	IEntity *pEnt;
	ray_hit RayHit;
	IEntity *pLocal=m_pGame->GetMyPlayer();
	while (pEnt=It->Next())
	{
		if (pEnt==pLocal)
			continue;
		if (!pEnt->IsTrackable())
			continue;
		IPhysicalEntity *pPE=pEnt->GetPhysics();
		if (!pPE)
			continue;
		CPlayer *pPlayer;
		IEntityContainer *pCnt=pEnt->GetContainer();
		if (!pCnt)
			continue;
		if (!pCnt->QueryContainerInterface(CIT_IPLAYER, (void **)&pPlayer))
			continue;
		if (!pPlayer->IsAlive() || !pPlayer->IsVisible()) // tiago: make sure only visible stuff is processed
			continue;

		pe_status_dynamics dynStats;

		Vec3 Min, Max;
		pEnt->GetBBox(Min, Max);
		Vec3 Center=(Max-Min)/2+Min;

		Vec3 Position;
		Vec3 Velocity;
		_SmartScriptObject pEntity(m_pScriptSystem);
		CScriptObjectVector pPosition(m_pScriptSystem);
		CScriptObjectVector pVelocity(m_pScriptSystem);
		float px, py, pz;
		float x, y, z;

		if (!pPE->GetStatus(&dynStats))
		{
			dynStats.v.Set(0,0,0);
		}
		else	
		{
			if (pszBoneName)	// if we want a bone instead of bbox-center lets do so...
			{
				IEntityCharacter *pIChar=pEnt->GetCharInterface();
				if (pIChar)
				{
					ICryCharInstance *cmodel=pIChar->GetCharacter(0);    
					if (cmodel)
					{
						ICryBone *pBone = cmodel->GetBoneByName(pszBoneName);
						if (pBone)
						{
							Center=pBone->GetBonePosition();


							Matrix44 m;
							m.SetIdentity();
							m=GetTranslationMat(pEnt->GetPos())*m;
							m=Matrix44::CreateRotationZYX(-pEnt->GetAngles()*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 



							Center=m.TransformPointOLD(Center);
						}
					} 
				} 
			}
		}

		Vec3 diff(Center-Cam.GetPos());

		if(GetLengthSquared(diff)>700*700)
			continue;
		if (m_pSystem->GetIPhysicalWorld()->RayWorldIntersection(vectorf(Cam.GetPos()), diff, 
			ent_terrain|ent_static,0, &RayHit, 1,pPE))
			continue;
		m_pSystem->GetIRenderer()->ProjectToScreen(Center.x, Center.y, Center.z, &px, &py, &pz);
		Position.x=(float)px*8.0f;
		Position.y=(float)py*6.0f;
		Position.z=(float)pz;
		if ((Position.x>=0.0f) &&
				(Position.y>=0.0f) &&
				(Position.x<=800.0f) &&
				(Position.y<=600.0f) &&
				(Position.z>0.0f))
		{
			Vec3 PrevPos=Center-(Vec3)dynStats.v;
			m_pSystem->GetIRenderer()->ProjectToScreen(PrevPos.x, PrevPos.y, PrevPos.z, &x, &y, &z);
			Velocity.x=(x-px)/100.0f;
			Velocity.y=(y-py)/100.0f;
			Velocity.z=CamVec*(Vec3)dynStats.v;
			pEntity->SetValue("pEntity", pEnt->GetScriptObject());
			pPosition=Position;
			pEntity->SetValue("Position", *pPosition);
			pVelocity=Velocity;
			pEntity->SetValue("Velocity", *pVelocity);
			float fVelLen=cry_sqrtf(Velocity.x*Velocity.x+Velocity.y*Velocity.y);
			pEntity->SetValue("VelLen", fVelLen);
			Vec3 pos2d=Position;
			pos2d.z=0;
			Vec3 screenCenter(400,300,0);
			pEntity->SetValue("DistFromCenter",GetDistance(screenCenter,pos2d));
			pTable->SetAt(i, *pEntity);
			
			i++;
		}		
	}
	return pH->EndFunction(*pTable);
}

int CScriptObjectGame::GetPlayerEntitiesInRadius(IFunctionHandler *pH)
{
  //CHECK_PARAMETERS(3);
  if(pH->GetParamCount()<3)
  {
    m_pScriptSystem->RaiseError("CScriptObjectGame::GetPlayerEntitiesInRadius wrong number of arguments"); 
    return pH->EndFunction();
  }

  ASSERT(m_pGame);
  ASSERT(m_pEntitySystem);
  _SmartScriptObject pVec(m_pScriptSystem, true);
  Vec3 Center;
  pH->GetParam(1, *pVec);
  pVec->GetValue("x", Center.x);
  pVec->GetValue("y", Center.y);
  pVec->GetValue("z", Center.z);
  float fRadius;
  pH->GetParam(2, fRadius);
  fRadius*=fRadius;	// square radius for faster check

  if(!m_pGame->IsClient())
  {
    return pH->EndFunctionNull();
  }

  // Get output table from script.
  _SmartScriptObject pTable(m_pScriptSystem,true);
  pH->GetParam(3, pTable);

  int iEntityMask=-1; 
  if(pH->GetParamCount()>3)
  {
    // 0 = return alive and trackable only, 1= return all
    pH->GetParam(4, iEntityMask);
  }

  EntitiesSet *psetPlayers=NULL;
  if (m_pGame->IsServer())
  {
    CXServer *pServer=m_pGame->GetServer();
    ASSERT(pServer);
    ASSERT(pServer->m_pISystem);
    psetPlayers=&(pServer->m_pISystem->GetPlayerEntities());
  }
  else
  {
    CXClient *pClient=m_pGame->GetClient();
    ASSERT(pClient);
    ASSERT(pClient->m_pISystem)
      psetPlayers=&(pClient->m_pISystem->GetPlayerEntities());
  }

  IEntityContainer *pCnt;
  CPlayer *pPlayer;
  int nCount=1;
  unsigned int playerIndex = 0;

  if (psetPlayers->empty())
  {
    return pH->EndFunctionNull();
  }

  bool bEmpty=true;
  for (EntitiesSetItor It=psetPlayers->begin();It!=psetPlayers->end();)
  {
    EntityId Id=(*It);   
    IEntity *pEntity=m_pEntitySystem->GetEntity(Id);

    // remove entity from list if not valid
    if((!pEntity) || ((pCnt=pEntity->GetContainer())==NULL) || (!pCnt->QueryContainerInterface(CIT_IPLAYER, (void **)&pPlayer)))
    {       
      EntitiesSetItor next = It;
      next++;
      psetPlayers->erase(It);
      It = next;
      continue;			
    }

    // skip not needed entities
    bool bEntityState=1;
    if(iEntityMask==0 || iEntityMask==-1)
    {
      bEntityState= !pEntity->IsTrackable() || !pPlayer->IsAlive();
    }
    if(iEntityMask==1)
    {
      bEntityState= !pEntity->IsTrackable();    
    }

    if(bEntityState)
    {
      ++It;
      continue;			
    }

    float fDist2=GetSquaredDistance(pEntity->GetPos(), Center);
    if (fDist2<=fRadius)
    {
      IScriptObject *pEntitySO = 0;      
      if (playerIndex >= m_pPlayersPool.size())
      {
        // Makes new pool object.
        pEntitySO = m_pScriptSystem->CreateObject();
        m_pPlayersPool.push_back( pEntitySO );
      }
      else
      {
        // Takes object from pool.
        pEntitySO = m_pPlayersPool[playerIndex];
      }

      bEmpty=false;      
      pEntitySO->SetValue("nId", pEntity->GetId()); // removed- not being used anywhere
      pEntitySO->SetValue("pEntity", pEntity->GetScriptObject());
      pEntitySO->SetValue("fDistance2", fDist2);
      pTable->SetAt(nCount++, pEntitySO);      
      playerIndex++;
    }
    ++It;
  }

  if(!bEmpty)
  {
    return pH->EndFunction();
  }

  return pH->EndFunction();
}
 
#define NUM_RADAR_TEXTURES 7
//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::DrawRadar(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(14);

  ASSERT(m_pEntitySystem);  
  float x, y, w, h, fRange;
  char *pRadarObjective;
  int nCookie=0;

  _SmartScriptObject pEntities(m_pScriptSystem, true);
  pH->GetParam(1, x);
  pH->GetParam(2, y);
  pH->GetParam(3, w);
  pH->GetParam(4, h);
  pH->GetParam(5, fRange);

  // get radar textures id
  INT_PTR pRadarTextures[NUM_RADAR_TEXTURES];
  memset(pRadarTextures, 0, NUM_RADAR_TEXTURES*sizeof(INT_PTR));

	for (int k=0;k<NUM_RADAR_TEXTURES;k++)
	{
		if (!(pH->GetParamUDVal(k+6, pRadarTextures[k], nCookie) && (nCookie==USER_DATA_TEXTURE)))		
			return pH->EndFunction();		
	} //k

  pH->GetParam(6+NUM_RADAR_TEXTURES, *pEntities); 
  pH->GetParam(6+NUM_RADAR_TEXTURES+1, pRadarObjective); 
  
  m_pGame->DrawRadar(x, y, w, h, fRange, pRadarTextures, &pEntities, pRadarObjective);
  return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::DrawHalfCircleGauge(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<10)
		CHECK_PARAMETERS(10);
	if (!m_pRenderer)
		return pH->EndFunctionNull();	// no renderer
	int nCookie=0;
	float x, y, w, h;	// size of gauge
	float u, v, uw, vh;	// tex-coords
	float r=1.0f, g=1.0f, b=1.0f, a=1.0f;
	INT_PTR nTid;	// texture
	float fValue;	// 0<=value<=100
	pH->GetParam(1, x);
	pH->GetParam(2, y);
	pH->GetParam(3, w);
	pH->GetParam(4, h);
	pH->GetParam(5, u);
	pH->GetParam(6, v);
	pH->GetParam(7, uw);
	pH->GetParam(8, vh);
	if (!(pH->GetParamUDVal(9, nTid, nCookie) && (nCookie==USER_DATA_TEXTURE)))
	{
		return pH->EndFunctionNull();	// invalid texture handle
	}
	pH->GetParam(10, fValue);
	if (pH->GetParamCount()>=14)
	{
		pH->GetParam(11, r);
		pH->GetParam(12, g);
		pH->GetParam(13, b);
		pH->GetParam(14, a);
	}
	if (fValue<0.0f)
		fValue=0.0f;
	if (fValue>100.0f)
		fValue=100.0f;
	struct _vtx_
	{
		float x,y,z;
		unsigned int c;
		float u,v;
	};
	unsigned int dwColor=((unsigned int)(a*255.0f)<<24) | ((unsigned int)(b*255.0f)<<16) | ((unsigned int)(g*255.0f)<<8) | (unsigned int)(r*255.0f);
	const _vtx_ vtxpos[6]={
		{x+1     , y  , 0.0f, dwColor, u        , v   },
		{x+1     , y+h, 0.0f, dwColor, u        , v+vh},
		{x+w*0.5f, y  , 0.0f, dwColor, u+uw*0.5f, v   },
		{x+w*0.5f, y+h, 0.0f, dwColor, u+uw*0.5f, v+vh},
		{x+w-1   , y  , 0.0f, dwColor, u+uw     , v   },
		{x+w-1   , y+h, 0.0f, dwColor, u+uw     , v+vh}
	};
  m_pRenderer->Set2DMode(true, 800, 600);
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
  m_pRenderer->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  m_pRenderer->SetTexture(nTid);
  m_pRenderer->SetTexClampMode(true);
  m_pRenderer->SetCullMode(R_CULL_DISABLE);


	float w2=w*0.5f;
	float uw2=uw*0.5f;
	if (fValue>75.0f)
	{
		const int nVerts=7;
		_vtx_ vtx[nVerts];
		vtx[0]=vtxpos[5]; vtx[1]=vtxpos[4]; vtx[2]=vtxpos[3]; vtx[3]=vtxpos[2]; vtx[4]=vtxpos[3]; vtx[5]=vtxpos[0]; vtx[6]=vtxpos[1];
		_vtx_ &pt=vtx[0];
		float fMult=(100.0f-fValue);
		pt.y=pt.y-(( h/25.0f)*fMult);
		pt.v=pt.v-((vh/25.0f)*fMult);
		CVertexBuffer vb(&vtx, VERTEX_FORMAT_P3F_COL4UB_TEX2F, nVerts);
		m_pRenderer->DrawTriStrip(&vb, nVerts);
	}else
	if (fValue>50.0f)
	{
		const int nVerts=7;
		_vtx_ vtx[nVerts];
		vtx[0]=vtxpos[4]; vtx[1]=vtxpos[4]; vtx[2]=vtxpos[3]; vtx[3]=vtxpos[2]; vtx[4]=vtxpos[3]; vtx[5]=vtxpos[0]; vtx[6]=vtxpos[1];
		_vtx_ &pt=vtx[0];
		float fMult=(75.0f-fValue);
		pt.x=pt.x-(( w2/25.0f)*fMult);
		pt.u=pt.u-((uw2/25.0f)*fMult);
		vtx[1]=vtx[0];
		CVertexBuffer vb(&vtx, VERTEX_FORMAT_P3F_COL4UB_TEX2F, nVerts);
		m_pRenderer->DrawTriStrip(&vb, nVerts);
	}else
	if (fValue>25.0f)
	{
		const int nVerts=5;
		_vtx_ vtx[nVerts];
		vtx[0]=vtxpos[2]; vtx[1]=vtxpos[2]; vtx[2]=vtxpos[3]; vtx[3]=vtxpos[0]; vtx[4]=vtxpos[1];
		_vtx_ &pt=vtx[0];
		float fMult=(50.0f-fValue);
		pt.x=pt.x-(( w2/25.0f)*fMult);
		pt.u=pt.u-((uw2/25.0f)*fMult);
		vtx[1]=vtx[0];
		CVertexBuffer vb(&vtx, VERTEX_FORMAT_P3F_COL4UB_TEX2F, nVerts);
		m_pRenderer->DrawTriStrip(&vb, nVerts);
	}else
	{
		const int nVerts=3;
		_vtx_ vtx[nVerts];
		vtx[0]=vtxpos[3]; vtx[1]=vtxpos[0]; vtx[2]=vtxpos[1];
		_vtx_ &pt=vtx[1];
		float fMult=(25.0f-fValue);
		pt.y=pt.y+(( h/25.0f)*fMult);
		pt.v=pt.v+((vh/25.0f)*fMult);
		CVertexBuffer vb(&vtx, VERTEX_FORMAT_P3F_COL4UB_TEX2F, nVerts);
		m_pRenderer->DrawTriStrip(&vb, nVerts);
	}

  m_pRenderer->SetState(GS_DEPTHWRITE);
	m_pRenderer->SetTexClampMode(false);
	m_pRenderer->Set2DMode(false, 0, 0);
	return pH->EndFunction(1);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::ShowIngameDialog(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<5)
		return (pH->EndFunctionNull());
	int nId=-1;
	int nFillId;
	int nSize;	
	const char *pszFontName;
	const char *pszEffectName;
	//const char *pszText=NULL;
	// read params
	pH->GetParam(1,nFillId);
	pH->GetParam(2,pszFontName);
	pH->GetParam(3,pszEffectName);
	pH->GetParam(4,nSize);
//	pH->GetParam(5,pszText);

	string sStr;
	wstring swStr;
	const char *sStringKey = 0;
	if (pH->GetParam(5,sStringKey))
	{
		m_pGame->m_StringTableMgr.Localize( sStringKey,swStr );
	}

	float fTimeout=0.0f;
	if (pH->GetParamCount()>=6)
		pH->GetParam(6,fTimeout);

	if (m_pGame && (!sStr.empty() || !swStr.empty()))
	{
		CIngameDialogMgr *pMgr=m_pGame->GetIngameDialogManager();
		if (pMgr)
		{
			nId=pMgr->AddDialog(m_pSystem, nFillId, pszFontName, pszEffectName, nSize, sStr,swStr, fTimeout);
		}
	}

	return (pH->EndFunction(nId));
}

int CScriptObjectGame::HideIngameDialog(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nId;
	// read params
	pH->GetParam(1,nId);
	if (m_pGame)
	{
		CIngameDialogMgr *pMgr=m_pGame->GetIngameDialogManager();
		if (pMgr)
		{
			pMgr->RemoveDialog(nId);
		}
	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::EnableUIOverlay(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int iEnable = 0;
	int iExclusiveInput = 0;

	if (pH->GetParamType(1) == svtNumber)
	{
		pH->GetParam(1, iEnable);
	}
	if (pH->GetParamType(2) == svtNumber)
	{
		pH->GetParam(2, iExclusiveInput);
	}

	m_pGame->EnableUIOverlay(iEnable != 0, iExclusiveInput != 0);

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::IsUIOverlay(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (m_pGame->IsUIOverlay())
	{
		return pH->EndFunction(1);
	}

	return pH->EndFunctionNull();
}
//------------------------------------------------------------------------------------------------- 


/*!return the team name of a certain entity
	@param the entity id
	@return the team name or nil if the entity is not in a team
*/
int CScriptObjectGame::GetEntityTeam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int id=0;
	if (pH->GetParam(1, id))
	{
		IXSystem *pSys=m_pGame->GetXSystem();

		int nTID =pSys->GetEntityTeam(id);
		if (nTID==-1)
		{
//			TRACE("Team is NULL");
			return pH->EndFunctionNull();
		}
		//TRACE("Team is %s", pTeam->GetName().c_str());
		//m_pGame->GetTeamManager()->DumpEnts();
		char sTeamName[256];
		pSys->GetTeamName(nTID,sTeamName);
		return pH->EndFunction(sTeamName);
	}else
		return pH->EndFunctionNull();
}

/*!return the material id by the material name
	@param sMaterialName the material name
	@return the material id or nil if the material doesn't exist or is not loaded in the current map
*/
int CScriptObjectGame::GetMaterialIDByName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sMaterialName;
	int nMaterialID;
	if(pH->GetParam(1,sMaterialName))
	{
		nMaterialID=m_pGame->m_XSurfaceMgr.GetSurfaceIDByMaterialName(sMaterialName);
		return pH->EndFunction(nMaterialID);
	}
	return pH->EndFunctionNull();
}

int CScriptObjectGame::ReloadMaterialPhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sMaterialName;
	if(pH->GetParam(1,sMaterialName))
		m_pGame->m_XSurfaceMgr.ReloadMaterialPhysics(sMaterialName);
	return pH->EndFunctionNull();
}


/*!return the material table passing is id
	@param the id of the material
	@return the material table or nil if the specified id is not related to any loaded material
*/
int CScriptObjectGame::GetMaterialBySurfaceID(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	//int nMaterialID;
	int surfaceID;
	if(pH->GetParam(1,surfaceID))
	{
		IScriptObject *pSO=m_pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(surfaceID);
		if(pSO)
		{
			return pH->EndFunction(pSO);
		}
		else
		{
			TRACE("[GetMaterialBySurfaceID] Warning MATERIAL IS NULL");
			return pH->EndFunctionNull();
		}
	}
	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::StartDemoPlay(IFunctionHandler *pH)
{
	const char *sFileName = "timedemo";
	
	if(pH->GetParamCount()==1)
	{
		pH->GetParam(1,sFileName);
	}

	if (sFileName)
		m_pGame->StartDemoPlay(sFileName);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::StopDemoPlay(IFunctionHandler *pH)
{
	m_pGame->StopDemoPlay();
	return pH->EndFunction();
}

int CScriptObjectGame::StartRecord(IFunctionHandler *pH)
{
	const char *sFileName = "timedemo";
	
	if(pH->GetParamCount()==1)
	{
		pH->GetParam(1,sFileName);
	}
	
	if (sFileName)
		m_pGame->StartRecording(sFileName);

	return pH->EndFunction();
}

int CScriptObjectGame::StopRecord(IFunctionHandler *pH)
{
	m_pGame->StopRecording();
	return pH->EndFunction();
}

/*! get the score of a cetain team
	@param sTeamName the team name
	@return the current team score or nil if the specified team doesn't exist
*/
int CScriptObjectGame::GetTeamScore(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sTeamName;
	
	if(pH->GetParam(1,sTeamName))
	{
		IXSystem *pSys=m_pGame->GetXSystem();
	
		int nTID=pSys->GetTeamId(sTeamName);

		if(nTID!=-1)
			return pH->EndFunction(pSys->GetTeamScore(nTID));
	}
	else
	{
		assert(0);				// don't pass nil as team name
	}

	return pH->EndFunction();
}

/*! get the score of a cetain team
	@param sTeamName the team name
	@return the current team score or nil if the specified team doesn't exist
*/
int CScriptObjectGame::GetTeamFlags(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sTeamName;
	if(pH->GetParam(1,sTeamName))
	{
		IXSystem *pSys=m_pGame->GetXSystem();

		int nTID=pSys->GetTeamId(sTeamName);
		if(nTID!=-1)
		{
			return pH->EndFunction(pSys->GetTeamFlags(nTID));
		}
	}
	//	}
	return pH->EndFunction();
}

/*! set/remove the scoreboard to a certain client connected to this srever
	@param reqid id of the player entity associated if this parameter is 0 brodcast the command to all clients
	@param isshown if !=nil activate the scoreboard if nil decativate it
	@return the current team score or nil if the specified team doesn't exist
*/
int CScriptObjectGame::ForceScoreBoard(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int reqid;
	bool isshown = false;
	pH->GetParam(1, reqid);
	pH->GetParam(2, isshown);
	CXServer *server = m_pGame->m_pServer;

	CXServer::XSlotMap &slots = server->GetSlotsMap();

	for(CXServer::XSlotMap::iterator i = slots.begin(); i != slots.end(); ++i)
	{
		CXServerSlot *slot = i->second;
		EntityId id = slot->GetPlayerId();
		if(id!=INVALID_WID)
		{
			IEntity *ent = server->m_pISystem->GetEntity(id);
			if(ent && ent->GetContainer())
			{
				CPlayer *pPlayer=0;
				CSpectator *pSpec=0;
				ent->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
				ent->GetContainer()->QueryContainerInterface(CIT_ISPECTATOR,(void**) &pSpec);

				if ((pPlayer || pSpec) && (!reqid || reqid==id))
				{
					slot->m_bForceScoreBoard = isshown;
				}
			}
		}
	}

	return pH->EndFunction();
}

int CScriptObjectGame::GetTagPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sTPName;
	ITagPoint *pTP=NULL;
	Vec3 vec(0,0,0);

	if(pH->GetParam(1,sTPName))
	{
		pTP=m_pGame->GetTagPoint(sTPName);
		if(!pTP) return pH->EndFunctionNull();

		pTP->GetPos(vec);
	}
	m_pGetTagPoint.Set(vec);
	return pH->EndFunction(m_pGetTagPoint);
}

int CScriptObjectGame::SetThirdPerson(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);

	bool bThirdPerson;
	pH->GetParam(1,bThirdPerson);

//[kirill] this check is done in m_pGame->SetViewMode
//	if (m_pGame->IsDevModeEnable())		
	m_pGame->SetViewMode(bThirdPerson);

	return pH->EndFunction();
}

int CScriptObjectGame::SetViewAngles(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	CScriptObjectVector oAng(m_pScriptSystem,true);
	if(pH->GetParam(1,oAng))
	{
		m_pGame->SetViewAngles(oAng.Get());
	}
	return pH->EndFunction();
}

int CScriptObjectGame::DumpEntities(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pSystem->GetILog()->Log("------------------------------------------------------------------------------------------");
	IEntityClassRegistry *pECR=m_pGame->GetClassRegistry();
	IEntityItPtr pEntities=m_pSystem->GetIEntitySystem()->GetEntityIterator();
	pEntities->MoveFirst();
	IEntity *pEnt=NULL;
	while((pEnt=pEntities->Next())!=NULL)
	{
		EntityClass *pClass=pECR->GetByClass(pEnt->GetEntityClassName());
		//if(m_pGame->GetWeaponSystem()->IsWeaponClass(pClass->cTypeID)) continue;
		if(m_pGame->GetWeaponSystemEx()->IsProjectileClass(pClass->ClassId)) continue;
		Vec3 pos = pEnt->GetPos();
		m_pSystem->GetILog()->Log("ENTITY class=%s/%d ent=%s/%d pos=(%.1f,%.1f,%.1f)", pClass->strClassName.c_str(), (int)pClass->ClassId, pEnt->GetName(), pEnt->GetId(), pos.x, pos.y, pos.z);
	}
	m_pSystem->GetILog()->Log("------------------------------------------------------------------------------------------");
	return pH->EndFunction();
}

// FIXME: tim, refactor these functions


struct SFoundSaveGame
{
	string szFileName, szLevel, szMission;
	int iHour, iMinute, iSecond, iYear, iMonth, iDay;
};

bool SortSaveGame(const SFoundSaveGame &a, const SFoundSaveGame &b)
{
	/*
	// Sort by save number.
	int len1 = a.szFileName.find('.');
	while (len1 > 0 && isdigit(a.szFileName[len1-1]))
		len1--;

	int len2 = b.szFileName.find('.');
	while (len2 > 0 && isdigit(b.szFileName[len2-1]))
		len2--;

	int num1 = atoi(a.szFileName.substr(len1).c_str());
	int num2 = atoi(b.szFileName.substr(len2).c_str());

	return num1 < num2;
	*/
	// Sort by time.
	if (a.iYear > b.iYear)
	{
		return 0;
	}
	else if (a.iYear < b.iYear)
	{
		return 1;
	}
	else if (a.iMonth > b.iMonth)
	{
		return 0;
	}
	else if (a.iMonth < b.iMonth)
	{
		return 1;
	}
	else if (a.iDay > b.iDay)
	{
		return 0;
	}
	else if (a.iDay < b.iDay)
	{
		return 1;
	}
	else if (a.iHour > b.iHour)
	{
		return 0;
	}
	else if (a.iHour < b.iHour)
	{
		return 1;
	}
	else if (a.iMinute > b.iMinute)
	{
		return 0;
	}
	else if (a.iMinute < b.iMinute)
	{
		return 1;
	}
	else if (a.iSecond > b.iSecond)
	{
		return 0;
	}
	else if (a.iSecond < b.iSecond)
	{
		return 1;
	}

	return 0;
}


int CScriptObjectGame::GetSaveGameList(IFunctionHandler *pH)
{
	intptr_t hEnumFile = -1L;
	__finddata64_t sFindData;
	_SmartScriptObject cList(m_pSystem->GetIScriptSystem(), false);
	std::vector<SFoundSaveGame> vSaveList;

	CHECK_PARAMETERS(1);

	char *szProfileName;

	pH->GetParam(1, szProfileName);

	string szSaveGameDir = "Profiles/player/";
	szSaveGameDir+= szProfileName;
	szSaveGameDir += "/Savegames/";

	//m_pSystem->GetILog()->Log("SEARCHING FOR SAVEGAMES (PROFILE: %s)!", szProfileName);

	string pattern = szSaveGameDir + "*.sav";

	if ((hEnumFile = _findfirst64(pattern.c_str(), &sFindData)) == -1L) 
	{
		_findclose(hEnumFile);
		hEnumFile = -1L;
		return pH->EndFunction(cList);
	}

	do
	{
	  if(sFindData.attrib&_A_SUBDIR) continue;

		string szSaveFilename = szSaveGameDir + sFindData.name;

		CDefaultStreamAllocator sa;
		CStream stm(300, &sa); 

		int bitslen = m_pSystem->GetCompressedFileSize((char *)szSaveFilename.c_str());

		if (bitslen)
		{
			stm.Resize(bitslen);

			int bitsread = m_pSystem->ReadCompressedFile((char *)szSaveFilename.c_str(), stm.GetPtr(), stm.GetAllocatedSize());

			if (bitsread)
			{
				stm.SetSize(bitsread);

				string szMagic;
				int iVersion;

				stm.Read(szMagic);
				stm.Read(iVersion);

				if ((szMagic == SAVEMAGIC))
				{
					m_pSystem->GetILog()->Log("\tfound savegame: %s", sFindData.name);

					string szLevelName;
					string szMissionName;

					stm.Read(szLevelName);
					stm.Read(szMissionName);

					unsigned char bHour, bMinute, bSecond, bDay, bMonth;
					unsigned short wYear;
					stm.Read(bHour);	// hour
					stm.Read(bMinute);	// minute
					stm.Read(bSecond);	// minute
					stm.Read(bDay);	// day
					stm.Read(bMonth);	// month
					stm.Read(wYear);	// year

					SFoundSaveGame Save;

					Save.iYear = (int)wYear;
					Save.iMonth = (int)bMonth;
					Save.iDay = (int)bDay;
					Save.iHour = (int)bHour;
					Save.iMinute = (int)bMinute;
					Save.iSecond = (int)bSecond;
					Save.szFileName = sFindData.name;
					Save.szLevel = szLevelName;
					Save.szMission = szMissionName;

					vSaveList.push_back(Save);
				}
				else
				{
					m_pSystem->GetILog()->Log("\tfound savegame: %s $4wrong version", sFindData.name);
				}
			}
			else
			{
				m_pSystem->GetILog()->Log("\tfound savegame: %s $4failed to read", sFindData.name);
			}
		}
		else
		{
			m_pSystem->GetILog()->Log("\tfound savegame: %s $4zero length", sFindData.name);
		}
	} while (_findnext64(hEnumFile, &sFindData) != -1);

	_findclose(hEnumFile);
	hEnumFile = 0;

	std::sort(vSaveList.begin(), vSaveList.end(), SortSaveGame);

	for (std::vector<SFoundSaveGame>::iterator it = vSaveList.begin(); it != vSaveList.end(); ++it)
	{
		_SmartScriptObject pCheckpoint(m_pSystem->GetIScriptSystem(), false);

		pCheckpoint->SetValue("Hour", it->iHour);
		pCheckpoint->SetValue("Minute", it->iMinute);
		pCheckpoint->SetValue("Second", it->iSecond);
		pCheckpoint->SetValue("Day", it->iDay);
		pCheckpoint->SetValue("Month", it->iMonth);
		pCheckpoint->SetValue("Year", it->iYear);

		pCheckpoint->SetValue("Filename", it->szFileName.c_str());
		pCheckpoint->SetValue("Level", it->szLevel.c_str());
		pCheckpoint->SetValue("Mission", it->szMission.c_str());

		cList->SetAt(cList->Count()+1, pCheckpoint);
	}

	//m_pSystem->GetILog()->Log("END OF SAVEGAMES (PROFILE: %s)!", szProfileName);

	return pH->EndFunction(cList);
}


int CScriptObjectGame::ToggleMenu(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pGame->SendMessage("Switch");
//	m_pGame->Update();
//	m_pGame->m_pSystem->GetIProcess()->SetPMessage("Switch");
//	m_pGame->Update();
//	m_pGame->m_pSystem->GetIProcess()->SetPMessage("Switch");
//	m_pGame->Update();
	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::ShowMenu(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pGame->GotoMenu();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::HideMenu(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pGame->GotoGame();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectGame::IsInMenu(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (m_pGame->IsInMenu())
		return pH->EndFunction(1);

	return pH->EndFunctionNull();
}


/*
//	calculates aproximate point of lending for granade on terrain,
//	Returns "hit probability"	[0,1]. 
//			1-granade lands at target location
//			0-granade lands more than explosion_radius from target
//	firePos		- start point
//	fireAngl	- fireing angles
//	target		- target location
//	name			- name of the grenade
//	example:
//	Game:TraceGrenade( firePos, fireAngl, ttarget:GetPos(), "ProjFlashbangGrenade" );
int CScriptObjectGame::TraceGrenade(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);

const char * sGrenadeName;
float timeStep = .03f;
float timeLimit = 30.0f;
Vec3 firePos;
Vec3 curPos;
Vec3 dir;
Vec3 target;
float	time = 0.0f;
float	vel;		// initial velocity
Vec3 gravity;
float	z;
float	radius;
CScriptObjectVector oVec(m_pScriptSystem,true);

	_SmartScriptObject obj(m_pScriptSystem);
	_SmartScriptObject explPar(m_pScriptSystem);
	_SmartScriptObject param(m_pScriptSystem);

	pH->GetParam(1,*oVec);
	firePos=oVec.Get();
	pH->GetParam(2,*oVec);
	dir=oVec.Get();
	pH->GetParam(3,*oVec);
	target=oVec.Get();
	pH->GetParam(4,sGrenadeName);

	dir=ConvertToRadAngles(dir);

	if(	!m_pScriptSystem->GetGlobalValue(sGrenadeName, obj))
	{
		return (pH->EndFunction( 0 ));
	}
	obj->GetValue("ExplosionParams", explPar);
	obj->GetValue("PhysParams", param);
	explPar->GetValue("radius", radius);
	param->GetValue("initial_velocity", vel);
	param->GetValue("gravity", oVec);
	gravity=oVec.Get();

	curPos = firePos;

	if( curPos.z<=m_pSystem->GetI3DEngine()->GetTerrainElevation(curPos.x, curPos.y) )
		curPos.z = m_pSystem->GetI3DEngine()->GetTerrainElevation(curPos.x, curPos.y) + .01f;

	while( curPos.z > (z=m_pSystem->GetI3DEngine()->GetTerrainElevation(curPos.x, curPos.y)) && time<timeLimit)
	{
		curPos.x = time*dir.x*vel + firePos.x;
		curPos.y = time*dir.y*vel + firePos.y;
		curPos.z = gravity.z*time*time/2.0f + dir.z*vel*time + firePos.z;

//m_pSystem->GetIRenderer()->DrawLabel(curPos, 1, "O");
		time += timeStep;
	}

	curPos.z = z;

//m_pSystem->GetIRenderer()->DrawLabel(curPos, 3, "O");

	float dist = (curPos-target).Length();

	if( dist == 0 )
		dist = 1;
	else if( dist >= radius )
		dist = 0;
	else
		dist = 1.0f - dist/radius;
	return (pH->EndFunction( dist ));
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//NEW STUFF
int CScriptObjectGame::SendMessage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszMsg;
	pH->GetParam(1, pszMsg);

	if(pszMsg)
		m_pGame->SendMessage(pszMsg);
	 else
		m_pScriptSystem->RaiseError("SendMessage() parameter is nil"); 

	return pH->EndFunction();
}




int CScriptObjectGame::GetEntityClassIDByClassName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sClassName;

	if(!pH->GetParam(1,sClassName))
		return pH->EndFunctionNull();

	IEntityClassRegistry *pECR=m_pGame->GetClassRegistry();
	EntityClass *pEC=pECR->GetByClass(sClassName);
	
	if(!pEC)
		return pH->EndFunctionNull();

	return pH->EndFunction((int)pEC->ClassId);
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!sets the camera field of view
	@param fFOV the field of view angle
*/
int CScriptObjectGame::SetCameraFov(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fFOV;
	pH->GetParam(1,fFOV);
	IEntity *pEntity;;
	CXClient *pClient=m_pGame->GetClient();
	pEntity=pClient->m_pISystem->GetLocalPlayer();
	if(pEntity)
	{
		if(pEntity->GetCamera())
		{

			IEntityCamera *pCamera=pEntity->GetCamera();
			if (pCamera && (fabs(fFOV-pCamera->GetFov())>0.001f))
			{

				pCamera->SetFov(fFOV,m_pRenderer->GetWidth(),m_pRenderer->GetHeight());
				pCamera->GetCamera().Init(m_pRenderer->GetWidth(),m_pRenderer->GetHeight(),fFOV);
				pCamera->GetCamera().Update();
			}
		}
	}
	return pH->EndFunction();
}


int CScriptObjectGame::GetCameraFov(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IEntity *pEntity;
	CXClient *pClient=m_pGame->GetClient();
	pEntity=pClient->m_pISystem->GetLocalPlayer();
	
	if(pEntity)
	{
		if(pEntity->GetCamera())
		{

			IEntityCamera *pCamera=pEntity->GetCamera();
			return pH->EndFunction(pCamera->GetFov());
		}
	}
	return pH->EndFunctionNull();
}

int CScriptObjectGame::ApplyStormToEnvironment(IFunctionHandler * pH)
{
//CHECK_PARAMETERS(3);

	CScriptObjectVector pVecOrigin(m_pScriptSystem,false);
	CScriptObjectVector pVecWindDirection(m_pScriptSystem,false);

	Vec3 vOrigin,vWind;
	float fRainAmount;

	pH->GetParam(1,pVecWindDirection);
	vWind=pVecWindDirection.Get();
	pH->GetParam(2,fRainAmount);

	// all these effects are client-side only
	IEntity *pEntity=NULL;
	CXClient *pClient=m_pGame->GetClient();

	if (pClient)
		pEntity=pClient->m_pISystem->GetLocalPlayer();

	if (pEntity)
	{
		if (pEntity->GetCamera())
		{
			IEntityCamera *pCamera=pEntity->GetCamera();
			Vec3 vPos=pEntity->GetCamera()->GetPos();
			if (m_p3DEngine->GetVisAreaFromPos(vPos)==NULL)
			{
				// enable rain only when the client is outdoor
				m_p3DEngine->SetRainAmount(fRainAmount);
				m_p3DEngine->SetWindForce(vWind);	
			}
		}
	}

	return pH->EndFunctionNull();
}


int CScriptObjectGame::CreateExplosion(IFunctionHandler *pH)
{
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pTempObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	Vec3 pos;
	float damage, rmin, rmax, radius, impulsive_pressure;
	float fDeafnessRadius=0.0f;
	float fDeafnessTime=0.0f;
	float fShakeFactor=1.0f;
	//float	falloff, curDamage;
	float	ImpactForceMul=45, ImpactForceMulFinal=62, ImpactForceMulFinalTorso=0;
	int nID;
	IEntity *shooter, *weapon=0;
	float rmin_occ = 0.1f;
	int nOccRes=0,nGrow=0;
	float fTerrainDefSize=0;
	INT_PTR nTerrainDecalId=0;
	int nCookie;
	int shooterSSID=0;
	pH->GetParam(1,*pObj);

	pObj->GetValue( "pos",*oVec );
	pos=oVec.Get();
	pObj->GetValue( "damage",damage );
	pObj->GetValue( "rmin",rmin );
	pObj->GetValue( "rmax",rmax );
	pObj->GetValue( "radius",radius );
	// give the explosion some control over the amount of shake which is being produced
	pObj->GetValue( "shake_factor", fShakeFactor);
	pObj->GetValue( "DeafnessRadius",fDeafnessRadius );
	pObj->GetValue( "DeafnessTime",fDeafnessTime );
	pObj->GetValue( "impulsive_pressure",impulsive_pressure );
	pObj->GetValue( "iImpactForceMul",ImpactForceMul);
	pObj->GetValue( "iImpactForceMulFinal",ImpactForceMulFinal);
	pObj->GetValue( "iImpactForceMulFinalTorso",ImpactForceMulFinalTorso);

	pObj->GetValue("rmin_occlusion", rmin_occ);
	pObj->GetValue("occlusion_res", nOccRes);
	pObj->GetValue("occlusion_inflate", nGrow);

	pObj->GetValue( "shooter",*pTempObj );		// it gets the EntityId from the Entity
	pTempObj->GetValue("id",nID);
	shooter=m_pEntitySystem->GetEntity(nID);		

	if(pObj->GetValue( "weapon",*pTempObj ))		// it gets the EntityId from the Entity
	{
		pTempObj->GetValue("id",nID);
		weapon=m_pEntitySystem->GetEntity(nID);
		if (!weapon)
			return pH->EndFunction();
	}

	if(!pObj->GetValue("shooterSSID", shooterSSID))
		shooterSSID=-1;																	// no known shooter client id (e.g. vehicle explosion)

	pObj->GetUDValue("decal_id",nTerrainDecalId,nCookie);
	if(nTerrainDecalId && nCookie==USER_DATA_TEXTURE)
		pObj->GetValue("terrain_deform_size",fTerrainDefSize);

	m_pGame->CreateExplosion(pos,damage,rmin,rmax,radius,impulsive_pressure,fShakeFactor,fDeafnessRadius,fDeafnessTime,ImpactForceMul,
		ImpactForceMulFinal,ImpactForceMulFinalTorso,rmin_occ,nOccRes,nGrow, shooter,shooterSSID,weapon, fTerrainDefSize,nTerrainDecalId);
	return pH->EndFunction();
}

int CScriptObjectGame::DrawLabel(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	const char *szParam=NULL;
	float size;
	
	pH->GetParam(1,*oVec);
	pH->GetParam(2,size);

	string sStr;
	wstring swStr;

	const char *sStringKey = 0;
	if (pH->GetParam(3,sStringKey))
	{
		m_pGame->m_StringTableMgr.Localize( sStringKey,swStr );
	}

	if (!sStr.empty())
		szParam=sStr.c_str();

	if (szParam)
	{
		float clr[4];
		clr[0] = clr[1] = clr[2] =clr[3] = 1.f;
		if (m_pRenderer)
			m_pRenderer->DrawLabelEx(oVec.Get(),size,clr,true,true,szParam);
	}

	return pH->EndFunction();
}

int CScriptObjectGame::GetInstantHit(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pShooter(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	IEntity *shooter;
	int nID;
	Vec3 pos, angles, dir;
	float fDistance;
	int res;

	pH->GetParam(1,*pObj);
	pObj->GetValue( "shooter",*pShooter );
	pShooter->GetValue("id",nID);
	shooter=m_pEntitySystem->GetEntity(nID);
	if(shooter==NULL)
	{
		TRACE("CScriptObjectSystem::GetInstantHit() shooter in nil");
		return pH->EndFunctionNull();
	}
	pObj->GetValue( "pos", *oVec );
	pos=oVec.Get();
	pObj->GetValue( "dir", *oVec );
	dir=oVec.Get();
	pObj->GetValue( "distance", fDistance );
	dir*=fDistance;
			
	IPhysicalEntity *skip=shooter->GetPhysics();
	ray_hit hit;
	res=m_pPhysicalWorld->RayWorldIntersection((const vectorf)pos,(const vectorf)dir, ent_all,	rwi_stop_at_pierceable,&hit,1, skip);
	
	if (res)
	{
		_SmartScriptObject pOut(m_pScriptSystem);
		CScriptObjectVector tVec(m_pScriptSystem);
		CScriptObjectVector tPos(m_pScriptSystem);
		CScriptObjectVector tRet(m_pScriptSystem);
		int nObjType;
		if (hit.pCollider)
		{
			IEntity *centycontact = (IEntity *)hit.pCollider->GetForeignData();
			if (centycontact)
			{
				nObjType=0;	// entity
				IScriptObject *pObj=centycontact->GetScriptObject();
				if(pObj)
					pOut->SetValue("target", pObj);
			}
			else
			{
				nObjType=1;	// stat obj
				pOut->SetToNull("target");
			}
		}else
		{
			nObjType=2; // terrain
			pOut->SetToNull("target");
		}
		pOut->SetValue("shooter", pShooter);
		pOut->SetValue("objtype", nObjType);
		tPos=(Vec3)hit.pt;
		pOut->SetValue("pos", *tPos);
		tVec=(Vec3)hit.n;
		pOut->SetValue("normal", *tVec);
		tRet=GetNormalized(dir);
		pOut->SetValue( "dir", *tRet );
		pOut->SetValue("target_material",m_pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(hit.surface_idx));
		
		return pH->EndFunction(*pOut);
	}else
		return pH->EndFunctionNull();
}

int CScriptObjectGame::GetMeleeHit(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pShooter(m_pScriptSystem,true);
	_SmartScriptObject pMeleeTarget(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	IEntity *shooter;
	IEntity *target = NULL;
	int nID;
	Vec3 pos, angles, dir;
	float fDistance;

	pH->GetParam(1,*pObj);
	pObj->GetValue( "shooter",*pShooter);
	pShooter->GetValue("id",nID);
	shooter=m_pEntitySystem->GetEntity(nID);
	if(shooter==NULL)
	{
		TRACE("CScriptObjectSystem::GetMeleeHit() shooter in nil");
		return pH->EndFunctionNull();
	}
	pObj->GetValue( "pos", *oVec );
	pos=oVec.Get();
	pObj->GetValue( "dir", *oVec );
	dir=oVec.Get();
	dir.Normalize();
	pObj->GetValue( "distance", fDistance );
	dir*=fDistance;
	
	if (pObj->GetValue("melee_target", *pMeleeTarget))
	{
		pMeleeTarget->GetValue("id",nID);
		target=m_pEntitySystem->GetEntity(nID);
	}

	IPhysicalEntity *skip=shooter->GetPhysics();

	Vec3 boxCenter = pos + 0.5f * dir;
	Vec3 offset(0.5f * fDistance, 0.5f * fDistance, 0.5f * fDistance);
	IPhysicalEntity **pList;

	// get all entities in the hit box
	int num = m_pPhysicalWorld->GetEntitiesInBox(boxCenter-offset, boxCenter+offset, pList, ent_all);

	// move position to ensure that we calc piercability correct
	float radius = 1.f;

	pos = pos - GetNormalized(dir) * radius;
	dir += radius * GetNormalized(dir);

	int res = 0;
	ray_hit closestHit;

	// check each entity in the box			
	for (int i = 0; i < num; ++i)
	{
		IEntity* pEntity = ((IEntity*)pList[i]->GetForeignData(OT_ENTITY));
		// skip the shooter
		if (pEntity)
		{
			if (pEntity == shooter)
				continue;
		}

		if (target == NULL || target == pEntity || pEntity==NULL )
		{
			ray_hit hit;
			// cast a 'fat' line segment
			if (m_pPhysicalWorld->CollideEntityWithBeam(pList[i], pos, dir, 0.3f, &hit))
			{
				if (hit.dist <= fDistance+radius)
				{
					if (res == 0 || hit.dist < closestHit.dist)
					{
						closestHit = hit;
					}
					res = 1;
				}
			}
		}
	}

	if (res)
	{
		_SmartScriptObject pOut(m_pScriptSystem);
		CScriptObjectVector tVec(m_pScriptSystem);
		CScriptObjectVector tPos(m_pScriptSystem);
		CScriptObjectVector tRet(m_pScriptSystem);
		int nObjType;
		if (closestHit.pCollider)
		{
			IEntity *centycontact = (IEntity *)closestHit.pCollider->GetForeignData(OT_ENTITY);
			if (centycontact)
			{
				nObjType=0;	// entity
				IScriptObject *pObj=centycontact->GetScriptObject();
				if(pObj)
					pOut->SetValue("target", pObj);

				if (target != NULL && target != centycontact)
					pOut->SetToNull("target");
			}
			else
			{
				nObjType=1;	// stat obj
				pOut->SetToNull("target");
			}
		}else
		{
			nObjType=2; // terrain
			pOut->SetToNull("target");
		}

		pOut->SetValue("shooter", pShooter);
		pOut->SetValue("objtype", nObjType);
		tPos=(Vec3)closestHit.pt;
		pOut->SetValue("pos", *tPos);
		tVec=(Vec3)closestHit.n;
		pOut->SetValue("normal", *tVec);
		tRet=GetNormalized(dir);
		pOut->SetValue( "dir", *tRet );
		pOut->SetValue("target_material",m_pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(closestHit.surface_idx));

		return pH->EndFunction(*pOut);
	}else
		return pH->EndFunctionNull();
}


//////////////////////////////////////////////////////////////////////////
bool CScriptObjectGame::_GetProfileFileNames( IFunctionHandler *pH, string &outSystem, string &outGame, const char *insCallerName )
{ 
	outSystem="system.cfg";
	outGame="game.cfg";

	if(pH->GetParamCount()>0)		// use given profile name or don't use profiles 
	{
		CHECK_PARAMETERS(1);

		const char *sProfileName;
		pH->GetParam(1,sProfileName);

		if(!sProfileName || sProfileName=="")
		{
			m_pScriptSystem->RaiseError("%s profilename is nil or empty",insCallerName);
			return false;
		}

		string sName;

		outSystem=string("Profiles/Player/")+sProfileName+"_"+outSystem;
		outGame=string("Profiles/Player/")+sProfileName+"_"+outGame;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::RemoveConfiguration(IFunctionHandler *pH)
{
	string sSystemCfg("system.cfg"), sGameCfg("game.cfg");

	if(m_pGame->m_bDedicatedServer)
		return pH->EndFunction();

	if (m_pGame->m_bEditor)
		return pH->EndFunction();

	char *szProfileName = 0;
	pH->GetParam(1, szProfileName);

	// profile is already specified in the string
	m_pGame->RemoveConfiguration(sSystemCfg,sGameCfg,szProfileName);

	return pH->EndFunction();
}


int CScriptObjectGame::LoadConfiguration(IFunctionHandler *pH)
{
	string sSystemCfg, sGameCfg;

	if(!_GetProfileFileNames(pH,sSystemCfg,sGameCfg,__FUNCTION__))
		return pH->EndFunction();
 
	//m_pScriptSystem->ExecuteFile(sSystemCfg.c_str(),true,true);
	//m_pScriptSystem->ExecuteFile(sGameCfg.c_str(),true,true);

	m_pGame->LoadConfiguration(sSystemCfg,sGameCfg);

	return pH->EndFunction();
}



int CScriptObjectGame::LoadConfigurationEx(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	if (pH->GetParamCount() == 2)
	{
		char *szSystemConfig = 0;
		char *szGameConfig = 0;

		pH->GetParam(1, szSystemConfig);
		pH->GetParam(2, szGameConfig);

		if (szSystemConfig && szGameConfig)
		{
			m_pGame->LoadConfiguration(szSystemConfig, szGameConfig);
		}
		else if (szSystemConfig)
		{
			m_pGame->LoadConfiguration(szSystemConfig, "");
		}
		else if (szGameConfig)
		{
			m_pGame->LoadConfiguration("", szGameConfig);
		}
	}

	return pH->EndFunction();
}

//
int CScriptObjectGame::SaveConfiguration(IFunctionHandler *pH)
{
	string sSystemCfg, sGameCfg;

	if(!_GetProfileFileNames(pH,sSystemCfg,sGameCfg,__FUNCTION__))
		return pH->EndFunction();

	if(m_pGame->m_bDedicatedServer)
		return pH->EndFunction();

	if (m_pGame->m_bEditor)
		return pH->EndFunction();

	char *szProfileName = 0;

	if (pH->GetParamCount() > 0)
	{
			pH->GetParam(1, szProfileName);
	}
	
	// profile is already specified in the string
	m_pGame->SaveConfiguration(sSystemCfg.c_str(),sGameCfg.c_str(),NULL);

	if (szProfileName)
	{
		string path = "profiles/player/";
		path += szProfileName;
#if defined(LINUX)
		mkdir( path.c_str(), 0xFFFF );
#else
		_mkdir( path.c_str() );
#endif
		path += "savegames/";
#if defined(LINUX)
		mkdir( path.c_str(), 0xFFFF );
#else
		_mkdir( path.c_str() );
#endif
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::DrawHealthBar(IFunctionHandler *pH)
{
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::LoadScript(IFunctionHandler *pH)
{
	if(pH->GetParamCount()>0)
	{
		const char *sPath;
		if(pH->GetParam(1,sPath))
		{
			bool bForceReload=false;

			if(pH->GetParamCount()>1) pH->GetParam(2,bForceReload);
			
			return pH->EndFunction(m_pGame->ExecuteScript(sPath,bForceReload));
		}
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::CreateRenderer(IFunctionHandler *pH)
{
	CScriptObjectRenderer *p=new CScriptObjectRenderer();
	m_vRenderersObjs.push_back(p);
	return pH->EndFunction(p->Create(m_pScriptSystem,m_pRenderer));
}

//! Generates a sound event on radar
//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::SoundEvent(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	float fThreat,fRadius; //fInterest,
	int nID;
	CScriptObjectVector oVec(m_pScriptSystem,true);	

	pH->GetParam(1,*oVec);
	pH->GetParam(2,fRadius);
	pH->GetParam(3,fThreat);
	pH->GetParam(4,nID);
	Vec3d pos = oVec.Get();

	CXClient *pCli=m_pGame->GetClient();
	if(pCli)
	{
		pCli->SoundEvent((EntityId)nID,pos,fRadius,fThreat);
	}
	
	return pH->EndFunction();
}

int CScriptObjectGame::CheckMap(IFunctionHandler *pH)
{
	char *szMapName = 0;
	char *szGameType = 0;

	if (pH->GetParamCount() > 0)
	{
		pH->GetParam(1, szMapName);
	}
	else
	{
		return pH->EndFunctionNull();
	}

	if (!szMapName)
	{
		return pH->EndFunctionNull();
	}

	if (strstr(szMapName, "/") || strstr(szMapName, "\\"))
	{
		return pH->EndFunctionNull();
	}

	string szMapPath = m_pGame->GetLevelsFolder() + "/" + szMapName;
	string szPak = szMapPath + "/*.pak";
 
	if (!m_pGame->OpenPacks(szPak.c_str()))
	{ 
		m_pGame->ClosePacks(szPak.c_str());
		return pH->EndFunctionNull();
	}

	if (pH->GetParamCount() == 1)
	{
		m_pGame->ClosePacks(szPak.c_str());

		return pH->EndFunction(1);
	}


	if (pH->GetParamCount() > 1)
	{
		pH->GetParam(2, szGameType);
	}

	string szXMLPath = szMapPath + "/LevelData.xml";

	XDOM::IXMLDOMDocumentPtr pLevelDataXML = m_pGame->GetSystem()->CreateXMLDocument();

	if(!pLevelDataXML->load(szXMLPath.c_str()))
	{
		m_pGame->ClosePacks(szPak.c_str());
		pLevelDataXML = 0;

		return pH->EndFunctionNull();
	}
	else if (!szGameType)
	{
		m_pGame->ClosePacks(szPak.c_str());
 
		return pH->EndFunction(1);
	}

	XDOM::IXMLDOMNodeListPtr pNodes;
#if !defined(LINUX64)
	if(pLevelDataXML != NULL) 
#else
	if(pLevelDataXML != 0) 
#endif
	{
		XDOM::IXMLDOMNodeListPtr	pMissionTagList;
		XDOM::IXMLDOMNodePtr			pMissionTag;

		pMissionTagList = pLevelDataXML->getElementsByTagName("Missions");

		if (pMissionTagList)
		{				
			pMissionTagList->reset();
			pMissionTag = pMissionTagList->nextNode();

			XDOM::IXMLDOMNodeListPtr pMissionList;

			pMissionList = pMissionTag->getElementsByTagName("Mission");

			if (pMissionList)
			{
				pMissionList->reset();  

				XDOM::IXMLDOMNodePtr pMission;

				while (pMission = pMissionList->nextNode())
				{						 
					XDOM::IXMLDOMNodePtr pName = pMission->getAttribute("Name");

#if defined(LINUX)
					RemoveCRLF(szGameType);
#endif
					if(stricmp(szGameType, pName->getText()) == 0)
					{
						m_pGame->ClosePacks(szPak.c_str());

						return pH->EndFunction(1);
					}
				} 

				pH->EndFunctionNull();
			} 
		}
	}

	m_pGame->ClosePacks(szPak.c_str());

	return pH->EndFunctionNull();
}

int CScriptObjectGame::GetMapDefaultMission(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	char *szMapName = 0;

	if (!pH->GetParam(1, szMapName))
	{
		return pH->EndFunctionNull();
	}

	if (!strlen(szMapName))
	{
		return pH->EndFunctionNull();
	}

	if (strstr(szMapName, "/") || strstr(szMapName, "\\"))
	{
		return pH->EndFunctionNull();
	}

	string szMapPath = m_pGame->GetLevelsFolder() + "/" + szMapName;
	string szPak = szMapPath + "/*.pak";

	if (!m_pGame->OpenPacks(szPak.c_str()))
	{ 
		m_pGame->ClosePacks(szPak.c_str());
		return pH->EndFunctionNull();
	}

	string szXMLPath = szMapPath + "/LevelData.xml";

	XDOM::IXMLDOMDocumentPtr pLevelDataXML = m_pGame->GetSystem()->CreateXMLDocument();

	if(!pLevelDataXML->load(szXMLPath.c_str()))
	{
		m_pGame->ClosePacks(szPak.c_str());
		pLevelDataXML = 0;

		return pH->EndFunctionNull();
	}

	XDOM::IXMLDOMNodeListPtr pNodes;
#if !defined(LINUX64)
	if(pLevelDataXML != NULL)
#else
	if(pLevelDataXML != 0)
#endif
	{
		XDOM::IXMLDOMNodeListPtr	pMissionTagList;
		XDOM::IXMLDOMNodePtr			pMissionTag;

		pMissionTagList = pLevelDataXML->getElementsByTagName("Missions");

		if (pMissionTagList)
		{				
			pMissionTagList->reset();
			pMissionTag = pMissionTagList->nextNode();

			XDOM::IXMLDOMNodeListPtr pMissionList;

			pMissionList = pMissionTag->getElementsByTagName("Mission");

			if (pMissionList)
			{
				pMissionList->reset();

				XDOM::IXMLDOMNodePtr pMission;

				while (pMission = pMissionList->nextNode())
				{						 
					XDOM::IXMLDOMNodePtr pName = pMission->getAttribute("Name");

					if (pName->getText())
					{
						m_pGame->ClosePacks(szPak.c_str());

						return pH->EndFunction(pName->getText());
					}
				}
			}
		}
	}

	m_pGame->ClosePacks(szPak.c_str());

	return pH->EndFunctionNull();
}

int CScriptObjectGame::CleanUpLevel(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pEntitySystem->EnableClient(false);
	m_pEntitySystem->EnableServer(false);
	m_pGame->MarkClientForDestruct();
	m_pGame->ShutdownServer();

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::SavePlayerPos(IFunctionHandler *pH)
{
	const char *sName = NULL;
	const char *sDesc = NULL;
	pH->GetParam(1,sName);
	pH->GetParam(2,sDesc);

	if (sName)
		m_pGame->DevMode_SavePlayerPos( 0,sName,sDesc );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::LoadPlayerPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sName;
	if (pH->GetParam(1,sName))
	{
		m_pGame->DevMode_LoadPlayerPos( 0,sName );
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::PlaySubtitle(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int nCookie=0;
	ISound *pSound=NULL;
	
	if(pH->GetParamUDVal(1,(USER_DATA &)pSound,nCookie) && pSound && (nCookie==USER_DATA_SOUND))	//AMD Port
	{
		m_pGame->PlaySubtitle(pSound);
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::GetModsList(IFunctionHandler * pH)
{	
	CGameMods *pMods=m_pGame->GetModsList();

	if(!pMods)
		return pH->EndFunctionNull();				// game was not Init

	_SmartScriptObject pObj(m_pScriptSystem);

	int n=1;
	_SmartScriptObject pModDesc(m_pScriptSystem);	
	pModDesc->SetValue("Title","- FarCry -");
	pModDesc->SetValue("Name","FarCry");
	pModDesc->SetValue("Version","1.3.3.0");
	pModDesc->SetValue("Author","Crytek");	
	pModDesc->SetValue("Website","$2www.crytek.com");	
	pModDesc->SetValue("Description","This is the normal FarCry game. \nLoad this in case you got another mod loaded,\nand you want to switch back to normal FarCry.");	
	pModDesc->SetValue("Folder","");	
	if ((!pMods->GetCurrentMod()) || (stricmp(pMods->GetCurrentMod(),"FarCry")==0))
	{
		pModDesc->SetValue("CurrentMod",true);
	}
	pObj->SetAt(n, pModDesc);

	if (pMods->m_mods.empty())
		return pH->EndFunction(pObj);

	n++;
	for (lstModsIt it=pMods->m_mods.begin();it!=pMods->m_mods.end();it++,n++)
	{	
		SGameModDescription *pMod=(*it);
		char szTemp[256];
		pMod->version.ToString(szTemp);
		_SmartScriptObject pModDesc1(m_pScriptSystem);
		pModDesc1->SetValue("Title",pMod->sTitle.c_str());
		pModDesc1->SetValue("Name",pMod->sName.c_str());
		pModDesc1->SetValue("Author",pMod->sAuthor.c_str());	
		pModDesc1->SetValue("Version",szTemp);
		pModDesc1->SetValue("Folder",pMod->sFolder.c_str());
		pModDesc1->SetValue("Website",pMod->sWebsite.c_str());
		pModDesc1->SetValue("Description",pMod->sDescription.c_str());
		if (stricmp(pMods->GetCurrentMod(),pMod->sName.c_str())==0)
		{
			pModDesc1->SetValue("CurrentMod",true);
		}
		pObj->SetAt(n, pModDesc1);
	}

	return pH->EndFunction(pObj);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::LoadMOD(IFunctionHandler * pH)
{
	const char *sName = NULL;
	bool bNeedsRestart;
	if (!pH->GetParam(1,sName))
		return (pH->EndFunctionNull());
	if (!pH->GetParam(2,bNeedsRestart))
		bNeedsRestart=false;

	if (sName)
	{
		if (m_pGame->GetModsInterface()->SetCurrentMod(sName,bNeedsRestart))
		{
			m_pGame->GetSystem()->GetILog()->Log("MOD %s loaded successfully",sName);
		}
		else
			m_pGame->GetSystem()->GetILog()->Log("MOD %s cannot be loaded",sName);
	}

	return (pH->EndFunction());
}


//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::GetCurrentModName(IFunctionHandler * pH)
{
	CGameMods *pMods=m_pGame->GetModsList();

	if(!pMods)
		return pH->EndFunctionNull();				// game was not Init

	assert(pMods->GetCurrentMod());

	return pH->EndFunction(pMods->GetCurrentMod());
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectGame::EnableQuicksave(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	bool bEnable;

	pH->GetParam(1,bEnable);
	m_pGame->AllowQuicksave(bEnable);

	return pH->EndFunction();
}
