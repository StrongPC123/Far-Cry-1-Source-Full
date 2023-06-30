//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: Client.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Client.h"
#include "CNP.h"
#include "IGame.h"
#include "IScriptSystem.h"

#ifndef NOT_USE_UBICOM_SDK
	#include "UbiSoftMemory.h"									// GS_WIN32
	#include "cdkeydefines.h"										// UBI.com AUTHORIZATION_ID_SIZE
	#include "NewUbisoftClient.h"								// NewUbisoftClient
#else
	#define AUTHORIZATION_ID_SIZE 20
#endif // NOT_USE_UBICOM_SDK


#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClient::CClient( CNetwork *pNetwork )
:	m_ctpEndpoint(pNetwork), m_ctpEndpoint2(pNetwork),
	cl_timeout(0), m_pbAuthorizationID(NULL), m_uiAuthorizationSize(0), m_bWaiting(0)
{
	m_pNetwork = pNetwork;
	m_ccpEndpoint.Init(this);
	m_smCCPMachine.Init(this);
}

CClient::~CClient()
{
	m_pNetwork->UnregisterClient(this);
	m_pSink=NULL;
	if (m_pbAuthorizationID)
		delete [] m_pbAuthorizationID;
}

bool CClient::Init(IClientSink *pSink)
{
	m_pSink=pSink;
	if(NET_FAILED(m_socketMain.Create()))return false;
	if(NET_FAILED(m_socketMain.Listen(0)))return false;
	m_ctpEndpoint.Init(this,false);
	m_ctpEndpoint2.Init(this,true);
	m_ccpEndpoint.Init(this);
	m_dwKeepAliveTimer=0;
	
	cl_timeout = GetISystem()->GetIConsole()->GetCVar("cl_timeout");

	return true;	
}
//////////////////////////////////////////////////////////////////////
// _IClientServices
//////////////////////////////////////////////////////////////////////
#if (defined(PS2) || defined(LINUX))
#define FAILED(value) (((unsigned int)(value))&0x80000000)
#endif


#ifdef _INTERNET_SIMULATOR
#include <stdlib.h>
#if !defined(LINUX)
#include <assert.h>
#endif

#include "IConsole.h"
#include "ITimer.h"
#endif

bool CClient::SendTo( CIPAddress &ip,CStream &stm )
{
	if(FAILED(m_socketMain.Send(stm.GetPtr(),BITS2BYTES(stm.GetSize()),&ip)))
		return false;
	return true;
}

bool CClient::Send(CStream &stm)
{

#ifndef _INTERNET_SIMULATOR
	if(FAILED(m_socketMain.Send(stm.GetPtr(),BITS2BYTES(stm.GetSize()),&m_ipServer)))return false;
	return true;
#else
	static ICVar *pVarPacketloss=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_packetloss");
	static ICVar *pVarMinPing=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_minping");
	static ICVar *pVarMaxPing=GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_maxping");

	int iMaxPing=pVarMinPing->GetIVal();
	int iMinPing=pVarMaxPing->GetIVal();

	if(iMinPing>iMaxPing)
		iMaxPing=iMinPing;

	if (pVarPacketloss->GetFVal()>0 || iMaxPing>0)
	{	
		DelayedPacket *delayed = new DelayedPacket;
		if(iMaxPing>0)
			delayed->m_fTimeToSend = GetISystem()->GetITimer()->GetCurrTime() + iMinPing + (rand() % (iMaxPing-iMinPing)) / 1000.0f;
		else
			delayed->m_fTimeToSend = GetISystem()->GetITimer()->GetCurrTime();
		delayed->m_dwLengthInBytes = BITS2BYTES(stm.GetSize());
		assert(delayed->m_dwLengthInBytes < sizeof(delayed->m_Data)/sizeof(delayed->m_Data[0]));
		memcpy(delayed->m_Data, stm.GetPtr(), delayed->m_dwLengthInBytes);
		m_delayedPacketList.push_back(delayed);
		return true;
	}
	else
	{
		if(FAILED(m_socketMain.Send(stm.GetPtr(),BITS2BYTES(stm.GetSize()),&m_ipServer)))return false;
		return true;
	}
#endif
}

bool CClient::SendSetup()
{
	m_ccpEndpoint.SendSetup();
	return true;
}

bool CClient::SendConnectResp()
{
	CStream stm;

	if (m_pbAuthorizationID)
		stm.WriteBits(m_pbAuthorizationID,m_uiAuthorizationSize*8);
	m_ccpEndpoint.SendConnectResp(stm);
	return true;
}

bool CClient::SendContextReady()
{
	m_ccpEndpoint.SendContextReady(m_stmContextReady);
	return true;
}

bool CClient::SendDisconnect(const char* szCause)
{
	m_ccpEndpoint.SendDisconnect(szCause);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CClient::SendSecurityResponse(CStream &stm)
{
	m_ccpEndpoint.SendSecurityResp(stm);
	return true;
}

bool CClient::SendPunkBusterMsg(CStream &stm)
{
	m_ccpEndpoint.SendPunkBusterMsg(stm);
	return true;
}

void CClient::OnContextSetup()
{ 
	m_stmContext.Seek(0);
	if(m_pSink)
		m_pSink->OnXContextSetup(m_stmContext);
	m_ctpEndpoint.Reset();
	m_ctpEndpoint2.Reset();
}

void CClient::OnServerReady()
{
	GetISystem()->GetILog()->Log("CClient::OnServerReady");
		
	/*	if(m_pSink)
		m_pSink->OnServerReady();*/
}

void CClient::OnConnect()
{
	if(m_pSink)
		m_pSink->OnXConnect();
}

void CClient::OnDisconnect(const char *szCause)
{
	if(m_pSink)
	{
		if(szCause==NULL)szCause="Undefined";
		m_pSink->OnXClientDisconnect(szCause);	// this pointer is destroyed after this call
	}
}


void CClient::OnCCPConnect(CStream &stm)
{
	CCPConnect ccpConnect;
	ccpConnect.Load(stm);

	if (ccpConnect.m_cResponse & SV_CONN_FLAG_PUNKBUSTER)
	{
		ICVar *cl_punkbuster = GetISystem()->GetIConsole()->GetCVar("cl_punkbuster");

		if (cl_punkbuster && cl_punkbuster->GetIVal() != 0)
		{
			m_pNetwork->InitPunkbusterClient(this);
		}
	}

	m_pNetwork->LockPunkbusterCVars();

	GetISystem()->GetILog()->Log("CClient::OnCCPConnect");

	m_ccpEndpoint.SetPublicCryptKey(ccpConnect.m_ServerVariables.nPublicKey);
	m_ctpEndpoint.SetPublicCryptKey(ccpConnect.m_ServerVariables.nPublicKey);
	m_ctpEndpoint2.SetPublicCryptKey(ccpConnect.m_ServerVariables.nPublicKey);

	m_ServerVariables=ccpConnect.m_ServerVariables;
	m_smCCPMachine.Update(SIG_CONNECT);
}

void CClient::OnCCPContextSetup(CStream &stm)
{
	GetISystem()->GetILog()->Log("CClient::OnCCPContextSetup");

	m_stmContext=stm;
	m_smCCPMachine.Update(SIG_CONTEXT_SETUP);
}

void CClient::OnCCPServerReady()
{
	GetISystem()->GetILog()->Log("CClient::OnCCPServerReady");

	m_smCCPMachine.Update(SIG_SERVER_READY);
}

void CClient::OnData(CStream &stm)
{
	/*
	BYTE msg;
	if(stm.ReadPkd(msg))
	{
		// Special control messages.
		{
			if (msg >= 250)
				return false;
		}
	}
*/
	
	// Seek back to 0;
	stm.Seek(0);

	NET_TRACE("Packet=%d \n ",BITS2BYTES(stm.GetSize()));
	if(m_pSink)
		m_pSink->OnXData(stm);
}

void CClient::OnCCPDisconnect(const char *szCause)
{
	m_smCCPMachine.Update(SIG_DISCONNECT,(ULONG_PTR)szCause);
}

//////////////////////////////////////////////////////////////////////
// IClient
//////////////////////////////////////////////////////////////////////


void CClient::Connect( const char *szIP, WORD wPort, const BYTE *pbAuthorizationID, unsigned int uiAuthorizationSize )
{
	assert(pbAuthorizationID);
	assert(uiAuthorizationSize>0);
	NET_ASSERT(m_pSink!=NULL); // you forgot something

	CIPAddress ip(wPort,szIP);
	m_bLocalHost=ip.IsLocalHost();
	m_ipServer=ip;
	m_socketMain.SetDefaultTarget(ip);

	if(m_pbAuthorizationID)
		delete [] m_pbAuthorizationID;

	// copy AuthorizationID
	m_uiAuthorizationSize = uiAuthorizationSize;
	m_pbAuthorizationID = new BYTE[m_uiAuthorizationSize];
	memcpy(m_pbAuthorizationID,pbAuthorizationID,m_uiAuthorizationSize);

	m_smCCPMachine.Update(SIG_START);
}

void CClient::Disconnect(const char* szCause)
{
	m_smCCPMachine.Update(SIG_ACTIVE_DISCONNECT,(ULONG_PTR)szCause);	// this pointer is destroyed after this call
}

void CClient::SendReliable(CStream &stm)
{
	m_ctpEndpoint.SendReliable(stm);
}

void CClient::SendUnreliable(CStream &stm)
{
	m_ctpEndpoint.SendUnreliable(stm);
}

void CClient::ContextReady(CStream &stm)
{
	GetISystem()->GetILog()->Log("CClient::ContextReady");

	if(m_smCCPMachine.GetCurrentStatus()==STATUS_PROCESSING_CONTEXT)
	{
		m_stmContextReady=stm;
		m_smCCPMachine.Update(SIG_CONTEXT_READY);
	}
	else
		GetISystem()->GetILog()->Log("Client::ContextReady !=STATUS_PROCESSING_CONTEXT %d",m_smCCPMachine.GetCurrentStatus());
}



bool CClient::Update(unsigned int nTime)
{
	#ifdef _INTERNET_SIMULATOR

		TDelayPacketList::iterator i;

		for (i = m_delayedPacketList.begin(); i != m_delayedPacketList.end();)
		{
			DelayedPacket *dp = (*i);
			if (dp->m_fTimeToSend <= GetISystem()->GetITimer()->GetCurrTime())
			{
				i = m_delayedPacketList.erase(i);
				// Send it
				// 2% packetloss
				if ((rand() %100) > GetISystem()->GetIConsole()->GetCVar("g_internet_simulator_packetloss")->GetFVal())
					m_socketMain.Send(dp->m_Data,dp->m_dwLengthInBytes,&m_ipServer);
				// Delete it
				delete dp;
			}
			else
				++i;
		}
#endif

	m_nCurrentTime=nTime;
	
	// was the timer reset ?
	if (m_dwKeepAliveTimer > m_nCurrentTime)
	{
		// reset our keep alive timer
		m_dwKeepAliveTimer = m_nCurrentTime;
	}

	int nRecvBytes;
/////////////////////////////////////////////////////////
	static CIPAddress ipFrom;
	static CStream buf;
	
	do
	{
		buf.Reset();
		nRecvBytes=0;
		m_socketMain.Receive(buf.GetPtr(),
			(int)BITS2BYTES(buf.GetAllocatedSize()),
			nRecvBytes,
			ipFrom);
/////////////////////////////////////////////////////////
		if(nRecvBytes>0)
		{
			buf.SetSize(BYTES2BITS(nRecvBytes));
			if(!ProcessPacket(buf,ipFrom))
			{
				GetISystem()->GetILog()->Log("NetDEBUG: ProcessPacket(buf,ipFrom) false");

				return false; // this object was destroyed
			}
			m_dwKeepAliveTimer=m_nCurrentTime;
		}
	} while(nRecvBytes>0);
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

	m_ccpEndpoint.Update(m_nCurrentTime, 0, NULL);
	m_smCCPMachine.Update();
	unsigned int CurrState=m_smCCPMachine.GetCurrentStatus();
	if(CurrState==STATUS_READY || CurrState==STATUS_WAIT_FOR_SERVER_READY)
	{
		static char szCause[]="@ServerTimeout";
		m_ctpEndpoint.Update(m_nCurrentTime, 0, NULL);
		m_ctpEndpoint2.Update(m_nCurrentTime, 0, NULL);

		if(cl_timeout && cl_timeout->GetFVal() && GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))
		{
			if(m_nCurrentTime-m_dwKeepAliveTimer > (cl_timeout->GetFVal() * 1000.0f) + m_pSink->GetTimeoutCompensation()) 
			{
				if (!m_bWaiting)
				{
					m_pSink->OnXServerTimeout();
				}

				m_bWaiting = 1;
			}
			else if (m_bWaiting)
			{
				m_pSink->OnXServerRessurect();

				m_bWaiting = 0;
			}
		}
	}
	else
	{
		m_dwKeepAliveTimer=m_nCurrentTime;
	}

	m_pNetwork->OnClientUpdate();

	return true;		// this object is still exising
}

void CClient::GetBandwidth(float &fIncomingKbPerSec,float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets )
{
	fIncomingKbPerSec=m_socketMain.m_fIncomingKbPerSec;
	fOutgoinKbPerSec=m_socketMain.m_fOutgoingKbPerSec;
	nIncomingPackets=m_socketMain.m_nIncomingPacketsPerSec;
	nOutgoingPackets=m_socketMain.m_nOutgoingPacketsPerSec;
}

void CClient::Release()
{
	delete this;
}

bool CClient::IsReady()
{
	return (m_smCCPMachine.GetCurrentStatus()==STATUS_READY)?true:false;
}

bool CClient::ProcessPacket(CStream &stmPacket,CIPAddress &ip)
{
	if (!m_pNetwork->CheckPBPacket( stmPacket,ip ))
		return false;

	CNP cnp;
	cnp.LoadAndSeekToZero(stmPacket);
	switch(cnp.m_cFrameType){
	case FT_CCP_CONNECT:
	case FT_CCP_DISCONNECT:
	case FT_CCP_CONTEXT_SETUP:
	case FT_CCP_SERVER_READY:
	case FT_CCP_ACK:
	case FT_CCP_SECURITY_QUERY:
	case FT_CCP_SECURITY_RESP:
	case FT_CCP_PUNK_BUSTER_MSG:
		if (!m_ccpEndpoint.Update(m_nCurrentTime,cnp.m_cFrameType,&stmPacket))
		{
			GetISystem()->GetILog()->Log("NetDEBUG: m_ccpEndpoint.Update false");

			return false;
		}
		break;
	case FT_CTP_DATA:
	case FT_CTP_ACK:
	case FT_CTP_NAK:
	case FT_CTP_PONG:
		if(m_smCCPMachine.GetCurrentStatus()==STATUS_READY)
		{
			if(cnp.m_bSecondaryTC)
			{
				m_ctpEndpoint2.Update(m_nCurrentTime,cnp.m_cFrameType,&stmPacket);
			}
			else
			{
				m_ctpEndpoint.Update(m_nCurrentTime,cnp.m_cFrameType,&stmPacket);
			}
		}
		else
		{
			GetISystem()->GetILog()->Log("CTP PACKET RECEIVED (CCP NOT READY!!) %d",m_smCCPMachine.GetCurrentStatus());
		}
		break;
/*	case FT_CQP_INFO_RESPONSE:
		{
			CQPInfoResponse cqpInfoResponse;
			cqpInfoResponse.Load(stmPacket);
			m_pSink->OnServerFound(ip,cqpInfoResponse.m_stmData);
		}
		break;*/
	default:
		GetISystem()->GetILog()->Log("NetDEBUG: cnp.m_cFrameType %d",(int)cnp.m_cFrameType);
		//NET_ASSERT(0);
		break;
	}

	return 1;
}


unsigned int CClient::GetPing()
{
	return m_ctpEndpoint.GetPing();
}

//////////////////////////////////////////////////////////////////////////
void CClient::OnCCPSecurityQuery(CStream &stm)
{
	m_pNetwork->OnSecurityMsgQuery(stm);
}


//////////////////////////////////////////////////////////////////////////
void CClient::OnCCPSecurityResp(CStream &stm)
{
}

//////////////////////////////////////////////////////////////////////////
void CClient::OnCCPPunkBusterMsg(CStream &stm)
{
	m_pNetwork->OnCCPPunkBusterMsg( m_ipServer,stm );
}


///////////////////////////////////////////////
void CClient::SetServerIP( const char *szServerIP )
{
	m_sServerIP = szServerIP;
}

///////////////////////////////////////////////
void CClient::OnCDKeyAuthorization( BYTE *pbAuthorizationID )
{
	if(!pbAuthorizationID)
	{
		static BYTE fakeid[AUTHORIZATION_ID_SIZE];

		pbAuthorizationID = fakeid;

		memset(fakeid,0,AUTHORIZATION_ID_SIZE);		// generated fake AuthorizationID
	}

	char *sSemicolon;
	unsigned short port = 0;
	char temp[256];
	strncpy(temp,m_sServerIP.c_str(),256);

	if(sSemicolon=strstr(temp,":"))
	{
		port=atoi(&sSemicolon[1]);
		sSemicolon[0]='\0';
	}

	if(port==0)
		port=DEFAULT_SERVERPORT;

	Connect(temp, port, pbAuthorizationID,AUTHORIZATION_ID_SIZE);
}



void CClient::InitiateCDKeyAuthorization( const bool inbCDAuthorization )
{
#ifndef NOT_USE_UBICOM_SDK
	if(inbCDAuthorization)
		m_pNetwork->m_pUbiSoftClient->Client_GetCDKeyAuthorizationID();			// OnXCDKeyAuthorization is called later
	else
#endif // NOT_USE_UBICOM_SDK
	{
		OnCDKeyAuthorization(0);	// 0 -> fake AuthorizationID is generated
	}
}
