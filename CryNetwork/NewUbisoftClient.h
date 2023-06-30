#ifndef __NEW_UBISOFT_CLIENT
#define __NEW_UBISOFT_CLIENT

#ifndef NOT_USE_UBICOM_SDK

// Facade design pattern over Ubisoft's own clMSClient and CRegisterby Scott Schmeisser
// This implements all functions and callbacks needed to use Ubisoft's "Master Server Style" matchmaking service in-game
// Class is implemented over these files: NewUbisoftClient.cpp implements the helper methods
// NewUbisoftMSClient.cpp implements the clMsClientClass methods.  This is for a game client that wants to get the list
// of game servers.
// NewUbisoftRegServer.cpp implements the CRegisterServer methods.  This is for a game server that wants to add it self
// to the list of game servers.

#if defined(WIN32)
#	define GS_WIN32
#else
#	define GS_LINUX
#endif

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	#include "MSClientClass.h"
#endif // EXCLUDE_UBICOM_CLIENT_SDK

#include "RegServerLib.h"
#include <string> // std string
#include "GSTypes.h"
#include "GSCDKeyDefines.h"

class CScriptObjectNewUbisoftClient;
struct ISystem;



enum UbiServerState
{
	NoUbiServer, //Don't use Ubi.com
	ServerDisconnected, //We're disconnected from Ubi.com try to reconnect
	CreatingServer, //We're trying to create a ubi.com server
	ServerConnected //We're connected to ubi.com
};

enum UbiClientState
{
	NoUbiClient, //Don't use Ubi.com
	ClientDisconnected, //We've disconnected from ubi.com and should try to reconnect.
	ClientLoggingIn, //We're logging in to ubi.com
	ClientLoggedIn, //We're Logged in to ubi.com
	GameServerDisconnected, //We've been disconnect from the game server and should rejoin.
	JoiningGameServer, //We're joining a game server
	JoinedGameServer, //We've joined a game server
	CreateUbiAccount //We're creating an account
};

class NewUbisoftClient : 
#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	private clMSClientClass, 
#endif // EXCLUDE_UBICOM_CLIENT_SDK
	private CRegisterServer
{
public:
	//! constructor
	NewUbisoftClient( const char *szLocalIPAddress );
	//! destructor
	~NewUbisoftClient();

	bool WriteStringToRegistry(const string &szKeyName, const string &szValueName, const string &szValue);
	bool ReadStringFromRegistry(const string &szKeyName, const string &szValueName, string &szValue);
	bool RemoveStringFromRegistry(const string &szKeyName, const string &szValueName);
	bool IsValueOnRegistry(const string &szKeyName, const string &szValueName);

	bool EncryptString(unsigned char *szOut, const unsigned char *szIn);
	bool DecryptString(unsigned char *szOut, const unsigned char *szIn);

	bool EncodeHex(unsigned char *szOut, const unsigned char *szIn);
	bool DecodeHex(unsigned char *szOut, const unsigned char *szIn);

	//! establish the connection to the script object (call this only once)
	//! \param inpObject must not be 0
	void SetScriptObject( CScriptObjectNewUbisoftClient *inpObject );
	//! \param inpSystem must not be 0
	void Init( ISystem *inpSystem );

	//! Must be called to send and receive messages from the Game Service.
	//! Must be called even while playing the game.
	bool Update();

	/////////////////////////////////////////////
	//
	// The methods to be used by the game client.
	// Implemented in NewUbisoftMSClient.cpp
	//
	/////////////////////////////////////////////

	//Login the client to the Game Service
	bool Client_AutoLogin();

	//Login the client to the Game Service
	bool Client_Login(const char *szUsername, const char *szPassword, bool bSavePassword=false);

	//Request Game Server list.  Game Servers are received by LUAGameServer.
	bool Client_RequestGameServers();

	// Tell the Game Service you are going to join the game server
	bool Client_JoinGameServer(int iLobbyID, int iRoomID);
	// Tell the Game Service you are going to rejoin the game server
	bool Client_ReJoinGameServer();
	// Tell the Game Service you have joined the game server
	bool Client_ConnectedToGameServer();
	// Tell the Game Service that you have left the game server
	bool Client_LeaveGameServer();

	// Disconnect from Game Service
	bool Client_Disconnect();

	// Create an account
	bool Client_CreateAccount(const char *szUsername, const char *szPassword);

	// Check if there is a connection
	bool Client_IsConnected();

	// Request the Authorization ID from the CDKey server
	bool Client_GetCDKeyAuthorizationID();

	// Will Check to see if we have the users CDKey.  If we don't it will ask the user.
	bool Client_CheckForCDKey();

	// Set the users CDKey.
	bool Client_SetCDKey(const char *szCDKey);

	//Request the Message Of The Day.  szLanguage is the two letter language code.
	//See http://www.w3.org/WAI/ER/IG/ert/iso639.htm
	bool Client_RequestMOTD(const char *szLanguage);


	/////////////////////////////////////////////
	//
	// The methods to be used by the game server.
	// Implemented in NewUbisoftRegServer.cpp
	//
	/////////////////////////////////////////////
	void Server_CheckCDKeys(bool bCheck) { m_bCheckCDKeys = bCheck; };

	// create the server on the Game Service
	bool Server_CreateServer(const char* szServerName,unsigned int uiMaxPlayer);

	// set the connection to the server
	void Server_SetGamePort(unsigned short usGamePort);

	// create the server on the Game Service
	bool Server_RecreateServer();

	// Update settings on the Game Service
	bool Server_UpdateServer(unsigned int uiMaxPlayers, unsigned short usPort);

	// Remove the server from Game Service
	bool Server_DestroyServer();

	// Check the AuthorizationID of a client.  This triggers CXServerSlot::OnPlayerAuthorization()
	bool Server_CheckPlayerAuthorizationID(BYTE bPlayerID,const BYTE *pubAuthorizationID);

	// Tells the CDKeyServer that a player has left the game.
	bool Server_RemovePlayer(BYTE bPlayerID);

	// Remove all the players from the game.
	bool Server_RemoveAllPlayers();


	// Script Callbacks -------------------------------------------

	void Client_LoginSuccess(const char *szUsername);
	void Client_LoginFail(const char *szText);
	void Client_GameServer(int iLobbyID, int iRoomID, const char *szServerName, const char *szIPAddress,
		const char *szLANIPAddress, int iMaxPlayers, int iNumPlayers);
	void Client_RequestFinished();
	void Client_JoinGameServerSuccess(const char *szIPAddress, const char *szLanIPAddress,unsigned short usPort);
	void Client_JoinGameServerFail(const char *szText);
	void Client_CreateAccountSuccess();
	void Client_CreateAccountFail(const char *szText);
	void Server_RegisterServerSuccess(GSint iLobbyID, GSint iRoomID);
	void Server_RegisterServerFail();
	void Server_LobbyServerDisconnected();
	void Server_PlayerJoin(const char *szUsername);
	void Server_PlayerLeave(const char *szUsername);

	// Pop up an error to the user.
	void CDKey_Failed(const char *szText);

	// Ask the user for a cdkey.
	void CDKey_GetCDKey();

	// The cdkey was successfully activated
	void CDKey_ActivationSuccess();

	// The cdkey activation failed.  The user should reenter the cdkey.
	void CDKey_ActivationFail(const char *szText);

	// --------------------------------------------------------------

	// --------------------------------------------------------------

	// Callback that receives the Activation ID from the CDKey Server
	void RcvActivationID(PREPLY_INFORMATION psReplyInfo, PVALIDATION_SERVER_INFO psValidationServerInfo,
			GSubyte *pucActivationID,GSubyte *pucGlobalID);
	// Callback that receives the Authorization ID from CDKeyServer.  This ID must be sent to the Game Server.
	void RcvAuthorizationID(PREPLY_INFORMATION psReplyInfo, PVALIDATION_SERVER_INFO psValidationServerInfo,
			GSubyte *pucAuhorizationID);
	// Callback that receives if a players cdkey is valid and the server should allow the player to connect
	void RcvValidationResponse(PREPLY_INFORMATION psReplyInfo, PVALIDATION_SERVER_INFO psValidationServerInfo,
			GSubyte *pucAuhorizationID, CDKEY_PLAYER_STATUS eStatus, GSubyte *pucGlobalID);
	// Callback that asks to see if a player is still on the game server.
	void RcvPlayerStatusRequest(PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuhorizationID);

private: // -----------------------------------------------------------------

	// Helper function to download gs.ini file from internet
	bool DownloadGSini(const char *szUsername);
	// Helper function to parse gs.ini file
	bool GetRouterAddress(int iIndex, char *szIPAddress, unsigned short *pusClientPort,
		unsigned short *pusRegServerPort);

	void RegServerDisconnected();
	void MSClientDisconnected();

	typedef std::vector<GSubyte> CDKeyIDVector;

	//CD Key methods
	bool InitCDKeySystem();
	bool GetCDKeyServerAddress(int iIndex, char *szIPAddress, unsigned short *pusPort);

	// Save a clients Authoriziation ID
	bool AddAuthorizedID(BYTE bPlayerID, const CDKeyIDVector &stAuthorizationID);

	//Get a player's ID based on their Authorization ID
	bool FindPlayerID(const CDKeyIDVector &stAuthorizationID, BYTE &bPlayer);
	//Get a player's Authorization ID based on their ID
	bool FindAuthorizedID(BYTE bPlayerID, CDKeyIDVector &stAuthorizationID);

	//Remove a players Authorization ID
	bool RemoveAuthorizedID(const CDKeyIDVector &stAuthorizationID);
	bool RemoveAuthorizedID(BYTE bPlayer);

	//Get the localization string for a CDKey error
	bool GetCDKeyErrorText(GSushort usError,string &strText);

	// Gets the error string and sends it to CDKey_Failed()
	bool CDKey_Error(GSushort usError);

	//Helper functions for Authorization IDs
	void CopyIDToVector(CDKeyIDVector &stVector, const GSubyte *pubArray, unsigned int uiSize);
	void CopyIDToString(const CDKeyIDVector &stVector, string &strString);


	//Save and load CDkey information.  Currently from the file cdkey.ini
	void SaveCDKey(const GSchar *szCDKey);
	void SaveActivationID(const GSubyte *pubActivationID);
	bool LoadCDKey(GSchar *szCDKey);
	bool LoadActivationID(GSubyte *pubActivationID);

	//Request the Activation Id from the CDKey server.
	bool RequestCDKeyActivationID();

	// These are the MSClient callbacks to implement.
	GSvoid GameServerCB(GSint iLobbyID,GSint iRoomID,GSshort siGroupType,
			GSchar *szGroupName,GSint iConfig,GSchar *szMaster,GSchar *szAllowedGames,
			GSchar *szGames,GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,
			GSint iSize,GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
			GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
			GSint iEventID);
	GSvoid ErrorCB(GSint iReason,GSint iLobbyID,GSint iRoomID);
	GSvoid InitFinishedCB(GSubyte ucType,GSint iError,GSchar *szUserName);
	GSvoid LoginDisconnectCB();
	GSvoid LobbyDisconnectCB();
	GSvoid RequestFinishedCB();
	GSvoid JoinFinishedCB(GSint iLobbyID,GSint iRoomID,
			GSvoid *vpGameData,GSint iSize,GSchar *szIPAddress,
			GSchar *szAltIPAddress,GSushort usPort);
	GSvoid AlternateInfoCB(GSint iLobbyID,GSint iRoomID,
			const GSvoid* pcAltGroupInfo,GSint iAltGroupInfoSize);
	GSvoid RequestMOTDCB(GSubyte ubType, GSchar *szUbiMOTD,
			GSchar *szGameMOTD, GSint iReason);

	GSvoid AccountCreationCB(GSubyte ucType, GSint iReason);
	GSvoid ModifyAccountCB(GSubyte ucType, GSint iReason);

	//We aren't implementing Ladder support so these callbacks are ignored.
	GSvoid MatchStartedCB(GSint iLobbyID,GSint iRoomID, GSuint uiMatchID){};
	GSvoid SubmitMatchCB(GSubyte ucType,GSint iReason, GSuint uiMatchID){};


	// These are the RegServer callbacks to implement
	GSvoid RegServerRcv_LoginRouterResult( GSubyte ucType, GSint lReason,
			const GSchar* szIPAddress );
	GSvoid RegServerRcv_RouterDisconnection();
	GSvoid RegServerRcv_RegisterServerResult( GSubyte pucType,GSint plReason,GSint iGroupID,
			const GSchar* szAddress,GSushort usPort,const GSchar* szSessionName );
	GSvoid RegServerRcv_RequestParentGroupResult( GSubyte ucType,
			GSint lReason, GSint iServerID,GSint iGroupID, const GSchar* szGroupName,
			GSuint uiNbPlayers, GSuint uiMaxPlayers );
	GSvoid RegServerRcv_LobbyServerLoginResults( GSubyte ucType,
			GSint iReason, GSint iLobbyServerID, GSint iGroupID );
	GSvoid RegServerRcv_LobbyServerUpdateGroupSettingsResults(
			GSubyte ucType, GSint iReason, GSint iGroupID );
	GSvoid RegServerRcv_LobbyServerDisconnection();
	GSvoid RegServerRcv_LobbyServerMemberNew( const GSchar* szMember,GSbool bSpectator,
		const GSchar* szIPAddress, const GSchar* szAltIPAddress,
		const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize,GSushort usPlayerStatus );
	GSvoid RegServerRcv_LobbyServerMemberLeft(const GSchar* szMember );
	GSvoid RegServerRcv_LobbyServerNewGroup (GSushort usRoomType,
		const GSchar* szRoomName,GSint iGroupID,GSint iLobbyServerID,GSint iParentGroupID,
		GSint uiGroupConfig,GSshort sGroupLevel,const GSchar* szMaster,const GSchar* szAllowedGames,
		const GSchar* szGame,const GSvoid* pGroupInfo,GSuint GroupInfoSize,GSuint uiMatchEventID,
		GSuint uiMaxPlayers,GSuint uiNbPlayers,GSuint uiMaxSpectators,GSuint uiNbSpectators,
		const GSchar* szGameVersion,const GSchar* szGSGameVersion,const GSchar* szIPAddress,
		const GSchar* szAltIPAddress );

	// We can ignore these RegServer callbacks
	GSvoid RegServerRcv_LobbyServerMatchStartReply( GSubyte ucType,
		GSint iReason, GSint iGroupID ){};
	GSvoid RegServerRcv_LobbyServerMatchFinishReply( GSubyte ucType,
		GSint iReason, GSint iGroupID ){};
	GSvoid RegServerRcv_LobbyServerGroupConfigUpdate(
		GSuint uiGroupConfig, GSint iGroupID ){};
	virtual GSvoid RegServerRcv_LobbyServerMemberUpdateInfo(const GSchar* szMember,
		const GSvoid* pPlayerInfo,GSuint uiPlayerInfoSize ){};
	GSvoid RegServerRcv_LobbyServerMemberUpdateStatus(const GSchar* szPlayer,
		GSushort usPlayerStatus ){};
/*#if defined(LINUX32)
	//dummy functions coming from new ubi.com sdk released for linux
	virtual GSvoid RegServerRcv_SubmitMatchResultReply( GSubyte ucType,
		GSint iReason, GSint iGroupID ){};
	virtual GSvoid RegServerRcv_MatchStarted( GSuint uiMatchID ){};
	virtual GSvoid RegServerRcv_FinalResult(GSuint uiMatchId, GSubyte ucType, GSint iReason, const LADDER_ROW *pResults, GSuint uiNumResult){};
#endif
*/
	// These are the login settings to use if we have to re-login
	string																	m_strUsername;				//!<
	string																	m_strPassword;				//!<

	// These are the settings to use when creating the game server
	string																	m_strGameServerName;	//!<
	unsigned int														m_uiMaxPlayers;				//!<
	unsigned short													m_usGamePort;					//!<

	int 																		m_iJoinedLobbyID;			//!< The id of the lobby we joined
	int 																		m_iJoinedRoomID;			//!< The id of the room we joined
	bool 																		m_bDownloadedGSini;		//!< Have we downloaded the gs.ini file yet.  We only need to do this once per game.s
	int 																		m_iServerLobbyID;			//!< The lobby id of the game server
	int 																		m_iServerRoomID;			//!< The room id of the game server

	UbiServerState													m_eServerState;
	UbiClientState													m_eClientState;

	//CD Key members
	GShandle																m_hCDKey; //The handle for the cdkey library.
	PVALIDATION_SERVER_INFO									m_pCDKeyServer;
	bool																		m_bCheckCDKeys; //If true the server will check cdkeys.

	typedef std::map<CDKeyIDVector,BYTE>		AuthorizedIDs;
	AuthorizedIDs														m_stAuthorizedIDs;

	CScriptObjectNewUbisoftClient *					m_pScriptObject;						//!<
	DWORD																		m_dwNextServerAbsTime;			//!< in seconds (0 if deactivated)
	DWORD																		m_dwNextClientAbsTime;			//!< in seconds (0 if deactivated)
	DWORD																		m_dwAccountCreateTime;			//!< in seconds (0 if deactivated)
	ILog *																	m_pLog;											//!<
	ISystem *																m_pSystem;									//!<

	ICVar *																	sv_authport;
	ICVar *																	sv_regserver_port;

	bool																		m_bSavePassword;
	bool																		m_bDisconnecting;

	friend class cCScriptObjectNewUbisoftClient;
};

#endif // NOT_USE_UBICOM_SDK

#endif //__UBISOFT_MSCLIENT
