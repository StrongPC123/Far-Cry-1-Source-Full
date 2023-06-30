//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: INetwork.h
//  Description: main header file
//
//	History:
//	-07/25/2001: Alberto Demichelis, Created
//  -07/20/2002: Martin Mittring, Cleaned up
//
//////////////////////////////////////////////////////////////////////

#ifndef _INETWORK_H_
#define _INETWORK_H_
#if defined(WIN32) || defined(WIN64)
	#ifdef CRYNETWORK_EXPORTS
	#define CRYNETWORK_API __declspec(dllexport)
	#else
	#define CRYNETWORK_API __declspec(dllimport)
#endif
#else
	#define CRYNETWORK_API 
#endif

#if _MSC_VER > 1000
#pragma warning(disable:4786)
#endif

#include "platform.h"

#if !defined(PS2) && !defined(_XBOX) && !defined(LINUX)
	#include <winsock.h>
#else
	#ifdef _XBOX
		#include <Xtl.h>
	#endif
	#ifdef LINUX
		#include <sys/socket.h>
	#endif
#endif

#include "Cry_Math.h"					// CVec3
#include "IBitStream.h"				// IBitStream

#define NRESULT	DWORD

#define NET_OK		0x00000000
#define NET_FAIL	0x80000000

#define NET_FAILED(a)(((a)&NET_FAIL)?1:0)
#define NET_SUCCEDED(a)(((a)&NET_FAIL)?0:1)

#define MAKE_NRESULT(severity, facility, code)(severity | facility | code)
#define NET_FACILITY_SOCKET 0x01000000

//! regular BSD sockets error
//@{
#define NET_EINTR						MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEINTR)
#define NET_EBADF						MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEBADF)
#define NET_EACCES					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEACCES)
#define NET_EFAULT					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEFAULT)
#define NET_EINVAL					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEINVAL)
#define NET_EMFILE					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEMFILE)
#define NET_WSAEINTR        MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEINTR)
#define NET_WSAEBADF        MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEBADF)
#define NET_WSAEACCES       MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEACCES)
#define NET_WSAEFAULT       MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEFAULT)
#define NET_WSAEINVAL       MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEINVAL)
#define NET_WSAEMFILE				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEMFILE)
#define NET_EWOULDBLOCK			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEWOULDBLOCK)
#define NET_EINPROGRESS			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEINPROGRESS)
#define NET_EALREADY				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEALREADY)
#define NET_ENOTSOCK				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENOTSOCK)
#define NET_EDESTADDRREQ		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEDESTADDRREQ)
#define NET_EMSGSIZE				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEMSGSIZE)
#define NET_EPROTOTYPE			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEPROTOTYPE)
#define NET_ENOPROTOOPT			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENOPROTOOPT)
#define NET_EPROTONOSUPPORT	MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEPROTONOSUPPORT)
#define NET_ESOCKTNOSUPPORT	MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAESOCKTNOSUPPORT)
#define NET_EOPNOTSUPP			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEOPNOTSUPP)
#define NET_EPFNOSUPPORT		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEPFNOSUPPORT)
#define NET_EAFNOSUPPORT		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEAFNOSUPPORT)
#define NET_EADDRINUSE			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEADDRINUSE)
#define NET_EADDRNOTAVAIL		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEADDRNOTAVAIL)
#define NET_ENETDOWN				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENETDOWN)
#define NET_ENETUNREACH			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENETUNREACH)
#define NET_ENETRESET				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENETRESET)
#define NET_ECONNABORTED		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAECONNABORTED)
#define NET_ECONNRESET			MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAECONNRESET)
#define NET_ENOBUFS					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENOBUFS)
#define NET_EISCONN					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEISCONN)
#define NET_ENOTCONN				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENOTCONN)
#define NET_ESHUTDOWN				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAESHUTDOWN)
#define NET_ETOOMANYREFS		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAETOOMANYREFS)
#define NET_ETIMEDOUT				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAETIMEDOUT)
#define NET_ECONNREFUSED		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAECONNREFUSED)
#define NET_ELOOP						MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAELOOP)
#define NET_ENAMETOOLONG		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENAMETOOLONG)
#define NET_EHOSTDOWN				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEHOSTDOWN)
#define NET_EHOSTUNREACH		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEHOSTUNREACH)
#define NET_ENOTEMPTY				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAENOTEMPTY)
#define NET_EPROCLIM				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEPROCLIM)
#define NET_EUSERS					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEUSERS)
#define NET_EDQUOT					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEDQUOT)
#define NET_ESTALE					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAESTALE)
#define NET_EREMOTE					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEREMOTE)
//@}

#ifdef _WIN32
//! extended winsock errors
//@{
#define NET_HOST_NOT_FOUND		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAHOST_NOT_FOUND)
#define NET_TRY_AGAIN					MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSATRY_AGAIN)			
#define NET_NO_RECOVERY				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSANO_RECOVERY)
#define NET_NO_DATA						MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSANO_DATA)
#define NET_NO_ADDRESS				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSANO_ADDRESS)
#define NET_SYSNOTREADY				MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSASYSNOTREADY)
#define NET_VERNOTSUPPORTED		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAVERNOTSUPPORTED)
#define NET_NOTINITIALISED		MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSANOTINITIALISED)
#define NET_EDISCON						MAKE_NRESULT(NET_FAIL, NET_FACILITY_SOCKET, WSAEDISCON)
//@}
#endif
// CryNet specific errors and messages
#define NET_FACILITY_CRYNETWORK	0x02000000

#define NET_NOIMPL							MAKE_NRESULT(NET_FAIL, NET_FACILITY_CRYNETWORK, 0x01)
#define NET_SOCKET_NOT_CREATED	MAKE_NRESULT(NET_FAIL, NET_FACILITY_CRYNETWORK, 0x02)


//#include "IPAddress.h"
//#include "Stream.h"

#include <vector>				// STL vector<>


#define DEFAULT_SERVERPORT				49001
#define DEFAULT_SERVERPORT_STR		"49001"

#define SERVER_MULTICAST_PORT	5678
//<<FIXME>> It can be changed
#define SERVER_MULTICAST_ADDRESS	"234.5.6.7"

#define SERVER_QUERY_MAX_PACKETS	(8)
#define SERVER_QUERY_PACKET_SIZE	(1120)




enum CryNetworkVarible
{
	cnvDataStreamTimeout=0
};

////////////////////////////////////////////////////////////////////////////////////////
// Interfaces
////////////////////////////////////////////////////////////////////////////////////////
struct IClient;
struct IServer;
struct IServerSnooper;
struct INETServerSnooper;
struct IClientSink;
struct IServerSlotFactory;
struct IServerSnooperSink;
struct INETServerSnooperSink;
struct IRConSystem;
struct ICompressionHelper;
class ICrySizer;
class CIPAddress;
class CStream;
struct ISystem;

/*! class factory of the Network module
	@see ::CreateNetwork()
*/
struct INetwork
{
	//! \return local IPAddress (needed if we have several servers on one machine), 0.0.0.0 if not used
	virtual DWORD GetLocalIP() const=0;

	//! also initialize Ubi.com integration (flaw in the UBI.com SDK - we would like to be able to set the IP later but they
	//! need it during initialization)
	//! \param dwLocalIP local IPAddress (needed if we have several servers on one machine)
	virtual void SetLocalIP( const char *szLocalIP )=0;

	/*! create a client object and return the related interface
			@param pSink a pointer to an object the inplements IClientSink [the object that will receive all notification during the connection]
			@param bClient if true the client will be only able to connect to the local server and
				will use a "fake connection" (memory based)
			@return an IClient interface
	*/
	virtual IClient *CreateClient(IClientSink *pSink,bool bLocal=false) = 0;

	/*! create and start a server ,return the related interface
			@param pFactory a pointer to an object the inplements IServerSlotFactory
				[the object that will receive all notification during the lifetime of the server]
			@param nPort local IP port where the server will listen
			@return an IServer interface
	*/
	virtual IServer *CreateServer(IServerSlotFactory *pFactory,WORD nPort, bool local=false) = 0;

	//! create an RCon System (remote control system)
	virtual IRConSystem *CreateRConSystem() = 0;
	/*! create an internet server snooper
	@param pSink id of the error
	@return Interface to a server snooper
	*/
	virtual INETServerSnooper *CreateNETServerSnooper(INETServerSnooperSink *pSink) = 0;
	/*! create a server snooper
		@param pSink id of the error
		@return Interface to a server snooper
	*/
	virtual IServerSnooper *CreateServerSnooper(IServerSnooperSink *pSink) = 0;
	/*! return the string representation of a socket error
		@param err id of the error
		@return string description of the error
	*/
	virtual const char *EnumerateError(NRESULT err) = 0;
	//! release the interface(and delete the object that implements it)
  virtual void Release() = 0;
	//!
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;
	//!
	//! \return interface pointer to the compression helper library, is always valid
	virtual ICompressionHelper *GetCompressionHelper() = 0;

	//! Submit to network list files, that must be matching on Client and Server.
	virtual void ClearProtectedFiles() = 0;
	virtual void AddProtectedFile( const char *sFilename ) = 0;
	//!
	//! \return 0 if there is no server registered at this port
	virtual IServer *GetServerByPort( const WORD wPort ) = 0;
	//! used to update things like the UBI.com services
	virtual void UpdateNetwork()=0;
	//! currently used to update UBI.com info and check CDKey
	//! If it is a UBI type server we should the register, if we have already registered this will do nothing.
	//! \param szServerName must not be 0
	//! \param dwPlayerCount >0
	virtual void OnAfterServerLoadLevel( const char *szServerName, const uint32 dwPlayerCount, const WORD wPort )=0;
	//! \return true=it's possible (e.g. logged into UBI.com), false=it's not possible
	virtual bool VerifyMultiplayerOverInternet() = 0;
	//! We have to tell Ubisoft that the client has successfully connected
	//! If ubisoft is not running this won't do anything.
	virtual void Client_ReJoinGameServer() = 0;
	//! \return 0 if there is no client
	virtual IClient *GetClient() = 0;
};


////////////////////////////////////////////////////////////////////////////////////////
/*! callback interface that must implement by the host that want to use IClient
*/
struct IClientSink
{
	/*! called by the client when the connection occur
	*/
	virtual void OnXConnect() = 0;
	/*! called by the client when the disconnection occur
		@param string representation of the disconnection cause
	*/
	virtual void OnXClientDisconnect(const char *szCause) = 0;
	/*! called by the client when the server send a contex setup.

		NOTES: for instance a context are all information that allow the client to load a
		level.
		A context setup is called every time the server want to load a new level.

		@param stmContext stream that contain the context informations(game dependent)
	*/
	virtual void OnXContextSetup(CStream &stmContext) = 0;
	/*! called by the client when some data is received
		@param stmContext stream that contain the data
	*/
	virtual void OnXData(CStream &stm) = 0;

	/*! called by the client when the server is very laggy (more than cl_timeout)
			that means that the client waits cl_timeout seconds, without any data from the server...
	*/
	virtual void OnXServerTimeout() = 0;
	/*! called by the client when the server responds, after a lot of time without doing so..
			that means that the client was treating the server as "timedout", and hoppefully waiting for it,
			and now, the server, magicaly responded...
	*/
	virtual void OnXServerRessurect() = 0;

	/*! called by the server when a timeout occurs..
			when a timeout is expected, because of the server loading for example,
			this function should return a number in milliseconds, that is the additional time to wait for the server.
			if not timeout is expected, this should return 0, and the normal timeout will take place.
	*/
	virtual unsigned int GetTimeoutCompensation() = 0;
	//!
	virtual void MarkForDestruct() = 0;
	//!
	virtual bool DestructIfMarked() = 0;
};


////////////////////////////////////////////////////////////////////////////////////////
/*! client interface
	this interface allow to connect and exchange data with a server

	REMARKS:
		when a disconnection occur the object that implements this interface
		CANNOT BE REUSED. This mean that the interface must be released 
		and a new IClient must be created for each connection.
*/
struct IClient
{
	/*! start the connection to a server
		@param szIP address of the server can be an ip address like 134.122.345.3 or a symbolic www.stuff.com
		@param pbAuthorizationID must not be 0
		@param iAuthorizationSize >0
		--@param wPort the remote port of the server
	*/
	virtual void Connect(const char *szIP, WORD wPort, const BYTE *pbAuthorizationID, unsigned int iAuthorizationSize) = 0;
	/*! start disconnect from a server
		@param szCause cause of the disconneciton that will be send to the server
	*/
	virtual void Disconnect(const char *szCause) = 0;
	/*! send reliable data to the server
		@param stm the bitstream that store the data
	*/
	virtual void SendReliable(CStream &stm) = 0;
	/*! send unreliable data to the server
		@param stm the bitstream that store the data
	*/
	virtual void SendUnreliable(CStream &stm) = 0;
	/*! notify the server that the contex setup was received and the client now is ready to 
		start to receive the data stream.
		usually called when the client finish to load the level.
		@param stm the bitstream that store the data that the server will receive(usally player name etc..)
	*/
	virtual void ContextReady(CStream &stm) = 0;
	/*! check if the client is ready to send data to the server
		@return true if the client is ready,false if not
	*/
	virtual bool IsReady() = 0;
	/*! called to update the client status
		@param nTime the current time in milliseconds
		@return true=this object is still exising, false=this object was destroyed

		REMARKS: to keep the connection working correctly this function must be called at least every frame
	*/
	virtual bool Update(unsigned int nTime) = 0;
	/*! get the average bandwidth used by the current connection
		@param fIncomingKbPerSec incoming kb per sec
		@param fOutgoingKbPerSec outgoing kb per sec
		@param nIncomingPackets per sec
		@param nOutgoingPackets per sec
	*/
	virtual void GetBandwidth( float &fIncomingKbPerSec, float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets )=0;
	/*! release the interface(and delete the object that implements it)
	*/
	virtual void Release() = 0;
	/*! get the average round trip delay through client and server
		@return the average ping in milliseconds
	*/
	virtual unsigned int GetPing()=0;
	//!
	virtual unsigned int GetRemoteTimestamp(unsigned int nTime)=0;
	//!
	virtual unsigned int GetPacketsLostCount()=0;
	//!
	virtual unsigned int GetUnreliablePacketsLostCount()=0;
	//! returns IP of server.
	virtual CIPAddress GetServerIP() const = 0;
	//!
	virtual void InitiateCDKeyAuthorization( const bool inbCDAuthorization ) = 0;
	//! \param pbAuthorizationID 0 if you wanna create a fake AuthorizationID, otherwise pointer to the AuthorizationID
	virtual void OnCDKeyAuthorization( BYTE *pbAuthorizationID ) = 0;
	//!
	virtual void SetServerIP( const char *szServerIP ) = 0;
};

struct IServerSnooper
{
	/*! query the LAN for servers
	*/
	virtual void SearchForLANServers(unsigned int nTime=0) = 0;
	virtual void Update(unsigned int nTime) = 0;
	//! release the interface(and delete the object that implements it)
  virtual void Release() = 0;
};

struct IServerSnooperSink
{
	/*! called by the client when some server is found
		@param ip IP address of the found server
		@param stmServerInfo stream containing all server informations(game dependent)
	*/
	virtual void OnServerFound(CIPAddress &ip, const string &szServerInfoString, int ping) = 0;
};


struct INetworkPacketSink
{
	virtual void OnReceivingPacket( const unsigned char inPacketID, CStream &stmPacket, CIPAddress &ip )=0;
};


struct INETServerSnooper
{
	//! query internet servers for info
	virtual void Update(unsigned int dwTime) = 0;
	//!
	virtual void AddServer(const CIPAddress &ip) = 0;
	//!
	virtual void AddServerList(const std::vector<CIPAddress> &vIP) = 0;
	//! release the interface(and delete the object that implements it)
	virtual void Release() = 0;
	//! clear the current list of servers
	virtual void ClearList() = 0;
};

struct INETServerSnooperSink
{
	/*! called by the client when some server is found
	@param ip IP address of the found server
	@param stmServerInfo stream containing all serer informations(game dependent)
	*/
	virtual void OnNETServerFound(const CIPAddress &ip, const string &szServerInfoString, int ping) = 0;

	/*! called by the client when some server timedout
	@param ip IP address of the dead server
	*/
	virtual void OnNETServerTimeout(const CIPAddress &ip) = 0;
};


//! interface to control servers remotely
struct IRConSystem
{
	//! query response packets
	//! Can specify optional client, to get server ip from.
	virtual void Update( unsigned int dwTime,IClient *pClient=NULL )=0;
	//! release the interface(and delete the object that implements it)
	virtual void Release()=0;
	//!
	virtual void ExecuteRConCommand( const char *inszCommand )=0;
	//!
	virtual void OnServerCreated( IServer *inpServer )=0;
};


////////////////////////////////////////////////////////////////////////////////////////
//! callback interface that must implement by the host that want to use ISererSlot
struct IServerSlotSink
{
	//! called by the serverslot when the connection occur
	virtual void OnXServerSlotConnect(const BYTE *pbAuthorizationID, unsigned int uiAuthorizationSize) = 0;
	/*! called by the serverslot when the disconnection occur
		@param string representation of the disconnection cause
	*/
	virtual void OnXServerSlotDisconnect(const char *szCause) = 0;
	/*! called by the serverslot when the client send a "context ready"
		@param stm bitstream that store the data sent by the client as answer of the context setup(usally player name etc..)
	*/
	virtual void OnContextReady(CStream &stm) = 0; //<<FIXME>> add some level validation code
	/*! called by the serverslot when some data is received
		@param stm bitstream that store the received data
	*/
	virtual void OnData(CStream &stm) = 0;
	//! 
	virtual void OnXPlayerAuthorization( bool bAllow, const char *szError, const BYTE *pGlobalID, 
		unsigned int uiGlobalIDSize ) = 0;
};

struct SServerSlotBandwidthStats
{
	//! constructor
	SServerSlotBandwidthStats()
	{
		Reset();
	}

	unsigned int		m_nReliableBitCount;				//!<
	unsigned int		m_nReliablePacketCount;			//!<
	unsigned int		m_nUnreliableBitCount;			//!<
	unsigned int		m_nUnreliablePacketCount;		//!<

	void Reset()
	{
		m_nReliableBitCount=0;
		m_nReliablePacketCount=0;
		m_nUnreliableBitCount=0;
		m_nUnreliablePacketCount=0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////
/*! server slot interface
	The server slot is the endpoint of a client connection on the server-side. Besically for
	every remote client a server slot exist on the server.
*/
struct IServerSlot
{
	/*! set the host object that will receive all server slot notifications
		@param pSink poiter to an object thath implements IServerSlotSink
	*/
	virtual void Advise(IServerSlotSink *pSink) = 0;
	/*! disconnect the client associated to this server slot
		@param szCause cause of the disconneciton that will be send to the client
	*/
	virtual void Disconnect(const char *szCause) = 0;
	/*! send a context setup to the client
		@param stm bitstream that store the context information(usually level name etc..)
	*/
	virtual bool ContextSetup(CStream &stm) = 0;
	/*! send reliable data to the client
		@param stm the bitstream that store the data
	*/
	virtual void SendReliable(CStream &stm,bool bSecondaryChannel=false) = 0;
	/*! send unreliable data to the client
		@param stm the bitstream that store the data
	*/
	virtual void SendUnreliable(CStream &stm) = 0;
	/*! check if the server slot is ready to send data to the client
		@return true if the serverslot is ready,false if not
	*/
	virtual bool IsReady() = 0;
	/*! get the unique id that identify the server slot on a server
		@return ID of the serverslot
	*/
	virtual unsigned char GetID()=0;
	
	// Return IP in integer form.
	virtual unsigned int GetClientIP() const = 0;
	//! release the interface(and delete the object that implements it)
	virtual void Release() = 0;
	/*! get the average round trip delay through client and server
		@return the average ping in milliseconds
	*/
  virtual unsigned int GetPing() = 0;
	//!
	virtual unsigned int GetPacketsLostCount() = 0;
	//!
	virtual unsigned int GetUnreliablePacketsLostCount() = 0;
	//! used for bandwidth calculations (to adjust the bandwidth)
	virtual void ResetBandwidthStats() = 0;
	//! used for bandwidth calculations (to adjust the bandwidth)
	virtual void GetBandwidthStats( SServerSlotBandwidthStats &out ) const = 0;
	//! just calles OnXPlayerAuthorization of the corresponding game specific object
	virtual void OnPlayerAuthorization( bool bAllow, const char *szError, const BYTE *pGlobalID, 
		unsigned int uiGlobalIDSize ) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////
//the application must implement this class
struct IServerSlotFactory
{
	virtual bool CreateServerSlot(IServerSlot *pIServerSlot) = 0;
	
	//! \return true=success, false otherwise
	//! fill the given string with server infos
	//! \note do not overwrite the string, just append to it
	virtual bool GetServerInfoStatus(string &szServerStatus) = 0;
	virtual bool GetServerInfoStatus(string &szName, string &szGameType, string &szMap, string &szVersion, bool *pbPassword, int *piPlayers, int *piMaxPlayers) = 0;
	virtual bool GetServerInfoRules(string &szServerRules) = 0;
	virtual bool GetServerInfoPlayers(string *vszStrings[4], int &nStrings) = 0;
	//! Called when someone sends XML request to server, this function must fill sResponse string with XML response.
	virtual bool ProcessXMLInfoRequest( const char *sRequest,const char *sRespone,int nResponseMaxLength ) = 0;
};

// the application should implement this class
struct IServerSecuritySink
{
	enum CheaterType
	{
		CHEAT_NOT_RESPONDING,
		CHEAT_NET_PROTOCOL,
		CHEAT_MODIFIED_FILE,
		CHEAT_MODIFIED_CODE,
		CHEAT_MODIFIED_VARS,
	};
	struct SSlotInfo
	{
		char playerName[32];
		int score;
		int deaths;
	};

	/*!	check the state of an ip address before creating the slot
			\return the state of the ip (banned or not)
	*/
	virtual bool IsIPBanned(const unsigned int dwIP) = 0;

	/*! ban an ip address
			\param dwIP the ip address to ban
	*/
	virtual void BanIP(const unsigned int dwIP) = 0;

	/*! ban an ip address
	\param dwIP the ip address to ban
	*/
	virtual void UnbanIP(const unsigned int dwIP) = 0;

	/*! Report cheating user.
	 *	
	 */
	virtual void CheaterFound( const unsigned int dwIP,int type,const char *sMsg ) = 0;

	/*! Request slot information from the game.
	 *	
	 */
	virtual bool GetSlotInfo(  const unsigned int dwIP,SSlotInfo &info , int nameOnly ) = 0;
};


enum EMPServerType
{
	eMPST_LAN=0,				//!< LAN
	eMPST_NET=1,				//!< e.g. ASE
	eMPST_UBI=2,				//!< UBI.com
};

////////////////////////////////////////////////////////////////////////////////////////
/*!Server interface
*/
struct IServer
{
	/*! called to update the server status, this update all serverslots too
		@param nTime the current time in milliseconds

		REMARKS: to keep the connection working correctly this function must be called at least every frame
	*/
	virtual void Update(unsigned int nTime) = 0;
	//! release the interface and delete the implemetation
	virtual void Release() = 0;
	//! set a server veriable
	virtual void SetVariable(enum CryNetworkVarible eVarName,unsigned int nValue) = 0;
	/*! get the average bandwidth used by all active connections
		@param fIncomingKbPerSec incoming kb per sec
		@param fOutgoingKbPerSec outgoing kb per sec
		@param nIncomingPackets per sec
		@param nOutgoingPackets per sec
	*/
	virtual void GetBandwidth( float &fIncomingKbPerSec, float &fOutgoinKbPerSec, DWORD &nIncomingPackets, DWORD &nOutgoingPackets )=0;
	/*!return the symbolic name of the localhost
		@return the symbolic name of the localhost
	*/
	virtual const char *GetHostName() = 0;
	//! \param inPacketID e.g. FT_CQP_RCON_COMMAND
	//! \param inpSink must not be 0
	virtual void RegisterPacketSink( const unsigned char inPacketID, INetworkPacketSink *inpSink )=0;

	/*! set the security sink
			\param pSecurirySink pointer to a class that implements the IServerSecuritySink interface
	*/
	virtual void SetSecuritySink(IServerSecuritySink *pSecuritySink) = 0;

	/*!	check the state of an ip address before creating the slot
		\return the state of the ip (banned or not)
	*/
	virtual bool IsIPBanned(const unsigned int dwIP) = 0;

	/*! ban an ip address
		\param dwIP the ip address to ban
	*/
	virtual void BanIP(const unsigned int dwIP) = 0;

	/*! ban an ip address
		\param dwIP the ip address to ban
	*/
	virtual void UnbanIP(const unsigned int dwIP) = 0;

	//! time complexity: O(n) n=connected server slots
	//! \return 0 if there is no serverslot with this client (was never there or disconnected)
	virtual IServerSlot *GetServerSlotbyID( const unsigned char ucId ) const = 0;

	//! to iterate through all clients (new clients ids are the lowest available at that time)
	virtual uint8 GetMaxClientID() const = 0;

	//! LAN/UBI/NET
	virtual EMPServerType GetServerType() const=0;
};

////////////////////////////////////////////////////////////////////////////////////////
// other stuff


// exports;
extern "C"{
	CRYNETWORK_API INetwork *CreateNetwork(ISystem *pSystem);
	typedef INetwork *(*PFNCREATENETWORK)(ISystem *pSystem);
}
#endif //_INETWORK_H_
