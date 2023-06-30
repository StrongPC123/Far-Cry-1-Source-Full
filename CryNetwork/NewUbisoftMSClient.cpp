#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK

#include "NewUbisoftClient.h"
#include "LobbyDefines.h"
#include "CommonDefines.h"


#if !defined(LINUX)
	#include "windows.h"
#endif

#if !defined(LINUX)
#include <assert.h>
#endif


#include "ScriptObjectNewUbisoftClient.h"		// CScriptObjectNewUbisoftClient


// the following libs are not in the project setting because we want to have then only in if NOT_USE_UBICOM_SDK is defined
/*#ifdef _DEBUG
	#pragma comment(lib,"libgsclient_debug.lib")
	#pragma comment(lib,"libgsconnect_debug.lib")
	#pragma comment(lib,"libgscrypto_debug.lib")
	#pragma comment(lib,"libgsutility_debug.lib")
	#pragma comment(lib,"libgsregserver_debug.lib")
	#pragma comment(lib,"libgssocket_debug.lib")
	#pragma comment(lib,"libgsproxyclient_debug.lib")
	#pragma comment(lib,"libgsresult_debug.lib")	
	#pragma comment(lib,"libgscdkey_debug.lib")
#else
	*/


#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	#pragma comment(lib,"libgsclient.lib")
	#pragma comment(lib,"libgsmsclient.lib")
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	#pragma comment(lib,"libgsconnect.lib")
	#pragma comment(lib,"libgscrypto.lib")
	#pragma comment(lib,"libgsutility.lib")
	#pragma comment(lib,"libgsregserver.lib")
	#pragma comment(lib,"libgssocket.lib")
	#pragma comment(lib,"libgsproxyclient.lib")
	#pragma comment(lib,"libgsresult.lib")
	#pragma comment(lib,"libgscdkey.lib")
//#endif


/*
gsnat,
gshttp,
gsdatacontainer,
gszlib,
*/


static DWORD g_dwKeepalifeLoginClient=60;		// in seconds
static DWORD g_dwAccountCreateTimeout=60; //Timeout between calls to CreateAcount in seconds

bool NewUbisoftClient::Client_IsConnected()
{
	if (m_eClientState >= ClientLoggedIn)
		return true;
	else
		return false;
}

bool NewUbisoftClient::Client_AutoLogin()
{
	if (m_strUsername.empty() || m_strPassword.empty())
	{
		string szHexUsername;
		string szHexPassword;

		if (!ReadStringFromRegistry("Ubi.com", "username", szHexUsername) || !ReadStringFromRegistry("Ubi.com", "password", szHexPassword))
		{
			return false;
		}

		char szEncUsername[256] = {0};
		char szEncPassword[256] = {0};
		char szUsername[256] = {0};
		char szPassword[256] = {0};

		DecodeHex((unsigned char *)szEncUsername, (unsigned char *)szHexUsername.c_str());
		DecodeHex((unsigned char *)szEncPassword, (unsigned char *)szHexPassword.c_str());

		DecryptString((unsigned char *)szUsername, (unsigned char *)szEncUsername);
		DecryptString((unsigned char *)szPassword, (unsigned char *)szEncPassword);

		m_strUsername = szUsername;
		m_strPassword = szPassword;
	}

	return Client_Login(m_strUsername.c_str(), m_strPassword.c_str());
}

bool NewUbisoftClient::Client_Login(const GSchar* szUsername, const GSchar* szPassword, bool bSavePassword)
{
	if (m_eClientState >= ClientLoggingIn)
		return false;

	// Save the username and password.
	m_strUsername = szUsername;
	m_strPassword = szPassword;
	m_bSavePassword = bSavePassword;

	// remove it now
	if (!m_bSavePassword)
	{
		RemoveStringFromRegistry("Ubi.com", "username");
		RemoveStringFromRegistry("Ubi.com", "password");
	}

	if (!DownloadGSini(szUsername))
	{
		Client_LoginFail(CONNECTIONFAILED);
		return false;
	}

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	// Go through numbered IP and Ports in the ini
	char szIPAddress[50];
	unsigned short usClientPort,usRegServerPort;
	int iIndex = 0;

	if (GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
	{
		while (clMSClientClass::Initialize(szIPAddress, usClientPort, szUsername, szPassword,
			UBISOFT_GAME_VERSION) == GS_FALSE)
		{
			iIndex++;
			if (!GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
			{
				MSClientDisconnected();
				return false;
			}
		}
	}
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	m_eClientState = ClientLoggingIn;
	return true;
}


bool NewUbisoftClient::Client_RequestGameServers()
{
	if (m_eClientState < ClientLoggedIn) // If we are logged in to the Game Service
		return false;

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (m_eClientState > ClientLoggedIn)
	{
		m_pLog->Log("Ubi.com: Client_RequestGameServers %i",(int)m_eClientState);
    clMSClientClass::LeaveGameServer(m_iJoinedLobbyID,m_iJoinedRoomID);
    m_eClientState = ClientLoggedIn;
	}

	clMSClientClass::RequestGameServers(GAME_NAME);
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	return true;
}

bool NewUbisoftClient::Client_JoinGameServer(int iLobbyID, int iRoomID)
{
	m_pLog->Log("Ubi.com: Client_JoinGameServer %i %i",iLobbyID, iRoomID);

	switch (m_eClientState)
	{
		case NoUbiClient:
		case ClientLoggingIn:
			return false;
		case ClientDisconnected:
		{
			// If we haven't logged in before
			if (m_strUsername.empty())
				return false;

			m_iJoinedLobbyID = iLobbyID;
			m_iJoinedRoomID = iRoomID;
			Client_Login(m_strUsername.c_str(),m_strPassword.c_str());
			return true;
		}

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
		case ClientLoggedIn:
		case GameServerDisconnected:
		{
			if (clMSClientClass::JoinGameServer(iLobbyID,iRoomID
				,"",GSGAMEVERSION,GAME_NAME,NULL,0))
			{
				m_iJoinedLobbyID = iLobbyID;
				m_iJoinedRoomID = iRoomID;
				m_eClientState = JoiningGameServer;
				return true;
			}
			else
				return false;
		}
		case JoinedGameServer:
			if ((m_iJoinedLobbyID == iLobbyID) && (m_iJoinedRoomID == iRoomID))
				return true;
			else
			{
				if (clMSClientClass::JoinGameServer(iLobbyID,iRoomID
					,"",GSGAMEVERSION,GAME_NAME,NULL,0))
				{
					m_iJoinedLobbyID = iLobbyID;
					m_iJoinedRoomID = iRoomID;
					m_eClientState = JoiningGameServer;
					return true;
				}
				else
					return false;
			}
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	}
	return false;
}

bool NewUbisoftClient::Client_ReJoinGameServer()
{
	// Wait untill it's time to reconnect
	if(m_dwNextClientAbsTime)
		if(m_pScriptObject->GetAbsTimeInSeconds() < m_dwNextClientAbsTime)
			return false;

	m_dwNextClientAbsTime = 0;
	switch (m_eClientState)
	{
		case NoUbiClient:
		case ClientLoggingIn:
		case JoiningGameServer:
		case JoinedGameServer:
			return false;
		case ClientDisconnected:
		case ClientLoggedIn:
		case GameServerDisconnected:
			Client_JoinGameServer(m_iJoinedLobbyID,m_iJoinedRoomID);
			return true;
	}
	return false;
}

bool NewUbisoftClient::Client_ConnectedToGameServer()
{
#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (clMSClientClass::GameServerConnected(m_iJoinedLobbyID,m_iJoinedRoomID))
		return true;
	else
#endif // EXCLUDE_UBICOM_CLIENT_SDK
		return false;
}

bool NewUbisoftClient::Client_LeaveGameServer()
{
#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (clMSClientClass::LeaveGameServer(m_iJoinedLobbyID,m_iJoinedRoomID))
	{
		m_iJoinedLobbyID = 0;
		m_iJoinedRoomID = 0;
		m_eClientState = ClientLoggedIn;
		return true;
	}
	else
#endif // EXCLUDE_UBICOM_CLIENT_SDK
		return false;
}

bool NewUbisoftClient::Client_Disconnect()
{
	if (m_eClientState == NoUbiClient)
		return true;

	m_bCheckCDKeys = false;

	if (m_eClientState != ClientLoggingIn)
	{
#ifndef EXCLUDE_UBICOM_CLIENT_SDK
		clMSClientClass::Uninitialize();
#endif // EXCLUDE_UBICOM_CLIENT_SDK

		m_iJoinedLobbyID = 0;
		m_iJoinedRoomID= 0;
		m_eClientState = NoUbiClient;
		m_bDisconnecting = 0;
	}
	else
	{
		m_bDisconnecting = 1;
	}

	Update();

	return true;
}

bool NewUbisoftClient::Client_CreateAccount(const char *szUsername, const char *szPassword)
{
	if (m_eClientState != NoUbiClient)
		return false;

	if (m_dwAccountCreateTime)
		if(m_pScriptObject->GetAbsTimeInSeconds() < m_dwAccountCreateTime)
			Client_CreateAccountFail(CREATEACCOUNTBLOCKED);

	m_eClientState = CreateUbiAccount;

	if (!DownloadGSini(szUsername))
		return false;

	// Go through numbered IP and Ports in the ini

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	char szIPAddress[50];
	unsigned short usClientPort,usRegServerPort;
	int iIndex = 0;

	if (GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
	{
		while (clMSClientClass::CreateAccount(szIPAddress, usClientPort, UBISOFT_GAME_VERSION, szUsername,
			szPassword, "","","","") == GS_FALSE)
		{
			iIndex++;
			if (!GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
				return false;
		}
	}
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	return true;
}

bool NewUbisoftClient::Client_RequestMOTD(const char *szLanguage)
{
	if (m_eClientState < ClientLoggedIn)
		return false;

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (RequestMOTD(szLanguage))
		return true;
	else
#endif // EXCLUDE_UBICOM_CLIENT_SDK
		return false;
}

///////////////////////////////////////////////////////
//
// MSClient Callbacks
//
///////////////////////////////////////////////////////

GSvoid NewUbisoftClient::GameServerCB(GSint iLobbyID,GSint iRoomID,GSshort siGroupType,
		GSchar *szGroupName,GSint iConfig,GSchar *szMaster,GSchar *szAllowedGames,
		GSchar *szGames,GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,
		GSint iSize,GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
		GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSint iEventID)
{
	char *szPort = (char*)vpInfo;
	char szGameIPAddress[32],szGameLANIPAddress[32];

	m_pLog->Log("Ubi.com: GameServerCB %s %i %i",szGroupName,iLobbyID, iRoomID);
	_snprintf(szGameIPAddress,32,"%s:%s",szIPAddress,szPort);
	_snprintf(szGameLANIPAddress,32,"%s:%s",szAltIPAddress,szPort);

	Client_GameServer(iLobbyID,iRoomID,szGroupName,szGameIPAddress,szGameLANIPAddress,
		uiMaxPlayer,uiNbrPlayer);
}

GSvoid NewUbisoftClient::ErrorCB(GSint iReason,GSint iLobbyID,GSint iRoomID)
{
	m_pLog->Log("Ubi.com: ErrorCB %i %i",iLobbyID, iRoomID);
	m_eClientState = GameServerDisconnected;
	m_dwNextClientAbsTime = m_pScriptObject->GetAbsTimeInSeconds() + g_dwKeepalifeLoginClient;
	switch (iReason)
	{
		default:
		case ERRORLOBBYSRV_UNKNOWNERROR:
			Client_JoinGameServerFail(UNKNOWNERROR);
			break;
		case ERRORLOBBYSRV_GROUPNOTEXIST:
			Client_JoinGameServerFail(GROUPNOTEXIST);
			break;
		case ERRORLOBBYSRV_NOMOREPLAYERS:
		case ERRORLOBBYSRV_NOMOREMEMBERS:
			Client_JoinGameServerFail(NOMOREPLAYERS);
			break;
	}
}

GSvoid NewUbisoftClient::InitFinishedCB(GSubyte ucType,GSint iError,GSchar *szUserName)
{
	if (m_bDisconnecting)
	{
		// fake logged in state
		// so we can actually get disconnected
		m_eClientState = ClientLoggedIn;

		Client_Disconnect();

		return;
	}

	if (ucType == GSSUCCESS)
	{
		m_strUsername = szUserName;
		m_eClientState = ClientLoggedIn;
		if (m_iJoinedLobbyID)
		{
			Client_JoinGameServer(m_iJoinedLobbyID,m_iJoinedRoomID);
			m_eClientState = JoiningGameServer;
		}
		Client_LoginSuccess(szUserName);
		Client_CheckForCDKey();
	}
	else
	{
		m_eClientState = NoUbiClient;
		switch (iError)
		{
			case ERRORSECURE_INVALIDACCOUNT:
				Client_LoginFail(INVALIDACCOUNT);
				break;
			case ERRORSECURE_INVALIDPASSWORD:
				Client_LoginFail(INVALIDPASSWORD);
				break;
			case ERRORSECURE_DATABASEFAILED:
				Client_LoginFail(DATABASEFAILED);
				break;
			case ERRORSECURE_BANNEDACCOUNT:
				Client_LoginFail(BANNEDACCOUNT);
				break;
			case ERRORSECURE_BLOCKEDACCOUNT:
				Client_LoginFail(BLOCKEDACCOUNT);
				break;
			case ERRORSECURE_LOCKEDACCOUNT:
				Client_LoginFail(LOCKEDACCOUNT);
				break;
			case ERRORROUTER_NOTDISCONNECTED:
				Client_LoginFail(NOTDISCONNECTED);
		}
	}
}

GSvoid NewUbisoftClient::LoginDisconnectCB()
{
	m_pLog->Log("Ubi.com: LoginDisconnectCB");
	if (m_eClientState == ClientLoggingIn)
		m_eClientState = NoUbiClient;
	else
		MSClientDisconnected();
}

GSvoid NewUbisoftClient::LobbyDisconnectCB()
{
	m_pLog->Log("Ubi.com: LobbyDisconnectCB");
	if (m_eClientState > GameServerDisconnected)
		m_eClientState = GameServerDisconnected;
}

GSvoid NewUbisoftClient::RequestFinishedCB()
{
	Client_RequestFinished();
}

GSvoid NewUbisoftClient::JoinFinishedCB(GSint iLobbyID,GSint iRoomID,
		GSvoid *vpGameData,GSint iSize,GSchar *szIPAddress,
		GSchar *szAltIPAddress,GSushort usPort)
{
	m_eClientState = JoinedGameServer;
	Client_ConnectedToGameServer();
	Client_JoinGameServerSuccess(szIPAddress,szAltIPAddress,usPort);
}

GSvoid NewUbisoftClient::AlternateInfoCB(GSint iLobbyID,GSint iRoomID,
			const GSvoid* pcAltGroupInfo,GSint iAltGroupInfoSize)
{
}

GSvoid NewUbisoftClient::AccountCreationCB(GSubyte ucType, GSint iReason)
{
	m_eClientState = NoUbiClient;
	if (ucType == GSSUCCESS)
	{
		m_dwAccountCreateTime = m_pScriptObject->GetAbsTimeInSeconds() + g_dwAccountCreateTimeout;
		Client_CreateAccountSuccess();
	}
	else
	{
		m_dwAccountCreateTime = 0;
		switch (ucType)
		{
			default:
				Client_CreateAccountFail(UNKNOWNERROR);
				break;
			case ERRORSECURE_USERNAMEEXISTS:
				Client_CreateAccountFail(USERNAMEEXISTS);
				break;
			case ERRORSECURE_USERNAMEMALFORMED:
				Client_CreateAccountFail(USERNAMEMALFORMED);
				break;
			case ERRORSECURE_USERNAMEFORBIDDEN:
				Client_CreateAccountFail(USERNAMEFORBIDDEN);
				break;
			case ERRORSECURE_USERNAMERESERVED:
				Client_CreateAccountFail(USERNAMERESERVED);
				break;
			case ERRORSECURE_PASSWORDMALFORMED:
				Client_CreateAccountFail(PASSWORDMALFORMED);
				break;
			case ERRORSECURE_PASSWORDFORBIDDEN:
				Client_CreateAccountFail(PASSWORDFORBIDDEN);
				break;
		}
	}
}

GSvoid NewUbisoftClient::ModifyAccountCB(GSubyte ucType, GSint iReason)
{
}

GSvoid NewUbisoftClient::RequestMOTDCB(GSubyte ubType, GSchar *szUbiMOTD, GSchar *szGameMOTD, GSint iReason)
{
	if (ubType == GSSUCCESS)
		m_pScriptObject->Client_MOTD(szUbiMOTD,szGameMOTD);
}

void NewUbisoftClient::MSClientDisconnected()
{
	// was this a requested disconnect ?
	if (m_eClientState != NoUbiClient)
	{
		m_eClientState = ClientDisconnected;
	}
	m_dwNextClientAbsTime = m_pScriptObject->GetAbsTimeInSeconds() + g_dwKeepalifeLoginClient;
}

#else // NOT_USE_UBICOM_SDK

// the following libs are excluded from the build in the project settings and here they are included
// because only if we don't use UBI.com we need them
#ifdef _DEBUG
	#pragma comment(lib,"libcmtd.lib")
#else
	#pragma comment(lib,"libcmt.lib")
#endif

#endif // NOT_USE_UBICOM_SDK
