//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: Server.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#ifndef _SERVER_H_
#define _SERVER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Interfaces.h"
#include <map>
#include <queue>
#include <list>

class CServerSlot;
class CNetwork;

#if !defined(LINUX)
#pragma warning(disable:4786) 
#endif

typedef std::map<CIPAddress,CServerSlot* > SLOTS_MAP;
typedef std::map<CIPAddress,CServerSlot* >::iterator SLOTS_MAPItr;

//#define _INTERNET_SIMULATOR

class CNetwork;
class ICrySizer;

class CServer : 
public IServer,
public _IServerServices
{
public:
	//! constructor
	CServer(CNetwork *pNetwork);
	//! destructor
	virtual ~CServer();

	// interface IServer ------------------------------------------------------------------

	virtual void GetBandwidth( float &fIncomingKbPerSec, float &fOutgoingKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets );
	virtual const char *GetHostName();
	virtual void Release();
	virtual void SetVariable(enum CryNetworkVarible eVarName,unsigned int nValue);
	virtual void Update(unsigned int nTime);
	virtual void RegisterPacketSink( const unsigned char inPacketID, INetworkPacketSink *inpSink );
	virtual void SetSecuritySink(IServerSecuritySink *pSecuritySink);
	virtual IServerSecuritySink* GetSecuritySink();
	virtual bool IsIPBanned(const unsigned int dwIP);
	virtual void BanIP(const unsigned int dwIP);
	virtual void UnbanIP(const unsigned int dwIP);
	virtual IServerSlot *GetServerSlotbyID( const unsigned char ucId ) const;
	virtual uint8 GetMaxClientID() const;
	virtual EMPServerType GetServerType() const;

	// interface _IServerServices ----------------------------------------------------------

	bool Send(CStream &stm,CIPAddress &ip);
	void UnregisterSlot(CIPAddress &ip);
	virtual void OnDestructSlot( const CServerSlot *inpServerSlot );
	void GetProtocolVariables(CNPServerVariables &sv);

	// -------------------------------------------------------------------------------------

	//!
	bool Init(IServerSlotFactory *pFactory,WORD nPort, bool local);

	//! return the associated IServerSlotFactory interface
	IServerSlotFactory *GetServerSlotFactory() { return m_pFactory; };
	//!
	CServerSlot *GetPacketOwner(CIPAddress &ip);
	//!
	void RegisterLocalServerSlot(CServerSlot *pSlot,CIPAddress &ip);
	//!
	void GetMemoryStatistics(ICrySizer *pSizer);

	//! Returns server port.
	int GetServerPort() { return m_wPort; };

	//! check if an ip address is lan or inet
	bool IsLANIP(const CIPAddress &ip);

private:
	//!
	void DispatchToServerSlots(CNP &cnp,CStream &stm, CIPAddress &ip);
	//!
	void ProcessSetup(CNP &cnp,CStream &stmStream,CIPAddress &ip);
	//!
	void ProcessInfoRequest(CStream &stmIn, CIPAddress &ip);
	void ProcessInfoXMLRequest(CStream &stmIn, CIPAddress &ip);
	//!
	void ProcessPacket(CStream &stmPacket,CIPAddress &ip);
	//!
	void ProcessMulticastPacket(CStream &stmPacket,CIPAddress &ip);
	//!
	unsigned char GenerateNewClientID();


	typedef std::map<unsigned char,INetworkPacketSink *> TPacketSinks;


	unsigned char							m_cLastClientID;				//!<
	CDatagramSocket						m_socketMain;						//!<
	CDatagramSocket						m_socketMulticast;			//!<
	bool											m_bMulticastSocket;			//!<

protected:
	SLOTS_MAP									m_mapSlots;							//!<
	IServerSlotFactory *			m_pFactory;							//!<

private:
	CNPServerVariables				m_ServerVariables;			//!<
	
	unsigned int							m_nCurrentTime;					//!<
	CNetwork *								m_pNetwork;							//!<
	WORD											m_wPort;								//!<
	bool											m_bListen;							//!<
	IServerSecuritySink				*m_pSecuritySink;

	TPacketSinks							m_PacketSinks;					//!< <inPacketID,callback interface>
	EMPServerType							m_MPServerType;					//!< depends on sv_Servertype at Init() time

	// -----------------------------------------------------------------------------------

#ifdef _INTERNET_SIMULATOR
	struct DelayedPacket2
	{
		float m_fTimeToSend;
		BYTE data[8192];
		int len;
		CIPAddress *address;
	};
	typedef std::list<DelayedPacket2*> TDelayPacketList2;

	TDelayPacketList2					m_delayedPacketList;		//!<
#endif
};

#endif //_SERVER_H_
