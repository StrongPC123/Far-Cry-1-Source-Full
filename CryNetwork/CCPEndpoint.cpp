//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: CCPEndpoint.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CCPEndpoint.h"
#include "Network.h"

#include <ISystem.h>
#include <IConsole.h>

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#ifndef WIN32
int GetCurrentProcessId()
{
	return 0;
}
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCCPEndpoint::CCCPEndpoint()
{
	m_pParent=0;
	Reset();
	//new char[100000];//test
}


void CCCPEndpoint::Init( _ICCPUser *pParent )
{
	m_pParent=pParent;
}

CCCPEndpoint::~CCCPEndpoint()
{
	while(!m_qOutgoingData.empty())
	{
		CCPPayload *pTemp=m_qOutgoingData.front();
		delete pTemp;
		m_qOutgoingData.pop();
	}
}

void CCCPEndpoint::Reset()
{
	m_bFrameExpected = false;
	m_bNextFrameToSend = false;
	m_bAckExpected=!m_bNextFrameToSend;
	EnableSend();
	m_ulTimeout = 0;
} 

void CCCPEndpoint::SetTimer()
{
	if (m_ulTimeout == 0)
		m_ulTimeout = m_nCurrentTime;
}

void CCCPEndpoint::StopTimer()
{
	m_ulTimeout = 0;
}

bool CCCPEndpoint::Update(unsigned int nTime, unsigned char cFrameType, CStream *pStm)
{
	m_nCurrentTime = nTime;
	// manage incoming frames
	if (cFrameType)
	{
		if (cFrameType == FT_CCP_ACK)
		{
			/// ACK///////////////////////////////////
			CCPAck ccpAck;
			ccpAck.Load(*pStm);
			if (ccpAck.m_bAck == m_bAckExpected)
			{
				NET_TRACE("[%08X] IN [CCP] RECEIVED ACK %02d \n",::GetCurrentProcessId(), ccpAck.m_bAck?1:0);
				StopTimer();
				EnableSend();
				m_stmRetrasmissionBuffer.Reset();
			}
			/*else
			{
				HandleTimeout(); 
				NET_TRACE("CCCPEndpoint::Update ACK OUT OF SEQ %d\n",ccpAck.m_bAck?1:0);
			}*/
			/////////////////////////////////////////
		}
		else
		{
			/// PAYLOAD///////////////////////////////
			if (!ProcessPayload(cFrameType, *pStm))
			{
				GetISystem()->GetILog()->Log("NetDEBUG: ProcessPayload false");
				return 0;
			}
			/////////////////////////////////////////
		}
	}
	
	// manage timeouts
	ProcessTimers();
	
	// manage outgoing frames
	if (m_qOutgoingData.empty() == false)
	{
		if (IsTimeToSend())
		{
			SendFrame();
		}
	}

	return 1;
}

void CCCPEndpoint::SendSetup()
{
	CCPSetup *pCCPSetup;
	pCCPSetup = new CCPSetup;

	ICVar *cl_password = GetISystem()->GetIConsole()->GetCVar("cl_password");

	assert(cl_password);

	if (cl_password->GetString())
	{
		pCCPSetup->m_sPlayerPassword = cl_password->GetString();
	}
	else
	{
		pCCPSetup->m_sPlayerPassword = "";
	}

	// Detect version 32/64bit.
#if defined(WIN64) || defined(LINUX64)
	pCCPSetup->m_nClientFlags |= CLIENT_FLAGS_64BIT;
#endif
	ICVar *cl_punkbuster = GetISystem()->GetIConsole()->GetCVar("cl_punkbuster");
	if (cl_punkbuster && cl_punkbuster->GetIVal() != 0)
		pCCPSetup->m_nClientFlags |= CLIENT_FLAGS_PUNK_BUSTER;

	if (GetISystem()->WasInDevMode())
		pCCPSetup->m_nClientFlags |= CLIENT_FLAGS_DEVMODE;

	// Detect client OS.
#ifdef WIN32
	OSVERSIONINFO OSVerInfo;
	OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVerInfo);
	pCCPSetup->m_nClientOSMinor = OSVerInfo.dwMinorVersion;
	pCCPSetup->m_nClientOSMajor = OSVerInfo.dwMajorVersion;
#endif

	//	pCCPSetup->m_cClientID = m_pParent->GetID();
	m_qOutgoingData.push(pCCPSetup);
}

void CCCPEndpoint::SendConnect(CNPServerVariables &sv)
{
	CCPConnect *pCCPConnect;
	pCCPConnect = new CCPConnect;
//	pCCPConnect->m_cClientID = m_pParent->GetID();
	//pCCPConnect->m_cNewClientID = cClientID;
	pCCPConnect->m_ServerVariables = sv;
	pCCPConnect->m_cResponse = 0;

	ICVar *pPunkBusterVar = GetISystem()->GetIConsole()->GetCVar("sv_punkbuster");
	
	if (pPunkBusterVar && pPunkBusterVar->GetIVal() != 0)
	{
		pCCPConnect->m_cResponse |= SV_CONN_FLAG_PUNKBUSTER; // punkbuster!
	}

	if (GetISystem()->WasInDevMode())
	{
		pCCPConnect->m_cResponse |= SV_CONN_FLAG_DEVMODE;
	}

	m_qOutgoingData.push(pCCPConnect);
}

void CCCPEndpoint::SendConnectResp(CStream &stm)
{
	CCPConnectResp *pCCPConnectResp;
	pCCPConnectResp = new CCPConnectResp();
//	pCCPConnectResp->m_cClientID = m_pParent->GetID();
	pCCPConnectResp->m_cResponse = 0;//<<FIXME>> put something more useful
	pCCPConnectResp->m_stmAuthorizationID = stm;
	m_qOutgoingData.push(pCCPConnectResp);
}

void CCCPEndpoint::SendContextSetup(CStream &stm)
{
	CCPContextSetup *pCCPContextSetup;
	pCCPContextSetup = new CCPContextSetup;
//	pCCPContextSetup->m_cClientID = m_pParent->GetID();
	pCCPContextSetup->m_stmData = stm;
	m_qOutgoingData.push(pCCPContextSetup);
}

void CCCPEndpoint::SendContextReady(CStream &stm)
{
	CCPContextReady *pCCPContextReady;
	pCCPContextReady = new CCPContextReady;
//	pCCPContextReady->m_cClientID = m_pParent->GetID();
	pCCPContextReady->m_stmData = stm;
	m_qOutgoingData.push(pCCPContextReady);
}

void CCCPEndpoint::SendServerReady()
{
	CCPServerReady *pCCPServerReady;
	pCCPServerReady = new CCPServerReady;
//	pCCPServerReady->m_cClientID = m_pParent->GetID();
	m_qOutgoingData.push(pCCPServerReady);
}

void CCCPEndpoint::SendDisconnect(const char* szCause)
{

	CStream stm;
	CCPDisconnect *pCCPDisconnect;
	pCCPDisconnect = new CCPDisconnect;
//	pCCPDisconnect->m_cClientID = m_pParent->GetID();
	pCCPDisconnect->m_sCause = szCause;
	pCCPDisconnect->m_bSequenceNumber=0;
  
	pCCPDisconnect->Save(stm);
	
	//the disconnect packet is send ignoring the current state to
	//increase the chances that the other endpoint will receive it
	//the seq number is ignored by the receiver
	m_pParent->Send(stm);
	delete pCCPDisconnect;
	//m_qOutgoingData.push(pCCPDisconnect);
}

//////////////////////////////////////////////////////////////////////////
void CCCPEndpoint::SendSecurityQuery(CStream &stm)
{
	CCPSecurityQuery *pCCP = new CCPSecurityQuery;
	pCCP->m_stmData = stm;
	m_qOutgoingData.push(pCCP);
}

//////////////////////////////////////////////////////////////////////////
void CCCPEndpoint::SendSecurityResp(CStream &stm)
{
	CCPSecurityResp *pCCP = new CCPSecurityResp;
	pCCP->m_stmData = stm;
	m_qOutgoingData.push(pCCP);
}

//////////////////////////////////////////////////////////////////////////
void CCCPEndpoint::SendPunkBusterMsg(CStream &stm)
{
	CCPPunkBusterMsg *pCCP = new CCPPunkBusterMsg;
	pCCP->m_stmData = stm;
	m_qOutgoingData.push(pCCP);
}

//////////////////////////////////////////////////////////////////////////
void CCCPEndpoint::ProcessTimers()
{
	if (m_ulTimeout)
		if ((m_nCurrentTime - m_ulTimeout)>TM_BUFFER_TIMER)
		{
			m_ulTimeout = 0;
			HandleTimeout();
		}
}

void CCCPEndpoint::HandleTimeout()
{
	NET_TRACE("[%08X]CCCPEndpoint::HandleTimeout()\n",::GetCurrentProcessId());
	///bool b;
	if(m_stmRetrasmissionBuffer.GetSize()==0)
	{
		CryError( "<CryNetworkut> (CCCPEndpoint::HandleTimeout) Empty retransmission buffer" );
	}
	m_stmRetrasmissionBuffer.Seek(0);
	m_pParent->Send(m_stmRetrasmissionBuffer);
	SetTimer();
}

void PrintPacket(CCPPayload ccpPayload)
{
	switch(ccpPayload.m_cFrameType)
	{
	case FT_CCP_SETUP:
		NET_TRACE("FT_CCP_SETUP\n");
		break;
	case FT_CCP_CONNECT:
		NET_TRACE("FT_CCP_CONNECT\n");
		break;
	case FT_CCP_CONNECT_RESP:
		NET_TRACE("FT_CCP_CONNECT_RESP\n");
		break;
	case FT_CCP_CONTEXT_SETUP:
		NET_TRACE("FT_CCP_CONTEXT_SETUP\n");
		break;
	case FT_CCP_CONTEXT_READY:
		NET_TRACE("FT_CCP_CONTEXT_READY\n");
		break;
	case FT_CCP_SERVER_READY:
		NET_TRACE("FT_CCP_SERVER_READY\n");
		break;
	case FT_CCP_DISCONNECT:
		NET_TRACE("FT_CCP_DISCONNECT\n");
		break;
	}
}

bool CCCPEndpoint::ProcessPayload(unsigned char cFrameType, CStream &stmStream)
{
	CCPPayload ccpPayload;
	ccpPayload.Load(stmStream);
	stmStream.Seek(0);
	
	//the disconnect packet is a destructive packet for the connection
	//so seq number is ignored
	if(cFrameType==FT_CCP_DISCONNECT)
	{
		GetISystem()->GetILog()->Log("NetDEBUG: FT_CCP_DISCONNECT");

		CCPDisconnect ccpDisconnect;
		ccpDisconnect.Load(stmStream);
		m_pParent->OnCCPDisconnect(ccpDisconnect.m_sCause.c_str());

		return 0;
	}
	else
	{
		if (ccpPayload.m_bSequenceNumber != m_bFrameExpected)
		{
			//SendAck(!m_bFrameExpected);
			NET_TRACE("CCCPEndpoint::ProcessPayload Packet OUT OF SEQ[%02d]\n",ccpPayload.m_bSequenceNumber?1:0);
			PrintPacket(ccpPayload);
		}else
		{
	    NET_TRACE("[%08X] IN [CCP] RECEIVED %02d \n",::GetCurrentProcessId(), ccpPayload.m_bSequenceNumber?1:0);
			INC_BOOL(m_bFrameExpected);
			NET_TRACE("[%08X] FRAME EXPECTED IS NOW [CCP] %02d \n",::GetCurrentProcessId(), m_bFrameExpected?1:0);
			switch (cFrameType)
			{
				///////////////////////////////////////////////////
			case FT_CCP_SETUP:
				{
					NET_TRACE("FT_CCP_SETUP\n");
					m_pParent->OnCCPSetup(stmStream);
				}
				break;
				///////////////////////////////////////////////////
			case FT_CCP_CONNECT:
				{
					NET_TRACE("FT_CCP_CONNECT\n");
					m_pParent->OnCCPConnect(stmStream);
				}
				break;
				///////////////////////////////////////////////////
			case FT_CCP_CONNECT_RESP:
				{
					NET_TRACE("FT_CCP_CONNECT_RESP\n");
					CCPConnectResp ccpConnectResp;
					ccpConnectResp.Load(stmStream);
					m_pParent->OnCCPConnectResp(ccpConnectResp.m_stmAuthorizationID);
				}
				break;
			case FT_CCP_CONTEXT_SETUP:
				{
					NET_TRACE("FT_CCP_CONTEXT_SETUP\n");
					CCPContextSetup ccpContextSetup;
					ccpContextSetup.Load(stmStream);
					m_pParent->OnCCPContextSetup(ccpContextSetup.m_stmData);
				}
				break;
				///////////////////////////////////////////////////
			case FT_CCP_CONTEXT_READY:
				{
					NET_TRACE("FT_CCP_CONTEXT_READY\n");
					CCPContextReady ccpContextReady;
					ccpContextReady.Load(stmStream);
					m_pParent->OnCCPContextReady(ccpContextReady.m_stmData);
				}
				break;
				///////////////////////////////////////////////////
			case FT_CCP_SERVER_READY:
				{
					NET_TRACE("FT_CCP_SERVER_READY\n");
					CCPServerReady ccpServerReady;
					ccpServerReady.Load(stmStream);
					m_pParent->OnCCPServerReady();
				}
				break;

			case FT_CCP_SECURITY_QUERY:
				{
					CCPSecurityQuery ccpSecurQuery;
					ccpSecurQuery.Load(stmStream);
					m_pParent->OnCCPSecurityQuery( ccpSecurQuery.m_stmData );
				}
				break;
			case FT_CCP_SECURITY_RESP:
				{
					CCPSecurityQuery ccpSecurResp;
					ccpSecurResp.Load(stmStream);
					m_pParent->OnCCPSecurityResp( ccpSecurResp.m_stmData );
				}
				break;
			case FT_CCP_PUNK_BUSTER_MSG:
				{
					CCPPunkBusterMsg ccpPBMsg;
					ccpPBMsg.Load(stmStream);
					m_pParent->OnCCPPunkBusterMsg( ccpPBMsg.m_stmData );
				}
				break;
				///////////////////////////////////////////////////
				/*case FT_CCP_DISCONNECT:
				{
				::OutputDebugString("FT_CCP_DISCONNECT\n");
				CCPDisconnect ccpDisconnect;
				ccpDisconnect.Load(stmStream);
				m_pParent->OnCCPDisconnect(ccpDisconnect.m_sCause.c_str());
				}
				break;*/
				///////////////////////////////////////////////////
			default: 
				GetISystem()->GetILog()->Log("NetDEBUG: cFrameType %d",(int)cFrameType);
				NET_ASSERT(0);
				break;
				///////////////////////////////////////////////////
			}
			
			SendAck(ccpPayload.m_bSequenceNumber);
		}
	}

	return 1;
}

void CCCPEndpoint::EnableSend()
{
	NET_TRACE("[%08X] SEND ENABLED\n",::GetCurrentProcessId());
	m_bReadyToSend = true;
}

void CCCPEndpoint::DisableSend()
{
	NET_TRACE("[%08X] SEND DISABLED\n",::GetCurrentProcessId());
	m_bReadyToSend = false;
}

bool CCCPEndpoint::IsTimeToSend()
{
	return m_bReadyToSend;
}

void CCCPEndpoint::SendFrame()
{
	CStream stm;
	CCPPayload *pCCPPayload;
	pCCPPayload = m_qOutgoingData.front();
	pCCPPayload->m_bSequenceNumber = m_bNextFrameToSend;
	pCCPPayload->Save(stm);
	
	m_qOutgoingData.pop();
	m_stmRetrasmissionBuffer = stm;
	m_pParent->Send(stm);
	SetTimer();
	NET_TRACE("[%08X] OUT [CCP] SENDING %02d \n",::GetCurrentProcessId(), pCCPPayload->m_bSequenceNumber?1:0);
	delete pCCPPayload;
	
	INC_BOOL(m_bNextFrameToSend);
	INC_BOOL(m_bAckExpected);
	DisableSend();
}

void CCCPEndpoint::SendAck(bool bSequenceNumber)
{
	CStream stm;
	CCPAck ccpAck;
	ccpAck.m_bAck = bSequenceNumber;
//	ccpAck.m_cClientID = m_pParent->GetID();
	ccpAck.Save(stm);
	////////////////////////
	NET_TRACE("[%08X] OUT [CCP] ACK SEQ %02d\n",::GetCurrentProcessId(), ccpAck.m_bAck);
	////////////////////////
	m_pParent->Send(stm);
}

void CCCPEndpoint::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(&m_qOutgoingData,m_qOutgoingData.size()*sizeof(CStream));
}
