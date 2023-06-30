//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServerRules.cpp
//  Description: Server rules class implementation.
//
//  History:
//  - August 9, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <Console.h>
#include "ScriptObjectVector.h"
//#include "GameDefs.h"
#include "XServerRules.h"
#include "WeaponClass.h"
#include "XNetwork.h"
//#include "TeamMgr.h"

//////////////////////////////////////////////////////////////////////////
CXServerRules::CXServerRules()
{
	m_init=false;	
	m_pScriptSystem=NULL;
	m_pGameRulesObj=NULL;
	m_pGame=NULL;
}

///////////////////////////////////////////////
CXServerRules::~CXServerRules()
{
	ShutDown();
}


bool CXServerRules::ChangeGameRules( const char *inszGameType )
{
	assert(inszGameType);
	char filename[512];

	sprintf(filename,"Scripts\\%s\\GameRules.lua",inszGameType);

	m_pScriptSystem->SetGlobalToNull("GameRules");
	//Never force Lua GC, m_pScriptSystem->ForceGarbageCollection();

	if(m_pScriptSystem->ExecuteFile(filename,true,true))
	{
		/*  deactivated because it's not needed right now
		m_pScriptSystem->BeginCall("OnAfterLoadGameRules");
		m_pScriptSystem->PushFuncParam(inszGameType);
		m_pScriptSystem->EndCall();
		*/
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CXServerRules::Init(CXGame *pGame, IConsole *pConsole,IScriptSystem *pScriptSystem, ILog *pLog)
{
	if(m_init)
		return true;
	m_pGame=pGame;
	m_pConsole=pConsole;

	m_pScriptSystem=pScriptSystem;

	if(!ChangeGameRules(m_pGame->g_GameType->GetString()))
	{
		if (pLog)
		{
			char sMessage[512];
			sprintf(sMessage, "GameRules for '%s' is not available. Loading default script.", m_pGame->g_GameType->GetString());
			pLog->Log(sMessage);
		}

		if(!ChangeGameRules("Default"))
		{
			//m_pConsole->Log( "Cannot load script %s",filename );
			return (false);
		}
	}
	
  //initialize rules
	m_pGameRulesObj=m_pScriptSystem->CreateEmptyObject();
	
	if(!m_pScriptSystem->GetGlobalValue("GameRules",m_pGameRulesObj))
		return false;

	m_pScriptSystem->BeginCall("GameRules","OnInit");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->EndCall();
	
	m_init=true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::Update()
{
	FUNCTION_PROFILER( GetISystem(), PROFILE_GAME );
	if(m_pGameRulesObj==NULL)
		return;
	m_pScriptSystem->BeginCall("GameRules","OnUpdate");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->EndCall();

}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::ShutDown()
{
	if(!m_init)
		return;
	m_pScriptSystem->BeginCall("GameRules","OnShutdown");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->EndCall();

	m_init=false;
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::CallVote(CScriptObjectServerSlot &sss, char *command, char *arg1)
{
	m_pScriptSystem->BeginCall("GameRules", "OnCallVote");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(sss.GetScriptObject());
	m_pScriptSystem->PushFuncParam(command);
	m_pScriptSystem->PushFuncParam(arg1);
	m_pScriptSystem->EndCall();
};

//////////////////////////////////////////////////////////////////////////
void CXServerRules::Vote(CScriptObjectServerSlot &sss, int vote)
{
	m_pScriptSystem->BeginCall("GameRules", "OnVote");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(sss.GetScriptObject());
	m_pScriptSystem->PushFuncParam(vote);
	m_pScriptSystem->EndCall();
};

//////////////////////////////////////////////////////////////////////////
void CXServerRules::Kill(CScriptObjectServerSlot &sss)
{
	m_pScriptSystem->BeginCall("GameRules", "OnKill");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(sss.GetScriptObject());
	m_pScriptSystem->EndCall();
};

//////////////////////////////////////////////////////////////////////////
void CXServerRules::MapChanged()
{
  m_pScriptSystem->BeginCall("GameRules", "OnMapChange");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->EndCall();
};

//////////////////////////////////////////////////////////////////////////
const char *CXServerRules::GetGameType()
{
	if(!m_init)
		return 0;

  char *md = m_pGame->g_GameType->GetString();
  
  HSCRIPTFUNCTION fun = m_pScriptSystem->GetFunctionPtr("GameRules", "ModeDesc");
  if(fun)
  {
      m_pScriptSystem->BeginCall(fun);
	    m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	    m_pScriptSystem->EndCall(md);
			m_pScriptSystem->ReleaseFunc(fun);
	};
	
	return md;
};

//////////////////////////////////////////////////////////////////////////
void CXServerRules::PrintEnterGameMessage(const char *playername,int color)
{
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnHitObject( const SWeaponHit &hit )
{
	_SmartScriptObject pObj(m_pScriptSystem);
	CScriptObjectVector oPos(m_pScriptSystem),oNormal(m_pScriptSystem),oDir(m_pScriptSystem);
	oPos=hit.pos;
	oNormal=hit.normal;
	oDir=hit.dir;

	pObj->SetValue("pos",*oPos);
	pObj->SetValue("normal",*oNormal);
	pObj->SetValue("dir",*oDir);
	pObj->SetValue("damage",hit.damage);
	pObj->SetValue("target",hit.target->GetScriptObject());
	pObj->SetValue("shooter",hit.target->GetScriptObject());
	pObj->SetValue("weapon",hit.weapon);
	pObj->SetValue("projectile",hit.projectile->GetScriptObject());

	m_pScriptSystem->BeginCall("GameRules","OnHitObject");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(*pObj);
	m_pScriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnHitPlayer( const SWeaponHit &hit )
{
	_SmartScriptObject pObj(m_pScriptSystem);
	CScriptObjectVector oPos(m_pScriptSystem),oNormal(m_pScriptSystem),oDir(m_pScriptSystem);
	oPos=hit.pos;
	oNormal=hit.normal;
	oDir=hit.dir;

	pObj->SetValue("pos",*oPos);
	pObj->SetValue("normal",*oNormal);
	pObj->SetValue("dir",*oDir);
	pObj->SetValue("damage",hit.damage);
	pObj->SetValue("target",hit.target->GetScriptObject());
	pObj->SetValue("shooter",hit.target->GetScriptObject());
	pObj->SetValue("weapon",hit.weapon);
	pObj->SetValue("projectile",hit.projectile->GetScriptObject());

	m_pScriptSystem->BeginCall("GameRules","OnHitPlayer");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(*pObj);
	m_pScriptSystem->EndCall();
}
  
//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnPlayerRespawn( IEntity *player )
{
	m_pScriptSystem->BeginCall("GameRules","OnPlayerRespawn");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(player->GetScriptObject());
	m_pScriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
int CXServerRules::OnClientConnect(IScriptObject *pSS,int nRequestedClassID)
{
	int nNewClassID;
	m_pScriptSystem->BeginCall("GameRules","OnClientConnect");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(pSS);
	m_pScriptSystem->PushFuncParam(nRequestedClassID);
	m_pScriptSystem->EndCall(nNewClassID);
  return nNewClassID;
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnClientDisconnect( IScriptObject *pSS )
{
	m_pScriptSystem->BeginCall("GameRules","OnClientDisconnect");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(pSS);
	m_pScriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnClientRequestRespawn( IScriptObject *pSS, const EntityClassId nRequestedClassID)
{
	m_pScriptSystem->BeginCall("GameRules","OnClientRequestRespawn");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(pSS);
	m_pScriptSystem->PushFuncParam(nRequestedClassID);
	m_pScriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::SetGameStuffScript(string sName)
{
	m_sGameStuffScript=sName;
}
	
//////////////////////////////////////////////////////////////////////////
string CXServerRules::GetGameStuffScript()
{
	return m_sGameStuffScript;
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnClientMsgText(EntityId sender, TextMessage &tm)
{
	const char *szMessageType=0;

	// Modified by Márcio
	//------------------------------------------------------------------------------------------------- 
	// TODO:
	// 
	// - add some kind of flood protection here
	// - add a cvar to enable logging of text messages
	// - add some kind of nauty language protection for sensible people :O
	//------------------------------------------------------------------------------------------------- 
	switch (tm.cMessageType)
	{
		case CMD_SAY:	
			szMessageType="say";
			if(m_pGame->GetXSystem()->GetEntityTeam(sender) == SPECTATORS_TEAM)
			{
				tm.uiTarget = SPECTATORS_TEAM;

				SendTeamTextMessage(sender, tm);
			}
			else SendWorldTextMessage(sender, tm);			
			break;

		case CMD_SAY_TEAM:
			szMessageType="sayteam";
			SendTeamTextMessage(sender, tm);
			break;

		case CMD_SAY_ONE:
			szMessageType="sayone";

			if (m_pGame->GetXSystem()->GetEntityTeam(sender) == SPECTATORS_TEAM)
				if (m_pGame->GetXSystem()->GetEntityTeam(tm.uiTarget) != SPECTATORS_TEAM)
					return;

			SendEntityTextMessage(sender, tm);
			break;
	}

	assert(szMessageType);

	if(szMessageType)
	{
		string sTarget="all";
		IEntity *pSender = m_pGame->GetXSystem()->GetEntity(tm.uiSender);

		if(tm.cMessageType==CMD_SAY_ONE)		// sayone
		{
			IEntity *pTarget = m_pGame->GetXSystem()->GetEntity(tm.uiTarget);

			if(pTarget)
				sTarget=pTarget->GetName();
		}
		else
		if(tm.cMessageType==CMD_SAY_TEAM)		// sayteam
		{
			char szTeamName[256];

			if(m_pGame->GetXSystem()->GetTeamName(tm.uiTarget,szTeamName))
				sTarget=szTeamName;
		}

		if(pSender)
		{
			// callback to script (e.g. to save chats to file)
			_SmartScriptObject pMultiplayerUtils(m_pScriptSystem, true);

			m_pScriptSystem->GetGlobalValue("MultiplayerUtils", *pMultiplayerUtils);

			m_pScriptSystem->BeginCall("MultiplayerUtils", "OnChatMessage");
			m_pScriptSystem->PushFuncParam(pMultiplayerUtils);
			m_pScriptSystem->PushFuncParam(tm.m_sText.c_str());
			m_pScriptSystem->PushFuncParam(pSender->GetName());
			m_pScriptSystem->PushFuncParam(sTarget.c_str());
			m_pScriptSystem->PushFuncParam(szMessageType);
			m_pScriptSystem->EndCall();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::SendWorldTextMessage(EntityId sender, TextMessage &tm)
{
	if (!m_pGame)
		return;
	CXServer::XSlotMap &mapXSlots=m_pGame->GetServer()->GetSlotsMap();
	for (CXServer::XSlotMap::iterator It=mapXSlots.begin();It!=mapXSlots.end();++It)
	{
		CXServerSlot *pSlot=It->second;
		if (!pSlot->IsReady())
			continue;
		pSlot->SendTextMessage(tm,false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::SendEntityTextMessage(EntityId sender, TextMessage &tm)
{
	if (!m_pGame)
		return;

	CXServer::XSlotMap &mapXSlots=m_pGame->GetServer()->GetSlotsMap();

	bool bSent = false;
	CXServerSlot *pSenderSlot = 0;

	// check for the entity slot
	// also check for the sender slot
	for (CXServer::XSlotMap::iterator It=mapXSlots.begin();It!=mapXSlots.end();++It)
	{
		CXServerSlot *pSlot=It->second;

		if (!pSlot->IsReady())
			continue;

		if (pSlot->GetPlayerId() == tm.uiTarget)
		{
			pSlot->SendTextMessage(tm, false);
			bSent = true;
		}
		if (pSlot->GetPlayerId() == tm.uiSender)
		{
			pSenderSlot = pSlot;
		}

		if (bSent && ((pSenderSlot && tm.uiSender) || !tm.uiSender))
		{
			break;
		}
	}

	// send a copy of the message to the client
	if (pSenderSlot)
	{
		pSenderSlot->SendTextMessage(tm, false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::SendTeamTextMessage(EntityId sender, TextMessage &tm)
{
	if (!m_pGame)
		return;
		
	CXServer::XSlotMap &mapXSlots=m_pGame->GetServer()->GetSlotsMap();
	IXSystem *pSys=m_pGame->GetServer()->m_pISystem;

	for (CXServer::XSlotMap::iterator It=mapXSlots.begin();It!=mapXSlots.end();++It)
	{
		CXServerSlot *pSlot=It->second;
		if (!pSlot->IsReady())
			continue;
		//CTeam *pTeam=pTeamMgr->GetEntityTeam(pSlot->m_wPlayerID);
		if(pSys->GetEntityTeam(pSlot->GetPlayerId())!=tm.uiTarget)
			continue;
		
		pSlot->SendTextMessage(tm,false);
	}
}

//////////////////////////////////////////////////////////////////////////
int CXServerRules::OnClientMsgJoinTeamRequest(CXServerSlot *pSS,BYTE nTeamId,const char *sClass)
{
	EntityId sender =pSS->GetPlayerId();
	char sTeamName[256];
	if(m_pGame->GetServer()->m_pISystem->GetEntityTeam(sender)==nTeamId)
		return TEAM_HAS_NOT_CHANGED;
	
	if(!m_pGame->GetServer()->m_pISystem->GetTeamName(nTeamId,sTeamName))
	{
		NET_TRACE("<<NET>>THE TEAM DOESNT EXISTS\n");
		return TEAM_HAS_NOT_CHANGED;
	}
	//pTeam->AddEntity(sender);
	m_pScriptSystem->BeginCall("GameRules","OnClientJoinTeamRequest");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(pSS->GetScriptObject());
	m_pScriptSystem->PushFuncParam(sTeamName);
	m_pScriptSystem->PushFuncParam(sClass);
	m_pScriptSystem->EndCall();
	return nTeamId;
}

//////////////////////////////////////////////////////////////////////////
int CXServerRules::OnClientCmd(CXServerSlot *pSS,const char *sCmd)
{
	m_pScriptSystem->BeginCall("GameRules","OnClientCmd");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(pSS->GetScriptObject());
	m_pScriptSystem->PushFuncParam(sCmd);
	m_pScriptSystem->EndCall();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CXServerRules::OnSpectatorSwitchModeRequest(IEntity *spec)
{
	m_pScriptSystem->BeginCall("GameRules","OnSpectatorSwitchModeRequest");
	m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
	m_pScriptSystem->PushFuncParam(spec->GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CXServerRules::OnAfterLoad()
{
	HSCRIPTFUNCTION hFunc = NULL;

	if (m_pGameRulesObj->GetValue("OnAfterLoad", hFunc))
	{
		m_pScriptSystem->BeginCall(hFunc);
		m_pScriptSystem->PushFuncParam(m_pGameRulesObj);
		m_pScriptSystem->EndCall();
	}
}