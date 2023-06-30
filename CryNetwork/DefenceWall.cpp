////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   DefenceWall.cpp
//  Version:     v1.00
//  Created:     25/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DefenceWall.h"
#include "Network.h"

#include "Client.h"
#include "Server.h"
#include "ServerSlot.h"

#include "ICryPak.h"

//#define LOGEVENTS							// don't use in public release

enum CHEAT_PROTECTION_LEVEL {
	CHEAT_LEVEL_DEFAULT = 1,
	CHEAT_LEVEL_RANDOMCHECKS,
	CHEAT_LEVEL_CODE,
};

// Max number of retried for single request.
#define MAX_CHECK_RETRIES 5
// Time in seconds during which client must return response to server.
#define WAIT_FOR_RESPONSE_TIME 20
// Check about every 5 mins.
#define RANDOM_CHECK_PERIOD 60*3

enum EDefenceWallRequestCode
{
	DEFWALL_UNKNOWN = 0,
	DEFWALL_CHECK_PAK = 1,
	DEFWALL_CHECK_PROTECTED_FILE = 2,
	DEFWALL_CHECK_DLL_CODE = 3,
};

//////////////////////////////////////////////////////////////////////////
CDefenceWall::CDefenceWall( CNetwork *pNetwork )
{
	m_pNetwork = pNetwork;
	m_pSystem = GetISystem();
	m_nNextRequestId = 1;
	m_pServer = 0;
	m_bServer = false;
	m_bLog = true;

	m_nNumOpenedPacksOnServer = 0;

	m_nEncryptKey[0] = 2962387638;
	m_nEncryptKey[1] = 1782685322;
	m_nEncryptKey[2] = 268651613;
	m_nEncryptKey[3] = 156356231;

#if defined(WIN64) || defined(LINUX64)
	m_b64bit = true;
#else
	m_b64bit = false;
#endif
}

//////////////////////////////////////////////////////////////////////////
CDefenceWall::~CDefenceWall()
{
	ClearAllPendingRequests();
	m_protectedFiles.clear();
	ClearClients();
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::SetServer( CServer *pServer )
{
	m_pServer = pServer;
	m_pClient = 0;
	m_bServer = true;
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::SetClient( CClient *pClient )
{
	m_pServer = 0;
	m_pClient = pClient;
	m_bServer = false;
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ClearClients()
{
	for (Clients::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		delete *it;
	}
	m_clients.clear();
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ClearAllPendingRequests()
{
	for (PendingChecks::iterator it = m_pendingChecks.begin(); it != m_pendingChecks.end(); it++)
	{
		SClientCheckContext* pCtx = *it;
		delete pCtx;
	}
	m_pendingChecks.clear();
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::FillStdServerProbes()
{
	m_stdServerProbes.clear();
	m_stdServerProbes.reserve(30);
	IDataProbe *pProbe = GetISystem()->GetIDataProbe();
	ICryPak::PakInfo* pPakInfo = m_pSystem->GetIPak()->GetPakInfo();

	unsigned int i;
	//////////////////////////////////////////////////////////////////////////
	// Collect PAK files.
	//////////////////////////////////////////////////////////////////////////
	m_nNumOpenedPacksOnServer = pPakInfo->numOpenPaks;
	for (i = 0; i < pPakInfo->numOpenPaks; i++)
	{
		SClientCheckContext ctx;
		ctx.nNumOpenedPaks = pPakInfo->numOpenPaks;
		if (ServerCreateFileProbe( pPakInfo->arrPaks[i].szFilePath,ctx,true ))
		{
			m_stdServerProbes.push_back(ctx);
		}
	}
	m_pSystem->GetIPak()->FreePakInfo( pPakInfo );

	// Do not compare user DLL`s.
	/*
#if !defined(LINUX)
	//////////////////////////////////////////////////////////////////////////
	// Collect Modules Code probes.
	//////////////////////////////////////////////////////////////////////////
	// Create module probe for all DLL modules.
	IDataProbe::SModuleInfo *pModules;
	int numModules = pProbe->GetLoadedModules( &pModules );
	for (int m = 0; m < numModules; m++)
	{
		SClientCheckContext ctx;
		if (ServerCreateModuleProbe( pModules[m].filename.c_str(),ctx ))
			m_stdServerProbes.push_back(ctx);
	}
#endif
	*/
}

//////////////////////////////////////////////////////////////////////////
bool CDefenceWall::ServerCreateModuleProbe( const char *sFilename,SClientCheckContext &ctx )
{
	char sModule[_MAX_PATH];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath( sFilename,0,0,fname,ext );
	_makepath( sModule,0,0,fname,ext );
	strlwr( sModule );

	// Skip comparing render dll code. (Can be OGL,D3D or NULL)
	if (strstr(sModule,"render"))
		return false;

	AddProtectedFile( (string("b")+"i"+"n"+"3"+"2"+"/"+sFilename).c_str() );

	if (m_pNetwork->GetCheatProtectionLevel() >= CHEAT_LEVEL_CODE)
	{
		ctx.nRequestCode = DEFWALL_CHECK_DLL_CODE;
		ctx.bExecutableCode = true;
		ctx.probe.sFilename = sModule;
#if !defined(LINUX)
		ctx.nFilenameHash = m_pSystem->GetIDataProbe()->GetHash( sModule );
		ctx.probe.nCodeInfo = rand()%3;
		if (!m_pSystem->GetIDataProbe()->GetRandomModuleProbe( ctx.probe ))
			return false;
		if (!m_pSystem->GetIDataProbe()->GetCode( ctx.probe ))
			return false;
#endif
#ifdef LOGEVENTS
		if (m_bLog) CryLog( "<DefenceWall> CreatedExeProbe[%d] %s %I64x",ctx.nRequestId,ctx.probe.sFilename.c_str(),ctx.probe.nCode );
#endif
	}
	else
	{
		return false;
	}


	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CDefenceWall::ServerCreateFileProbe( const char *sFilename,SClientCheckContext &ctx,bool bRandomPart )
{
#if !defined(LINUX)
	// Get current language.
	ICVar *pLanguage=m_pSystem->GetIConsole()->GetCVar("g_language");
	if (pLanguage && pLanguage->GetString())
	{
		// Skip validation of language specific paks.
		char fname[_MAX_FNAME];
		_splitpath( sFilename,NULL,NULL,fname,NULL );
		if (strnicmp(fname,pLanguage->GetString(),strlen(pLanguage->GetString())) == 0)
		{
			// This is language pak... skip checking it.
			return false;
		}
	}

	string file = sFilename;
	GetRelativeFilename(file);

	ctx.nRequestCode = DEFWALL_CHECK_PAK;
	ctx.bExecutableCode = false;
	ctx.probe.sFilename = file;
	ctx.nFilenameHash = m_pSystem->GetIDataProbe()->GetHash( file.c_str() );
	if (bRandomPart)
	{
		ctx.probe.nCodeInfo = rand()%3;
		if (!m_pSystem->GetIDataProbe()->GetRandomFileProbe( ctx.probe,true ))
			return false;
		if (!m_pSystem->GetIDataProbe()->GetCode( ctx.probe ))
			return false;
	}
	else
	{
		ctx.probe.nCodeInfo = rand()%3;
		ctx.probe.nSize = 0xFFFFFFFF;
		if (!m_pSystem->GetIDataProbe()->GetCode( ctx.probe ))
			return false;
	}
#ifdef LOGEVENTS
	if (m_bLog) CryLog( "<DefenceWall> CreatedFileProbe[%d] %s %I64x",ctx.nRequestId,ctx.probe.sFilename.c_str(),ctx.probe.nCode );
#endif
#endif
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::FirstTimeClientValidation( ClientInfo *pClientInfo )
{
	CStream outstream;

	for (unsigned int i = 0; i < m_stdServerProbes.size(); i++)
	{
		SClientCheckContext &ctx = m_stdServerProbes[i];
		if (ctx.bExecutableCode && pClientInfo->b64bit != m_b64bit) // Cannot compare clients with 32/64bit difference.
			continue;
		IssueRequest( pClientInfo,outstream,ctx );
	}
	// Send network request to this client IP.
	SendSecurityQueryToClient( pClientInfo->ip,outstream );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::RandomClientValidation( ClientInfo *pClientInfo )
{
	CStream outstream;
	unsigned int n = 0;

	// Ignore if no standart probes yet.
	if (m_stdServerProbes.empty())
		return;

	bool bRepeat = false;
	while (bRepeat) {
		bRepeat = false;
		n = rand() % m_stdServerProbes.size();
		assert( n < m_stdServerProbes.size() );
		if (n >= m_stdServerProbes.size())
			bRepeat = true;
		if (m_stdServerProbes[n].bExecutableCode && pClientInfo->b64bit != m_b64bit) // Cannot compare clients with 32/64bit difference.
			bRepeat = true;
	}

	SClientCheckContext &ctx = m_stdServerProbes[n];
	IssueRequest( pClientInfo,outstream,ctx );

	// Send network request to this client IP.
	SendSecurityQueryToClient( pClientInfo->ip,outstream );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::IssueRequest( ClientInfo *pClientInfo,CStream &outstream,SClientCheckContext &ctx )
{
	CTimeValue currTime = m_pNetwork->GetCurrentTime();
	//////////////////////////////////////////////////////////////////////////
	// Schedule new random check time for this client.
	CTimeValue timediff;
	timediff.SetSeconds( RANDOM_CHECK_PERIOD + (RANDOM_CHECK_PERIOD*rand())/RAND_MAX );
	if (m_pNetwork->GetCheatProtectionLevel() == 5)
		timediff.SetSeconds( 2 );
	pClientInfo->nextRandomCheckTime = currTime + timediff;
	//////////////////////////////////////////////////////////////////////////

	ctx.clientIP = pClientInfo->ip;
	if (ctx.nRetries == 0) // If repeating request do not assign new requestId.
	{
		ctx.nRequestId = m_nNextRequestId++;
	}
	ctx.requestTime = currTime;
	pClientInfo->lastRequestTime = currTime;
	SClientCheckContext *pCtx = new SClientCheckContext;
	*pCtx = ctx;

	if (ctx.nRetries == 0)
	{
		m_pendingChecks.push_back(pCtx);
	}

#ifdef LOGEVENTS
	if (m_bLog) CryLog( "<DefenceWall> IssueRequest[%d] %s %I64x (ofs=%d,size=%d)",pCtx->nRequestId,pCtx->probe.sFilename.c_str(),pCtx->probe.nCode,pCtx->probe.nOffset,pCtx->probe.nSize );
#endif

	CStream stm;
	WriteStreamRequest( *pCtx,stm );

	outstream.Write( stm );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::WriteStreamRequest( SClientCheckContext &ctx,CStream &stm )
{
	stm.Write( ctx.nRequestCode );
	stm.WritePacked( ctx.nRequestId );
	stm.WritePacked( ctx.nFilenameHash );
	stm.WritePacked( ctx.probe.nCodeInfo );
	stm.WritePacked( ctx.probe.nOffset );
	stm.WritePacked( ctx.probe.nSize );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ReadStreamRequest( SClientCheckContext &ctx,CStream &stm )
{
	stm.Read( ctx.nRequestCode );
	stm.ReadPacked( ctx.nRequestId );
	stm.ReadPacked( ctx.nFilenameHash );
	stm.ReadPacked( ctx.probe.nCodeInfo );
	stm.ReadPacked( ctx.probe.nOffset );
	stm.ReadPacked( ctx.probe.nSize );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::WriteStreamResponse( SClientCheckContext &ctx,CStream &stm )
{
	// Also crypt stream here.
	stm.Write( ctx.nRequestCode );
	stm.WritePacked( ctx.nRequestId );
	stm.WritePacked( ctx.nNumOpenedPaks );
	stm.WritePacked( ctx.nClientStatusFlags );
	stm.WriteBits( (BYTE*)&ctx.probe.nCode,64 ); // write int64
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ReadStreamResponse( SClientCheckContext &ctx,CStream &stm )
{
	stm.Read( ctx.nRequestCode );
	stm.ReadPacked( ctx.nRequestId );
	stm.ReadPacked( ctx.nNumOpenedPaks );
	stm.ReadPacked( ctx.nClientStatusFlags );
	stm.ReadBits( (BYTE*)&ctx.probe.nCode,64 ); // read int64
}

//////////////////////////////////////////////////////////////////////////
SClientCheckContext* CDefenceWall::FindRequest( int nRequestId )
{
	for (PendingChecks::iterator it = m_pendingChecks.begin(); it != m_pendingChecks.end(); it++)
	{
		SClientCheckContext* pCtx = *it;
		if (pCtx->nRequestId == nRequestId)
		{
			return pCtx;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::RemoveRequest( SClientCheckContext* pCtx )
{
	for (PendingChecks::iterator it = m_pendingChecks.begin(); it != m_pendingChecks.end(); it++)
	{
		if (pCtx == *it)
		{
			m_pendingChecks.erase(it);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::UnifyFilename( string &sFilename )
{
	string file = sFilename;
	std::replace( file.begin(),file.end(),'\\','/' );
	strlwr( const_cast<char*>(file.c_str()) );
	sFilename = file;
}

void CDefenceWall::GetRelativeFilename( string &sFilename )
{
	UnifyFilename(sFilename);

	// Get current folder.
	char szCurrDir[_MAX_PATH];
	GetCurrentDirectory( sizeof(szCurrDir),szCurrDir );
	string sCurDir = szCurrDir;

	UnifyFilename(sCurDir);

	// Get relative path.
	if (strncmp(sFilename.c_str(),sCurDir.c_str(),sCurDir.size()) == 0 && sFilename.size()+1 >= sCurDir.size())
	{
		// First part of file name is current dir.
		sFilename = sFilename.substr(sCurDir.size()+1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::EncryptStream( CStream &stm )
{
	unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
	int len = stm.GetSize()/8;
	if (len >= 8)
	{
		TEA_ENCODE( pBuffer,pBuffer,TEA_GETSIZE(len),m_nEncryptKey );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::DecryptStream( CStream &stm )
{
	unsigned int* pBuffer = (unsigned int*)stm.GetPtr();
	int len = stm.GetSize()/8;
	if (len >= 8)
	{
		TEA_DECODE( pBuffer,pBuffer,TEA_GETSIZE(len),m_nEncryptKey );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::SendSecurityQueryToClient( CIPAddress &clientIP,CStream &stm )
{
	if (m_pServer)
	{
		CServerSlot *pSlot = m_pServer->GetPacketOwner( clientIP );
		if (pSlot)
		{
			EncryptStream(stm);
#ifdef LOGEVENTS
			if (m_bLog) CryLog( "<DefenceWall> Sending Stream to Client %d bytes",stm.GetSize()/8 );
#endif
			pSlot->SendSecurityQuery(stm);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::SendSecurityRespToServer( CStream &stm )
{
	if (m_pClient)
	{
		EncryptStream(stm);
		m_pClient->SendSecurityResponse(stm);
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::OnServerRequest( CStream &stm )
{
	DecryptStream(stm);
	m_clientOutputStream.Reset();
	while (!stm.EOS())
	{
		// When client recieves server request.
		SClientCheckContext ctx;
		ReadStreamRequest( ctx,stm );

		// Perform required check on client.
		switch (ctx.nRequestCode)
		{
		case DEFWALL_CHECK_PAK:
		case DEFWALL_CHECK_PROTECTED_FILE:
		case DEFWALL_CHECK_DLL_CODE:
			OnValidateClientContext( ctx );
			break;
		default:
			// Unknown server request.
#ifdef LOGEVENTS
			if (m_bLog) CryLog( "Unknown Server Request Code" );
#endif
			break;
		}
	}
	if (m_clientOutputStream.GetSize() > 0)
	{
		// Send client Outputstring back to server.
		SendSecurityRespToServer( m_clientOutputStream );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::OnClientResponse( CIPAddress &clientIP,CStream &stm )
{
	// When server recieves client response.
	DecryptStream(stm);

	int nServerStatusFlags = GetSystemStatusFlags();
	
	while (!stm.EOS())
	{
		SClientCheckContext clientResponse;
		ReadStreamResponse( clientResponse,stm );

		SClientCheckContext &ctx = clientResponse;
#ifdef LOGEVENTS
		if (m_bLog) CryLog( "<DefenceWall> OnClientResponse[%d] %s %I64x",ctx.nRequestId,ctx.probe.sFilename.c_str(),ctx.probe.nCode );
#endif

		if (clientResponse.nClientStatusFlags != nServerStatusFlags)
		{
#ifdef LOGEVENTS
			if (m_bLog) CryLog( "<DefenceWall> Wrong Vars! %d!=%d",clientResponse.nClientStatusFlags,nServerStatusFlags );
#endif
			// Client have different vars set.
			PunkDetected( clientIP,IServerSecuritySink::CHEAT_MODIFIED_VARS );
			return;
		}

		// Server here checks that client response is valid and validation code match the one stored on server.
		SClientCheckContext *pServerCtx = FindRequest( clientResponse.nRequestId );
		if (pServerCtx)
		{
			// Compare client code.
			if (pServerCtx->probe.nCode != clientResponse.probe.nCode)
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> Wrong Code! %d!=%d",clientResponse.probe.nCode,pServerCtx->probe.nCode );
#endif
				// Wrong Code!
				PunkDetected( clientIP,(pServerCtx->bExecutableCode)?IServerSecuritySink::CHEAT_MODIFIED_CODE:IServerSecuritySink::CHEAT_MODIFIED_FILE );
				return;
			}
			else if (pServerCtx->nRequestCode == DEFWALL_CHECK_PAK && pServerCtx->nNumOpenedPaks != clientResponse.nNumOpenedPaks)
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> Wrong Num Paks! %d!=%d",clientResponse.nNumOpenedPaks,pServerCtx->nNumOpenedPaks );
#endif
				// Client have different number of open packs then server!
				PunkDetected( clientIP,IServerSecuritySink::CHEAT_MODIFIED_FILE );
				return;
			}
		}
		else
		{
			// Client send request code that server already doesnt have.
			if (clientResponse.nRequestId >= m_nNextRequestId)
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> Too Big Request Id! %d!=%d",clientResponse.nClientStatusFlags,nServerStatusFlags );
#endif
				// Possible cheating with the protocol... Client cannot recieve such a big request id before server issues it.
				PunkDetected( clientIP,IServerSecuritySink::CHEAT_NET_PROTOCOL );
				return;
			}
			else
			{
				// this request code was already checked.
			}
		}
		// Remove request from list.
		RemoveRequest( pServerCtx );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::OnValidateClientContext( SClientCheckContext &ctx )
{
#if defined(LINUX)
	return;
#else
	bool bGotProbe = true;
	// Perform required check on client.
	switch (ctx.nRequestCode)
	{
	case DEFWALL_CHECK_PAK:
		{
			// Find out which filename to check.
			ICryPak::PakInfo* pPakInfo = m_pSystem->GetIPak()->GetPakInfo();
			ctx.nNumOpenedPaks = pPakInfo->numOpenPaks;
			for (unsigned int i = 0; i < pPakInfo->numOpenPaks; i++)
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> PAK: %s",pPakInfo->arrPaks[i].szFilePath );
#endif

				string file = pPakInfo->arrPaks[i].szFilePath;
				GetRelativeFilename(file);
				u32 nFilenameHash = m_pNetwork->GetStringHash(file.c_str());
				if (nFilenameHash == ctx.nFilenameHash)
				{
					// Refering to the same file.
					ctx.probe.sFilename = file;
					break;
				}
			}
			m_pSystem->GetIPak()->FreePakInfo( pPakInfo );
		}
		break;
	case DEFWALL_CHECK_PROTECTED_FILE:
		{
			for (unsigned int i = 0; i < m_protectedFiles.size(); i++)
			{
				ProtectedFile &pf = m_protectedFiles[i];
				if (pf.nFilenameHash == ctx.nFilenameHash)
				{
					// We are refering to the same file.
					ctx.probe.sFilename = pf.filename;
					break;
				}
			}
		}
		break;
	case DEFWALL_CHECK_DLL_CODE:
		{
			// Create module probe for all DLL modules.
			IDataProbe::SModuleInfo *pModules;
			int numModules = m_pSystem->GetIDataProbe()->GetLoadedModules( &pModules );
			for (int i = 0; i < numModules; i++)
			{
				char sModule[_MAX_PATH];
				char fname[_MAX_FNAME];
				char ext[_MAX_EXT];
				_splitpath( pModules[i].filename.c_str(),0,0,fname,ext );
				_makepath( sModule,0,0,fname,ext );
				strlwr( sModule );

				u32 nFilenameHash = m_pSystem->GetIDataProbe()->GetHash( sModule );
				if (ctx.nFilenameHash == nFilenameHash)
				{
					ctx.probe.sFilename = sModule;
					bGotProbe = m_pSystem->GetIDataProbe()->GetModuleProbe(ctx.probe);
				}
			}
		}
		break;
	default:
		// Unknown request...
#ifdef LOGEVENTS
		if (m_bLog) CryLog( "Server Sent Unknown request" );
#endif
		return;
	}

	// initialize code to be random.
	ctx.probe.nCode = ((u64)rand()) | (((u64)rand())<<16)  | (((u64)rand())<<32) | (((u64)rand())<<48);

	// Calc hash code of this file.
	if (bGotProbe && m_pSystem->GetIDataProbe()->GetCode( ctx.probe ))
	{
		// Code retrieved.
	}
	else
	{
		// Failed...
	}
	// Send code back to server.

#ifdef LOGEVENTS
	if (m_bLog) CryLog( "<DefenceWall> OnServerRequest[%d] %s %I64x (ofs=%d,size=%d)",ctx.nRequestId,ctx.probe.sFilename.c_str(),ctx.probe.nCode,ctx.probe.nOffset,ctx.probe.nSize );
#endif

	// Every response should contain these.
	ctx.nClientStatusFlags = GetSystemStatusFlags();
	WriteStreamResponse( ctx,m_clientOutputStream );
#endif //LINUX
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ClearProtectedFiles()
{
	//m_bLog = m_pNetwork->GetLogLevel() == 1024;

	ClearAllPendingRequests();
	m_protectedFiles.clear();

	if (m_pNetwork->GetCheatProtectionLevel() > 0)
		FillStdServerProbes();

	//////////////////////////////////////////////////////////////////////////
	// Add default protected files.
	//////////////////////////////////////////////////////////////////////////
	// Bin32\FarCry.exe
	//AddProtectedFile( (string("b")+"i"+"n"+"3"+"2"+"/"+"f"+"a"+"r"+"c"+"r"+"y"+"."+"e"+"x"+"e").c_str() );

	// Bin32\XRenderD3D9.exe
	//AddProtectedFile( (string("b")+"i"+"n"+"3"+"2"+"/"+"x"+"r"+"e"+"n"+"d"+"e"+"r"+"d"+"3"+"d"+"9"+"."+"d"+"l"+"l").c_str() );
	// Bin32\XRenderNULL.exe
	//AddProtectedFile( (string("b")+"i"+"n"+"3"+"2"+"/"+"x"+"r"+"e"+"n"+"d"+"e"+"r"+"n"+"u"+"l"+"l"+"."+"d"+"l"+"l").c_str() );
	// Bin32\XRenderOGL.exe
	//AddProtectedFile( (string("b")+"i"+"n"+"3"+"2"+"/"+"x"+"r"+"e"+"n"+"d"+"e"+"r"+"o"+"g"+"l"+"."+"d"+"l"+"l").c_str() );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::AddProtectedFile( const char *sFilename )
{
#if !defined(LINUX)
	string file = sFilename;
	UnifyFilename( file );

	ProtectedFile pf;
	pf.filename = file;
	pf.nFilenameHash = m_pNetwork->GetStringHash(pf.filename.c_str());

	// Create also STD server probe if on server.
	if (m_bServer && m_pNetwork->GetCheatProtectionLevel() > 0)
	{
		// Calc hash code of this file.
		SDataProbeContext probe;
		probe.nCodeInfo = rand()%3;
		probe.nOffset = 0;
		probe.nSize = 0xFFFFFFFF; // all file.
		probe.sFilename = file;
		if (!m_pSystem->GetIDataProbe()->GetRandomFileProbe( probe,false ))
			return;
		if (!m_pSystem->GetIDataProbe()->GetCode( probe ))
			return;
		pf.nHashCode = probe.nCode;

		SClientCheckContext ctx;
		ctx.nRequestCode = DEFWALL_CHECK_PROTECTED_FILE;
		ctx.bExecutableCode = false;
		ctx.nFilenameHash = pf.nFilenameHash;
		ctx.probe = probe;
		m_stdServerProbes.push_back(ctx);

#ifdef LOGEVENTS
		if (m_bLog) CryLog( "<DefenceWall> CreatedFileProbe[%d] %s %I64x",ctx.nRequestId,ctx.probe.sFilename.c_str(),ctx.probe.nCode );
#endif
	}

	m_protectedFiles.push_back(pf);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::ServerUpdate()
{
	if (!m_bServer)
		return;

//	m_bLog = m_pNetwork->GetLogLevel() == 1024;

	// Ignore if no standart probes yet.
	if (m_stdServerProbes.empty())
		return;

	CTimeValue currTime = m_pNetwork->GetCurrentTime();
	float fCurrSeconds = currTime.GetSeconds();

	std::vector<CIPAddress> notRespondingClients;
	notRespondingClients.clear();

	PendingChecks::iterator it,next;
	// See if any requests expired and were not responded.
	for (it = m_pendingChecks.begin(); it != m_pendingChecks.end(); it = next)
	{
		next = it; next++;
		SClientCheckContext &ctx = *(*it);
		// Check how long request is pending already.
		float fRequestTime = ctx.requestTime.GetSeconds();
		if (fCurrSeconds - fRequestTime > WAIT_FOR_RESPONSE_TIME)
		{
			// More then response seconds since request.
			if (ctx.nRetries < MAX_CHECK_RETRIES)
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> Repeat Request[%d] (Retry:%d)",ctx.nRequestId,ctx.nRetries );
#endif
				ctx.nRetries++;
				ClientInfo *pClientInfo = FindClientInfo(ctx.clientIP);
				if (pClientInfo)
				{
					// Try to issue the same request again.
					CStream outstream;
					IssueRequest( pClientInfo,outstream,ctx );
					SendSecurityQueryToClient( ctx.clientIP,outstream );
				}
				else
				{
#ifdef LOGEVENTS
					if (m_bLog) CryLog( "<DefenceWall> Delete Request[%d] Unknown Client!",ctx.nRequestId );
#endif
					m_pendingChecks.erase(it); // something wrong with this request, not client for this ip.
				}
			}
			else
			{
#ifdef LOGEVENTS
				if (m_bLog) CryLog( "<DefenceWall> No Resonse from Client for Request(%d)",ctx.nRequestId);
#endif
				// 3 Requests without an answer.... (assume cheating client).
				notRespondingClients.push_back( ctx.clientIP );
				m_pendingChecks.erase(it);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	{
		// If any client not responding.
		for (unsigned int i = 0; i < notRespondingClients.size(); i++)
		{
			PunkDetected( notRespondingClients[i],IServerSecuritySink::CHEAT_NOT_RESPONDING );
		}
	}

	if (m_pNetwork->GetCheatProtectionLevel() >= CHEAT_LEVEL_RANDOMCHECKS)
	{
		//////////////////////////////////////////////////////////////////////////
		// Go over clients and check if they need random check yet.
		for (Clients::const_iterator clit = m_clients.begin(); clit != m_clients.end(); ++clit)
		{
			ClientInfo *ci = *clit;
			if (currTime > ci->nextRandomCheckTime)
			{
				// Make random request to this client.
				RandomClientValidation( ci );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::OnAddClient( CIPAddress &clientIP )
{
	// Check if this client already added.
	for (Clients::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		// This client already added.
		if ((*it)->ip == clientIP)
			return;
	}

	bool b64bit = false;
	CServerSlot *pSlot = m_pServer->GetPacketOwner( clientIP );
	if (pSlot)
	{
		if (pSlot->IsLocalSlot())
			return;
		b64bit = pSlot->GetClientFlags() & CLIENT_FLAGS_64BIT;
	}
	ClientInfo *ci = new ClientInfo;
	ci->lastRequestTime = 0;
	ci->nextRandomCheckTime = 0;
	ci->ip = clientIP;
	ci->b64bit = b64bit;
	
	m_clients.push_back( ci );

	// Start Validation.
	FirstTimeClientValidation( ci );
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::OnDisconnectClient( CIPAddress &clientIP )
{
	// Remove all requests to this ip.
	PendingChecks::iterator next;
	for (PendingChecks::iterator it = m_pendingChecks.begin(); it != m_pendingChecks.end(); it = next)
	{
		next = it; next++;
		SClientCheckContext* pCtx = *it;
		if (pCtx->clientIP == clientIP)
		{
			// This ip should be removed.
			delete pCtx;
			next = m_pendingChecks.erase(it);
		}
	}
	for (Clients::iterator clit = m_clients.begin(); clit != m_clients.end(); ++clit)
	{
		ClientInfo *ci = *clit;
		if (ci->ip == clientIP)
		{
			m_clients.erase(clit);
			delete ci;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CDefenceWall::ClientInfo* CDefenceWall::FindClientInfo( CIPAddress &clientIP  ) const
{
	for (Clients::const_iterator clit = m_clients.begin(); clit != m_clients.end(); ++clit)
	{
		ClientInfo *ci = *clit;
		if (ci->ip == clientIP)
		{
			return ci;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CDefenceWall::PunkDetected( CIPAddress &clientIP,int type )
{
	ClientInfo *ci = FindClientInfo(clientIP);
	if (ci)
	{
		ci->bSuspectedPunk = true;
	}
	m_pNetwork->PunkDetected( clientIP );
	if (m_pServer)
	{
		if (m_pServer->GetSecuritySink())
		{
			m_pServer->GetSecuritySink()->CheaterFound( clientIP.GetAsUINT(),type,"" );
		}
	}
}

enum EDFSystemStatusFlags
{
	SYS_STATUS_FORCE_NONDEVMODE		= 0x0001,
	SYS_STATUS_IN_DEVMODE					= 0x0002,
	SYS_STATUS_WAS_IN_DEVMODE			= 0x0004,
	SYS_STATUS_PAK_PRIORITY				= 0x0008,
};
//////////////////////////////////////////////////////////////////////////
int CDefenceWall::GetSystemStatusFlags()
{
	int flags = 0;
	if (GetISystem()->GetForceNonDevMode())
		flags |= SYS_STATUS_FORCE_NONDEVMODE;
	// Check dev mode.
	if (GetISystem()->IsDevMode())
		flags |= SYS_STATUS_IN_DEVMODE;
	// Check if was started in dev mode.
	if (GetISystem()->WasInDevMode())
		flags |= SYS_STATUS_WAS_IN_DEVMODE;
	ICVar *pVar = GetISystem()->GetIConsole()->GetCVar("sys_PakPriority");
	if (pVar && pVar->GetIVal() != 0)
		flags |= SYS_STATUS_PAK_PRIORITY;

	return flags;
}
