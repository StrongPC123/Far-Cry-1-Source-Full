
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectClient.cpp
//
//  Description: 
//		ScriptObjectClient.cpp: implementation of the CScriptObjectClient class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectClient.h"
#include "XClient.h"
#include "XClient.h"

//////////////////////////////////////////////////////////////////////
_DECLARE_SCRIPTABLEEX(CScriptObjectClient)

//////////////////////////////////////////////////////////////////////
CScriptObjectClient::CScriptObjectClient()
{
	m_pClient=NULL;
}

//////////////////////////////////////////////////////////////////////
CScriptObjectClient::~CScriptObjectClient()
{
	if(m_pSoundEventPos)
		m_pSoundEventPos->Release();
}

//////////////////////////////////////////////////////////////////////
void CScriptObjectClient::Create(IScriptSystem *pScriptSystem,CXGame *pGame,CXClient *pClient)
{
	m_pGame=pGame;
	m_pClient=pClient;
	InitGlobal(pScriptSystem,"Client",this);
	m_pSoundEventPos=pScriptSystem->CreateObject();
	
}

//////////////////////////////////////////////////////////////////////
void CScriptObjectClient::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectClient>::InitializeTemplate(pSS);

	REG_FUNC(CScriptObjectClient,GetGameStartTime);
	REG_FUNC(CScriptObjectClient,GetGameState);
	REG_FUNC(CScriptObjectClient,GetGameStateString);
	REG_FUNC(CScriptObjectClient,CallVote);
	REG_FUNC(CScriptObjectClient,Vote);
	REG_FUNC(CScriptObjectClient,Kill);
	REG_FUNC(CScriptObjectClient,JoinTeamRequest);
	REG_FUNC(CScriptObjectClient,SendCommand);
	REG_FUNC(CScriptObjectClient,GetPing);
	REG_FUNC(CScriptObjectClient,Say);
	REG_FUNC(CScriptObjectClient,SayTeam);
	REG_FUNC(CScriptObjectClient,SayOne);
	REG_FUNC(CScriptObjectClient,GetSoundsEventsPos);
	REG_FUNC(CScriptObjectClient,SetName);
	REG_FUNC(CScriptObjectClient,SetBitsPerSecond);
	REG_FUNC(CScriptObjectClient,SetUpdateRate);
	REG_FUNC(CScriptObjectClient,GetServerCPUTargetName);
	REG_FUNC(CScriptObjectClient,GetServerOSTargetName);

	pSS->SetGlobalValue("CGS_INPROGRESS", 0);
	pSS->SetGlobalValue("CGS_COUNTDOWN", 1);
	pSS->SetGlobalValue("CGS_PREWAR", 2);
	pSS->SetGlobalValue("CGS_INTERMISSION", 3);
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetGameStartTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction((int)m_pClient->m_fGameLastTimeReceived - m_pClient->m_nGameLastTime);
}

/*!return the current gamestate (PREWAR,INTERMISSION etc...)
return the current game state
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetGameState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pClient->m_nGameState);
}

/*!return the name of the current gamestate (PREWAR,INTERMISSION etc...)
return the string containing the name of the current game state
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetGameStateString(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	if(m_pClient->m_nGameState==CGS_INPROGRESS)
	{
		int curtime = (int)(m_pGame->m_pSystem->GetITimer()->GetCurrTime()-m_pClient->m_fGameLastTimeReceived)+m_pClient->m_nGameLastTime;
		char buf[100];
		sprintf(buf, "%2d:%02d", curtime/60, curtime%60);
		return pH->EndFunction(buf);
	}
	else if(m_pClient->m_nGameState==CGS_PREWAR)
	{
		return pH->EndFunction("PREWAR");
	}
	else
	{
		return pH->EndFunction("");
	};
}

/*!call a vote during a multiplayer game
	@param command symbolic name of the command
	@param argument of the command
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::CallVote(IFunctionHandler *pH)
{
    const char *command = "", *arg1 = "";
    
    switch(pH->GetParamCount())
    {
        case 1:
            pH->GetParam(1, command);
            break;
        
        case 2:
            pH->GetParam(1, command);
            pH->GetParam(2, arg1);
            break;
        
        default:
            m_pGame->m_pSystem->GetILog()->Log("wrong number of parameters to callvote");
            return pH->EndFunction();
    }; 
    
    CStream stm;
    stm.Write(command);
    stm.Write(arg1);
    m_pClient->SendReliableMsg(XCLIENTMSG_CALLVOTE, stm);
    
    return pH->EndFunction();
}

/*!send a vote to the server during a multiplayer game
	@param vote string containing "yes" or "no"
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::Vote(IFunctionHandler *pH)
{
    CHECK_PARAMETERS(1);
    const char *vote = "";
    pH->GetParam(1, vote);
    CStream stm;
    if(stricmp(vote, "yes")==0)
    {
        stm.Write(1);
        m_pClient->SendReliableMsg(XCLIENTMSG_VOTE, stm);
    }
    else if(stricmp(vote, "no")==0)
    {
        stm.Write(0);
        m_pClient->SendReliableMsg(XCLIENTMSG_VOTE, stm);
    }
    else
    {
        m_pGame->m_pSystem->GetILog()->Log("vote yes or no");
    };
    return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::Kill(IFunctionHandler *pH)
{
    CHECK_PARAMETERS(0);
    CStream stm;
    m_pClient->SendReliableMsg(XCLIENTMSG_KILL, stm);
    return pH->EndFunction();
}

/*!send a request to the server to join a specified team
	if the team is "spectators" the caller client will leave the
	game and become a spectator.
	@param sTeamName team name
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::JoinTeamRequest(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1);
	if(pH->GetParamCount()>0)
	{
		const char *sTeamName;
		const char *sClass="";
		pH->GetParam(1,sTeamName);
		int nTeamId=m_pClient->m_pISystem->GetTeamId(sTeamName);
		if (nTeamId>=0)
		{
			if(pH->GetParamCount()>1)
			{
				pH->GetParam(2,sClass);
			}

			CStream stm;
			stm.Write((BYTE)nTeamId);
			stm.Write(sClass);
			m_pClient->SendReliableMsg(XCLIENTMSG_JOINTEAMREQUEST, stm);
		}
		else
		{
			m_pGame->m_pLog->Log("team \"%s\" doesn't exist!", sTeamName);
		};
	}
	else
	{
		IConsole *pConsole=m_pGame->GetSystem()->GetIConsole();
		pConsole->PrintLine("(JoinTeamRequest/team)the command require at least one parameter");
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SetBitsPerSecond(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int bits;
	if(pH->GetParam(1,bits))
		m_pClient->SetBitsPerSecond(bits);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SetUpdateRate(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int countpersec;
	if(pH->GetParam(1,countpersec))
		m_pClient->SetUpdateRate(countpersec);
	return pH->EndFunction();
}

/*!(LEGACY)send a string command to the server
	@param sString the string to express the command
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SendCommand(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sString;
	if(pH->GetParam(1,sString))
		m_pClient->SendCommand(sString);
	return pH->EndFunction();
}

/*!return the ping of the local client
	@return the current ping in milliseconds
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetPing(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pClient->GetPing());
}

/*!send a string to the server in order to broadcast it to alla othe client
	used to implement in-game chat
	@param sString the string to broadcast
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::Say(IFunctionHandler *pH)
{
	int iParamCount = pH->GetParamCount();

	if (iParamCount < 1)
	{
		return pH->EndFunction();
	}

	char *szParam = 0;
	string szText;

	for(int i = 1; i <= iParamCount; i++)
	{
		pH->GetParam(i, szParam);

		if (i != 1)
		{
			szText.push_back(' ');
		}

		if (szParam)
		{
			szText = szText + szParam;
		}
	}

	char szTrimmed[65] = {0};

	strncpy(szTrimmed, szText.c_str(), 64);
  
	TextMessage pTextMessage;

	pTextMessage.cMessageType = CMD_SAY;
	pTextMessage.uiTarget = 0;
//	pTextMessage.stmPayload.Write(szTrimmed);
	pTextMessage.m_sText=szTrimmed;

	m_pClient->SendTextMessage(pTextMessage);

	return pH->EndFunction();
}

/*!send a string to the server in order to broadcast it to all member of the local client team
	used to implement in-game chat
	@param sString the string to broadcast
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SayTeam(IFunctionHandler *pH)
{
	int iParamCount = pH->GetParamCount();

	if (iParamCount < 1)
	{
		return pH->EndFunction();
	}

	int iTeamID = m_pClient->m_pISystem->GetEntityTeam(m_pClient->GetPlayerId());

	if (iTeamID == -1 || iTeamID == SPECTATORS_TEAM)
	{
		m_pGame->m_pLog->Log("sayteam: you are not in a team or spectating!");

		return pH->EndFunctionNull();
	}

	// don't send any team messages from non-team games (FFA)
	char sTeamName[256];
	if (m_pClient->m_pISystem->GetTeamName(iTeamID, sTeamName) && strcmp(sTeamName, "players") == 0)
	{
		m_pGame->m_pLog->Log("sayteam: used in a non-team game!");
		return pH->EndFunctionNull();
	}

	char *szParam;
	string szText;

	for(int i = 1; i <= iParamCount; i++)
	{
		pH->GetParam(i, szParam);

		if (i != 1)
		{
			szText.push_back(' ');
		}

		if (szParam)
		{
			szText = szText + szParam;
		}
	}

	char szTrimmed[65] = {0};

	strncpy(szTrimmed, szText.c_str(), 64);

	TextMessage pTextMessage;

	pTextMessage.cMessageType = CMD_SAY_TEAM;
	pTextMessage.uiTarget = iTeamID;
//	pTextMessage.stmPayload.Write(szTrimmed);
	pTextMessage.m_sText=szTrimmed;

	m_pClient->SendTextMessage(pTextMessage);

	return pH->EndFunction();
}

/*!send a string to the server in order to deliver it to a cetain player
	used to implement in-game chat
	@param target entity id or name of the target
	@param sString the string to broadcast
*/
//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SayOne(IFunctionHandler *pH)
{
	int iParamCount = pH->GetParamCount();

	if (iParamCount < 2)
	{
		return pH->EndFunction();
	}

	int iEntityID = -1;
	char *szEntityName = 0;
	IEntity *pEntity = 0;

	IXSystem *pXSystem = m_pClient->m_pISystem;
	
	if (pH->GetParam(1, iEntityID))
	{
		pEntity = pXSystem->GetEntity(iEntityID);
	}
	else if (pH->GetParam(1, szEntityName))
	{
		pEntity = pXSystem->GetEntity(szEntityName);
	}

	if (!pEntity)
	{
		m_pClient->AddHudMessage("Player not found!", 3);

		m_pGame->m_pLog->Log("Player not found!");

		return pH->EndFunction();
	}

	char *szParam;
	string szText;

	for(int i = 2; i <= iParamCount; i++)
	{
		pH->GetParam(i, szParam);

		if (i != 2)
		{
			szText.push_back(' ');
		}

		if (szParam)
		{
			szText = szText + szParam;
		}
	}

	char szTrimmed[65] = {0};

	strncpy(szTrimmed, szText.c_str(), 64);

	TextMessage pTextMessage;

	pTextMessage.cMessageType = CMD_SAY_ONE;
	pTextMessage.uiTarget = pEntity->GetId();
//	pTextMessage.stmPayload.Write(szTrimmed);
	pTextMessage.m_sText=szTrimmed;

	m_pClient->SendTextMessage(pTextMessage);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetSoundsEventsPos(IFunctionHandler *pH)
{
	m_pSoundEventPos->SetValue("front",m_pClient->m_fFrontSound);
	m_pSoundEventPos->SetValue("back",m_pClient->m_fBackSound);
	m_pSoundEventPos->SetValue("left",m_pClient->m_fLeftSound);
	m_pSoundEventPos->SetValue("right",m_pClient->m_fRightSound);

	return pH->EndFunction(m_pSoundEventPos);
}



//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::SetName(IFunctionHandler *pH)
{
	if ((pH->GetParamCount() != 0) && ((pH->GetParamType(1) == svtString) || (pH->GetParamType(1) == svtNumber)))
	{
		const char *szName = 0;
		pH->GetParam(1, szName);

		if(!szName)
			return pH->EndFunction();

		char sTemp[65];

		CXServerSlot::ConvertToValidPlayerName(szName,sTemp,sizeof(sTemp));
		//CXServerSlot::ConvertToValidPlayerName(szName,sTemp);

		int len = strlen(sTemp);

		if(len)
		{	
			CStream stm;

			stm.Write(sTemp);

			m_pGame->m_pLog->Log("Send SetName '%s'",sTemp);
			m_pGame->p_name->Set(sTemp);

			m_pClient->SendReliableMsg(XCLIENTMSG_NAME, stm);

			return pH->EndFunction();
		}
	}

	char nameline[64] = {0};
	sprintf(nameline, "name = \"%s\"", m_pGame->p_name->GetString());

	m_pGame->GetSystem()->GetIConsole()->PrintLine(nameline);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::AIState(IFunctionHandler *pH)
{
	CStream stm;

	int nDummy=0;
	stm.Write(nDummy);

	m_pClient->SendReliableMsg(XCLIENTMSG_AISTATE, stm);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetServerCPUTargetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pClient->m_GameContext.GetCPUTarget());
}

//////////////////////////////////////////////////////////////////////
int CScriptObjectClient::GetServerOSTargetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pClient->m_GameContext.GetOSTarget());
}