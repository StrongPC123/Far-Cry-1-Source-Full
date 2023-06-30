#ifndef NETSERVERSNOOPER_H
#define NETSERVERSNOOPER_H


#include <INetwork.h>

#if defined(LINUX) || defined(WIN64)
#include <map>
#endif
#ifdef WIN64
	#define hash_map map
#else
#if defined(LINUX)
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#endif

#include <IConsole.h>

#define NET_SNOOPER_RETRY				(2)
#define NET_SNOOPER_MAXWAITING	(20)
#define NET_SNOOPER_TIMEOUT			(1500)	// 1 seconds timeout per try

typedef struct NETSnooperServer
{
	CIPAddress			ipAddress;		//!< 
	DWORD						dwStartTime;	//!< 
	DWORD						dwTimeout;		//!< when current time >= dwTimeout, increase cTry, and retry
	byte						cTry;					//!< when cTry > NET_SNOOPER_RETRY, remove from list
	bool						bDoing;				//!< 
} NETSnooperServer;


typedef std::map<CIPAddress, NETSnooperServer>					 HMServerTable;
typedef std::map<CIPAddress, NETSnooperServer>::iterator HMServerTableItor;


class CNETServerSnooper: public INETServerSnooper, public INetworkPacketSink
{
public:
	//! constructor
	CNETServerSnooper();
	//! destructor
	~CNETServerSnooper();

	bool Create(ISystem *pSystem, INETServerSnooperSink *pSink);

	// interface INetworkPacketSink --------------------------------------

	virtual void OnReceivingPacket( const unsigned char inPacketID, CStream &stmPacket, CIPAddress &ip );

	// interface INETServerSnooper ----------------------------------------

	virtual void Release();
	virtual void Update(unsigned int dwTime);
	virtual void AddServer(const CIPAddress &ip);
	virtual void AddServerList(const std::vector<CIPAddress> &vIP);
	virtual void ClearList();

private: 	// ------------------------------------------------------------

	void QueryServer(CIPAddress &ip);
	void ProcessPacket(CStream &stmPacket, CIPAddress &ip);
	void ProcessTimeout();
	bool ProcessNext();

	ISystem	*									m_pSystem;						//!<
	
	unsigned int							m_dwCurrentTime;			//!<
	int												m_iWaitingCount;			//!<

	INETServerSnooperSink	*		m_pSink;							//!<
	CDatagramSocket						m_sSocket;						//!<
	HMServerTable							m_hmServerTable;			//!<

	ICVar	*										cl_snooptimeout;			//!<
	ICVar	*										cl_snoopretries;			//!<
	ICVar	*										cl_snoopcount;				//!<
};

#endif // NETSERVERSNOOPER_H
