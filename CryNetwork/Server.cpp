//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: Server.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Network.h"
#include "CNP.h"
#include "Server.h"
#include "ServerSlot.h"
#include "ILog.h"
#include "IConsole.h"
#include "NewUbisoftClient.h"								// NewUbisoftClient
#include <IScriptSystem.h>

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif


#if !defined(WIN64) && !defined(LINUX64) && !defined(NOT_USE_ASE_SDK)

#pragma comment(lib, "ASEQuerySDK.lib")

static CServer *g_pServer = 0;

extern "C"{
#include "ASEQuerySDK.h"

//------------------------------------------------------------------------------------------------- 
void ASEQuery_wantstatus()
{
	if (!g_pServer || !g_pServer->GetServerSlotFactory())
	{
		ASEQuery_status("", "", "", "", 1, 0, 0);

		return;
	}

	string szName, szGameType, szMap, szVersion;
	bool bPassword = false;
	int nPlayers = 0;
	int nMaxPlayers = 0;
	g_pServer->GetServerSlotFactory()->GetServerInfoStatus(szName, szGameType, szMap, szVersion, &bPassword, &nPlayers, &nMaxPlayers);

	ASEQuery_status(szName.c_str(), szGameType.c_str(), szMap.c_str(), szVersion.c_str(), bPassword, nPlayers, nMaxPlayers);
}

//------------------------------------------------------------------------------------------------- 
void ASEQuery_wantrules()
{
	IScriptSystem *pSS = GetISystem()->GetIScriptSystem();

	_SmartScriptObject QueryHandler(pSS, 1);

	if (!pSS->GetGlobalValue("QueryHandler", (IScriptObject *)QueryHandler))
	{
		return;
	}

	_SmartScriptObject ServerRules(pSS, 1);
	pSS->BeginCall("QueryHandler", "GetServerRules");
	pSS->PushFuncParam((IScriptObject *)QueryHandler);
	pSS->EndCall((IScriptObject *)ServerRules);

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
				ASEQuery_addrule(szRuleName, szRuleValue);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------- 
void ASEQuery_wantplayers()
{
	IScriptSystem *pSS = GetISystem()->GetIScriptSystem();

	_SmartScriptObject QueryHandler(pSS, 1);

	if (!pSS->GetGlobalValue("QueryHandler", (IScriptObject *)QueryHandler))
	{
		return;
	}

	_SmartScriptObject PlayerStats(pSS, 1);
	pSS->BeginCall("QueryHandler", "GetPlayerStats");
	pSS->PushFuncParam((IScriptObject *)QueryHandler);
	pSS->EndCall((IScriptObject *)PlayerStats);

	for (int i = 1; i <= PlayerStats->Count(); i++)
	{
		_SmartScriptObject Player(pSS, 1);

		if (PlayerStats->GetAt(i, (IScriptObject *)Player))
		{
			char *szName = 0;
			char *szTeam = 0;
			char *szSkin = 0;
			char *szScore = 0;
			char *szPing = 0;
			char *szTime = 0;

			Player->GetValue("Name", (const char* &)szName);
			Player->GetValue("Team", (const char* &)szTeam);
			Player->GetValue("Skin", (const char* &)szSkin);
			Player->GetValue("Score", (const char* &)szScore);
			Player->GetValue("Ping", (const char* &)szPing);
			Player->GetValue("Time", (const char* &)szTime);

			ASEQuery_addplayer(szName, szTeam, szSkin, szScore, szPing, szTime);
		}
	}
}
} // extern "C"
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServer::CServer(CNetwork *pNetwork)
{
	m_cLastClientID = 0;
	m_pFactory = NULL;
	m_ServerVariables.nDataStreamTimeout = 30000;// 30 seconds
	m_pNetwork=pNetwork;
	m_wPort=0;
	m_bMulticastSocket=true;
	m_pSecuritySink=0;
	m_MPServerType=eMPST_LAN;
}

CServer::~CServer()
{
	//------------------------------------------------------------------------------------------------- 
	// ASE Deinitialization
	//------------------------------------------------------------------------------------------------- 
	//------------------------------------------------------------------------------------------------- 
#if !defined(WIN64) && !defined(LINUX64) && !defined(NOT_USE_ASE_SDK)
	ASEQuery_shutdown();
#endif
	//------------------------------------------------------------------------------------------------- 

#ifndef NOT_USE_UBICOM_SDK
	// If it is a UBI type server we should unregister
	m_pNetwork->m_pUbiSoftClient->Server_DestroyServer();
#endif // NOT_USE_UBICOM_SDK

	m_pNetwork->UnregisterServer(m_wPort);
}

EMPServerType CServer::GetServerType() const
{
	return m_MPServerType;
}


//////////////////////////////////////////////////////////////////////
// IServer
//////////////////////////////////////////////////////////////////////

bool CServer::Init(IServerSlotFactory *pFactory, WORD wPort, bool listen)
{
	CIPAddress ipMulticast(SERVER_MULTICAST_PORT, SERVER_MULTICAST_ADDRESS);
	
	m_pFactory = pFactory;
	
	if(m_bListen = listen)
	{
		CIPAddress ipLocal;
		
		ipLocal.m_Address.ADDR = m_pNetwork->GetLocalIP();

		// only create the multicast socket if it's not internet server
		ICVar *sv_ServerType = GetISystem()->GetIConsole()->GetCVar("sv_ServerType");			assert(sv_ServerType);

		m_MPServerType=eMPST_LAN;
		
		if(stricmp(sv_ServerType->GetString(),"UBI")==0)
			m_MPServerType=eMPST_UBI;
		else if(stricmp(sv_ServerType->GetString(),"NET")==0)
			m_MPServerType=eMPST_NET;

		// if this is a lan server
		//if (m_MPServerType==eMPST_LAN)
		{
			if (NET_SUCCEDED(m_socketMulticast.Create()))
			{
				if (NET_SUCCEDED(m_socketMulticast.Listen(SERVER_MULTICAST_PORT, &ipMulticast, &ipLocal)))
				{
					m_bMulticastSocket=true;
				}
			}
		}
/*		else
		{
			m_bMulticastSocket=false;
		}
		*/

		if (NET_FAILED(m_socketMain.Create()))
			return false;

		if (NET_FAILED(m_socketMain.Listen(wPort, 0, &ipLocal)))
			return false;

		//------------------------------------------------------------------------------------------------- 
		// ASE Initialization
		//------------------------------------------------------------------------------------------------- 
		//------------------------------------------------------------------------------------------------- 
		DWORD	dwLocalIP = GetISystem()->GetINetwork()->GetLocalIP();
		char	*szIP = 0;
		CIPAddress ip;

		if (dwLocalIP)
		{
			ip.m_Address.ADDR = dwLocalIP;
			szIP = ip.GetAsString();
		}

#if !defined(WIN64) && !defined(LINUX64) && !defined(NOT_USE_ASE_SDK)
		ASEQuery_initialize((int)wPort, m_MPServerType!=eMPST_LAN ? 1 : 0, szIP);
#endif
		//------------------------------------------------------------------------------------------------- 
	};

	m_wPort=wPort;

	return true;
}


#ifdef _INTERNET_SIMULATOR
#include <stdlib.h>
#if !defined(LINUX)
#include <assert.h>
#endif

#include "ITimer.h"
#endif


void CServer::Update(unsigned int nTime)
{
	//------------------------------------------------------------------------------------------------- 
	// ASE Update
	//------------------------------------------------------------------------------------------------- 
	//------------------------------------------------------------------------------------------------- 
#if !defined(WIN64) && !defined(LINUX64) && !defined(NOT_USE_ASE_SDK)
	g_pServer = this;
	ASEQuery_check();
#endif
	//------------------------------------------------------------------------------------------------- 

#ifdef _INTERNET_SIMULATOR
		static ICVar *pVarPacketloss=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_packetloss");

		TDelayPacketList2::iterator i;

		for (i = m_delayedPacketList.begin(); i != m_delayedPacketList.end();)
		{
			DelayedPacket2 *dp = (*i);
			if (dp->m_fTimeToSend <= GetISystem()->GetITimer()->GetCurrTime())
			{
				i = m_delayedPacketList.erase(i);
				// Send it
				if ((rand() %100) > pVarPacketloss->GetFVal())
					m_socketMain.Send(dp->data,dp->len,dp->address);
				// Delete it
				delete dp->address;
				delete dp;
			}
			else
				++i;
		}
#endif

	m_nCurrentTime = nTime;
	int nRecvBytes;
	//	do{
	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
		static CIPAddress ipFrom;
		static CStream buf;
		
		/////////////////////////////////////////////////////////
		if(m_bListen)
		do
		{
			buf.Reset();
			nRecvBytes = 0;
			m_socketMain.Receive(buf.GetPtr(),
				(int)BITS2BYTES(buf.GetAllocatedSize()),
				nRecvBytes,
				ipFrom);
						
			///////////////////////////////////////////////////////
			if (nRecvBytes>0)
			{
				buf.SetSize(BYTES2BITS(nRecvBytes));
				ProcessPacket(buf, ipFrom);
			}
		}while (nRecvBytes>0);

		/////////////////////////////////////////////////////////
		// handle multicast packets
		/////////////////////////////////////////////////////////
		if(m_bMulticastSocket && m_bListen && m_MPServerType==eMPST_LAN)
		{
			do
			{
				buf.Reset();
				nRecvBytes = 0;
				m_socketMulticast.Receive(buf.GetPtr(),
					(int)BITS2BYTES(buf.GetAllocatedSize()),
					nRecvBytes,
					ipFrom);
				
				///////////////////////////////////////////////////////
				if (nRecvBytes>0)
				{
					buf.SetSize(BYTES2BITS(nRecvBytes));
					ProcessMulticastPacket(buf, ipFrom);
				}
			}while (nRecvBytes>0);
		}
		/////////////////////////////////////////////////////////
		// Update slots State machine
		SLOTS_MAPItr itr = m_mapSlots.begin();
		
		while (itr != m_mapSlots.end())
		{
			CServerSlot *pSlot = itr->second;
			if (pSlot->IsActive())
			{
				pSlot->Update(m_nCurrentTime, NULL, NULL);
			}
			++itr;
		}

	m_pNetwork->OnServerUpdate();
}

void CServer::GetBandwidth( float &fIncomingKbPerSec, float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets )
{
	fIncomingKbPerSec = m_socketMain.m_fIncomingKbPerSec;
	fOutgoinKbPerSec = m_socketMain.m_fOutgoingKbPerSec;
	nIncomingPackets=m_socketMain.m_nIncomingPacketsPerSec;
	nOutgoingPackets=m_socketMain.m_nOutgoingPacketsPerSec;
}

void CServer::SetVariable(enum CryNetworkVarible eVarName, unsigned int nValue)
{
	switch (eVarName)
	{
		case cnvDataStreamTimeout:
			m_ServerVariables.nDataStreamTimeout = nValue;
			break;
		default:
			NET_ASSERT(0);
			break;
	}
}

void CServer::GetProtocolVariables(CNPServerVariables &sv)
{
	sv = m_ServerVariables;
}

void CServer::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////
// _IServerServices
//////////////////////////////////////////////////////////////////////
bool CServer::Send(CStream &stm, CIPAddress &ip)
{
#ifndef _INTERNET_SIMULATOR
	DWORD nSize = BITS2BYTES(stm.GetSize());

	if(NET_SUCCEDED(m_socketMain.Send(stm.GetPtr(), nSize, &ip)))
		return true;

	return false;
#else

	static ICVar *pVarPacketloss=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_packetloss");
	static ICVar *pVarMinPing=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_minping");
	static ICVar *pVarMaxPing=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_maxping");

	int iMaxPing=pVarMaxPing->GetIVal();
	int iMinPing=pVarMinPing->GetIVal();

	if(iMinPing>iMaxPing)
		iMaxPing=iMinPing;

	if (pVarPacketloss->GetFVal()>0 || iMaxPing>0)
	{
		DelayedPacket2 *delayed = new DelayedPacket2;

		int iRand=0;

		if(iMaxPing>iMinPing)
			iRand=rand() % (iMaxPing-iMinPing);

		if(iMaxPing>0)
			delayed->m_fTimeToSend = GetISystem()->GetITimer()->GetCurrTime() + (iMinPing + iRand) / 1000.0f;
		else
			delayed->m_fTimeToSend = GetISystem()->GetITimer()->GetCurrTime();
		delayed->len = BITS2BYTES(stm.GetSize());
		delayed->address = new CIPAddress(ip);
		assert(delayed->len < sizeof(delayed->data)/sizeof(delayed->data[0]));
		memcpy(delayed->data, stm.GetPtr(), delayed->len);
		m_delayedPacketList.push_back(delayed);
		return true;
	}
	else
	{
		DWORD nSize = BITS2BYTES(stm.GetSize());

		if(NET_SUCCEDED(m_socketMain.Send(stm.GetPtr(), nSize, &ip)))
			return true;

		return false;
	}
#endif

}

void CServer::OnDestructSlot( const CServerSlot *inpServerSlot )
{
	SLOTS_MAPItr itr = m_mapSlots.begin();
		
	while (itr != m_mapSlots.end())
	{
		CServerSlot *pSlot = itr->second;
		if(pSlot==inpServerSlot)
		{
			m_mapSlots.erase(itr);
			return;
		}
		++itr;
	}

	assert(0);			// can't be
}

void CServer::UnregisterSlot(CIPAddress &ip)
{
	SLOTS_MAPItr itor=m_mapSlots.find(ip);
	if(itor!=m_mapSlots.end())
	{
		IServerSlot *pServerSlot=itor->second;

		pServerSlot->Advise(NULL);		// remove connection to IServerSlotSink (CXServerSlot)
	}
}

void CServer::RegisterLocalServerSlot(CServerSlot *pSlot,CIPAddress &ip)
{
	m_mapSlots.insert(SLOTS_MAPItr::value_type(ip,pSlot));
	if(m_pFactory)
		m_pFactory->CreateServerSlot(pSlot);
}

CServerSlot *CServer::GetPacketOwner(CIPAddress &ip)
{
	SLOTS_MAPItr itor;
	itor=m_mapSlots.find(ip);
	if(itor==m_mapSlots.end())
		return 0;
	return itor->second;
}

//////////////////////////////////////////////////////////////////////
// core
//////////////////////////////////////////////////////////////////////
void TraceUnrecognizedPacket( const char *inszTxt, CStream &stmPacket, CIPAddress &ip)
{
#ifdef _DEBUG
	OutputDebugString("\n");
	OutputDebugString(inszTxt);
	OutputDebugString("\n");

	static char sTemp[1024];
	static BYTE cBuf[1024];
	DWORD nCount;
	::OutputDebugString("-------------------------------\n");
	sprintf(sTemp,"INVALID PACKET FROM [%s]\n",ip.GetAsString(true));
	::OutputDebugString(sTemp);
	stmPacket.GetBuffer(cBuf,1024);
	nCount=BYTES2BITS(stmPacket.GetSize());
	for(DWORD n=0;n<nCount;n++)
	{
		sprintf(sTemp,"%02X ",cBuf[n]);
		::OutputDebugString(sTemp);
		if(n && (n%16)==0)
			::OutputDebugString("\n");
	}
#endif
}

void CServer::ProcessPacket(CStream &stmPacket, CIPAddress &ip)
{
	if (!m_pNetwork->CheckPBPacket( stmPacket,ip ))
		return;

	CNP cnp;
	cnp.LoadAndSeekToZero(stmPacket);
	switch (cnp.m_cFrameType)
	{
		// these packets will not be checked for ban
		// since they are sent just too often,
		// and would slow down the server
		// they will be reject anyway, because no banned ip can have a server slot associated with it
	case FT_CCP_DISCONNECT:
	case FT_CCP_ACK:
	case FT_CTP_DATA:
	case FT_CTP_ACK:
	case FT_CTP_NAK:
	case FT_CTP_PONG:
		DispatchToServerSlots(cnp, stmPacket,ip);
		break;

	default:
		{
			if (m_pSecuritySink->IsIPBanned(ip.GetAsUINT()))
			{
				return;
			}

			// these packets' ip will be checked for ban
			// since they are not sent very often
			switch(cnp.m_cFrameType)
			{
			case FT_CCP_CONNECT:
			case FT_CCP_CONNECT_RESP:
			case FT_CCP_CONTEXT_READY:
			case FT_CCP_SECURITY_QUERY:
			case FT_CCP_SECURITY_RESP:
			case FT_CCP_PUNK_BUSTER_MSG:
				DispatchToServerSlots(cnp, stmPacket,ip);
				break;
			case FT_CCP_SETUP:
				ProcessSetup(cnp, stmPacket, ip);
				break;
			case FT_CQP_INFO_REQUEST:
				ProcessInfoRequest(stmPacket, ip);
				break;
			case FT_CQP_XML_REQUEST:
				ProcessInfoXMLRequest(stmPacket,ip);
				break;
			default:
				{
					TPacketSinks::iterator it = m_PacketSinks.find(cnp.m_cFrameType);

					if(it!=m_PacketSinks.end())
					{
						INetworkPacketSink *pSink = (*it).second;

						pSink->OnReceivingPacket(cnp.m_cFrameType,stmPacket,ip);
						break;
					}
				}
				TraceUnrecognizedPacket("ProcessPacket",stmPacket,ip);
				break;
			}
		}
	};
}

void CServer::RegisterPacketSink( const unsigned char inID, INetworkPacketSink *inpSink )
{
	assert(m_PacketSinks.count(inID)==0);

	m_PacketSinks[inID] = inpSink;
}

void CServer::SetSecuritySink(IServerSecuritySink *pSecuritySink)
{
	m_pSecuritySink = pSecuritySink;
}

//////////////////////////////////////////////////////////////////////////
IServerSecuritySink* CServer::GetSecuritySink()
{
	return m_pSecuritySink;
}

bool CServer::IsIPBanned(const unsigned int dwIP)
{
	if (m_pSecuritySink)
	{
		return m_pSecuritySink->IsIPBanned(dwIP);
	}

	return false;
}

void CServer::BanIP(const unsigned int dwIP)
{
	if (m_pSecuritySink)
	{
		m_pSecuritySink->BanIP(dwIP);
	}
}

void CServer::UnbanIP(const unsigned int dwIP)
{
	if (m_pSecuritySink)
	{
		m_pSecuritySink->UnbanIP(dwIP);
	}
}

void CServer::ProcessMulticastPacket(CStream &stmPacket, CIPAddress &ip)
{
	CNP cnp;
	cnp.LoadAndSeekToZero(stmPacket);
	switch (cnp.m_cFrameType)
	{
		case FT_CQP_INFO_REQUEST:
			ProcessInfoRequest(stmPacket, ip);
			break;
		case FT_CQP_XML_REQUEST:
			ProcessInfoXMLRequest(stmPacket,ip);
			break;
	default:
		TraceUnrecognizedPacket("ProcessMulticastPacket",stmPacket,ip);
		break;
	};
}

void CServer::ProcessSetup(CNP &cnp, CStream &stmStream, CIPAddress &ip)
{
	CServerSlot *pSSlot;
	CServerSlot *pTemp=GetPacketOwner(ip);

	if(pTemp!=NULL)
	{
		NET_TRACE("Setup discarded for IP [%s]\n",pTemp->GetIP().GetAsString(true));
		return;
	}

	if (m_pSecuritySink)
	{
		if (m_pSecuritySink->IsIPBanned(ip.GetAsUINT()))
		{
			NET_TRACE("Setup discarded for IP [%s] (BANNED)\n",ip.GetAsString(true));

			return;
		}
	}

	NET_TRACE("Setup accepted for IP [%s]\n",ip.GetAsString(true));

	pSSlot = new CServerSlotImpl(m_pNetwork,this);

	/////////////////////////////////////////////////////////!!!!
	int nID = GenerateNewClientID(); 
	/////////////////////////////////////////////////////////!!!!
	pSSlot->Start((BYTE)nID, ip);

	// build event
	if (m_pFactory)
	{
		if (m_pFactory->CreateServerSlot(pSSlot) == true)
		{
			m_mapSlots.insert(SLOTS_MAPItr::value_type(ip, pSSlot));
			//m_mapIPs.insert(IPS_MAPItr::value_type(ip,nID));

			// if this is a lan server
			if(m_MPServerType==eMPST_LAN)
			{
				if (!IsLANIP(ip))
				{
					pSSlot->Disconnect("@LanIPOnly");
				}
			}

			// get server password cvar
			ICVar *sv_password = GetISystem()->GetIConsole()->GetCVar("sv_password");
			assert(sv_password);

			// check if server is password protected
			if (sv_password->GetString() && (strlen(sv_password->GetString()) > 0))
			{
				CCPSetup pccp;

				pccp.Load(stmStream);

				if (!pccp.m_sPlayerPassword.size() || (strcmp(sv_password->GetString(), pccp.m_sPlayerPassword.c_str()) != 0))
				{
					pSSlot->Disconnect("@InvalidServerPassword");

					return;
        }
			}
			pSSlot->Update(m_nCurrentTime, &cnp, &stmStream);
		}
		else
		{
			//<<FIXME>> ?!?!?!?
			//pSSlot->Update(m_nCurrentTime, &cnp, &stmStream);
			pSSlot->Disconnect("@ConnectionRejected");
			//pSSlot->Update(m_nCurrentTime, NULL, NULL);
		}
	}
}


bool CServer::IsLANIP(const CIPAddress &ip)
{
	unsigned char ipb[4];
#if defined(LINUX)
	ipb[0] = ip.m_Address.sin_addr_win.S_un.S_un_b.s_b1;
	ipb[1] = ip.m_Address.sin_addr_win.S_un.S_un_b.s_b2;
	ipb[2] = ip.m_Address.sin_addr_win.S_un.S_un_b.s_b3;
	ipb[3] = ip.m_Address.sin_addr_win.S_un.S_un_b.s_b4;
#else
	ipb[0] = ip.m_Address.sin_addr.S_un.S_un_b.s_b1;
	ipb[1] = ip.m_Address.sin_addr.S_un.S_un_b.s_b2;
	ipb[2] = ip.m_Address.sin_addr.S_un.S_un_b.s_b3;
	ipb[3] = ip.m_Address.sin_addr.S_un.S_un_b.s_b4;
#endif

	if (ipb[0] == 127)
		return true;
	if (ipb[0] == 10)
		return true;
	if ((ipb[0] == 192) && (ipb[1] == 168))
		return true;
	if (ipb[0] == 172)
		if ((ipb[1] >= 16) && (ipb[1] <= 31))
			return true;

	return false;
}

void CServer::DispatchToServerSlots(CNP &cnp, CStream &stm, CIPAddress &ip)
{
	SLOTS_MAPItr itr;
	itr = m_mapSlots.find(ip);
	if (itr != m_mapSlots.end())
	{
		(itr->second)->Update(m_nCurrentTime, &cnp, &stm); // update the server slot
	}
	else
	{
		NET_TRACE("CServer::DispatchToServerSlots() Unknown client ID\n");
	}
}

void CServer::ProcessInfoRequest(CStream &stmIn, CIPAddress &ip)
{
	if (stmIn.GetSize() < 32)
	{
		return; // must be at least 16bytes(64bits) int
	}

	CQPInfoRequest	Query;
	CQPInfoResponse Response;
	CStream					stmPacket;

	Query.Load(stmIn);	// load and seek to zero
	stmIn.Seek(0);

	if (!Query.IsOk())
	{
		return;
	}

	assert((stmIn.GetSize() % 8) == 0);
	assert(m_pFactory);

	if (Query.szRequest == "ping")
	{
		Response.szResponse = "X";// identify the packet
		Response.Save(stmPacket);

		assert((stmPacket.GetSize() % 8) == 0);

		if (stmPacket.GetSize())
		{
			m_socketMain.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), &ip);
		}
	}
	// status
	else if (Query.szRequest == "status")
	{
#if defined(WIN64) && !defined(NDEBUG)
		//workarround for bug caused by string reallocation across dll's and not shared crt libs in debug mode
		Response.szResponse.resize(0);
		Response.szResponse.reserve(100);
		Response.szResponse += "S";	// identify the packet
#else
		Response.szResponse = "S";	// identify the packet
#endif
		if (!m_pFactory->GetServerInfoStatus(Response.szResponse))
		{
			return;
		} 

		Response.Save(stmPacket);

		assert((stmPacket.GetSize() % 8) == 0);

		if (stmPacket.GetSize())
		{
			m_socketMain.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), &ip);
		}
	}
	// rules
	else if (Query.szRequest == "rules")
	{
		Response.szResponse = "R";		// identify the packet

		if (!m_pFactory->GetServerInfoRules(Response.szResponse))
		{
			return;
		}

		Response.Save(stmPacket);

		assert((stmPacket.GetSize() % 8) == 0);
		m_socketMain.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), &ip);
	}
	// players
	else if (Query.szRequest == "players")
	{
		string szString[SERVER_QUERY_MAX_PACKETS];
		string *vszString[SERVER_QUERY_MAX_PACKETS];
		int	nStrings = 0;

		for (int i = 0; i < SERVER_QUERY_MAX_PACKETS; i++)
		{
			vszString[i] = &szString[i];
			szString[i] = "P";
		}

		if (!m_pFactory->GetServerInfoPlayers(vszString, nStrings))
		{
			return;
		}

		for (int j = 0; j < nStrings; j++)
		{
			Response.szResponse = *vszString[j];
			Response.Save(stmPacket);

			assert((stmPacket.GetSize() % 8) == 0);
			m_socketMain.Send(stmPacket.GetPtr(), BITS2BYTES(stmPacket.GetSize()), &ip);

			stmPacket.Reset();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CServer::ProcessInfoXMLRequest(CStream &stmIn,CIPAddress &ip)
{
	CQPXMLRequest cqpRequest;
	CQPXMLResponse cqpResponse;

	cqpRequest.Load(stmIn);

	char sResponse[MAX_REQUEST_XML_LENGTH+1];
	strcpy(sResponse,"");

	// ask the application to process this XML request.
	if (m_pFactory)
	{
		if(!m_pFactory->ProcessXMLInfoRequest( cqpRequest.m_sXML.c_str(),sResponse,sizeof(sResponse)))
			return;
	}

	CStream stmOut;
	cqpResponse.m_sXML = sResponse;
	cqpResponse.Save(stmOut);

	m_socketMain.Send(stmOut.GetPtr(), BITS2BYTES(stmOut.GetSize()), &ip);
}

//////////////////////////////////////////////////////////////////////////
IServerSlot *CServer::GetServerSlotbyID( const unsigned char ucId ) const
{
	for(SLOTS_MAP::const_iterator it=m_mapSlots.begin();it!=m_mapSlots.end();++it)
	{
		if(it->second->GetID()==ucId)
			return it->second;
	}

	return 0;				// not found
}


//////////////////////////////////////////////////////////////////////////
unsigned char CServer::GenerateNewClientID()
{
	for(unsigned char id=1;id<0xFF;id++)				// find a free client id
	{
		bool bFree=true;

		for(SLOTS_MAPItr it=m_mapSlots.begin();it!=m_mapSlots.end();++it)
		{
			if(it->second->GetID()==id)
			{
				bFree=false;									// this one is occupied
				break;
			}
		}

		if(bFree)
			return id;			// found one
	}

	assert(0);				// 256 players are already connected ?
	return 0;
}


uint8 CServer::GetMaxClientID() const
{
	uint8 ucMax=0;

	// save solution (slow, but speed is not a concern here)
	for(SLOTS_MAP::const_iterator it=m_mapSlots.begin();it!=m_mapSlots.end();++it)
	{
		uint8 ucId=it->second->GetID();

		if(ucId>ucMax)
			ucMax=ucId;
	}

	return ucMax;
}


const char *CServer::GetHostName()
{
	return m_socketMain.GetHostName();
}

void CServer::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(this,sizeof(CServer));
	SLOTS_MAPItr itor=m_mapSlots.begin();
	while(itor!=m_mapSlots.end())
	{
		itor->second->GetMemoryStatistics(pSizer);
	}
}