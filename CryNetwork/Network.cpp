// Network.cpp: implementation of the CNetwork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "Client.h"
#include "ClientLocal.h"
#include "ServerSlot.h"
#include "ServerSnooper.h"
#include "NETServerSnooper.h"
#include "Server.h"
#include "RConSystem.h"
#include "DefenceWall.h"
#if !defined(NOT_USE_PUNKBUSTER_SDK)
#include "PunkBusterInterface.h"
#endif
#include "ITimer.h"

#ifndef NOT_USE_UBICOM_SDK
	#include "NewUbisoftClient.h"								// NewUbisoftClient
	#include "ScriptObjectNewUbisoftClient.h"		// CScriptObjectNewUbisoftClient
#endif // NOT_USE_UBICOM_SDK

#define ANTI_CHEATS

#include "platform.h"
#if defined(LINUX)
#include "INetwork.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
unsigned int CNetwork::m_nCryNetInitialized = 0;

struct CryNetError CNetwork::m_neNetErrors[]=
{
	{NET_OK, "No Error"},
	{NET_FAIL, "Generic Error"},
	// SOCKET
	{NET_EINTR, "WSAEINTR - interrupted function call"},
	{NET_EBADF, "WSAEBADF - Bad file number"},
	{NET_EACCES, "WSAEACCES - error in accessing socket"},
	{NET_EFAULT, "WSAEFAULT - bad address"},
	{NET_EINVAL, "WSAEINVAL - invalid argument"},
	{NET_EMFILE, "WSAEMFILE - too many open files"},
	{NET_EWOULDBLOCK, "WSAEWOULDBLOCK - resource temporarily unavailable"},
	{NET_EINPROGRESS, "WSAEINPROGRESS - operation now in progress"},
	{NET_EALREADY, "WSAEALREADY - operation already in progress"},
	{NET_ENOTSOCK, "WSAENOTSOCK - socket operation on non-socket"},
	{NET_EDESTADDRREQ, "WSAEDESTADDRREQ - destination address required"},
	{NET_EMSGSIZE, "WSAEMSGSIZE - message to long"},
	{NET_EPROTOTYPE, "WSAEPROTOTYPE - protocol wrong type for socket"},
	{NET_ENOPROTOOPT, "WSAENOPROTOOPT - bad protocol option"},
	{NET_EPROTONOSUPPORT, "WSAEPROTONOSUPPORT - protocol not supported"},
	{NET_ESOCKTNOSUPPORT, "WSAESOCKTNOSUPPORT - socket type not supported"},
	{NET_EOPNOTSUPP, "WSAEOPNOTSUPP - operation not supported"},
	{NET_EPFNOSUPPORT, "WSAEPFNOSUPPORT - protocol family not supported"},
	{NET_EAFNOSUPPORT, "WSAEAFNOSUPPORT - address family not supported by protocol"},
	{NET_EADDRINUSE, "WSAEADDRINUSE - address is in use"},
	{NET_EADDRNOTAVAIL, "WSAEADDRNOTAVAIL - address is not valid in context"},
	{NET_ENETDOWN, "WSAENETDOWN - network is down"},
	{NET_ENETUNREACH, "WSAENETUNREACH - network is unreachable"},
	{NET_ENETRESET, "WSAENETRESET - network dropped connection on reset"},
	{NET_ECONNABORTED, "WSACONNABORTED - software caused connection aborted"},
	{NET_ECONNRESET, "WSAECONNRESET - connection reset by peer"},
	{NET_ENOBUFS, "WSAENOBUFS - no buffer space available"},
	{NET_EISCONN, "WSAEISCONN - socket is already connected"},
	{NET_ENOTCONN, "WSAENOTCONN - socket is not connected"},
	{NET_ESHUTDOWN, "WSAESHUTDOWN - cannot send after socket shutdown"},
	{NET_ETOOMANYREFS, "WSAETOOMANYREFS - Too many references: cannot splice"},
	{NET_ETIMEDOUT, "WSAETIMEDOUT - connection timed out"},
	{NET_ECONNREFUSED, "WSAECONNREFUSED - connection refused"},
	{NET_ELOOP, "WSAELOOP - Too many levels of symbolic links"},
	{NET_ENAMETOOLONG, "WSAENAMETOOLONG - File name too long"},
	{NET_EHOSTDOWN, "WSAEHOSTDOWN - host is down"},
	{NET_EHOSTUNREACH, "WSAEHOSTUNREACH - no route to host"},
	{NET_ENOTEMPTY, "WSAENOTEMPTY - Cannot remove a directory that is not empty"},
	{NET_EUSERS, "WSAEUSERS - Ran out of quota"},
	{NET_EDQUOT, "WSAEDQUOT - Ran out of disk quota"},
	{NET_ESTALE, "WSAESTALE - File handle reference is no longer available"},
	{NET_EREMOTE, "WSAEREMOTE - Item is not available locally"},
	// extended winsock errors(not BSD compliant)
#ifdef _WIN32	
	{NET_EPROCLIM, "WSAEPROCLIM - too many processes"},
	{NET_HOST_NOT_FOUND, "WSAHOST_NOT_FOUND - host not found"},
	{NET_TRY_AGAIN, "WSATRY_AGAIN - non-authoritative host not found"},
	{NET_NO_RECOVERY, "WSANO_RECOVERY - non-recoverable error"},
	{NET_NO_DATA, "WSANO_DATA - valid name, no data record of requested type"},
	{NET_NO_ADDRESS, "WSANO_ADDRESS - (undocumented)"},
	{NET_SYSNOTREADY, "WSASYSNOTREADY - network subsystem is unavailable"},
	{NET_VERNOTSUPPORTED, "WSAVERNOTSUPPORTED - winsock.dll verison out of range"},
	{NET_NOTINITIALISED, "WSANOTINITIALISED - WSAStartup not yet performed"},
	{NET_NO_DATA, "WSANO_DATA - valid name, no data record of requested type"},
	{NET_EDISCON, "WSAEDISCON - graceful shutdown in progress"},
#endif	
	// XNetwork specific
	{NET_NOIMPL, "XNetwork - Function not implemented"},
	{NET_SOCKET_NOT_CREATED, "XNetwork - socket not yet created"},
	{0, 0} // sentinel
};

CNetwork::CNetwork()
{
	m_dwLocalIP=0;
	m_pNetCompressPackets = 0;
	m_pNetLog = 0;
	m_pDefenceWall = 0;
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	m_pPunkBuster = 0;
#endif
	m_bHaveServer = false;
	m_bHaveClient = false;

#ifndef NOT_USE_UBICOM_SDK
	m_pScriptObjectNewUbisoftClient=0;
	m_pUbiSoftClient=0;
#endif // NOT_USE_UBICOM_SDK

	m_pScriptSystem=0;
	m_pClient=0;
}

CNetwork::~CNetwork()
{
#ifndef NOT_USE_UBICOM_SDK
	SAFE_DELETE(m_pUbiSoftClient);
	SAFE_DELETE(m_pScriptObjectNewUbisoftClient);
	CScriptObjectNewUbisoftClient::ReleaseTemplate();
#endif // NOT_USE_UBICOM_SDK

	SAFE_RELEASE( m_pNetCompressPackets );
	SAFE_RELEASE( m_pNetLog );
	SAFE_DELETE( m_pDefenceWall );
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	SAFE_DELETE( m_pPunkBuster );
#endif
}


ICompressionHelper *CNetwork::GetCompressionHelper()
{
	return &m_Compressor;
}


void CNetwork::SetLocalIP( const char *szLocalIP )
{
	CIPAddress ip;

	ip.Set(0,(char *)szLocalIP);

	m_dwLocalIP=ip.GetAsUINT();
	
#ifndef NOT_USE_UBICOM_SDK
	// Ubi.com cannot change the IP later than creation
	{
		CIPAddress localip;
		localip.Set(0,GetLocalIP());
		m_pUbiSoftClient=new NewUbisoftClient(localip.GetAsString());

		m_pScriptObjectNewUbisoftClient=new CScriptObjectNewUbisoftClient;
		CScriptObjectNewUbisoftClient::InitializeTemplate(m_pScriptSystem);
		m_pScriptObjectNewUbisoftClient->Init(m_pScriptSystem,m_pSystem,m_pUbiSoftClient);
	}
#endif // NOT_USE_UBICOM_SDK
}

DWORD CNetwork::GetLocalIP() const
{
	return m_dwLocalIP;
}

bool CNetwork::Init( IScriptSystem *pScriptSystem )
{
	assert(pScriptSystem);
	
	m_pScriptSystem = pScriptSystem;

	if(CNetwork::m_nCryNetInitialized)
		return true;
	
#if !defined(PS2) && !defined(LINUX)
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	// WARNING :use a later version require a change in the multicast code with 
	//			a WS2's specific one
	//			not compatible with other platforms
	wVersionRequested = MAKEWORD(1, 1);
	
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return false;
	}
	
	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return false; 
	}
#endif	

	CNetwork::m_nCryNetInitialized+=1;
	int n=0;
	while(m_neNetErrors[n].sErrorDescription!='\0'){
		m_mapErrors[m_neNetErrors[n].nrErrorCode]=m_neNetErrors[n].sErrorDescription;
		n++;
	}

	/// Register Console Vars related to network.
	m_pSystem = GetISystem();

	CreateConsoleVars();

#ifdef ANTI_CHEATS
	m_pDefenceWall = new CDefenceWall(this);
#endif //ANTI_CHEATS
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	m_pPunkBuster = new CPunkBusterInterface(this);
	PBsdk_SetPointers ( m_pPunkBuster ) ;
#endif
	LogNetworkInfo();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::CreateConsoleVars()
{
	m_pNetCompressPackets = m_pSystem->GetIConsole()->CreateVariable( "net_compress_packets","1",0 );
	m_pNetLog = m_pSystem->GetIConsole()->CreateVariable( "net_log","0",0 );
	//m_pNetCheatProtection = m_pSystem->GetIConsole()->CreateVariable( "net_cheatprotection","0",VF_DUMPTODISK );
	m_pNetCheatProtection = m_pSystem->GetIConsole()->CreateVariable( "net_cheatprotection","0",VF_READONLY );
	m_pNetCheatProtection->ForceSet("0");

	m_pSystem->GetIConsole()->CreateVariable("sv_ServerType", "LAN",0,
		"Specifies the server connection type (next server creation, not case sensitive) 'UBI', 'LAN' or 'NET'\n"
		"Usage: sv_ServerType UBI\n");
	m_pSystem->GetIConsole()->CreateVariable("sv_regserver_port","",0,
		"Sets the local port for a registering the server on Ubi.com GS.\n"
		"Usage: sv_regserver_port portnumber\n"
		"Default is '0' (first free).");
	m_pSystem->GetIConsole()->CreateVariable("sv_authport","",0,
		"Sets the local port for a CDKey authentication comunications.\n"
		"Usage: sv_auth_port portnumber\n"
		"Default is '0' (first free).");
}

//////////////////////////////////////////////////////////////////////////
IClient *CNetwork::CreateClient(IClientSink *pSink, bool bLocal)
{
	assert(!m_pClient);			// only one client at a time is allowed

	m_bHaveClient = true;
	
	if(bLocal)
	{
		CClientLocal *pClient=new CClientLocal(this,pSink);

		m_pClient=pClient;

		return pClient;
	}
	else
	{
		CClient *pClient = new CClient( this );

		if(!pClient->Init(pSink))
		{
			delete pClient;
			return NULL;
		}
		if (GetCheatProtectionLevel() > 0)
		{
			if (m_pDefenceWall)
				m_pDefenceWall->SetClient(pClient);
		}

		m_pClient=pClient;

		return pClient;
	}

	return NULL;
}

IServer *CNetwork::CreateServer(IServerSlotFactory *pFactory, WORD nPort, bool listen)
{
	m_bHaveServer = true;
	CServer *pServer = new CServer(this);
	if (!pServer->Init(pFactory, nPort, listen))
	{
		delete pServer;
		return NULL;
	}
	m_mapServers.insert(MapServersItor::value_type(nPort, pServer));

	if (GetCheatProtectionLevel() > 0)
	{
		if (m_pDefenceWall)
			m_pDefenceWall->SetServer(pServer);
	}

	IConsole *pConsole = GetISystem()->GetIConsole();
	assert(pConsole);

	ICVar *sv_punkbuster = pConsole->GetCVar("sv_punkbuster");

	if (sv_punkbuster && sv_punkbuster->GetIVal() != 0)
	{
		InitPunkbusterServer(listen, pServer);

		// if this is a lan server
		if(pServer->GetServerType()==eMPST_LAN)
		{
			// set pb variables to lan defaults
			pConsole->ExecuteString("pb_sv_lan 1", 0, 1);
			pConsole->ExecuteString("pb_sv_guidrelax 7", 0, 1);
		}
	}

#ifndef NOT_USE_UBICOM_SDK
	m_pUbiSoftClient->Server_SetGamePort(nPort);	// set the connection to the server
#endif // NOT_USE_UBICOM_SDK

	LockPunkbusterCVars();

	return pServer;
}

INETServerSnooper *CNetwork::CreateNETServerSnooper(INETServerSnooperSink *pSink)
{
	CNETServerSnooper *pSnooper = new CNETServerSnooper;

	if (!pSnooper->Create(GetISystem(), pSink))
	{
		delete pSnooper;
		return 0;
	}

	return pSnooper;
}

IRConSystem *CNetwork::CreateRConSystem()
{
	CRConSystem *pRCon = new CRConSystem;

	if(!pRCon->Create(GetISystem()))
	{
		delete pRCon;
		return 0;
	}

	return pRCon;
}


IServerSnooper *CNetwork::CreateServerSnooper(IServerSnooperSink *pSink)
{
	CServerSnooper *pSnooper=new CServerSnooper;
	if(!pSnooper->Init(pSink))
	{
		delete pSnooper;
		return NULL;
	}
	return pSnooper;
}

CServerSlotLocal *CNetwork::ConnectToLocalServerSlot(CClientLocal *pClient, WORD wPort)
{
	MapServersItor itor;
	
	if(m_mapServers.empty())
		return NULL;
	//check if the local server exists
	itor=m_mapServers.find(wPort);

	if(itor==m_mapServers.end()){
		itor=m_mapServers.begin();
	}
	//<<FIXME>> make the port variable
	CIPAddress ip(0,"127.0.0.1");

	CServerSlotLocal *pServerSlot=new CServerSlotLocal(itor->second,pClient,ip,this);
	
	itor->second->RegisterLocalServerSlot(pServerSlot,ip);

	return pServerSlot;
}

const char *CNetwork::EnumerateError(NRESULT err)
{
	ERROR_MAPItor itor;
	itor = m_mapErrors.find(err);
	if (itor != m_mapErrors.end())
	{
		return itor->second;
	}
	return "Unknown";
}

void CNetwork::Release()
{
	CNetwork::m_nCryNetInitialized -= 1;
	
	if (CNetwork::m_nCryNetInitialized)
		return;
#if !defined(LINUX)
	#if !defined(PS2)
		else	
			WSACleanup();
	#else
		CSocketManager::releaseImpl();
		delete &ISocketManagerImplementation::getInstance();
	#endif		
#endif		
	delete this;
}


IServer *CNetwork::GetServerByPort( const WORD wPort )
{
	//check if the local server exists
	MapServersItor itor=m_mapServers.find(wPort);

	if(itor==m_mapServers.end())
		return 0;											// not found

	return itor->second;
}

void CNetwork::UnregisterServer(WORD wPort)
{
#ifndef NOT_USE_UBICOM_SDK
	// We have to tell Ubisoft that the client has finished the game.
	// If ubisoft is not running this won't do anything.
	m_pUbiSoftClient->Client_LeaveGameServer();
#endif // NOT_USE_UBICOM_SDK

	if(m_bHaveServer)
	{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
		if (m_pPunkBuster)
			m_pPunkBuster->ShutDown(false);
#endif
	}

	m_mapServers.erase(wPort);
}

void CNetwork::UnregisterClient( IClient *pClient )
{
	assert(pClient);
	assert(m_pClient==pClient);

	m_pClient=0;

	if(m_bHaveClient)
	{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
		if (m_pPunkBuster)
			m_pPunkBuster->ShutDown(true);
#endif
	}
}

void CNetwork::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(m_neNetErrors,sizeof(m_neNetErrors));
	
#ifndef NOT_USE_UBICOM_SDK
	pSizer->AddObject( m_pScriptObjectNewUbisoftClient, sizeof *m_pScriptObjectNewUbisoftClient);
#endif // NOT_USE_UBICOM_SDK
}

bool CNetwork::IsPacketCompressionEnabled() const
{
	return m_pNetCompressPackets->GetIVal() != 0;
}

//////////////////////////////////////////////////////////////////////////
CTimeValue CNetwork::GetCurrentTime()
{
	return m_pSystem->GetITimer()->GetCurrTimePrecise();
}

//////////////////////////////////////////////////////////////////////////
u32 CNetwork::GetStringHash( const char *szString )
{
#if defined(LINUX)
	return 0;
#else

#ifdef _DATAPROBE
	return m_pSystem->GetIDataProbe()->GetHash( szString );
#else
	return 0;
#endif

#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::PunkDetected( CIPAddress &ip )
{
	// Disconnect.
}



//////////////////////////////////////////////////////////////////////////
void CNetwork::UpdateNetwork()
{
#ifndef NOT_USE_UBICOM_SDK
	// Update ubisoft
	m_pUbiSoftClient->Update();
#endif // NOT_USE_UBICOM_SDK
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::OnClientUpdate()
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		m_pPunkBuster->Update(true);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::OnServerUpdate()
{
	if (m_pDefenceWall && GetCheatProtectionLevel() > 0)
		m_pDefenceWall->ServerUpdate();
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		m_pPunkBuster->Update(false);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::ClearProtectedFiles()
{
	if (m_pDefenceWall)
		m_pDefenceWall->ClearProtectedFiles();
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::AddProtectedFile( const char *sFilename )
{
	if (m_pDefenceWall)
		m_pDefenceWall->AddProtectedFile( sFilename );
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::AddClientToDefenceWall( CIPAddress &clientIP )
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	//we want PB to work on devmode servers so handle this first
	if (m_pPunkBuster)
		m_pPunkBuster->OnAddClient(clientIP);
#endif
	// if in dev mode ignore it.
	if (!m_pSystem->GetForceNonDevMode())
		return;

	if (m_pDefenceWall && GetCheatProtectionLevel() > 0)
		m_pDefenceWall->OnAddClient( clientIP );
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::RemoveClientFromDefenceWall( CIPAddress &clientIP )
{
	if (m_pDefenceWall)
		m_pDefenceWall->OnDisconnectClient( clientIP );
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		m_pPunkBuster->OnDisconnectClient(clientIP);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::OnSecurityMsgResponse( CIPAddress &ipAddress,CStream &stm )
{
	if (m_pDefenceWall)
		m_pDefenceWall->OnClientResponse(ipAddress,stm);
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::OnSecurityMsgQuery( CStream &stm )
{
	if (m_pDefenceWall)
		m_pDefenceWall->OnServerRequest(stm);
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::OnCCPPunkBusterMsg( CIPAddress &ipAddress,CStream &stm )
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		m_pPunkBuster->OnCCPPunkBusterMsg( ipAddress,stm );
#endif
}

//////////////////////////////////////////////////////////////////////////
int CNetwork::GetLogLevel()
{
	return m_pNetLog->GetIVal();
}

//////////////////////////////////////////////////////////////////////////
int CNetwork::GetCheatProtectionLevel()
{
	//return m_pNetCheatProtection->GetIVal();
	return 0; // disable the cheat protection..
}

//////////////////////////////////////////////////////////////////////////
bool CNetwork::CheckPBPacket(CStream &stmPacket,CIPAddress &ip)
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		return m_pPunkBuster->CheckPBPacket(stmPacket,ip);
#endif
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::ValidateClient( CServerSlot *pSlot )
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
		return m_pPunkBuster->ValidateClient( pSlot );
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::InitPunkbusterClient(CClient *pClient)
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
	{
		m_pPunkBuster->Init(true, false);
		m_pPunkBuster->SetClient(pClient);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::InitPunkbusterClientLocal(CClientLocal *pClientLocal)
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
	{
		m_pPunkBuster->Init(true, true);
		m_pPunkBuster->SetClientLocal(pClientLocal);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::InitPunkbusterServer(bool bLocal, CServer *pServer)
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
	{
		m_pPunkBuster->Init(false, bLocal);
		m_pPunkBuster->SetServer(pServer);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::LockPunkbusterCVars()
{
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	if (m_pPunkBuster)
	{
		m_pPunkBuster->LockCVars();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CNetwork::LogNetworkInfo()
{
	DWORD i; 
	char	buf[256];
	struct	hostent *hp;

	if(!gethostname(buf, sizeof(buf))) 
	{
		hp = gethostbyname(buf);
		if (hp) 
		{			
			CryLogAlways("network hostname: %s",hp->h_name);
			
			i = 0;

			while(hp->h_aliases[i]) 
			{
				CryLogAlways("  alias: %s\n", hp->h_aliases[i]);
				i++;
			}

			i = 0;

			while(hp->h_addr_list[i])
			{
				sockaddr_in temp;

				memcpy(&(temp.sin_addr), hp->h_addr_list[i], hp->h_length);

#if defined(LINUX)
				const in_addr_windows *pin_addr_win = reinterpret_cast<const in_addr_windows*>(&temp.sin_addr);
				CryLogAlways("  ip:%d.%d.%d.%d",		//  port:%d  family:%x",	
					(int)(pin_addr_win->S_un.S_un_b.s_b1),
					(int)(pin_addr_win->S_un.S_un_b.s_b2),
					(int)(pin_addr_win->S_un.S_un_b.s_b3),
					(int)(pin_addr_win->S_un.S_un_b.s_b4));
#else
				CryLogAlways("  ip:%d.%d.%d.%d",		//  port:%d  family:%x",	
					(int)(temp.sin_addr.S_un.S_un_b.s_b1),
					(int)(temp.sin_addr.S_un.S_un_b.s_b2),
					(int)(temp.sin_addr.S_un.S_un_b.s_b3),
					(int)(temp.sin_addr.S_un.S_un_b.s_b4));
			//		(int)temp.sin_port,(unsigned int)temp.sin_family);
#endif
				i++;
			}		
		}
	}
}


//////////////////////////////////////////////////////////////////////////
IClient *CNetwork::GetClient()
{
	return m_pClient;
}


//////////////////////////////////////////////////////////////////////////
void CNetwork::OnAfterServerLoadLevel( const char *szServerName, const uint32 dwPlayerCount, const WORD wPort )
{
	assert(szServerName);
	assert(dwPlayerCount>0);

//	m_pSystem->GetILog()->Log("Ubi.com DEBUG OnAfterServerLoadLevel() 1");

	IServer *pServer=GetServerByPort(wPort);

	if(!pServer)
	{
		assert(pServer);		// internal error
		return;
	}

#ifndef NOT_USE_UBICOM_SDK
	if(m_pSystem->GetIGame()->GetModuleState(EGameMultiplayer) && pServer->GetServerType()!=eMPST_LAN)
	{
		// if it's a ubi-server, publish the server
		if(pServer->GetServerType()==eMPST_UBI)
			m_pUbiSoftClient->Server_CreateServer(szServerName,dwPlayerCount);

		// if it's a internet server, check CDKey
//		m_pSystem->GetILog()->Log("Ubi.com DEBUG OnAfterServerLoadLevel() 2");
		m_pUbiSoftClient->Server_CheckCDKeys(1);
	}
	else
	{
//		m_pSystem->GetILog()->Log("Ubi.com DEBUG OnAfterServerLoadLevel() 3");
		m_pUbiSoftClient->Server_DestroyServer();
		m_pUbiSoftClient->Server_CheckCDKeys(0);
	}
#endif // NOT_USE_UBICOM_SDK
}

bool CNetwork::VerifyMultiplayerOverInternet()
{
#ifndef NOT_USE_UBICOM_SDK
	if(!m_pUbiSoftClient->Client_IsConnected())
	{
		// if this is a ubi.com server, and we don't have a ubi.com connection
		// disconnect, login, and retry
		GetClient()->Disconnect("@UserDisconnected");

		HSCRIPTFUNCTION pfnOnConnectNeedUbi = m_pScriptSystem->GetFunctionPtr("Game", "OnConnectNeedUbi");

		if (pfnOnConnectNeedUbi)
		{
			_SmartScriptObject pGameScriptObject(m_pScriptSystem,true);
			m_pScriptSystem->GetGlobalValue("Game",*pGameScriptObject);

			m_pScriptSystem->BeginCall(pfnOnConnectNeedUbi);
			m_pScriptSystem->PushFuncParam(pGameScriptObject);
			m_pScriptSystem->EndCall();

			m_pScriptSystem->ReleaseFunc(pfnOnConnectNeedUbi);
		}

		return false;
	}
#endif // NOT_USE_UBICOM_SDK

	return true;
}

void CNetwork::Client_ReJoinGameServer()
{
#ifndef NOT_USE_UBICOM_SDK
	// We have to tell Ubisoft that the client has successfully connected
	// If ubisoft is not running this won't do anything.
	m_pUbiSoftClient->Client_ReJoinGameServer();
#endif NOT_USE_UBICOM_SDK
}
