////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   DefenceWall.h
//  Version:     v1.00
//  Created:     25/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: ...
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DefenceWall_h__
#define __DefenceWall_h__

#if !defined(LINUX)
#pragma once
#endif

#include <TimeValue.h>
#include <IDataProbe.h>
#include <list>

class CNetwork;
class CServer;
class CClient;

struct SClientCheckContext
{
	// Sequential id of request.
	unsigned int nRequestId;
	// Time of last request.
	CTimeValue requestTime;
	// how many times this check was tried.
	int nRetries;
	// Ip of the client we are requesting.
	CIPAddress clientIP;
	// Hash of the filename to check.
	unsigned int nFilenameHash;
	// Request code (what check to do).
	unsigned char nRequestCode;
	// Number of opened pack files.
	unsigned int nNumOpenedPaks;
	// Probe data.
	SDataProbeContext probe;
	//! True if it is code request (it depends on 32/64bit versions)
	bool bExecutableCode;
	//! Various flags recieved from client.
	unsigned int nClientStatusFlags;

	SClientCheckContext()
	{
		bExecutableCode = false;
		nRequestCode = 0;
		nRequestId = 0;
		nRetries = 0;
		nNumOpenedPaks = 0;
		nFilenameHash = 0;
		nClientStatusFlags = 0;
	}
};

// No comments...
class CDefenceWall
{
public:
	CDefenceWall( CNetwork *pNetwork );
	~CDefenceWall();

	// This is server.
	void SetServer( CServer *pServer );
	// This is client.
	void SetClient( CClient *pClient );

	void ClearProtectedFiles();
	void AddProtectedFile( const char *sFilename );

	void ServerUpdate();

	//////////////////////////////////////////////////////////////////////////
	// Some important events.
	//////////////////////////////////////////////////////////////////////////
	void OnAddClient(  CIPAddress &clientIP );
	void OnDisconnectClient( CIPAddress &clientIP );
	void ClearClients();

	// When client recieves server request.
	void OnServerRequest( CStream &stm );
	// When server recieves client response.
	void OnClientResponse( CIPAddress &clientIP,CStream &stm );

private:
	// This structure describe connected client on server.
	struct ClientInfo
	{
		// Info about this client.
		CIPAddress ip;
		bool bSuspectedPunk;
		// He`s running 64bit version.
		bool b64bit;
		CTimeValue lastRequestTime;
		CTimeValue nextRandomCheckTime;
		ClientInfo() { bSuspectedPunk = false; b64bit = false; };
	};

	void SendSecurityQueryToClient( CIPAddress &clientIP,CStream &stm );
	void SendSecurityRespToServer( CStream &stm );

	// Called on client, in response to server request.
	void OnValidateClientContext( SClientCheckContext &ctx );

	// Validate data on specified client IP.
	void FirstTimeClientValidation( ClientInfo *pClientInfo );
	void RandomClientValidation( ClientInfo *pClientInfo );

	void IssueRequest( ClientInfo *pClientInfo,CStream &outstream,SClientCheckContext &ctx );
	void WriteStreamRequest( SClientCheckContext &ctx,CStream &stm );
	void ReadStreamRequest( SClientCheckContext &ctx,CStream &stm );
	void WriteStreamResponse( SClientCheckContext &ctx,CStream &stm );
	void ReadStreamResponse( SClientCheckContext &ctx,CStream &stm );
	SClientCheckContext* FindRequest( int nRequestId );
	void RemoveRequest( SClientCheckContext* pCtx );
	void ClearAllPendingRequests();

	// Make all filenames compatable.
	void UnifyFilename( string &sFilename );
	void GetRelativeFilename( string &sFilename );
	ClientInfo* FindClientInfo( CIPAddress &clientIP ) const;
	void PunkDetected( CIPAddress &clientIP,int type );

	void EncryptStream( CStream &stm );
	void DecryptStream( CStream &stm );

	void FillStdServerProbes();
	bool ServerCreateFileProbe( const char *sFilename,SClientCheckContext &ctx,bool bRandomPart );
	bool ServerCreateModuleProbe( const char *sFilename,SClientCheckContext &ctx );
	int GetSystemStatusFlags();

private:
	typedef std::list<SClientCheckContext*> PendingChecks;
	PendingChecks m_pendingChecks;
	CNetwork *m_pNetwork;
	unsigned int m_nNextRequestId;

	ISystem *m_pSystem;
	CServer *m_pServer;
	CClient *m_pClient;
	// True if server, false if client.
	bool m_bServer;
	bool m_b64bit; // Running in 64bit version.
	bool m_bLog;
	unsigned int m_nEncryptKey[4];

	// Output Stream.
	CStream m_clientOutputStream;

	struct ProtectedFile
	{
		string filename;
		u32 nFilenameHash;
		uint64 nHashCode;
	};
	// List of protected files.
	std::vector<ProtectedFile> m_protectedFiles;

	// List of connected clients.
	// For up to 32-64 clients, vector is oky.
	typedef std::vector<ClientInfo*> Clients;
	Clients m_clients;

	// List of Standart probes checked for each client, (stored on server only).
	// Updated on each call to ClearProtectedFiles.
	std::vector<SClientCheckContext> m_stdServerProbes;

	int m_nNumOpenedPacksOnServer;
};

#endif // __DefenceWall_h__

