//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ServerSlot.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNP.h"
#include "ServerSlot.h"
#include "ClientLocal.h"
#include "Server.h"
#include "IGame.h"
#include "IDataProbe.h"

#ifndef NOT_USE_UBICOM_SDK
	#include "NewUbisoftClient.h"								// NewUbisoftClient
#endif // NOT_USE_UBICOM_SDK

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerSlotImpl::CServerSlotImpl( CNetwork *pNetwork,_IServerServices *pParent)
	:CServerSlot(pNetwork),m_ctpEndpoint(pNetwork),m_ctpEndpoint2(pNetwork)
{
	m_ccpEndpoint.Init(this);
	m_smCCPMachine.Init(this);
	m_pParent = pParent;
	m_bActive = false;
	m_dwKeepAliveTimer = 0xFFFFFFFF;
	m_nCurrentTime = 0; //m_nCurrentTime;
	pParent->GetProtocolVariables(m_ServerVariables);

	m_bClientAdded = false;

	m_nClientFlags = 0;
	m_nClientOS_Minor = 0;
	m_nClientOS_Major = 0;

	//////////////////////////////////////////////////////////////////////////
	// Generate random public key for this client.
	GetISystem()->GetIDataProbe()->RandSeed(GetTickCount());
	m_nPublicKey = GetISystem()->GetIDataProbe()->GetRand();
	m_ccpEndpoint.SetPublicCryptKey( m_nPublicKey );
	m_ctpEndpoint.SetPublicCryptKey( m_nPublicKey );
	m_ctpEndpoint2.SetPublicCryptKey( m_nPublicKey );
	//////////////////////////////////////////////////////////////////////////
}

CServerSlotImpl::~CServerSlotImpl()
{
	m_pParent->OnDestructSlot(this);
}

bool CServerSlotImpl::IsActive()
{
	return m_bActive;
}

void CServerSlotImpl::Disconnect(const char *szCause)
{
	m_smCCPMachine.Update(SIG_SRV_ACTIVE_DISCONNECT, (DWORD_PTR)szCause);
	SendDisconnect(szCause);
}

bool CServerSlotImpl::ContextSetup(CStream &stm)
{
	m_stmContext = stm;
	m_smCCPMachine.Update(SIG_SRV_CONTEXT_SETUP);
	return true;
}

unsigned int CServerSlotImpl::GetUnreliablePacketsLostCount()
{
	return m_ctpEndpoint.GetUnreliableLostPackets();
}

void CServerSlotImpl::SendReliable(CStream &stm,bool bSecondaryChannel)
{
	m_BandwidthStats.m_nReliablePacketCount++;
	m_BandwidthStats.m_nReliableBitCount+=stm.GetSize();

	if(bSecondaryChannel)
		m_ctpEndpoint2.SendReliable(stm);
	 else
		m_ctpEndpoint.SendReliable(stm);
}

void CServerSlotImpl::SendUnreliable(CStream &stm)
{
	m_BandwidthStats.m_nUnreliablePacketCount++;
	m_BandwidthStats.m_nUnreliableBitCount+=stm.GetSize();

	m_ctpEndpoint.SendUnreliable(stm);
}


void CServerSlotImpl::Update(unsigned int nTime, CNP *pCNP, CStream *pStream)
{
	m_nCurrentTime = nTime;
	
	if (pCNP)
	{
		// avoid useless packet parsing 
		if(m_smCCPMachine.GetCurrentStatus() != STATUS_SRV_DISCONNECTED)
		{
			ProcessPacket(pCNP->m_cFrameType,pCNP->m_bSecondaryTC, pStream);
			m_dwKeepAliveTimer = m_nCurrentTime;
		}
	}
	else
	{
		m_ccpEndpoint.Update(m_nCurrentTime, NULL, NULL);
		m_smCCPMachine.Update();

		if (/*m_smCCPMachine.GetCurrentStatus() == STATUS_CONNECTED ||*/ m_smCCPMachine.GetCurrentStatus() == STATUS_SRV_READY)
		{
			static char szCause[] = "@ClientTimeout";
			m_ctpEndpoint.Update(m_nCurrentTime, NULL, NULL);
			m_ctpEndpoint2.Update(m_nCurrentTime, NULL, NULL);
			// after n seconds without any incoming packets the connection will be considered lost
			if(m_ServerVariables.nDataStreamTimeout && GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))
				if (m_nCurrentTime - m_dwKeepAliveTimer>m_ServerVariables.nDataStreamTimeout)
					Disconnect(szCause);
		}
		else
		{
			m_dwKeepAliveTimer = m_nCurrentTime;
		}
	}
}

bool CServerSlotImpl::IsReady()
{
	return (m_smCCPMachine.GetCurrentStatus() == STATUS_SRV_READY)?true:false;
}


unsigned int CServerSlotImpl::GetPing()
{
	return m_ctpEndpoint.GetPing();
}
//////////////////////////////////////////////////////////////////////
//_IServerSlotServices
//////////////////////////////////////////////////////////////////////

void CServerSlotImpl::Start(unsigned char cClientID, CIPAddress &ip)
{
	m_ipAddress = ip;
	m_bActive = true;
	m_cClientID = cClientID;
	m_ctpEndpoint.Init(this,false);
	m_ctpEndpoint2.Init(this,true);
	m_ccpEndpoint.Init(this);
	//	m_smCCPMachine.Update(SIG_SETUP);
}

bool CServerSlotImpl::Send(CStream &stm)
{
	m_pParent->Send(stm, m_ipAddress);
	return true;
}

bool CServerSlotImpl::SendConnect()
{
	CNPServerVariables sv;
	m_pParent->GetProtocolVariables(sv);
	sv.nPublicKey = m_nPublicKey;
	m_ccpEndpoint.SendConnect(sv);
	return true;
}
 
bool CServerSlotImpl::SendContextSetup()
{
// 	GetISystem()->GetILog()->Log("CServerSlotImpl::SendContextSetup");

	m_ccpEndpoint.SendContextSetup(m_stmContext);
	// reset the data layer (CTP) 
	m_ctpEndpoint.Reset();
	m_ctpEndpoint2.Reset();
	return true;
}

bool CServerSlotImpl::SendServerReady()
{
//	GetISystem()->GetILog()->Log("CServerSlotImpl::SendServerReady");
	m_ccpEndpoint.SendServerReady();
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CServerSlotImpl::SendSecurityQuery(CStream &stm)
{
	m_ccpEndpoint.SendSecurityQuery(stm);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CServerSlotImpl::SendPunkBusterMsg(CStream &stm)
{
	m_ccpEndpoint.SendPunkBusterMsg(stm);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CServerSlotImpl::SendDisconnect(const char *szCause)
{
	m_ccpEndpoint.SendDisconnect(szCause);
	return true;
}

void CServerSlotImpl::OnConnect()
{
	NET_ASSERT(m_pSink);
	if(m_pSink)
	{
#ifndef NOT_USE_UBICOM_SDK
		// If its a multiplayer game check the cdkey
		if(GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))
			m_pNetwork->m_pUbiSoftClient->Server_CheckPlayerAuthorizationID(GetID(),m_pbAuthorizationID);
		else
#endif // NOT_USE_UBICOM_SDK
		{
			m_pSink->OnXPlayerAuthorization(true,"",NULL,0);
		}

		m_pSink->OnXServerSlotConnect(m_pbAuthorizationID,m_uiAuthorizationSize);
	}
}

void CServerSlotImpl::OnContextReady()
{
//	GetISystem()->GetILog()->Log("CServerSlotImpl::OnContextReady");
	if (m_pSink)
		m_pSink->OnContextReady(m_stmContextReady);
	SendServerReady();

	// Run Checks for this client.
	if (!m_bClientAdded)
		m_pNetwork->AddClientToDefenceWall( m_ipAddress );
	m_bClientAdded = true;
}

void CServerSlotImpl::OnData(CStream &stm)
{
	if (m_pSink)
		m_pSink->OnData(stm);
}

void CServerSlotImpl::OnDisconnect(const char *szCause)
{
	m_pNetwork->RemoveClientFromDefenceWall( m_ipAddress );

  if (m_pSink)
	{
		if(szCause==NULL)
			szCause="Undefined";

#ifndef NOT_USE_UBICOM_SDK
		m_pNetwork->m_pUbiSoftClient->Server_RemovePlayer(GetID());
#endif // NOT_USE_UBICOM_SDK

		m_pSink->OnXServerSlotDisconnect(szCause);
	}
	m_pParent->UnregisterSlot(GetIP());
	m_bActive = 0;
}


void CServerSlotImpl::Release()
{
	assert(this);
	delete this;
}

void CServerSlotImpl::OnCCPSetup(CStream &stm)
{
	CCPSetup ccpSetup;
	ccpSetup.Load(stm);
	bool versiomatch = !(ccpSetup.m_cProtocolVersion!=CNP_VERSION);

	m_nClientFlags = ccpSetup.m_nClientFlags;
	m_nClientOS_Minor = ccpSetup.m_nClientOSMinor;
	m_nClientOS_Major = ccpSetup.m_nClientOSMajor;

	// Can report here what OS client is running.

	// warning this is never called(look at CServerSlotImpl::Start() )
	m_smCCPMachine.Update(SIG_SRV_SETUP,versiomatch?1:0);

	// kick a player that tries to join the server, with cheats enabled
	if ((m_nClientFlags & CLIENT_FLAGS_DEVMODE) && !GetISystem()->WasInDevMode())
	{
		Disconnect("@Kicked");
	}
}

void CServerSlotImpl::OnCCPConnectResp(CStream &stm)
{
	unsigned int uiSize = stm.GetSize();

	if (uiSize)
	{
		assert(uiSize%8==0);
		m_pbAuthorizationID = new BYTE[uiSize/8];
		stm.ReadBits(m_pbAuthorizationID,uiSize);
		m_uiAuthorizationSize = uiSize/8;
	}
	m_smCCPMachine.Update(SIG_SRV_CONNECT_RESP);

	m_pNetwork->ValidateClient(this);
}

void CServerSlotImpl::OnCCPContextReady(CStream &stm)
{
	m_stmContextReady = stm;
	m_smCCPMachine.Update(SIG_SRV_CONTEXT_READY);
}

void CServerSlotImpl::OnCCPDisconnect(const char *szCause)
{
	m_smCCPMachine.Update(SIG_SRV_DISCONNECT, (DWORD_PTR)szCause);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CServerSlotImpl::ProcessPacket(unsigned char cFrameType,bool bSTC, CStream *pStream)
{
	switch (cFrameType)
	{
			// signaling
			case FT_CCP_SETUP:
			case FT_CCP_CONNECT_RESP:
			case FT_CCP_CONTEXT_READY:
			case FT_CCP_DISCONNECT:
			case FT_CCP_ACK:
			case FT_CCP_SECURITY_QUERY:
			case FT_CCP_SECURITY_RESP:
			case FT_CCP_PUNK_BUSTER_MSG:
				m_ccpEndpoint.Update(m_nCurrentTime, cFrameType, pStream);
				break;
				// transport
			case FT_CTP_DATA:
			case FT_CTP_ACK:
			case FT_CTP_NAK:
			case FT_CTP_PONG:
				if(bSTC)
				{
//					::OutputDebugString("<<NET LOW>>Packet received in secondary channel\n");
					m_ctpEndpoint2.Update(m_nCurrentTime, cFrameType, pStream);
				}
				else
				{
					m_ctpEndpoint.Update(m_nCurrentTime, cFrameType, pStream);
				}
				
				break;
			default:
				NET_ASSERT(0);
				break;
	};	
}


void CServerSlot::ResetBandwidthStats()
{
	m_BandwidthStats.Reset();
}

void CServerSlot::GetBandwidthStats( SServerSlotBandwidthStats &out ) const
{
	out=m_BandwidthStats;
}


void CServerSlotImpl::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(this,sizeof(CServerSlotImpl));
	m_ctpEndpoint.GetMemoryStatistics(pSizer);
	m_ccpEndpoint.GetMemoryStatistics(pSizer);
}

//////////////////////////////////////////////////////////////////////////
void CServerSlotImpl::OnCCPSecurityQuery(CStream &stm)
{
}

//////////////////////////////////////////////////////////////////////////
void CServerSlotImpl::OnCCPSecurityResp(CStream &stm)
{
	// Notify defence wall
	m_pNetwork->OnSecurityMsgResponse( m_ipAddress,stm );
}

//////////////////////////////////////////////////////////////////////////
void CServerSlotImpl::OnCCPPunkBusterMsg(CStream &stm)
{
	m_pNetwork->OnCCPPunkBusterMsg( m_ipAddress,stm );
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//FAKE SERVERSLOT IMPLEMENTATION
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CServerSlotLocal::CServerSlotLocal(CServer *pServer,CClientLocal *pClient,CIPAddress &ip,CNetwork *pNetwork ) 
	:CServerSlot(pNetwork)
{
	m_cClientID=0;
	m_pClient=pClient;
	m_pServer=pServer;
	m_ipAddress=ip;
	m_bContextSetup=false;
	m_nUpdateCounter=0;
	m_bClientAdded = false;
}

CServerSlotLocal::~CServerSlotLocal()
{
	if(m_pServer==NULL)
	{
		//call alberto
		NET_ASSERT(0);
	}

	m_pNetwork->RemoveClientFromDefenceWall( m_ipAddress );

	m_pServer->OnDestructSlot(this);
	if(m_pClient)
	{
		m_pClient->OnDestructServerSlot();
	}
}

void CServerSlotLocal::Disconnect(const char *szCause)
{
	if(m_pSink)
		m_pSink->OnXServerSlotDisconnect(szCause);

	if(m_pClient)
		m_pClient->OnDisconnenct(szCause);
}

void CServerSlotLocal::OnDisconnect(const char *szCause)
{
	m_pNetwork->RemoveClientFromDefenceWall( m_ipAddress );
}

bool CServerSlotLocal::ContextSetup(CStream &stm)
{
	//queue a context setup and 
	//the m_nUpdateCounter make sure
	//that there will be 4 updates of delay
	//before the local client receive this packet.
	//This avoid that the client start to
	//play before the local server is ready(hack?)
	m_bContextSetup=true;
	m_nUpdateCounter=0;
	m_stmContextSetup=stm;

	// Run Checks for this client.
	if (!m_bClientAdded)
		m_pNetwork->AddClientToDefenceWall( m_ipAddress );
	m_bClientAdded = true;
	
	return true;	
}

void CServerSlotLocal::OnContextReady()
{
}

void CServerSlotLocal::SendReliable(CStream &stm,bool bSecondaryChannel)
{
	if(m_pClient)
		m_pClient->PushData(stm);
}

void CServerSlotLocal::SendUnreliable(CStream &stm)
{
	if(m_pClient)
		m_pClient->PushData(stm);
}


void CServerSlotLocal::PushData(CStream &stm)
{
	m_qData.push(stm);
}


void CServerSlotLocal::Release()
{
	assert(this);
	delete this;
}

void CServerSlotLocal::UpdateSlot()
{
	if(m_bContextSetup)
	{
		if(m_nUpdateCounter>4)
		{
			if(m_pClient)
				m_pClient->OnContextSetup(m_stmContextSetup);

			m_bContextSetup=false;
			m_nUpdateCounter=0;
		}
		else
			m_nUpdateCounter++;
	}
	else
	{
		while(!m_qData.empty())
		{
			if(m_pSink)
				m_pSink->OnData(m_qData.front());

			m_qData.pop();
		}
	}
}

void CServerSlotLocal::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(this,sizeof(CServerSlotLocal));
	pSizer->AddObject(&m_qData,m_qData.size()*sizeof(CStream));
}

void CServerSlotLocal::OnCCPConnectResp(CStream &stm)
{
	unsigned int uiSize;

	stm.Read(uiSize);
	if (uiSize)
	{
		assert(uiSize%8==0);
		m_pbAuthorizationID = new BYTE[uiSize/8];
		stm.ReadBits(m_pbAuthorizationID,uiSize);
		m_uiAuthorizationSize = uiSize/8;
	}

	if(m_pSink)
	{
		#ifndef NOT_USE_UBICOM_SDK
		// If its a multiplayer game check the cdkey
		if(GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))
			m_pNetwork->m_pUbiSoftClient->Server_CheckPlayerAuthorizationID(GetID(),m_pbAuthorizationID);
		else
#endif // NOT_USE_UBICOM_SDK
		{
			m_pSink->OnXPlayerAuthorization(true,"",NULL,0);
		}

		m_pSink->OnXServerSlotConnect(m_pbAuthorizationID,m_uiAuthorizationSize);
	}
}


void CServerSlot::OnPlayerAuthorization( bool bAllow, const char *szError, const BYTE *pGlobalID, 
	unsigned int uiGlobalIDSize )
{
	m_pSink->OnXPlayerAuthorization(bAllow,szError,pGlobalID,uiGlobalIDSize);
	m_pbGlobalID = new BYTE[uiGlobalIDSize];
	m_uiGlobalIDSize = uiGlobalIDSize;
	memcpy( m_pbGlobalID,pGlobalID,uiGlobalIDSize );
}
