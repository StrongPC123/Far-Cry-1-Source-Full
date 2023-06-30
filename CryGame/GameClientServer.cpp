
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameMisc.cpp
//  
//	History:
//	-October	31,2003: created
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"
#include "UIHud.h"
#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"

//! create the server
//////////////////////////////////////////////////////////////////////////
bool CXGame::StartupServer(bool listen, const char *szName)
{
	m_pLog->Log("Creating the server");

	ShutdownServer();	// to be sure

	int nPort = sv_port->GetIVal();

	// set port and create server
	if(!m_pServer)
		m_pServer = new CXServer(this, nPort, szName, listen);

	if (!m_pServer || !m_pServer->IsOK()) // Check if the server has been created
	{
		// failed, lets try a different port
		m_pLog->Log("Server creation failed ! Try with another port");
		SAFE_DELETE(m_pServer);
		m_pServer = new CXServer(this, nPort+1, szName, listen);
		sv_port->Set(nPort+1);
		if (!m_pServer || !m_pServer->IsOK()) // Check if the server has been created
		{
			SAFE_DELETE(m_pServer);
			m_pLog->Log("Server creation failed !");
			return false;
		}
	}

	if (m_pRConSystem)
		m_pRConSystem->OnServerCreated(m_pServer->m_pIServer);

	m_pLog->Log("Server created");

//	m_pServer->Update();				// server is created but map wasn't set yet we don't want to allow connects before

	return true;
}

//////////////////////////////////////////////////////////////////////
//! shutdown the server
void CXGame::ShutdownServer()
{
	if(!m_pServer)
		return;

	if(!m_pServer->IsInDestruction())
	{
		m_pLog->Log("Shutdown CXServer");
		SAFE_DELETE(m_pServer);
		m_pLog->Log("CXServer shutdowned");
	}
}

//////////////////////////////////////////////////////////////////////
//! create the client for a multiplayer session
bool CXGame::StartupClient()
{
	m_pLog->Log("Creating the Client");

	ShutdownClient();	// to be sure

	m_pClient = new CXClient;

	if (!m_pClient->Init(this)) // Check if the client has been created
	{
		ShutdownClient();

		m_pLog->Log("Client creation failed !");
		return false;
	}

	m_pLog->Log("Client created");

	return true;
}

//! create the client for a singleplayer session
//! the client will use a fake connection
//////////////////////////////////////////////////////////////////////////
bool CXGame::StartupLocalClient()
{
	m_pLog->Log("Creating the LocalClient");

	m_pClient = new CXClient;	

	if (!m_pClient->Init(this,true)) // Check if the client has been created
	{
		ShutdownClient();

		m_pLog->Log("LocalClient creation failed !");
		return false;
	}

	m_pLog->Log("LocalClient created");

	return true;
}
//////////////////////////////////////////////////////////////////////
//! shutdown the client
void CXGame::ShutdownClient()
{
	if (!m_pClient) 
		return;
	m_pLog->Log("Disconnect the client");
	m_pClient->XDisconnect("@ClientHasQuit");
	m_pLog->Log("Shutdown the Client");

	m_pClient->MarkForDestruct();
	m_pClient->DestructIfMarked();
	m_pClient=NULL;
}

bool CXGame::IsClient()
{
	return m_pClient!=NULL && !m_pClient->m_bSelfDestruct;
}

//////////////////////////////////////////////////////////////////////
//! mark the client for deletion
void CXGame::MarkClientForDestruct()
{
	if(m_pClient)
		m_pClient->MarkForDestruct();
}

/*! implementation of IServerSnooperSink::OnServerFound
	called when a server is found after a CXGame::RefreshServerList() call
	@param ip address of the server
	@param stmServerInfo stream sent by the server to identitfy himself
	@param ping average lantency of the server response
*/
//////////////////////////////////////////////////////////////////////////
void CXGame::OnServerFound(CIPAddress &ip, const string &szServerInfoString, int ping)
{
	SXServerInfos ServerInfos;

	if(ServerInfos.Read(szServerInfoString))
	{
		ServerInfos.IP = CIPAddress(ServerInfos.nPort,ip.GetAsString());		// get the game port from the packet
		ServerInfos.nPing = ping;
		m_ServersInfos[ServerInfos.IP] = ServerInfos;
		TRACE("CXGame::OnServerFound %s[%s]==>%s",ServerInfos.strName.c_str(),ServerInfos.IP.GetAsString(true),ServerInfos.strMap.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnNETServerFound(const CIPAddress &ip, const string &szServerInfoString, int ping)
{
	SXServerInfos ServerInfos;

	bool bOk=ServerInfos.Read(szServerInfoString);

	if(bOk || IsDevModeEnable())			// in DevMode we still wanna see these servers
	{
		ServerInfos.IP = ip;
		ServerInfos.nPing = ping;

		m_pScriptObjectGame->OnNETServerFound((CIPAddress&)ip, ServerInfos);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnNETServerTimeout(const CIPAddress &ip)
{
	m_pScriptObjectGame->OnNETServerTimeout((CIPAddress&)ip);
}

/*! search the LAN for FarCry servers
	remove all "old" servers from the server list before start searching
*/
//////////////////////////////////////////////////////////////////////////
void CXGame::RefreshServerList()
{
	m_ServersInfos.clear();
	if(m_pServerSnooper)
		m_pServerSnooper->SearchForLANServers(GetCurrentTime());
	TRACE("Refresh for lan");
}
