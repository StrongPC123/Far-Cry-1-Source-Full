#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK

#include "NewUbisoftClient.h"
#include "LobbyDefines.h"
#include "CommonDefines.h"
#include "IConsole.h"									// ICVar

#if !defined(LINUX)
#include <assert.h>
#endif


#if defined(WIN32) || defined(WIN64)
#include "windows.h"
#endif

#include "ScriptObjectNewUbisoftClient.h"					// CScriptObjectNewUbisoftClient

using namespace std;

static const char GUESTUSERNAME[33]="Ubi_Guest";
static const char GUESTPASSWORD[17]="testtest";

static DWORD g_dwKeepalifeCreateServer=60;		// in seconds


bool NewUbisoftClient::Server_CreateServer( const char* szServerName,unsigned int uiMaxPlayer )
{
//	m_pLog->Log("Ubi.com: DEBUG NewUbisoftClient::Server_CreateServer() 1");

	// The server should check cdkeys
	if ((m_eServerState == CreatingServer) || (m_eServerState == ServerConnected))
		return false;
	m_eServerState = ServerDisconnected;

//	m_pLog->Log("Ubi.com: DEBUG NewUbisoftClient::Server_CreateServer() 2");

	if (!DownloadGSini(GUESTUSERNAME))
		return false;

//	m_pLog->Log("Ubi.com: DEBUG NewUbisoftClient::Server_CreateServer() 3");

	m_dwNextServerAbsTime = 0;
	m_strGameServerName = szServerName;
	m_uiMaxPlayers = uiMaxPlayer;

	if (m_strGameServerName.size() > 32)
	{
		m_strGameServerName.resize(29);
		m_strGameServerName += "...";
	}

	return Server_RecreateServer();
}

void NewUbisoftClient::Server_SetGamePort( unsigned short usGamePort )
{
	m_usGamePort = usGamePort;
}

bool NewUbisoftClient::Server_RecreateServer()
{
	// Only recreate the server if were disconnected
	if (m_eServerState != ServerDisconnected)
		return false;

	// Wait untill it's time to reconnect
	if(m_dwNextServerAbsTime)
		if(m_pScriptObject->GetAbsTimeInSeconds() < m_dwNextServerAbsTime)
			return false;

	IServer *pServer = m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort);

	// if this is a lan server
	if(pServer && pServer->GetServerType()==eMPST_LAN)
		return false;

//	m_pLog->Log("Ubi.com: DEBUG Server_RecreateServer() 3");

	m_pLog->Log("\001Ubi.com: Server_RecreateServer");

	m_dwNextServerAbsTime = 0;
	m_eServerState = CreatingServer;

	// Go through numbered IP and Ports in the ini
	char szIPAddress[50];
	unsigned short usClientPort,usRegServerPort;
	int iIndex = 0;

	if (GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
	{
		while (CRegisterServer::RegServerSend_RouterConnect(szIPAddress, usRegServerPort) == GS_FALSE)
		{
			m_pLog->Log("\001Ubi.com: RegServerSend_RouterConnect '%s:%d' failed",szIPAddress,(int)usRegServerPort);
			iIndex++;
			if (!GetRouterAddress(iIndex,szIPAddress,&usClientPort,&usRegServerPort))
			{
				RegServerDisconnected();
				return false;
			}
		}
	}

	CRegisterServer::RegServerSend_LoginRouter(GUESTUSERNAME,GUESTPASSWORD,UBISOFT_GAME_VERSION);

	return true;
}

bool NewUbisoftClient::Server_UpdateServer(unsigned int uiMaxPlayers, unsigned short usPort)
{
	if (RegServerSend_UpdateGroupSettings(m_iServerRoomID,-1,-1,-1,uiMaxPlayers,-1,NULL,NULL,-1,NULL,-1,NULL,-1,usPort))
		return true;
	else
		return false;
}

bool NewUbisoftClient::Server_DestroyServer()
{
	IServer *pServer = m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort);

	if(!pServer)
		return false;

	// If the server type isn't UBI then do nothing
	if(pServer->GetServerType()!=eMPST_UBI)
		return false;

	//We no longer want to run a ubi.com server
	m_eServerState = NoUbiServer;
	m_dwNextServerAbsTime = 0;
	// The server stop checking cdkeys
	RegServerSend_RouterDisconnect();
	RegServerSend_LobbyServerClose();

	for (int i = 0; i < 10; i++)
	{
		Sleep(100);

		Update();
	}

	m_eServerState = NoUbiServer;

	return true;
}

////////////////////////////////////////////////////////////////////
//
// The Server callbacks
//
////////////////////////////////////////////////////////////////////
GSvoid NewUbisoftClient::RegServerRcv_LoginRouterResult( GSubyte ucType, GSint lReason,
		const GSchar* szIPAddress )
{
	if (ucType == GSFAIL)
	{
		m_pLog->Log("\001Ubi.com: LoginRouterResult failed");
		Server_RegisterServerFail();
		RegServerDisconnected();
		return;
	}

	CRegisterServer::RegServerSend_RequestParentGroupOnLobby(GAME_NAME);
}

GSvoid NewUbisoftClient::RegServerRcv_RouterDisconnection()
{
	//Server_RouterDisconnected();
}

GSvoid NewUbisoftClient::RegServerRcv_RegisterServerResult( GSubyte ucType,GSint plReason,GSint iGroupID,
		const GSchar* szAddress,GSushort usPort,const GSchar* szSessionName )
{
	if (ucType == GSSUCCESS)
	{
		int iPort = sv_regserver_port->GetIVal();

		if (!RegServerSend_LobbyServerConnection(szAddress, usPort,iPort,10))
		{
			m_pLog->Log("\001Ubi.com: LobbyServerConnection failed %s %i",szAddress,usPort);
			RegServerDisconnected();
			Server_RegisterServerFail();
			return;
		}

		RegServerSend_LobbyServerLogin(GUESTUSERNAME,iGroupID);

	}
	else
	{
		m_pLog->Log("\001Ubi.com: RegisterServerResult failed");
		RegServerDisconnected();
	}
}

GSvoid NewUbisoftClient::RegServerRcv_RequestParentGroupResult( GSubyte ucType,
		GSint lReason, GSint iServerID,GSint iGroupID, const GSchar* szGroupName,
		GSuint uiNbPlayers, GSuint uiMaxPlayers )
{
	if (ucType == GSSUCCESS)
	{
		// if the server id less them or equal to 0 then we have finished receiving the list of Parent Groups
		if (iServerID <=0)
		{
			GSchar szData[100];

			_snprintf(szData,100,"%i",m_usGamePort);

			m_pLog->Log("\001Ubi.com: RequestParentGroupResult success");
			// We will let the library pick the best parent group to register on.
			CRegisterServer::RegServerSend_RegisterServerOnLobby(0,0,m_strGameServerName.c_str(),
					GAME_NAME,ROOM_UBI_CLIENTHOST_REGSERVER,m_uiMaxPlayers,0,"",szData,sizeof(szData),NULL,0,NULL,0,
					m_usGamePort,"",GSGAMEVERSION,GS_FALSE,GS_FALSE);
		}
	}
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerLoginResults( GSubyte ucType,
		GSint iReason, GSint iLobbyServerID, GSint iGroupID )
{
	if (ucType == GSSUCCESS)
	{
		RegServerSend_RouterDisconnect();
		m_iServerLobbyID = iLobbyServerID;
		m_iServerRoomID = iGroupID;
		m_pLog->Log("\001Ubi.com Game Server Register Success");
		m_eServerState = ServerConnected;
		if (m_eClientState != NoUbiClient)
		{
			m_iJoinedLobbyID = iLobbyServerID;
			m_iJoinedRoomID = iGroupID;
		}
		Server_RegisterServerSuccess(iLobbyServerID, iGroupID);
	}
	else
	{
		RegServerDisconnected();
		m_pLog->Log("\001Ubi.com Game Server Register Failed");
		Server_RegisterServerFail();
	}
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerUpdateGroupSettingsResults(
		GSubyte ucType, GSint iReason, GSint iGroupID )
{
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerDisconnection()
{
	RegServerDisconnected();
	Server_LobbyServerDisconnected();
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerMemberNew( const GSchar* szMember,GSbool bSpectator,
	const GSchar* szIPAddress, const GSchar* szAltIPAddress,
	const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize,GSushort usPlayerStatus )
{
	Server_PlayerJoin(szMember);
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerMemberLeft(const GSchar* szMember )
{
	Server_PlayerLeave(szMember);
}

GSvoid NewUbisoftClient::RegServerRcv_LobbyServerNewGroup (GSushort usRoomType,
	const GSchar* szRoomName,GSint iGroupID,GSint iLobbyServerID,GSint iParentGroupID,
	GSint uiGroupConfig,GSshort sGroupLevel,const GSchar* szMaster,const GSchar* szAllowedGames,
	const GSchar* szGame,const GSvoid* pGroupInfo,GSuint GroupInfoSize,GSuint uiMatchEventID,
	GSuint uiMaxPlayers,GSuint uiNbPlayers,GSuint uiMaxSpectators,GSuint uiNbSpectators,
	const GSchar* szGameVersion,const GSchar* szGSGameVersion,const GSchar* szIPAddress,
	const GSchar* szAltIPAddress )
{
}

void NewUbisoftClient::RegServerDisconnected()
{
	m_pLog->Log("\001Ubi.com: RegServerDisconnected");
	m_eServerState = ServerDisconnected;
	m_dwNextServerAbsTime = m_pScriptObject->GetAbsTimeInSeconds() + g_dwKeepalifeCreateServer;
}


#endif // NOT_USE_UBICOM_SDK
