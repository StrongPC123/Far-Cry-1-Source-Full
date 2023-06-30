// ScriptObjectNewUbisoftClient.cpp: implementation of the CScriptObjectNewUbisoftClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK

#include "ScriptObjectNewUbisoftClient.h"
#include "NewUbisoftClient.h"
#include <ITimer.h>														// ITimer
#include "IConsole.h"													// IConsole

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectNewUbisoftClient)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptObjectNewUbisoftClient::CScriptObjectNewUbisoftClient()
{
	m_pUbiSoftClient=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptObjectNewUbisoftClient::~CScriptObjectNewUbisoftClient()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CScriptObjectNewUbisoftClient::ReleaseTemplate()
{
/*	SAFE_RELEASE(m_pLobbyInfo);
	SAFE_RELEASE(m_pRoomInfo);
	SAFE_RELEASE(m_pMemberInfo);
	SAFE_RELEASE(m_pFriendInfo);
	SAFE_RELEASE(m_pConnectInfo);
*/
	_ScriptableEx<CScriptObjectNewUbisoftClient>::ReleaseTemplate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CScriptObjectNewUbisoftClient::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectNewUbisoftClient>::InitializeTemplate(pSS);

	//REG_FUNC(CScriptObjectNewUbisoftClient,Client_GetStoredUsername);
	//REG_FUNC(CScriptObjectNewUbisoftClient,Client_GetStoredPassword);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_AutoLogin);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_Login);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_RequestGameServers);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_JoinGameServer);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_ConnectedToGameServer);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_LeaveGameServer);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_Disconnect);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_IsConnected);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_SetCDKey);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_CreateAccount);
	REG_FUNC(CScriptObjectNewUbisoftClient,Client_RequestMOTD);
#define REGISTER_CONSTANT(c)	pSS->SetGlobalValue("UBI_"#c, c)

//	REGISTER_CONSTANT(DISCONNECTED);

#undef REGISTER_CONSTANT
}


void CScriptObjectNewUbisoftClient::Init( IScriptSystem *pScriptSystem, ISystem *pSystem, NewUbisoftClient *inUbiSoftClient )
{
	m_pSystem=pSystem;
	m_pConsole=pSystem->GetIConsole();
	m_pScriptSystem = pScriptSystem;
	m_pUbiSoftClient=inUbiSoftClient;
	InitGlobal(pScriptSystem,"NewUbisoftClient",this);

	inUbiSoftClient->SetScriptObject(this);
	inUbiSoftClient->Init(m_pSystem);
}


DWORD	CScriptObjectNewUbisoftClient::GetAbsTimeInSeconds()
{
	DWORD dwRet=(DWORD)(m_pSystem->GetITimer()->GetCurrTime());

	return dwRet;
}
//REG_FUNC(CScriptObjectNewUbisoftClient,Client_GetStoredUsername);
//REG_FUNC(CScriptObjectNewUbisoftClient,Client_GetStoredPassword);
//REG_FUNC(CScriptObjectNewUbisoftClient,Client_AutoLogin);

/*
int CScriptObjectNewUbisoftClient::Client_GetStoredUsername(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	string szHexUsername;

	m_pUbiSoftClient->ReadStringFromRegistry("Ubi.com", "username", szHexUsername);

	char szEncUsername[256] = {0};
	char szUsername[256] = {0};

	m_pUbiSoftClient->DecodeHex((unsigned char *)szEncUsername, (unsigned char *)szHexUsername.c_str());
	m_pUbiSoftClient->DecryptString((unsigned char *)szUsername, (unsigned char *)szEncUsername);

	return pH->EndFunction(szUsername);
}


int CScriptObjectNewUbisoftClient::Client_GetStoredPassword(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	string szHexPassword;

	m_pUbiSoftClient->ReadStringFromRegistry("Ubi.com", "password", szHexPassword);

	char szEncPassword[256] = {0};
	char szPassword[256] = {0};

	m_pUbiSoftClient->DecodeHex((unsigned char *)szEncPassword, (unsigned char *)szHexPassword.c_str());
	m_pUbiSoftClient->DecryptString((unsigned char *)szPassword, (unsigned char *)szEncPassword);

	return pH->EndFunction(szPassword);
}
*/

int CScriptObjectNewUbisoftClient::Client_AutoLogin(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pUbiSoftClient->Client_AutoLogin());
}

/*! Login the Game Client to the Game Service
*/
int CScriptObjectNewUbisoftClient::Client_Login(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);

	const char *szUsername;
	const char *szPassword;
	bool				bSavePassword = false;

	pH->GetParam(1,szUsername);
	pH->GetParam(2,szPassword);
	pH->GetParam(3,bSavePassword);

	return pH->EndFunction(m_pUbiSoftClient->Client_Login(szUsername,szPassword, bSavePassword));
}

/*! Request the list of game servers
*/
int CScriptObjectNewUbisoftClient::Client_RequestGameServers(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pUbiSoftClient->Client_RequestGameServers());
}

/*! Tell the Game Service that you are going to join a game server
*/
int CScriptObjectNewUbisoftClient::Client_JoinGameServer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int iLobbyID,iRoomID;

	pH->GetParam(1,iLobbyID);
	pH->GetParam(2,iRoomID);

	return pH->EndFunction(m_pUbiSoftClient->Client_JoinGameServer(iLobbyID,iRoomID));
}

/*! Tell the Game Service that you have connected to a game server
*/
int CScriptObjectNewUbisoftClient::Client_ConnectedToGameServer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pUbiSoftClient->Client_ConnectedToGameServer());
}

/*! Tell the Game Service that you have left the game server. No parameters
*/
int CScriptObjectNewUbisoftClient::Client_LeaveGameServer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pUbiSoftClient->Client_LeaveGameServer());
}

/*! Disconnect from Game Service. No parameters
*/
int CScriptObjectNewUbisoftClient::Client_Disconnect(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pUbiSoftClient->Client_Disconnect());
}

/*! Create an account. szUsername (string), szPassword (string)
*/
int CScriptObjectNewUbisoftClient::Client_CreateAccount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *szUsername;
	const char *szPassword;

	pH->GetParam(1,szUsername);
	pH->GetParam(2,szPassword);

	return pH->EndFunction(m_pUbiSoftClient->Client_CreateAccount(szUsername,szPassword));
}

int CScriptObjectNewUbisoftClient::Client_IsConnected(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (m_pUbiSoftClient->Client_IsConnected())
	{
		m_pSystem->GetILog()->Log("Client connected");
		return pH->EndFunction(1);
	}
	else
	{
		m_pSystem->GetILog()->Log("Client NOT connected");
		return pH->EndFunctionNull();
	}
}

int CScriptObjectNewUbisoftClient::Client_SetCDKey(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *szCDKey;

	pH->GetParam(1,szCDKey);
	
	if (m_pUbiSoftClient->Client_SetCDKey(szCDKey))
	{
		return pH->EndFunction(1);
	}
	else
	{
		return pH->EndFunctionNull();
	}
}

int CScriptObjectNewUbisoftClient::Client_RequestMOTD(IFunctionHandler *pH)
{
	if (m_pUbiSoftClient->Client_RequestMOTD("EN"))
	{
		return pH->EndFunction(1);
	}
	else
	{
		return pH->EndFunctionNull();
	}
}


int CScriptObjectNewUbisoftClient::Server_DestroyServer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pUbiSoftClient->Server_DestroyServer());
}


/////////////////////////////////////////////////////////////////////////////
//
// 
//
/////////////////////////////////////////////////////////////////////////////
void CScriptObjectNewUbisoftClient::Client_LoginSuccess(const char *szUsername)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_LoginSuccess");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szUsername);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_LoginFail(const char *szError)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_LoginFail");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szError);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_GameServer(int iLobbyID, int iRoomID, const char *szGameServer,
										 const char *szIPAddress, const char *szLANIPAddress, int iMaxPlayers,
										 int iNumPlayers)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_GameServer");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(iLobbyID);
	m_pScriptSystem->PushFuncParam(iRoomID);
	m_pScriptSystem->PushFuncParam(szGameServer);
	m_pScriptSystem->PushFuncParam(szIPAddress);
	m_pScriptSystem->PushFuncParam(szLANIPAddress);
	m_pScriptSystem->PushFuncParam(iMaxPlayers);
	m_pScriptSystem->PushFuncParam(iNumPlayers);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_RequestFinished()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_RequestFinished");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_JoinGameServerSuccess(const char *szIPAddress, const char *szLanIPAddress,
	unsigned short usPort)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_JoinGameServerSuccess");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szIPAddress);
	m_pScriptSystem->PushFuncParam(szLanIPAddress);
	m_pScriptSystem->PushFuncParam(usPort);
	m_pScriptSystem->EndCall();

}
void CScriptObjectNewUbisoftClient::Client_JoinGameServerFail(const char *szError)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_JoinGameServerFail");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szError);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_CreateAccountSuccess()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_CreateAccountSuccess");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_CreateAccountFail(const char *szText)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_CreateAccountFail");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szText);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Client_MOTD(const char *szUbiMOTD, const char *szGameMOTD)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Client_MOTD");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szUbiMOTD);
	m_pScriptSystem->PushFuncParam(szGameMOTD);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Server_RegisterServerSuccess(int iLobbyID, int iRoomID)
{
	CStream stm;

	stm.Write(iLobbyID);
	stm.Write(iRoomID);

	m_pScriptSystem->BeginCall("NewUbisoftClient","Server_RegisterServerSuccess");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(iLobbyID);
	m_pScriptSystem->PushFuncParam(iRoomID);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Server_RegisterServerFail()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Server_RegisterServerFail");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Server_LobbyServerDisconnected()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Server_LobbyServerDisconnected");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Server_PlayerJoin(const char *szUsername)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Server_PlayerJoin");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szUsername);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::Server_PlayerLeave(const char *szUsername)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","Server_PlayerLeave");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szUsername);
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::CDKey_Failed(const char *szText)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","CDKey_Failed");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szText);
	m_pScriptSystem->EndCall();
}
void CScriptObjectNewUbisoftClient::CDKey_GetCDKey()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","CDKey_GetCDKey");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}

void CScriptObjectNewUbisoftClient::CDKey_ActivationSuccess()
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","CDKey_ActivationSuccess");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->EndCall();
}
void CScriptObjectNewUbisoftClient::CDKey_ActivationFail(const char *szText)
{
	m_pScriptSystem->BeginCall("NewUbisoftClient","CDKey_ActivationFail");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szText);
	m_pScriptSystem->EndCall();
}

#endif // NOT_USE_UBICOM_SDK
