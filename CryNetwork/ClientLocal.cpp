// ClientLocal.cpp: implementation of the CClientLocal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "ServerSlot.h"
#include "ClientLocal.h"
#include <IConsole.h>									// ICVar

#ifndef NOT_USE_UBICOM_SDK
	#include "UbiSoftMemory.h"					// GS_WIN32
	#include "cdkeydefines.h"						// UBI.com AUTHORIZATION_ID_SIZE
	#include "NewUbisoftClient.h"								// NewUbisoftClient
#else
	#define AUTHORIZATION_ID_SIZE 20
#endif // NOT_USE_UBICOM_SDK


#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientLocal::CClientLocal( CNetwork *pNetwork, IClientSink *pSink ) 
{
	m_pSink=pSink;
	m_pNetwork=pNetwork;
	m_bReady=false;
	m_pServerSlot=NULL;

}

CClientLocal::~CClientLocal()
{
	m_pNetwork->UnregisterClient(this);

	if(m_pServerSlot)
		m_pServerSlot->ResetSink();
}

void CClientLocal::Connect( const char *szIP, WORD wPort, const BYTE *pbAuthorizationID, unsigned int uiAuthorizationSize )
{
	assert(pbAuthorizationID);
	assert(uiAuthorizationSize>0);

	m_pServerSlot=m_pNetwork->ConnectToLocalServerSlot(this,wPort);
	if(m_pSink)m_pSink->OnXConnect();

	CStream stAuthorizationID;

	stAuthorizationID.Write(uiAuthorizationSize*8);
	stAuthorizationID.WriteBits(const_cast<BYTE*>(pbAuthorizationID),uiAuthorizationSize*8);

	ICVar *sv_punkbuster = GetISystem()->GetIConsole()->GetCVar("sv_punkbuster");
	ICVar *cl_punkbuster = GetISystem()->GetIConsole()->GetCVar("cl_punkbuster");

	if (sv_punkbuster && sv_punkbuster->GetIVal() != 0)
	{
		if (cl_punkbuster && cl_punkbuster->GetIVal() != 0)
		{
			m_pNetwork->InitPunkbusterClientLocal(this);
		}
	}
	m_pNetwork->LockPunkbusterCVars();

	m_pServerSlot->OnCCPConnectResp(stAuthorizationID);
}

void CClientLocal::Disconnect(const char *szCause)
{
	GetISystem()->GetILog()->Log("NetDEBUG: CClientLocal::Disconnect");

	if(m_pServerSlot)
		m_pServerSlot->OnCCPDisconnect(szCause);
	if (m_pSink)
		m_pSink->OnXClientDisconnect(szCause);

	ResetSink();
}

void CClientLocal::OnDisconnenct(const char *szCause)
{
	if(m_pSink)
		m_pSink->OnXClientDisconnect(szCause);
}

void CClientLocal::SendReliable(CStream &stm)
{
	if(m_pServerSlot)
		m_pServerSlot->PushData(stm);
}

void CClientLocal::SendUnreliable(CStream &stm)
{
	if(m_pServerSlot)
		m_pServerSlot->PushData(stm);
}

void CClientLocal::ContextReady(CStream &stm)
{
	m_pServerSlot->OnCCPContextReady(stm);
}


bool CClientLocal::IsReady()
{
	return ((m_pSink && m_pServerSlot)?true:false);
}

bool CClientLocal::Update(unsigned int nTime)
{
	int iCount=0;

	while(!m_qData.empty())
	{
		if(m_pSink)
		{
			DWORD dwQueueSize=m_qData.size();

			m_pSink->OnXData(m_qData.front());

//		assert(m_qData.size()==dwQueueSize);		// the OnData might add data to the stack

			// [Sergiy] iCount can reach more than 1000 right after loading a level (in Editor, at least)
			assert(iCount<10000);		// otherwise an endless loop would occur

			iCount++;
		}
		m_qData.pop();
	}
	
	if(m_pServerSlot)
		m_pServerSlot->UpdateSlot();

	m_pNetwork->OnClientUpdate();

	return true; // this object is still exising
}

void CClientLocal::GetBandwidth( float &fIncomingKbPerSec, float &fOutgoingKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets )
{
	fIncomingKbPerSec=0;
	fOutgoingKbPerSec=0;
	nIncomingPackets=0;
	nOutgoingPackets=0;
}

void CClientLocal::Release()
{
	delete this;
}


///////////////////////////////////////////////
void CClientLocal::SetServerIP( const char *szServerIP )
{
	m_sServerIP = szServerIP;
}


void CClientLocal::InitiateCDKeyAuthorization( const bool inbCDAuthorization )
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

///////////////////////////////////////////////
void CClientLocal::OnCDKeyAuthorization( BYTE *pbAuthorizationID )
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
