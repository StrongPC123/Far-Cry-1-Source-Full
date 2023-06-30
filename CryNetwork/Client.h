//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: Client.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef _CLIENT_H_
#define _CLIENT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Interfaces.h"
#include "ClientStateMachine.h"
#ifdef NO_GNB
	#include "CTPEndpoint.h"
#else
	#include "CTPEndpointGNB.h"
#endif

#include "CCPEndpoint.h"
#include <queue>
#include <list>
#include <IConsole.h>							// ICVar
#include "Network.h"

// REMOVE THIS WHEN NOT TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#define _INTERNET_SIMULATOR

class CClient :
public IClient,
public _IClientServices
{
public:
	//! constructor
	CClient( CNetwork *pNetwork );
	//! destructor
	virtual ~CClient();

	//!
	bool Init(IClientSink *pSink);
	//! @return true=disconnect and this pointer is destroyed, false otherwise
	bool ProcessPacket(CStream &stmPacket,CIPAddress &ip);

//_IClientServices
	bool Send(CStream &stm);
	bool SendTo( CIPAddress &ip,CStream &stm);
	bool SendSetup();
	bool SendConnectResp();
	bool SendContextReady();
	bool SendDisconnect(const char* szCause);
	void OnConnect();
	void OnContextSetup();
	void OnServerReady();
	void OnDisconnect(const char *szCause);
	void OnData(CStream &stm);
//
	void OnCCPSetup(CStream &stm){}//this is a client-->server packet
	void OnCCPConnect(CStream &stm);
	void OnCCPConnectResp(CStream &stm){}//this is a client-->server packet
	void OnCCPContextSetup(CStream &stm);
	void OnCCPContextReady(CStream &stm){}//this is a client->server packet
	void OnCCPServerReady();//this is a client->server packet
	void OnCCPDisconnect(const char* szCause);
	void OnCCPSecurityQuery(CStream &stm);
	void OnCCPSecurityResp(CStream &stm);
	void OnCCPPunkBusterMsg(CStream &stm);

	// interface IClient ----------------------------------------------------------------------------

	virtual void Connect(const char *szIP, WORD wPort, const BYTE *pbAuthorizationID, unsigned int iAuthorizationSize);	
	virtual void Disconnect(const char *szCause);
	virtual void ContextReady(CStream &stm);
	virtual void SendReliable(CStream &stm);
	virtual void SendUnreliable(CStream &stm);
	virtual bool IsReady();
	virtual bool Update(unsigned int nTime);
	virtual void GetBandwidth( float &fIncomingKbPerSec, float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets);
	virtual void Release();
	virtual unsigned int GetPing();
	virtual unsigned int GetRemoteTimestamp(unsigned int nTime)
	{
		return m_ctpEndpoint.GetRemoteTimestamp(nTime);
	}
	virtual unsigned int GetPacketsLostCount()
	{
		return m_ctpEndpoint.GetLostPackets();
	}
	virtual unsigned int GetUnreliablePacketsLostCount()
	{
		return m_ctpEndpoint.GetUnreliableLostPackets();
	}
	virtual void OnCDKeyAuthorization( BYTE *pbAuthorizationID );
	virtual void SetServerIP( const char *szServerIP );
	virtual void InitiateCDKeyAuthorization( const bool inbCDAuthorization );

	// ---------------------------------------------------------------------------
	
	bool SendSecurityResponse(CStream &stm);
	bool SendPunkBusterMsg(CStream &stm);
	CIPAddress GetServerIP() const { return m_ipServer; };

private: // ----------------------------------------------------------------

	string									m_sServerIP;						//!<
	CNetwork *							m_pNetwork;
	CClientStateMachine			m_smCCPMachine;
#ifdef NO_GNB
	CCTPClient							m_ctpEndpoint;
#else
	CCTPEndpointGNB					m_ctpEndpoint;
	CCTPEndpointGNB					m_ctpEndpoint2;
#endif
	CCCPEndpoint						m_ccpEndpoint;
	CDatagramSocket					m_socketMain;
	CIPAddress							m_ipServer;

	CStream									m_stmContext;
	CStream									m_stmContextReady;
	//after 3 second without any incoming packets the connection will be considered lost
	ICVar	*									cl_timeout;
	DWORD										m_dwKeepAliveTimer;

	BYTE *									m_pbAuthorizationID;		//!< CD Key AuthorizationID (use new and delete)
	unsigned int						m_uiAuthorizationSize;	//!< Size of AuthorizationID in bytes

protected: // ------------------------------------------------------------------

	IClientSink *						m_pSink;

//REMOTE PROTOCOL VARIBLES (update by the connect packet)
struct CNPServerVariables m_ServerVariables;
	
	unsigned int						m_nCurrentTime;
	bool										m_bLocalHost;			//!< the client is connected to the local server (no timeouts)
	bool										m_bWaiting;				//!< the client is waiting for the server.. probably the server just dropped, or is stalled for some time..


#ifdef _INTERNET_SIMULATOR
	struct DelayedPacket
	{
		float						m_fTimeToSend;					//!<
		BYTE						m_Data[8192];						//!<
		unsigned int		m_dwLengthInBytes;			//!<
	};
	typedef std::list<DelayedPacket*>	TDelayPacketList;

	TDelayPacketList					m_delayedPacketList;	//!<
#endif
};

#endif // _CLIENT_H_
