// Network.h: interface for the CNetwork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORK_H__9C2C8689_2D3A_4729_9DBD_A8A930264655__INCLUDED_)
#define AFX_NETWORK_H__9C2C8689_2D3A_4729_9DBD_A8A930264655__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CompressionHelper.h"			// CCompressionHelper
#include "TimeValue.h"

class CServer;
class CClient;
class CServerSlotLocal;
class CServerSlot;
class CClientLocal;
class CDefenceWall;
struct IScriptSystem;
class NewUbisoftClient;
class CScriptObjectNewUbisoftClient;

#if !defined(NOT_USE_PUNKBUSTER_SDK)
class CPunkBusterInterface;
#endif

// server connection flags
// returned by the server on the m_cResponse field of the CCPConnect packet
#define SV_CONN_FLAG_PUNKBUSTER (1 << 0)	// punkbuster is on
#define SV_CONN_FLAG_DEVMODE		(1 << 1)	// devmode is on


typedef std::map<NRESULT,const char*> ERROR_MAP;
typedef ERROR_MAP::iterator ERROR_MAPItor;

typedef std::map<WORD,CServer *> MapServers;
typedef MapServers::iterator MapServersItor;

struct CryNetError{
	NRESULT nrErrorCode; 
	const char *sErrorDescription;
};

/*! module class factory
	@see INetwork
*/
class CNetwork :public INetwork
{
public:
	//! constructor
	CNetwork();
	//! destructor
	virtual ~CNetwork();

	//! \param pScriptSystem must not be 0
	bool Init( IScriptSystem *pScriptSystem );

	//! Get pointer to the system interface.
	ISystem* GetSystem() { return m_pSystem; };

	//! 
	//! unregister a server from the local servers map
	void UnregisterServer( WORD wPort );
	//!
	void UnregisterClient( IClient *pClient );
	//! connect a local client to a local serverslot
	//! this function is used to emulate a network connection on the same machine
	//! \param pClient
	//! \param wPort
	CServerSlotLocal *ConnectToLocalServerSlot( CClientLocal *pClient, WORD wPort );

	// interface INetwork -------------------------------------------------------------

	virtual DWORD GetLocalIP() const;
	virtual void SetLocalIP( const char *szLocalIP );
	virtual IClient *CreateClient(IClientSink *pSink,bool bLocal);
	virtual IServer *CreateServer(IServerSlotFactory *pFactory,WORD nPort, bool listen);
	virtual INETServerSnooper *CreateNETServerSnooper(INETServerSnooperSink *pSink);
	virtual IServerSnooper *CreateServerSnooper(IServerSnooperSink *pSink);
	virtual IRConSystem *CreateRConSystem();
	virtual const char *EnumerateError(NRESULT err);
	virtual void Release();
	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual ICompressionHelper *GetCompressionHelper();
	virtual void ClearProtectedFiles();
	virtual void AddProtectedFile( const char *sFilename );
	virtual IClient *GetClient();
	virtual IServer *GetServerByPort( const WORD wPort );
	virtual void UpdateNetwork();
	virtual void OnAfterServerLoadLevel( const char *szServerName, const uint32 dwPlayerCount, const WORD wPort );
	virtual bool VerifyMultiplayerOverInternet();
	virtual void Client_ReJoinGameServer();

	// -------------------------------------------------------------------------------

	bool IsPacketCompressionEnabled() const;

	//////////////////////////////////////////////////////////////////////////
	// Network update functions.
	//////////////////////////////////////////////////////////////////////////
	//! Called from update of client.
	void OnClientUpdate();
	//! Called from update of server.
	void OnServerUpdate();

	//////////////////////////////////////////////////////////////////////////
	// Defence Wall related methods.
	//////////////////////////////////////////////////////////////////////////
	void AddClientToDefenceWall( CIPAddress &clientIP );
	void RemoveClientFromDefenceWall( CIPAddress &clientIP );
	void PunkDetected( CIPAddress &ip );
	void OnSecurityMsgResponse( CIPAddress &ipAddress,CStream &stm );
	void OnSecurityMsgQuery( CStream &stm );
	void OnCCPPunkBusterMsg( CIPAddress &ipAddress,CStream &stm );

	CTimeValue GetCurrentTime();
	u32 GetStringHash( const char *szString );
	int GetLogLevel();
	int GetCheatProtectionLevel();
	bool CheckPBPacket(CStream &stmPacket,CIPAddress &ip);
	void ValidateClient( CServerSlot *pSlot );

	void InitPunkbusterClient(CClient *pClient);
	void InitPunkbusterClientLocal(CClientLocal *pClientLocal);
	void InitPunkbusterServer(bool bLocal, CServer *pServer);
	void LockPunkbusterCVars();

#ifndef NOT_USE_UBICOM_SDK

	CScriptObjectNewUbisoftClient *			m_pScriptObjectNewUbisoftClient;		//!<
	NewUbisoftClient *									m_pUbiSoftClient;										//!< for internet multiplayer with ubi.com (if game is running this pointer is always valid)
#endif // NOT_USE_UBICOM_SDK

private: // -------------------------------------------------------------------------

	void CreateConsoleVars();
	void LogNetworkInfo();

	IScriptSystem	*					m_pScriptSystem;					//!< 0 before Init()
	ISystem *								m_pSystem;								//!<
	ICVar *									m_pNetCompressPackets;		//!<
	ICVar *									m_pNetLog;								//!<
	ICVar *									m_pNetCheatProtection;		//!<
#if !defined(NOT_USE_PUNKBUSTER_SDK)
	CPunkBusterInterface *	m_pPunkBuster;						//!<
#endif
	bool										m_bHaveServer;						//!<
	bool										m_bHaveClient;						//!<

	CCompressionHelper			m_Compressor;							//!< used for various kind of compressions (e.g. text compression)
	MapServers							m_mapServers;							//!<
	ERROR_MAP								m_mapErrors;							//!<
	static CryNetError			m_neNetErrors[];					//!<
	static unsigned int		m_nCryNetInitialized;			//!<
	DWORD										m_dwLocalIP;							//!< default: 0.0.0.0 local IPAddress (needed if we have several servers on one machine)
	CDefenceWall *					m_pDefenceWall;						//!<
	IClient *								m_pClient;								//!< pointer to the active client, otherwise 0 if there is not client active
};

#endif // !defined(AFX_NETWORK_H__9C2C8689_2D3A_4729_9DBD_A8A930264655__INCLUDED_)
