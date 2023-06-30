//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: Interfaces.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef _INTERFACES_H_
#define _INTERFACES_H_

class CServerSlot;

struct _IServerServices
{
	virtual bool Send(CStream &stm, CIPAddress &ip) = 0;
	virtual void UnregisterSlot(CIPAddress &ip) = 0;
	virtual void OnDestructSlot( const CServerSlot *inpServerSlot ) = 0;
	virtual void GetProtocolVariables(struct CNPServerVariables &sv) = 0;
};


struct _IEndpointUser
{
	virtual bool Send(CStream &stm) = 0;
	virtual void OnData(CStream &stm) = 0;
//	virtual unsigned char GetID() = 0;
};

struct _ICCPUser :
public _IEndpointUser
{
	//!
	virtual void OnCCPSetup(CStream &stm) = 0;
	//! this is a server->client packet
	virtual void OnCCPConnect(CStream &stm) = 0;
	//!
	virtual void OnCCPConnectResp(CStream &stm) = 0;
	//! this is a server->client packet
	virtual void OnCCPContextSetup(CStream &stm) = 0;
	//!
	virtual void OnCCPContextReady(CStream &stm) = 0;
	//! this is a server->client packet
	virtual void OnCCPServerReady() = 0;
	//!
	virtual void OnCCPDisconnect(const char *szCause) = 0;
	// Sends a security check message, client must respond to it.
	virtual void OnCCPSecurityQuery(CStream &stm) = 0;
	// Respond to a security message, will be checked on server.
	virtual void OnCCPSecurityResp(CStream &stm) = 0;
	// Punk buster message.
	virtual void OnCCPPunkBusterMsg(CStream &stm) = 0;
};

struct _IServerSlotServices: 
public _ICCPUser
{
	virtual void Start(unsigned char cClientID,CIPAddress &ip) = 0;
	virtual bool SendConnect() = 0;
	virtual bool SendContextSetup() = 0;
	virtual bool SendServerReady() = 0;
	virtual bool SendDisconnect(const char *szCause) = 0;
	virtual void OnConnect() = 0;
	virtual void OnContextReady() = 0;
	virtual void OnDisconnect(const char *szCause) = 0;
};


struct _IClientServices : 
public _ICCPUser
{
	virtual bool SendSetup() = 0;
	virtual bool SendConnectResp() = 0;
	virtual bool SendContextReady() = 0;
	virtual bool SendDisconnect(const char *szCause) = 0;
	virtual void OnConnect() = 0;
	virtual void OnContextSetup() = 0;
	virtual void OnServerReady() = 0;
	virtual void OnDisconnect(const char *szCause) = 0;
};

#endif //_INTERFACES_H_
