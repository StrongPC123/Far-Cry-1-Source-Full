//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServer.cpp
//  Description: SXServerInfos && CXServer implementations.
//
//  History:
//  - August 3, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSystemServer.h"
#include <I3dengine.h>

#include <IEntitySystem.h>

#include "XPlayer.h"
#include "Spectator.h"
#include "XVehicleSystem.h"
#include "PlayerSystem.h"
#include "XVehicle.h"
#include "ScriptObjectPlayer.h"
#include "ScriptObjectVehicle.h"
#include "ScriptObjectSpectator.h"
#include "TagPoint.h"
#include <IRenderer.h>

#if defined(LINUX)
	#include "WinBase.h"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////
void CXServer::OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity )
{
	m_pISystem->OnSpawnContainer(ed,pEntity);
}

const char *CXServer::GetMsgName( XSERVERMSG inValue )
{
	switch(inValue)
	{
#define ADDNAME(name) case XSERVERMSG_##name:	return(#name);
		ADDNAME(UPDATEENTITY)
		ADDNAME(ADDENTITY)
		ADDNAME(REMOVEENTITY)
		ADDNAME(TIMESTAMP)
		ADDNAME(TEXT)
		ADDNAME(SETPLAYERSCORE)
		ADDNAME(SETENTITYSTATE)
//		ADDNAME(OBITUARY)
		ADDNAME(SETTEAMSCORE)
		ADDNAME(SETTEAMFLAGS)
		ADDNAME(SETPLAYER)
		ADDNAME(CLIENTSTRING)
		ADDNAME(CMD)
		ADDNAME(SETTEAM)
		ADDNAME(ADDTEAM)
		ADDNAME(REMOVETEAM)
		ADDNAME(SETENTITYNAME)
		ADDNAME(BINDENTITY)
		ADDNAME(SCOREBOARD)
		ADDNAME(SETGAMESTATE)
		ADDNAME(TEAMS)
		ADDNAME(SYNCVAR)
		ADDNAME(EVENTSCHEDULE)
		ADDNAME(UNIDENTIFIED)
#undef ADDNAME
		default: assert(0);
	}

	return 0;
}

///////////////////////////////////////////////
void CXServer::OnSpawn(IEntity *ent, CEntityDesc & ed )
{
	m_pISystem->OnSpawn(ent,ed);

	bool bSend = true;
	
	bSend=!m_bIsLoadingLevel;		// during loading we don't sync entities

	XSlotMap::iterator i = m_mapXSlots.begin();
	while(i != m_mapXSlots.end())
	{
		i->second->OnSpawnEntity(ed,ent,bSend);
		++i;
	}
}

///////////////////////////////////////////////
void CXServer::OnRemove(IEntity *ent)
{
	XSlotMap::iterator i = m_mapXSlots.begin();
	while(i != m_mapXSlots.end())
	{
		//TRACE("CXServer::OnRemove [%d]",ent->GetId());
		i->second->OnRemoveEntity(ent);
		++i;
	}
}


///////////////////////////////////////////////
unsigned int CXServer::GetMaxUpdateRate() const
{
	assert(sv_maxupdaterate);

	return sv_maxupdaterate->GetIVal();
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
CXServer::CXServer(CXGame *pGame, WORD nPort, const char *szName, bool listen)
{
	assert(pGame);

	m_pGame = pGame;
	m_pTimer = pGame->m_pSystem->GetITimer(); 
	IConsole *pConsole=m_pGame->GetSystem()->GetIConsole();			assert(pConsole);

	sv_name = pConsole->GetCVar("sv_name");
	sv_password = pConsole->GetCVar("sv_password");
	sv_maxplayers = pConsole->GetCVar("sv_maxplayers");
	sv_maxupdaterate = pConsole->GetCVar("sv_maxupdaterate");

	sv_maxrate = pConsole->GetCVar("sv_maxrate");
	sv_maxrate_lan = pConsole->GetCVar("sv_maxrate_lan");

	sv_netstats = pConsole->GetCVar("sv_netstats");
	sv_max_scheduling_delay = pConsole->GetCVar("sv_max_scheduling_delay");
	sv_min_scheduling_delay = pConsole->GetCVar("sv_min_scheduling_delay");
	m_bIsLoadingLevel=false;

  m_bListen = listen;
    
	m_mapXSlots.clear();
	
	float	timeout = m_pGame->sv_timeout->GetFVal()*1000;

	// create the entity system sink
	m_pGame->GetSystem()->GetIEntitySystem()->SetSink(this);

	// create the system interface
	m_pISystem = new CXSystemServer(this,m_pGame,m_pGame->m_pLog);

	// get this info before we set the server
	m_pGame->GetSystem()->SetForceNonDevMode(!m_pGame->IsDevModeEnable());

	// fill m_ServerInfo structure
	GetServerInfo();

	// create the server
	m_pIServer = m_pGame->CreateServer(this,nPort,m_bListen);
	if (!m_pIServer)
	{
			m_pGame->m_pLog->Log("!!---------Server creation failed---------!!");
			m_bOK = false;
			return;
	}
	else
		m_bOK = true;
  
	m_pIServer->SetSecuritySink(this);
	m_pIServer->SetVariable(cnvDataStreamTimeout,(unsigned int)timeout);
	
	m_ServerInfos.nPort = nPort;
	m_ServerInfos.VersionInfo = GetISystem()->GetFileVersion();

	// initialise the game context
	m_GameContext.dwNetworkVersion = NETWORK_FORMAT_VERSION;
	m_GameContext.strMission = "";

	m_ScriptObjectServer.Create(m_pGame->GetScriptSystem(),m_pISystem,m_pGame);
	m_ScriptObjectServer.SetServer(this);
	m_bInDestruction=false;

	LoadBanList();

	IScriptSystem *pScriptSystem = GetISystem()->GetIScriptSystem();
	assert(pScriptSystem);

	_SmartScriptObject pMapCycle(pScriptSystem);
	pScriptSystem->GetGlobalValue("MapCycle", pMapCycle);

	if (((IScriptObject *)pMapCycle) != 0)
	{
		HSCRIPTFUNCTION pfnInit = 0;
		if (pMapCycle->GetValue("Init", pfnInit))
		{			
			pScriptSystem->BeginCall(pfnInit);
			pScriptSystem->PushFuncParam((IScriptObject *)pMapCycle);
			pScriptSystem->EndCall();

			pScriptSystem->ReleaseFunc(pfnInit);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
unsigned CXServer::MemStats()
{
	unsigned size = sizeof *this;

	XSlotMap::iterator itr=m_mapXSlots.begin();

	for(; itr!=m_mapXSlots.end(); itr++)
		size += (itr->second)->MemStats() + sizeof (CXServerSlot*);	

	return size;
}



///////////////////////////////////////////////
CXServer::~CXServer()
{
// ClearSlots should remove all players later anyway
//	if (m_pGame->m_pUbiSoftClient)
//	{
//		m_pGame->m_pUbiSoftClient->Server_RemoveAllPlayers();
//	}

	m_bInDestruction=true;

	// update the network, to process any pending disconnect
	UpdateXServerNetwork();

//	m_pGame->m_pLog->Log("~CXServer 1");

	// shut down server game rules. (incorrect place but left for SP stability)
	m_ServerRules.ShutDown();

//	m_pGame->m_pLog->Log("~CXServer 2");

	// delele the entity system sink
	m_pGame->GetSystem()->GetIEntitySystem()->RemoveSink(this);

//	m_pGame->m_pLog->Log("~CXServer 3");

	// remove the slots which are still connected
	ClearSlots();

	// update the network, to process any pending disconnect
	UpdateXServerNetwork();

	// shut down server game rules. (correct place)
//		m_ServerRules.ShutDown();
	m_pGame->GetScriptSystem()->SetGlobalToNull("GameRules");		// workaround to minimize risk

	//	m_pGame->m_pLog->Log("~CXServer 4");

	// release the IServer interface
	SAFE_RELEASE(m_pIServer);

//	m_pGame->m_pLog->Log("~CXServer 5");

	// release the system interface
	SAFE_RELEASE(m_pISystem);

	if(m_pGame->GetScriptSystem())
	{
		m_pGame->GetScriptSystem()->SetGlobalToNull("Server");
		//Never force Lua GC, m_pScriptSystem->ForceGarbageCollection();
	}

	m_pGame = NULL;	
}


bool CXServer::IsInDestruction() const
{
	return m_bInDestruction;
}


void CXServer::DrawNetStats(IRenderer *pRenderer)
{
	if (!pRenderer)
		return;

	if(sv_netstats->GetIVal() & 0x1)	// display net statistics
	{
		int y=30;
		XSlotMap::iterator i = m_mapXSlots.begin();
		SlotNetStats ss;
		while(i != m_mapXSlots.end())
		{
			CXServerSlot *pSlot = i->second;
			pSlot->GetNetStats(ss);
			pRenderer->TextToScreen(10.0f,(float)y,"[%s] PING[%d] LOST[%d] ULOST[%d] MAXSNAP=%dbits LASTSNAP=%dbits",
				ss.name.c_str(),ss.ping,ss.packetslost,ss.upacketslost,ss.maxsnapshotbitsize,ss.lastsnapshotbitsize);
			y+=3;
			++i;
		}
		float fIncomingKbPerSec,fOutgoingKbPerSec;
		DWORD nIncomingPacketsPerSec,nOutgoingPacketsPerSec;
		m_pIServer->GetBandwidth(fIncomingKbPerSec,fOutgoingKbPerSec,nIncomingPacketsPerSec,nOutgoingPacketsPerSec);

		float fPacketSize=(20+8)*8.0f/1024.0f;		// 20bytes IP + 8bytes UDP in kBit

		y++;

		pRenderer->TextToScreen(10.0f,(float)y,"BANDWIDTH IN=%.2f/%.2f Kbits/sec (%d) OUT=%.2f/%.2f Kbits/sec (%d)",
			fIncomingKbPerSec, fIncomingKbPerSec+fPacketSize*nIncomingPacketsPerSec, nIncomingPacketsPerSec,
			fOutgoingKbPerSec, fOutgoingKbPerSec+fPacketSize*nOutgoingPacketsPerSec, nOutgoingPacketsPerSec);

	// just for internal testing purpose
#ifndef REDUCED_FOR_PUBLIC_RELEASE
		y+=3;
		pRenderer->TextToScreen(0,(float)y,"Subpackets produced (might not be sent):");

		static DWORD sResetCount=0;

		bool bNewDataArrived = m_NetStats.GetResetCount()!=sResetCount;

		sResetCount=m_NetStats.GetResetCount();

		for(int i=0;;i++)
		{
			DWORD dwRelCount,dwUnrelCount,dwItem;
			size_t nBitSize;

			if(!m_NetStats.GetStats(i,dwItem,dwRelCount,dwUnrelCount,nBitSize))
				break;

			pRenderer->TextToScreen(10,y+(i+1)*3.0f,"(Rel:%d Unrel:%d Bits:%d) %s",dwRelCount,dwUnrelCount,nBitSize,GetMsgName((XCLIENTMSG)dwItem));

			if(bNewDataArrived)
				m_pGame->m_pLog->Log("Server (Rel:%d Unrel:%d Bits:%d) %s",dwRelCount,dwUnrelCount,nBitSize,GetMsgName((XCLIENTMSG)dwItem));
		}

		if(bNewDataArrived)
			m_pGame->m_pLog->Log("-------<netstats end>");

#endif
	}

	const float fX = 10.0f;
	const float fY = 440.0f;
	const float fGraphWidth = 780.0f;
	const float fGraphHeight = 150.0f;

	if(sv_netstats->GetIVal() & 0x2)	// display a updatecount graph
	{
		pRenderer->Set2DMode(1, 800, 600);
		pRenderer->SetMaterialColor(1,1,1,1);

		float fScale = fGraphWidth / 1000.0f;

		XSlotMap::iterator sit = m_mapXSlots.begin();

		CXServerSlot *pSlot=0;
			
		if(sit!=m_mapXSlots.end())
			pSlot=sit->second;

		if(pSlot)
		{
			IEntity *pPlayerEntity = m_pISystem->GetEntity(pSlot->GetPlayerId());

			if(pPlayerEntity)
			{
				Vec3d vPos = pPlayerEntity->GetPos();

				int n = 0;
				for (NetEntityListItor eit = pSlot->m_Snapshot.m_lstNetEntities.begin(); eit != pSlot->m_Snapshot.m_lstNetEntities.end(); ++eit)
				{
					float fDist = sqrtf(eit->GetDistanceTo(vPos));

					if(eit->GetEntity()->GetClassId()==SYNCHED2DTABLE_CLASS_ID)			// no positional entity
						fDist = 500.0f;

					if(fDist >= 1000)
						continue;

					int i;

					if(m_pGame->GetSystem()->GetIEntitySystem()->IsDynamicEntityId(eit->GetEntity()->GetId()))
						i=(int)(0xffff-eit->GetEntity()->GetId());
					else
						i=1000-(int)(eit->GetEntity()->GetId());

					if(i>=1000)i=1000-1;		// clamp in max range
					if(i<0)i=0;							// clamp in min range

					float x = fX + fDist * fScale;
					float h = m_NetStats.GetSumGraphValue(i);
					float y = fY + fGraphHeight - h;

					pRenderer->Draw2dLine(x, y, x, y+h);
				}

				pRenderer->Draw2dLine(fX, fY+fGraphHeight, fX + fGraphWidth, fY+fGraphHeight);
			}
		}

		pRenderer->Set2DMode(0, 0, 0);
	}
}

///////////////////////////////////////////////
bool CXServer::CreateServerSlot(IServerSlot *pIServerSlot)
{
	// check if there are to many players
	if((int)(m_mapXSlots.size())>=sv_maxplayers->GetIVal())
	{
		// if this is not a dedicated server and
		// we have less than one slot, it means that our local client cannot connect
		// so let's open a slot for him
		if (!m_pGame->m_pSystem->IsDedicated() && sv_maxplayers->GetIVal() < 1)
		{
			sv_maxplayers->Set(1);
		}
		else
		{
			NET_TRACE("<<NET>>REJECTING CONNECTION SERVER FULL");
			pIServerSlot->Disconnect("@ServerFull");
			return false;
		}
	}

	CXServerSlot *pSlot = new CXServerSlot(this,pIServerSlot);
	NET_TRACE("<<NET>>REGISTERING SERVER SLOT");
	RegisterSlot(pSlot);
	return true;
	 
}

#define ADDSTRING(c, s)	{ (c) += s; c.push_back('\0'); }
#define ADDBOOL(c, b)	{ (c).push_back(b ? '\1' : '\0'); }
#define ADDCHAR(c, ch)	{ (c).push_back((char)ch); }
#if defined(WIN64)
	#define ADDINT(c, i)	{ char t=(char)(i & 0x000000FF);c+=t;t=(char)((i&0x0000FF00)>>8);c+=t;t=(char)((i&0x00FF0000)>>16);c+=t;t=(char)((i&0xFF000000)>>24);c+=t;}
#else
	#define ADDINT(c, i)	{ for(int j=0;j<4;j++) (c).push_back(((char *)&(i))[j]); }
#endif
#define ADDRULE(c, name, value)	(c) += name; (c).push_back('\0'); (c) += value; (c).push_back('\0');

//------------------------------------------------------------------------------------------------- 
bool CXServer::GetServerInfoStatus(string &szServerStatus)
{
	if (!GetServerInfo())
		return false;

	int nPort = m_ServerInfos.nPort;
	
	char szVersion[128];
	m_ServerInfos.VersionInfo.ToString(szVersion);
  
	ADDINT(szServerStatus, nPort);
	ADDCHAR(szServerStatus, m_ServerInfos.nComputerType);
	ADDSTRING(szServerStatus, szVersion);
	ADDSTRING(szServerStatus, m_ServerInfos.strName);
	ADDSTRING(szServerStatus, m_ServerInfos.strMod);
	ADDSTRING(szServerStatus, m_ServerInfos.strGameType);
	ADDSTRING(szServerStatus, m_ServerInfos.strMap);
	ADDCHAR(szServerStatus, m_ServerInfos.nPlayers);
	ADDCHAR(szServerStatus, m_ServerInfos.nMaxPlayers);
	ADDBOOL(szServerStatus, (m_ServerInfos.nServerFlags & SXServerInfos::FLAG_PASSWORD));
	ADDBOOL(szServerStatus, (m_ServerInfos.nServerFlags & SXServerInfos::FLAG_CHEATS));
	ADDBOOL(szServerStatus, (m_ServerInfos.nServerFlags & SXServerInfos::FLAG_NET));
	ADDBOOL(szServerStatus, (m_ServerInfos.nServerFlags & SXServerInfos::FLAG_PUNKBUSTER));

	return true;
}

//------------------------------------------------------------------------------------------------- 
bool CXServer::GetServerInfoStatus(string &szName, string &szGameType, string &szMap, string &szVersion, bool *pbPassword, int *piPlayers, int *piMaxPlayers)
{
	if (!GetServerInfo())
		return false;

	szName = m_ServerInfos.strName;
	szGameType = m_ServerInfos.strGameType;
	szMap = m_ServerInfos.strMap;
	char szLocalVersion[128];
	m_ServerInfos.VersionInfo.ToString(szLocalVersion);
	szVersion = szLocalVersion;
	*pbPassword = (m_ServerInfos.nServerFlags & SXServerInfos::FLAG_PASSWORD);
	*piPlayers = m_ServerInfos.nPlayers;
	*piMaxPlayers = m_ServerInfos.nMaxPlayers;

	return true;
}

//------------------------------------------------------------------------------------------------- 
bool CXServer::GetServerInfoRules(string &szServerRules)
{
	IScriptSystem *pSS = GetISystem()->GetIScriptSystem();

	_SmartScriptObject QueryHandler(pSS, 1);

	if (!pSS->GetGlobalValue("QueryHandler", (IScriptObject *)QueryHandler))
	{
		return false;
	}

	_SmartScriptObject ServerRules(pSS, 1);
	pSS->BeginCall("QueryHandler", "GetServerRules");
	pSS->PushFuncParam((IScriptObject *)QueryHandler);
	pSS->EndCall((IScriptObject *)ServerRules);

	int nPos = szServerRules.size();
	int nRules = 0;

	for (int i = 1; i <= ServerRules->Count(); i++)
	{
		_SmartScriptObject Rule(pSS, 1);

		if (ServerRules->GetAt(i, (IScriptObject *)Rule))
		{
			char *szRuleName = 0;
			char *szRuleValue = 0;

			Rule->GetAt(1, szRuleName);
			Rule->GetAt(2, szRuleValue);

			if (szRuleValue && szRuleName)
			{
				++nRules;
				ADDRULE(szServerRules, szRuleName, szRuleValue);
			}
		}
	}

	szServerRules.insert(nPos, 1, (char)nRules);

	return true;
}

//------------------------------------------------------------------------------------------------- 
bool CXServer::GetServerInfoPlayers(string *vszStrings[4], int &nStrings)
{
	IScriptSystem *pSS = GetISystem()->GetIScriptSystem();

	_SmartScriptObject QueryHandler(pSS, 1);

	if (!pSS->GetGlobalValue("QueryHandler", (IScriptObject *)QueryHandler))
	{
		return false;
	}

	_SmartScriptObject PlayerStats(pSS, 1);
	pSS->BeginCall("QueryHandler", "GetPlayerStats");
	pSS->PushFuncParam((IScriptObject *)QueryHandler);
	pSS->EndCall((IScriptObject *)PlayerStats);

	string	szPlayer;
	string	vszRString[6];
	string	*pszCurrent = &vszRString[0];
	int			iCurrentPos = pszCurrent->size();
	int			nPlayers = 0;
	
	nStrings = 1;

	for (int i = 1; i <= PlayerStats->Count(); i++)
	{
		_SmartScriptObject Player(pSS, 1);

		if (PlayerStats->GetAt(i, (IScriptObject *)Player))
		{
			char *szName = 0;
			char *szTeam = 0;
			char *szSkin = 0;
			int	 iScore = 0;
			int	 iPing = 0;
			int  iTime = 0;

			Player->GetValue("Name", (const char* &)szName);
			Player->GetValue("Team", (const char* &)szTeam);
			Player->GetValue("Skin", (const char* &)szSkin);
			Player->GetValue("Score", iScore);
			Player->GetValue("Ping", iPing);
			Player->GetValue("Time", iTime);

			szPlayer.resize(0);

			ADDSTRING(szPlayer, szName ? szName : "");
			ADDSTRING(szPlayer, szTeam ? szTeam : "");
			ADDSTRING(szPlayer, szSkin ? szSkin : "");
			ADDINT(szPlayer, iScore);
			ADDINT(szPlayer, iPing);
			ADDINT(szPlayer, iTime);

			if (pszCurrent->size() + szPlayer.size() > SERVER_QUERY_PACKET_SIZE)
			{
				pszCurrent->insert(iCurrentPos, 1, (char)nPlayers);
				++pszCurrent;
				++nStrings;

				if (nStrings <= SERVER_QUERY_MAX_PACKETS)
				{
					iCurrentPos = pszCurrent->size();
				}
				else
				{
					nStrings = SERVER_QUERY_MAX_PACKETS;
					break;
				}
				
				nPlayers = 0;
			}

			(*pszCurrent) += szPlayer;

			++nPlayers;
		}
	}

	pszCurrent->insert(iCurrentPos, 1, (char)nPlayers);

	for (int i = 0; i < nStrings; i++)
	{
		char n = i+1;

		ADDCHAR(*vszStrings[i], n);
		ADDCHAR(*vszStrings[i], nStrings);

		(*vszStrings[i]) += vszRString[i];
	}

	return true;
}
#undef ADDSTRING
#undef ADDINT
#undef ADDBOOL
#undef ADDRULE

///////////////////////////////////////////////
bool CXServer::GetServerInfo()
{
	const char *szGameType = m_ServerRules.GetGameType();

	if(!szGameType)
		return false;

	IGameMods *pGameMods=m_pGame->GetModsInterface();

	if(!pGameMods)
		return false;			// Game Init failed?

  m_ServerInfos.strName = sv_name->GetString();
  m_ServerInfos.strGameType = szGameType;
	m_ServerInfos.strMod = pGameMods->GetCurrentMod();
  m_ServerInfos.nPlayers = GetNumPlayers();
  m_ServerInfos.nMaxPlayers = sv_maxplayers->GetIVal(); 
	m_ServerInfos.nServerFlags = 0;

	if (sv_password->GetString() && (strlen(sv_password->GetString()) > 0))
	{
		m_ServerInfos.nServerFlags |= SXServerInfos::FLAG_PASSWORD;
	}

	if(m_pIServer->GetServerType()!=eMPST_LAN)
	{
		m_ServerInfos.nServerFlags |= SXServerInfos::FLAG_NET;
	}

	if (m_pGame->IsDevModeEnable())
	{
		m_ServerInfos.nServerFlags |= SXServerInfos::FLAG_CHEATS;
	}

	ICVar *pPunkBusterVar = GetISystem()->GetIConsole()->GetCVar("sv_punkbuster");
	if (pPunkBusterVar && pPunkBusterVar->GetIVal() != 0)
	{
		m_ServerInfos.nServerFlags |= SXServerInfos::FLAG_PUNKBUSTER;
	}
	
	m_ServerInfos.nComputerType = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CXServer::ProcessXMLInfoRequest( const char *sRequest,const char *sRespone,int nResponseMaxLength )
{
	XmlNodeRef reqNode = GetISystem()->LoadXmlFromString( sRequest );
	if (!reqNode)
		return false;

	if (strlen(sRespone) > 0)
		return true;
	return false;
}

///////////////////////////////////////////////
void CXServer::RegisterSlot(CXServerSlot *pSlot)
{
	NET_TRACE("<<NET>>CXServer::RegisterSlot %d",pSlot->GetID());
	m_mapXSlots.insert(XSlotMap::iterator::value_type(pSlot->GetID(),pSlot));
}

///////////////////////////////////////////////
void CXServer::UnregisterXSlot(DWORD nClientID)
{
	NET_TRACE("<<NET>>CXServer::UnregisterXSlot");
}

///////////////////////////////////////////////
void CXServer::ClearSlots()
{
	XSlotMap::iterator itor;

	 //Disconnect all slots
	itor = m_mapXSlots.begin();
	while(itor != m_mapXSlots.end())
	{
		CXServerSlot *pSlot=itor->second;

		if (m_pGame->IsMultiplayer() && !pSlot->IsLocalHost())
			pSlot->Disconnect("@ServerShutdown");

		++itor;
	}

	//Update The network to send the disconnection
	UpdateXServerNetwork();

	itor = m_mapXSlots.begin();
	while(itor != m_mapXSlots.end())
	{
		delete itor->second;
		++itor;
	}
	m_mapXSlots.clear();
}

///////////////////////////////////////////////
void CXServer::Update()
{
	// limit sv_maxplayers
	{
		if(sv_maxplayers->GetIVal()>MAXPLAYERS_LIMIT)
			sv_maxplayers->Set(MAXPLAYERS_LIMIT);

		int iPlayerCount=GetNumPlayers();

		if(sv_maxplayers->GetIVal()<iPlayerCount)
			sv_maxplayers->Set(iPlayerCount);
	}

	// kick "excess" players
	// if this is a dedicated server, minimum can be 0, if we are not, minimum must be, so that our local client is not kicked
	while(m_mapXSlots.size() > (int)(max(sv_maxplayers->GetIVal(), (m_pGame->m_pSystem->IsDedicated() ? 0 : 1))))
	{
		CXServerSlot *pSlot = (m_mapXSlots.rbegin()->second);

		if (!pSlot->IsLocalHost())
		{
			pSlot->Disconnect("@Kicked");
		}
	}


	// update server rules.
	m_ServerRules.Update();
	if(!m_pIServer) return;
	UpdateXServerNetwork();
	float time = m_pTimer->GetCurrTime();
	bool sendevents=m_pGame->UseFixedStep() && m_pGame->HasScheduledEvents(); 
	// Garbage collection and update of the slots
	XSlotMap::iterator i = m_mapXSlots.begin();
	while(i != m_mapXSlots.end())
	{
		CXServerSlot *pSlot = i->second;

		if(pSlot->IsXServerSlotGarbage())
		{
			delete pSlot;
			XSlotMap::iterator inext = i; inext++;
			m_mapXSlots.erase(i);
			i = inext;
//			NET_TRACE("<<NET>>CXServer::Update::Remove A Garbage"); // <<FIXME>> remove...
		}
		else
		{
			ASSERT(pSlot->m_Snapshot.GetSendPerSecond());

			float fRelTime = time - pSlot->m_Snapshot.GetLastUpdate();

			bool sendsnap = m_pGame->IsMultiplayer() && (fRelTime)>(1.f/pSlot->m_Snapshot.GetSendPerSecond());

			if(fRelTime<0)						// timer was reseted
			{
				fRelTime=0;
				sendsnap=true; 
			}

			pSlot->Update(sendsnap,sendevents&&sendsnap);
			++i;
		}		
	}

	//incrementing the random seed
//	m_pISystem->SetRandomSeed(m_pISystem->GetRandomSeed()+1);

	UpdateXServerNetwork(); // [Anton] if we formed a snapshot - then send it now

///////////////////////////////////////////////////////////////////////
//SNAPSHOT UPDATE
///////////////////////////////////////////////////////////////////////

	m_NetStats.Update(m_pTimer->GetCurrTimePrecise());		// keep statistics for one sec
	m_NetStats.AddToUpdateCount(1);
}

void CXServer::UpdateXServerNetwork()
{
	FUNCTION_PROFILER( GetISystem(), PROFILE_GAME );
	if(m_pIServer)
		m_pIServer->Update(GetCurrentTime());
};

///////////////////////////////////////////////
// return the current context
bool CXServer::GetContext(SXGameContext &ctxOut)
{
	ctxOut = m_GameContext;
	return false;
}

void CXServer::AddRespawnPoint(ITagPoint *pPoint)
{
	m_pGame->m_pLog->Log("CXServer::AddRespawnPoint '%s'",pPoint->GetName());

	m_vRespawnPoints.insert(RespawnPointMap::iterator::value_type(pPoint->GetName(),pPoint));
	m_CurRespawnPoint=m_vRespawnPoints.begin();
}

//////////////////////////////////////////////////////////////////////////
void CXServer::RemoveRespawnPoint(ITagPoint *pPoint)
{
	RespawnPointMap::iterator itor=m_vRespawnPoints.begin();

	while(itor!=m_vRespawnPoints.end())
	{
		if((itor->second)==pPoint)
		{
			m_vRespawnPoints.erase(itor);
			return;
		}
		++itor;
	}
}

///////////////////////////////////////////////
// get a specific respawn point
ITagPoint* CXServer::GetFirstRespawnPoint()
{
	if(m_vRespawnPoints.empty())
		return NULL;

	m_CurRespawnPoint=m_vRespawnPoints.begin();

	return m_CurRespawnPoint->second;
}


///////////////////////////////////////////////
// get a specific respawn point
ITagPoint* CXServer::GetNextRespawnPoint()
{
	if(m_vRespawnPoints.empty())
		return NULL;

	++m_CurRespawnPoint;

	if(m_CurRespawnPoint==m_vRespawnPoints.end())
		return 0;

	return m_CurRespawnPoint->second;
}

///////////////////////////////////////////////
// get a specific respawn point
ITagPoint* CXServer::GetPrevRespawnPoint()
{
	if(m_vRespawnPoints.empty())
		return NULL;

	RespawnPointMap::reverse_iterator	revCurRespawnPoint(m_CurRespawnPoint);

	if( revCurRespawnPoint!=m_vRespawnPoints.rend() )
	{
		if(++revCurRespawnPoint == m_vRespawnPoints.rend() )
		{
			m_CurRespawnPoint = m_vRespawnPoints.begin();
		}
		else
			m_CurRespawnPoint = revCurRespawnPoint.base();
	}
	else
	{
		revCurRespawnPoint = m_vRespawnPoints.rbegin();
		++revCurRespawnPoint;
		m_CurRespawnPoint = revCurRespawnPoint.base();
	}

//	if(m_CurRespawnPoint==m_vRespawnPoints.end())
//	{
//		m_CurRespawnPoint = m_vRespawnPoints.begin();
//		return NULL;
//	}

	return m_CurRespawnPoint->second;
}


///////////////////////////////////////////////
// get a specific respawn point
ITagPoint* CXServer::GetRespawnPoint(char *name)
{
	RespawnPointMap::iterator itr=m_vRespawnPoints.find(name);

	if(itr == m_vRespawnPoints.end())
		return NULL;

	return itr->second;
}

///////////////////////////////////////////////
// get a random respawn point
ITagPoint* CXServer::GetRandomRespawnPoint(const char *sFilter)
{
	if(m_vRespawnPoints.empty())
	{
		m_pGame->m_pLog->Log("CXServer::GetRandomRespawnPoint NO RESPAWN POINT");
		return NULL; // no respawn point
	}
	
	int RandomRespawn = 0;

	ITagPoint *point;
	int count;

	RespawnPointMap::iterator itr;
	if(sFilter==NULL)
	{
		count=m_vRespawnPoints.size();

		itr=m_vRespawnPoints.begin();
	}
	else
	{
		count=m_vRespawnPoints.count(sFilter);
		
		if(!count)
		{
			m_pGame->m_pLog->Log("CXServer::GetRandomRespawnPoint NO RESPAWN POINT[%s]",sFilter);
			return false; // no respawn point
		}
		itr=m_vRespawnPoints.find(sFilter);
	}

	RandomRespawn=rand() % count;

//	m_pGame->m_pLog->Log("CXServer::GetRandomRespawnPoint '%s' %d/%d",
//		sFilter?sFilter:"<no filter>",RandomRespawn,count);

	while( RandomRespawn-- ) 
		++itr;

//  point = m_vRespawnPoints[RandomRespawn];
	point = itr->second;
	ASSERT(point);
/*
	if(point)
	{
		Vec3 pos;
		point->GetPos(pos);

		m_pGame->m_pLog->Log("CXServer::GetRandomRespawnPoint        (%.2f %.2f %.2f)",
			pos.x,pos.y,pos.z );
	}
	else
		m_pGame->m_pLog->Log("CXServer::GetRandomRespawnPoint        (0)");
*/
	return point;
}

///////////////////////////////////////////////
void CXServer::BroadcastReliable(XSERVERMSG msg, CStream &stmIn,bool bSecondaryChannel)
{
	CStream stmToSend;

	stmToSend.WritePkd(msg);
	stmToSend.Write(stmIn);
	
	// now... broadcast !
	XSlotMap::iterator i;

	for(i=m_mapXSlots.begin(); i!=m_mapXSlots.end(); ++i)
	{
		CXServerSlot *pSlot = i->second;
		
		if(!pSlot->IsXServerSlotGarbage() && pSlot->IsReady() && pSlot->IsContextReady())
			pSlot->SendReliable(stmToSend,bSecondaryChannel);
	}	
}

void CXServer::BroadcastUnreliable(XSERVERMSG msg, CStream &stmIn,int nExclude)
{
	CStream stmToSend;

	stmToSend.WritePkd(msg);
	stmToSend.Write(stmIn);
	
	// now... broadcast !
	XSlotMap::iterator i;

	for(i=m_mapXSlots.begin(); i!=m_mapXSlots.end(); ++i)
	{
		CXServerSlot *pSlot = i->second;
		
		if(!pSlot->IsXServerSlotGarbage() && pSlot->IsReady() && !nExclude==pSlot->GetID() && pSlot->IsContextReady())
			pSlot->SendUnreliable(stmToSend);
	}	
}

void CXServer::BroadcastText(const char *sText, float fLifeTime)
{
	// now... broadcast !
	XSlotMap::iterator i;

	for(i=m_mapXSlots.begin(); i!=m_mapXSlots.end(); ++i)
	{
		CXServerSlot *pSlot = i->second;
		
		if(!pSlot->IsXServerSlotGarbage() && pSlot->IsReady() && pSlot->IsContextReady())
			pSlot->SendText(sText, fLifeTime);
	}	
}


void CXServer::OnClientMsgText(EntityId sender,CStream &stm)
{
	TextMessage tm;
	tm.Read(stm);
	m_ServerRules.OnClientMsgText(sender,tm);
}

void CXServer::BindEntity(EntityId idParent,EntityId idChild,unsigned char cParam)
{
	IEntity *pChild=m_pISystem->GetEntity(idChild);
	IEntity *pParent=m_pISystem->GetEntity(idParent);
	CStream stm;
	stm.Write(idParent);
	stm.Write(idChild);
	stm.Write(cParam);
	stm.Write(true);
	stm.Write(pParent->GetAngles(1));
	stm.Write(pChild->GetAngles(1));
	BroadcastReliable(XSERVERMSG_BINDENTITY,stm,false);
}

void CXServer::UnbindEntity(EntityId idParent,EntityId idChild,unsigned char cParam)
{
	IEntity *pChild=m_pISystem->GetEntity(idChild);
	IEntity *pParent=m_pISystem->GetEntity(idParent);
	CStream stm;
	stm.Write(idParent);
	stm.Write(idChild);
	stm.Write(cParam);
	stm.Write(false);
	stm.Write(pParent->GetAngles(1));
	stm.Write(pChild->GetAngles(1));
	BroadcastReliable(XSERVERMSG_BINDENTITY,stm,false);
}

void CXServer::OnMapChanged()
{
	m_ServerRules.MapChanged();
};

int CXServer::GetNumPlayers()
{
	//the num of players is the num of connected slots also if are spectators the count
	return m_mapXSlots.size();
}

void CXServer::AddToTeam(const char *sTeam,int eid)
{
	int nTID=m_pISystem->GetTeamId(sTeam);
	if(nTID!=-1)
	{
		m_pISystem->SetTeam(eid,nTID);
		CStream stm;
		WRITE_COOKIE(stm);
		stm.Write((EntityId)eid);
		stm.Write((BYTE)nTID);
		WRITE_COOKIE(stm);
		BroadcastReliable(XSERVERMSG_SETTEAM, stm,false);
	}
}

void CXServer::RemoveFromTeam(int eid)
{
	m_pISystem->SetTeam(eid,0xFF);
	CStream stm;
	BYTE nNoTeam=0xFF;
	WRITE_COOKIE(stm);
	stm.Write((EntityId)eid);
	stm.Write(nNoTeam);
	WRITE_COOKIE(stm);
	BroadcastReliable(XSERVERMSG_SETTEAM, stm,false);
}

void CXServer::AddTeam(const char *sTeam)
{
	int nTID=m_pISystem->GetTeamId(sTeam);
	if(nTID==-1)
	{
		nTID=m_pISystem->AddTeam(sTeam);
		CStream stm;
		WRITE_COOKIE(stm);
		stm.Write(sTeam);
		stm.Write((BYTE)nTID);
		WRITE_COOKIE(stm);
		BroadcastReliable(XSERVERMSG_ADDTEAM, stm,false);
	}
}

void CXServer::RemoveTeam(const char *sTeam)
{
	int nTID=m_pISystem->GetTeamId(sTeam);
	if(nTID!=-1)
	{
		m_pISystem->RemoveTeam(nTID);
		CStream stm;
		stm.Write((BYTE)nTID);
		BroadcastReliable(XSERVERMSG_REMOVETEAM, stm,false);
	}
}

void CXServer::SendRequestScriptHash( EntityId Entity, const char *szPath, const char *szKey )
{
	CStream stm;
	
	IBitStream *pBitStream = m_pGame->GetIBitStream();

	uint32 dwServerStartHash=0;								// todo change

	pBitStream->WriteBitStream(stm,Entity,eEntityId);									// e.g. INVALID_WID for globals, otherwise it's and entity
	pBitStream->WriteBitStream(stm,szPath,255,eASCIIText);						// e.g. "cnt.myTable"
	pBitStream->WriteBitStream(stm,szKey,255,eASCIIText);							// e.g. "luaFunc1" or "" 
	pBitStream->WriteBitStream(stm,dwServerStartHash,eDoNotCompress);	// start hash

	BroadcastReliable(XSERVERMSG_REQUESTSCRIPTHASH,stm,true);		// send in secondary channel to prevent stall

	m_pGame->m_pLog->Log("RequestScriptHash '%s' '%s'",szPath,szKey);
}

void CXServer::SetTeamScore(const char *sTeam,int score)
{
	int nTID=m_pISystem->GetTeamId(sTeam);
	if(nTID!=-1)
	{
		CStream stm;
		WRITE_COOKIE(stm);
		int curscore=m_pISystem->GetTeamScore(nTID);
		if(curscore==score)return;
		m_pISystem->SetTeamScore(nTID,score);
		stm.Write((BYTE)nTID);
		stm.Write((short)score);
		WRITE_COOKIE(stm);
		BroadcastReliable(XSERVERMSG_SETTEAMSCORE, stm,true);
	}
}

void CXServer::SetTeamFlags(const char *sTeam,int flags)
{
	int nTID=m_pISystem->GetTeamId(sTeam);
	if(nTID!=-1)
	{
		CStream stm;
		WRITE_COOKIE(stm);
		int curflags=m_pISystem->GetTeamFlags(nTID);
		if(curflags==flags)return;
		m_pISystem->SetTeamFlags(nTID,flags);
		stm.Write((BYTE)nTID);
		stm.WritePkd(flags);
		WRITE_COOKIE(stm);
		BroadcastReliable(XSERVERMSG_SETTEAMFLAGS, stm,true);
	}
}

void CXServer::BroadcastCommand(const char *sCmd)
{
	// now... broadcast !
	XSlotMap::iterator i;

	for(i=m_mapXSlots.begin(); i!=m_mapXSlots.end(); ++i)
	{
		CXServerSlot *pSlot = i->second;
		
		if(!pSlot->IsXServerSlotGarbage() && pSlot->IsReady() && pSlot->IsContextReady())
			pSlot->SendCommand(sCmd);
	}
}

void CXServer::BroadcastCommand(const char *sCmd, const Vec3 &invPos, const Vec3 &invNormal, 
															 const EntityId inId, const unsigned char incUserByte )
{
	// now... broadcast !
	XSlotMap::iterator i;

	for(i=m_mapXSlots.begin(); i!=m_mapXSlots.end(); ++i)
	{
		CXServerSlot *pSlot = i->second;
		
		if(!pSlot->IsXServerSlotGarbage() && pSlot->IsReady() && pSlot->IsContextReady())
			pSlot->SendCommand(sCmd, invPos, invNormal, inId, incUserByte);
	}	
}

//////////////////////////////////////////////////////////////////////////
void CXServer::SyncVariable(ICVar *p)
{
	CStream stm;
	stm.Write(p->GetName());
	stm.Write(p->GetString());

	XSlotMap::iterator i = m_mapXSlots.begin();
	while(i != m_mapXSlots.end())
	{
		CXServerSlot *pSlot = i->second;
		
		if(pSlot->IsXServerSlotGarbage() || (!pSlot->IsReady()) )
		{
			++i;
			continue;
		}			
		
		pSlot->SendReliableMsg(XSERVERMSG_SYNCVAR,stm,true,p->GetName());
		
		++i;
	}	
}

//////////////////////////////////////////////////////////////////////////
void CXServer::SyncAIState(void )
{
	CStream stm;

	// [marco] petar add your data into the stream - please
	// make it as small as possible 
	int nDummy=123;
	stm.Write(nDummy);	

	XSlotMap::iterator i = m_mapXSlots.begin();
	while(i != m_mapXSlots.end())
	{
		CXServerSlot *pSlot = i->second;

		if (pSlot->IsXServerSlotGarbage() || (!pSlot->IsReady()) )
		{
			++i;
			continue;
		}			

		// [marco] petar, now it broadcast AI state to all clients - 
		// if you need to send only to a certain client which is better
		// please restrict the range
		// I suppose you need it reliable - in this
		// case do not abuse bandwidth
		pSlot->SendReliableMsg(XSERVERMSG_AISTATE,stm,false);

		++i;
	}	// i
}

//////////////////////////////////////////////////////////////////////////
unsigned int CXServer::GetSchedulingDelay()
{
	assert(sv_min_scheduling_delay);
	assert(sv_max_scheduling_delay);

	if(!sv_min_scheduling_delay || !sv_max_scheduling_delay)
	{
		m_pGame->m_pLog->LogError("CXServer::GetSchedulingDelay error");
		return 100;
	}

	unsigned int nDelay=sv_min_scheduling_delay->GetIVal();
	unsigned int nMaxDelay=sv_max_scheduling_delay->GetIVal();

	XSlotMap::iterator i = m_mapXSlots.begin();

	for(;i!=m_mapXSlots.end();++i)
	{
		CXServerSlot *slot=i->second;			assert(slot);

		if(slot)
			nDelay = max(nDelay,min(nMaxDelay, slot->GetPing()*3>>2)); // half-ping multiplied by 1.5
	}

	return m_pGame->SnapTime(nDelay*0.001f,1.0f);
}

//////////////////////////////////////////////////////////////////////////
CXServerSlot* CXServer::GetServerSlotByIP( unsigned int clientIP ) const
{
	for(XSlotMap::const_iterator itor=m_mapXSlots.begin();itor!=m_mapXSlots.end();++itor)
	{
		CXServerSlot *slot=itor->second;
		if(slot->GetIServerSlot()->GetClientIP() == clientIP)
			return itor->second;
	}
	return 0;
}

//------------------------------------------------------------------------------------------------- 
bool CXServer::IsIDBanned(const BannedID &ID)
{
	for (BannedIDListItor it = m_vBannedIDList.begin(); it != m_vBannedIDList.end(); ++it)
	{
		if (ID == *it)
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------- 
void CXServer::BanID(const BannedID &ID)
{
	if (!IsIDBanned(ID))
	{
		m_vBannedIDList.push_back(ID);
	}

	SaveBanList(1, 0);
}

//------------------------------------------------------------------------------------------------- 
void CXServer::UnbanID(const BannedID &ID)
{
	for (BannedIDListItor it = m_vBannedIDList.begin(); it != m_vBannedIDList.end(); ++it)
	{
		if (ID == *it)
		{
			m_vBannedIDList.erase(it);
			SaveBanList(1, 0);

			return;
		}
	}
}

//------------------------------------------------------------------------------------------------- 
bool CXServer::IsIPBanned(const unsigned int dwIP)
{
	for (BannedIPListItor it = m_vBannedIPList.begin(); it != m_vBannedIPList.end(); ++it)
	{
		if (dwIP == *it)
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------- 
void CXServer::BanIP(const unsigned int dwIP)
{
	if (!IsIPBanned(dwIP))
	{
		m_vBannedIPList.push_back(dwIP);
	}

	SaveBanList(0, 1);
}

//------------------------------------------------------------------------------------------------- 
void CXServer::UnbanIP(const unsigned int dwIP)
{
	for (BannedIPListItor it = m_vBannedIPList.begin(); it != m_vBannedIPList.end(); ++it)
	{
		if (dwIP == *it)
		{
			m_vBannedIPList.erase(it);
			SaveBanList(0, 1);

			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXServer::CheaterFound( const unsigned int dwIP,int type,const char *sMsg )
{
	CXServerSlot *pSlot = GetServerSlotByIP(dwIP);
	if (pSlot)
	{
		string str = string("Client ") +  pSlot->GetName() +" detected to be a $3Cheater";
		BroadcastText( str.c_str() );

		CryLogAlways( "<CHEATER> #%d %s",pSlot->GetID(),(const char*)pSlot->GetName() );
		// Kick offender.
		if (!pSlot->IsLocalHost())
		{
			if (m_pGame->sv_cheater_kick->GetIVal())
				pSlot->Disconnect( sMsg );
		}
	}
	if (m_pGame->sv_cheater_ban->GetIVal() && m_pGame->sv_cheater_kick->GetIVal())
		BanIP(dwIP);
}

//////////////////////////////////////////////////////////////////////////
bool CXServer::GetSlotInfo(  const unsigned int dwIP,SSlotInfo &info , int nameOnly )
{
	memset ( &info , 0 , sizeof ( info ) ) ;
	CXServerSlot *pSlot = GetServerSlotByIP(dwIP);
	if (pSlot)
	{
		strncpy( info.playerName,pSlot->GetName(),sizeof(info.playerName) );
		info.playerName[sizeof(info.playerName)-1] = 0;

		if ( *info.playerName == 0 && pSlot->CanSpawn() ) strcpy ( info.playerName , "*NO NAME*" ) ;

		if ( nameOnly ) return true;

		IEntity *pPlayerEntity = m_pISystem->GetEntity(pSlot->GetPlayerId());
		if(pPlayerEntity && pPlayerEntity->GetContainer())
		{
			CPlayer *pPlayer = NULL;
			pPlayerEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void **)&pPlayer);
			if (pPlayer)
			{
				info.score = pPlayer->m_stats.score;
				info.deaths = pPlayer->m_stats.deaths;
			}
		}
		return true;
	}
	return false;
}

void CXServer::SaveBanList(bool bSaveID, bool bSaveIP)
{
	if (bSaveIP)
	{
		GetISystem()->GetILog()->Log("\001Saving banned IP list...");

		FILE *hFile = fopen("bannedip.txt", "w+");

		if (!hFile)
		{
			GetISystem()->GetILog()->Log("\001Failed to open bannedip.txt for writing!");

			return;
		}

		for (BannedIPList::iterator it = m_vBannedIPList.begin(); it != m_vBannedIPList.end(); ++it)
		{
			char szLine[256];

			CIPAddress ip;
			ip.m_Address.ADDR = *it;

			sprintf(szLine, "%s\n", ip.GetAsString());
			fputs(szLine, hFile);
#if defined(LINUX)
			RemoveCRLF(szLine);
#endif
		}

		fclose(hFile);
	}

	
	if (bSaveID)
	{
		GetISystem()->GetILog()->Log("\001Saving banned ID list...");

		FILE *hFile = fopen("bannedid.txt", "w+");

		if (!hFile)
		{
			GetISystem()->GetILog()->Log("\001Failed to open bannedid.txt for writing!");

			return;
		}

		for (BannedIDList::iterator it = m_vBannedIDList.begin(); it != m_vBannedIDList.end(); ++it)
		{
			char szLine[256] = {0};

			BannedID ban(*it);

			char szBanID[256] = {0};

			for (int i = 0; i < ban.bSize; i++)
			{
				sprintf(&szBanID[i*2], "%02x", ban.vBanID[i]);
			}
			
			sprintf(szLine, "%-36s %s\n", szBanID, it->szName.c_str());
			fputs(szLine, hFile);
#if defined(LINUX)
			RemoveCRLF(szLine);
#endif
		}

		fclose(hFile);
	}
}

void CXServer::LoadBanList(bool bLoadID, bool bLoadIP)
{
	if (bLoadIP)
	{
		// load banned ips
		m_vBannedIPList.clear();
		GetISystem()->GetILog()->Log("\001Loading banned IP list...");

		FILE *hFile = fopen("bannedip.txt", "r");

		if (!hFile)
		{
			GetISystem()->GetILog()->Log("\001ERROR: failed to open bannedip.txt for reading!");

			return;
		}

		char szLine[1024] = {0};

		while(fgets(szLine, 1024, hFile))
		{
			char szIP[1024] = {0};

			// remove trailing spaces
			if (sscanf(szLine, "%s", szIP) != 1)
			{
				continue;
			}

			CIPAddress ip(0, szIP);

			if (ip.GetAsUINT())
			{
				m_vBannedIPList.push_back(ip.GetAsUINT());
			}
		}

		fclose(hFile);
		hFile = 0;
	}


	if (bLoadID)
	{
		// load banned ids
		m_vBannedIDList.clear();
		GetISystem()->GetILog()->Log("\001Loading banned ID list...");

		FILE *hFile = fopen("bannedid.txt", "r");

		if (!hFile)
		{
			GetISystem()->GetILog()->Log("ERROR: failed to open bannedid.txt for reading!");

			return;
		}

		char szLine[1024] = {0};

		while(fgets(szLine, 1024, hFile))
		{
			char szName[1024] = {0};
			char szBanID[1024] = {0};
			char szByte[5] = {'0', 'x', 0, 0, 0};

			// remove trailing spaces
			if (sscanf(szLine, "%s %s", szBanID, szName) != 2)
			{
				if (sscanf(szLine, "%s", szBanID) != 1)
				{
					continue;
				}
				szName[0] = 0;
			}

			int isize = strlen(szBanID);

			BannedID ban;

			ban.bSize = isize >> 1;
			ban.szName = szName;

			// must be at least a dword
			if (ban.bSize < 8)
			{
				continue;
			}

			for (int i = 0; i < ban.bSize; i++)
			{
				szByte[2] = szBanID[i*2];
				szByte[3] = szBanID[i*2+1];

				int b = 0;
				sscanf(szByte, "%x", &b);

				ban.vBanID[i] = b;
			}

			m_vBannedIDList.push_back(ban);
		}

		fclose(hFile);
	}
}