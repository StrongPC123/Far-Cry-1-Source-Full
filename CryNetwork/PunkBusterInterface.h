////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PunkBusterInterface.h
//  Version:     v1.00
//  Created:     1/3/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Interface to the PunkBuster from CryEngine.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(NOT_USE_PUNKBUSTER_SDK)

#ifndef __PunkBusterInterface_h__
#define __PunkBusterInterface_h__
#pragma once

#include "IConsole.h"
#include "../PunkBuster/pbcommon.h"

class CNetwork;
class CServer;
class CClient;
class CClientLocal;
class CServerSlot;

/*!	Wrapper arround PunkBuster.
*/
class CPunkBusterInterface : public IConsoleVarSink, public IOutputPrintSink
{
public:
	CPunkBusterInterface( CNetwork *pNetwork );
	~CPunkBusterInterface();

	//! Called when initializing client and server.
	void Init( bool bClient , bool isLocalServer );
	//! Called when shut downing client and server.
	void ShutDown( bool bClient );

	// This is server.
	void SetServer( CServer *pServer );
	// This is client.
	void SetClient( CClient *pClient );
	void SetClientLocal( CClientLocal *pClient );

	//! Locks the punkbuster cvars 
	void LockCVars();

	//! Unlocks the punkbuster cvars
	void UnlockCVars();

	//! Updates PunkBuster, called every frame.
	void Update( bool bClient );
	
	//! Called when message from server or client recieved.
	void OnCCPPunkBusterMsg( CIPAddress &ipAddress,CStream &stm );

	//! When new client joins server.
	void OnAddClient( CIPAddress &clientIP );
	//! When client disconnect server.
	void OnDisconnectClient( CIPAddress &clientIP );

	bool CheckPBPacket(CStream &stmPacket,CIPAddress &ip);
	void ValidateClient( CServerSlot *pSlot );

	//////////////////////////////////////////////////////////////////////////
	// IConsoleVarSink
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnBeforeVarChange( ICVar *pVar,const char *sNewValue );
	//////////////////////////////////////////////////////////////////////////
	// IOutputPrintSink
	//////////////////////////////////////////////////////////////////////////
	virtual void Print( const char *inszText );
	//////////////////////////////////////////////////////////////////////////
	
//private:
	bool ReadStringFromRegistry(const string &szKeyName, const string &szValueName, string &szValue);
	bool LoadCDKey( string &sCDKey );

	void CheaterFound( CIPAddress &clientIP,int type,const char *sMsg );
	void SendMsgToClient( CIPAddress &clientIP,CStream &stm );
	void SendMsgToServer( CStream &stm );

//private:
	ISystem *m_pSystem;
	CNetwork *m_pNetwork;

	CServer* m_pServer;
	CClient* m_pClient;
	CClientLocal* m_pClientLocal;

	bool m_bClInitialized , m_bSvInitialized , m_bSinglePlayer ;
	ICVar *cl_punkbuster;
	ICVar *sv_punkbuster;
	ICVar *fs_homepath;
	ICVar *sys_punkbuster_loaded;
};


#endif // __PunkBusterInterface_h__
#endif // NOT_USE_PUNKBUSTER_SDK
