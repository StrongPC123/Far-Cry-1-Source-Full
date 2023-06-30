// ClientLocal.h: interface for the CClientLocal class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CLIENTLOCAL_H_
#define _CLIENTLOCAL_H_

#include <queue>


class CNetwork;
class CServerSlotLocal;
struct IClientSink;

/*!fake CClient implementation for local clients
 doesn't use the network!*/
class CClientLocal : 
public IClient  
{
public:
	//! constructor
	CClientLocal(CNetwork *pNetwork,IClientSink *pSink);
	//! destructor
	virtual ~CClientLocal();

	// interface IClient --------------------------------------------------------------------

	virtual void Connect(const char *szIP, WORD wPort, const BYTE *pbAuthorizationID, unsigned int iAuthorizationSize);
	virtual void Disconnect(const char *szCause);
	virtual void SendReliable(CStream &stm);
	virtual void SendUnreliable(CStream &stm);
	virtual void ContextReady(CStream &stm);
	virtual bool IsReady();
	virtual bool Update(unsigned int nTime);
	virtual void GetBandwidth( float &fIncomingKbPerSec, float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets );
	virtual void Release();
	virtual unsigned int GetPing(){return 0;}
	virtual unsigned int GetRemoteTimestamp(unsigned int nTime){return nTime;}
	virtual unsigned int GetPacketsLostCount(){return 0;}
	virtual unsigned int GetUnreliablePacketsLostCount(){return 0;}
	virtual void OnCDKeyAuthorization( BYTE *pbAuthorizationID );
	virtual void SetServerIP( const char *szServerIP );
	virtual void InitiateCDKeyAuthorization( const bool inbCDAuthorization );

	// -------------------------------------------------------------------------------------

	void ResetSink(){m_pSink=NULL;};
	void PushData(CStream &stm){ m_qData.push(stm);}
	void OnConnect(CNPServerVariables sv){ if(m_pSink)m_pSink->OnXConnect();}
	void OnContextSetup(CStream &stm){ if(m_pSink)m_pSink->OnXContextSetup(stm);}
	void OnDisconnenct(const char *szCause);
	void OnCCPSecurityQuery(CStream &stm) {};
	void OnCCPSecurityResp(CStream &stm) {};
	void OnCCPPunkBusterMsg(CStream &stm) {};

	void OnDestructServerSlot(){ m_pServerSlot=0; }
	virtual CIPAddress GetServerIP() const { return CIPAddress(); };

private: 	// -------------------------------------------------------------------------------------

	string										m_sServerIP;						//!<
	IClientSink *							m_pSink;								//!<
	CNetwork *								m_pNetwork;							//!<
	CServerSlotLocal *				m_pServerSlot;					//!<
	bool											m_bReady;								//!<
	STREAM_QUEUE							m_qData;								//!<
};

#endif //_CLIENTLOCAL_H_
