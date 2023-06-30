//****************************************************************************
//*   Author:  Pierre-Luc Rigaux  gsdevelopers@ubisoft.com
//*   Date:    5/31/02 11:00:53 AM
 /*!  \file   RegServerLibrary.h
  *   \brief  The main header for the RegServerLibrary
  *
  *   The main page for the Regiser Server library
  */
//****************************************************************************

/*!
\mainpage gs-sdk-regserver
\section intro Introduction
 The Register Server Library allows a game server to register on lobby-servers.


\section description Description
 This library is for registering a game server on lobby-servers. <br> <br>

 We offer two types of interface: a C interface in RegServerLibrary.h and a C++
 interface in RegServerLib.h. <br> <br>
 
 Using the library, the game server first connects to a router.  Once the game
 server has connected it asks for the list of available lobbies which support
 the game.  Then it asks to create a room in a lobby. After receiving the
 creation confirmation, the game server connects to the lobby-server which hosts
 the lobby. The room will stay active until the game server is disconnects
 from the lobby-server.


*/
#ifndef _REGSERVERLIBRARY_H_
#define _REGSERVERLIBRARY_H_

extern "C" {
/*! @defgroup Callbacks Callbacks
	\brief List of the Library's callbacks

    @{
*/


//============================================================================
// CallBack CBRegServerRcv_LoginRouterResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:11:54 PM
/*!
 \brief	 The login result of the router.
 \par       Description: 
 The result of logging in to the router.
 
 \par Related Function:
 RegServerSend_LoginRouter()

 \param	ucType	GSSUCCESS or GSFAIL
 \param	lReason	The reason of the failure
 \param	szIPAddress	The IP address of the router

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LoginRouterResult)(GSubyte ucType,
		GSint lReason, const GSchar* szIPAddress);

//============================================================================
// CallBack CB	CBRegServerRcv_RouterDisconnection
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:01 PM
/*!
 \brief	 Called when you are disconnected from the router.
 \par       Description:
 Called when you are disconnected from the router.


*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_RouterDisconnection)();

//============================================================================
// CallBack CBRegServerRcv_RegisterServer
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:15 PM
/*!
 \brief	 Result of registering server
 \par       Description:
 The response from telling the router you want to register a server on a lobby
 
 \par Related Function:
 RegServerSend_RegisterServerOnLobby()
 
 \param	ucType	 GSSUCCESS or GSFAIL
 \param	lReason	Reason of the failure
 \param	iGroupID	The group ID of the created room
 \param	szAddress	The address of the lobbyserver
 \param	usPort	The port of the lobby server
 \param	szSessionName	The name of the created room

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_RegisterServer)(GSubyte ucType,
		GSint lReason, GSint iGroupID, const GSchar* szAddress, GSushort usPort,
		const GSchar* szSessionName);

//============================================================================
// CallBack CBRegServerRcv_RequestParentGroup
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:21 PM
/*!
 \brief	 Gives the List of the Lobbies. 
 \par       Description:
 This Callback is called for each lobby sends by the router.
 You know than you received the last lobby when the variable iGroupID = 0.
 
 \par Related Function:
 RegServerSend_RequestParentGroupOnLobby()
 
 \param	ucType	GSSUCCESS or GSFAIL
 \param	lReason	Reason of the failure
 \param	iLobbyServerID	The lobby server ID
 \param	iGroupID The GroupID of the lobby ( aka Basic Group or Parent Group )
 \param	szGroupName	The name of the Lobby
 \param	uiNbPlayers The number of the player of the Lobby	
 \param uiMaxPlayers The Number of Maximum players allowed in that lobby
*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_RequestParentGroup)(GSubyte ucType, 
		GSint lReason, GSint iLobbyServerID,	GSint iGroupID,
		const GSchar* szGroupName, GSuint uiNbPlayers, GSuint uiMaxPlayers );


//============================================================================
// CallBack CBRegServerRcv_LobbyServerLogin
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:34 PM
/*!
 \brief	 The result logging in to the lobby server. 
 \par       Description:
 The response from logging in to the lobby server
 
 \par Related Function:
 RegServerSend_LobbyServerLogin()

 \param	ucType GSSUCCESS or GSFAIL
 \param	iReason	Reason of the failure
 \param	iLobbyServerID	The LobbyServerID
 \param	iGroupID The GroupID of the registred room

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerLogin)( GSubyte ucType,
		GSint iReason, GSint iLobbyServerID, GSint iGroupID );

//============================================================================
// CallBack CBRegServerRcv_LobbyServerUpdateGroupSettings
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:39 PM
/*!
 \brief   The result of updating group settings. 
 \par       Description:
 The response from updating your group settings
 
 \par Related Function:
 RegServerSend_UpdateGroupSettings()
 
 \param	ucType	GSSUCCESS or GSFAIL
 \param	iReason	Reason of the failure
 \param	iGroupID	The group ID of the group you tried to update

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerUpdateGroupSettings)
		(GSubyte ucType, GSint iReason, GSint iGroupID );

//============================================================================
// CallBack CBRegServerRcv_LobbyServerDisconnection
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:44 PM
/*!
 \brief	   The game server has disconnected from the Lobby Server
 \par       Description:
 Received when the connection between the game server and lobby-server has been
 droped.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerDisconnection)();

//============================================================================
// CallBack CBRegServerRcv_LobbyServerMemberNew
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/13/2002 5:12:48 PM
/*!
 \brief	 A new member has joined the room.
 \par       Description:
 A member joined the room registred by this game server.

 \param	szMember	The Name of the member ( array size NIKNAMELENTH )
 \param	bSpectator	Tell if the member tried to join as spectator ( GS_TRUE )
 or an player ( GS_FALSE )
 \param szIPAddress The IP address of the member
 \param szAltIPAddress The Alternate IP address of the member ( the local ip
 address behind the NAT )
 \param pPlayerInfo Pointer on of the player info's buffer.
 \param uiPlayerInfoSize The player info's buffer size.
 \param usPlayerStatus The player ptatus as define in LobbyDefines.h.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMemberNew)(
		const GSchar* szMember, GSbool bSpectator, const GSchar* szIPAddress,
		const GSchar* szAltIPAddress, const GSvoid* pPlayerInfo,
		GSuint uiPlayerInfoSize, GSushort usPlayerStatus );


//============================================================================
// CallBack CBRegServerRcv_LobbyServerMemberLeft 
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:05 AM
/*!
 \brief	 A member left the room.
 \par       Description: 
 A member left the room. 

 \param	 szMember The name of the member.	

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMemberLeft )(
		const GSchar* szMember );


//============================================================================
// CallBack CBRegServerRcv_LobbyServerMatchStartReply
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:17 AM
/*!
 \brief	 Result of starting a match 
 \par       Description:
  The response from trying to start a match.

 \par Related Function:
 RegServerSend_MatchStart()

 \param	ucType GSFAIL or GSSUCCESS.
 \param	GSint iReason	The reason of the Failure.
 \param	GSint iGroupID The group ID.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMatchStartReply)(
		GSubyte ucType, GSint iReason, GSint iGroupID );

//============================================================================
// CallBack CBRegServerRcv_LobbyServerMatchFinishReply
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:24 AM
/*!
 \brief	  Result of finishing a match 
 \par       Description: 
  The response from trying to finish a match.

 \par Related Function:
 RegServerSend_MatchFinish()

 \param	ucType GSFAIL or GSSUCCESS.
 \param	GSint iReason	The reason of the Failure.
 \param	GSint iGroupID The group ID.
 
*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMatchFinishReply)(
		GSubyte ucType, GSint iReason, GSint iGroupID );

//============================================================================
// CallBack CBRegServerRcv_LobbyServerGroupConfigUpdate
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:30 AM
/*!
 \brief	 The Group Config has changed.

 \par       Description:
 This callback tells the game server that the group config was updated.
 
 \par Related Function:
 RegServerSend_MatchStart()<br>
 RegServerSend_MatchFinish()


 \param	uiGroupConfig	The group config and status as defined in LobbyDefines.h
 \param	iGroupID The group ID.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerGroupConfigUpdate)(
		GSuint uiGroupConfig, GSint iGroupID );

//============================================================================
// CallBack CBRegServerRcv_LobbyServerMemberUpdateStatus
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:38 AM
/*!
 \brief	 A member status has been updated.
 \par       Description:
 A member updated his status as defined in LobbyDefines.h.
 This information if useful to know if the member finished playing the match.

 \param	 szMember	The Name of the member.
 \param	usMemberStatus	 The member status.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMemberUpdateStatus)(
		const GSchar* szMember, GSushort usMemberStatus );


//============================================================================
// CallBack CBRegServerRcv_LobbyServerNewUpdateGroup 
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:46 AM
/*!
 \brief	 Settings of the room as it is on the lobby-server.
 \par       Description:
 Settings of the room as it is on the lobby-server. Received at the
 begining and when the group is updated.

 \param	usRoomType	The type of the room as define in LobbyDefines.h.
 \param	 szRoomName	The name of the room.
 \param	iGroupID The ID of the the group.
 \param	iLobbyServerID The ID of the lobby-server.	
 \param	iParentGroupID The ID of the parent Lobby.	
 \param	uiGroupConfig The group config or status as defined in LobbyDefines.h.	
 \param	sGroupLevel The Level of the the registred group in the lobby-server
 	group tree.	
 \param	 szMaster The master of the group. In the case of the regserver it
 correspond to the alias.	
 \param	 szAllowedGames	The list of games allowed in the room ( useless on room
 worked only on lobbies )
 \param	 szGame	The game that represent that room.
 \param	 pGroupInfo The group info buffer.	
 \param	uiGroupInfoSize The size of the group info buffer.	
 \param	uiMatchEventID The match event of the group.
 \param	uiMaxPlayers The maximum players allowed to join the room.	
 \param	uiNbPlayers The number of player on the room.	
 \param	uiMaxSpectators The maximum spectators allowed to join the room.	
 \param	uiNbSpectators The number of spector on the room.	
 \param	 szGameVersion The game version.	
 \param	 szGSGameVersion The game version given by the Game Service team.
 \param	 szIPAddress The ip address of the regserver detected by the router.	
 \param	 szAltIPAddress The alternate IP address of the regserver.	

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerNewUpdateGroup )
	( GSushort usRoomType,
		const GSchar* szRoomName,
		GSint iGroupID,
		GSint iLobbyServerID,
		GSint iParentGroupID,
		GSint uiGroupConfig,
		GSshort sGroupLevel,
		const GSchar* szMaster,
		const GSchar* szAllowedGames,
		const GSchar* szGame,
		const GSvoid* pGroupInfo,
		GSuint uiGroupInfoSize,
		GSuint uiMatchEventID,
		GSuint uiMaxPlayers,
		GSuint uiNbPlayers,	
		GSuint uiMaxSpectators,
		GSuint uiNbSpectators,
		const GSchar* szGameVersion,
		const GSchar* szGSGameVersion,
		const GSchar* szIPAddress,
		const GSchar* szAltIPAddress );


//============================================================================
// CallBack CBRegServerRcv_LobbyServerMemberUpdateInfo
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 5:52:55 PM
/*!
 \brief	Receive when a player's info changes.
 \par       Description:
	Receive when a player's info changes.

 \param	 szMember	The Name of the Member.
 \param	 pPlayerInfo	The player info buffer.
 \param	GSuint uiPlayerInfoSize	The player info buffer size.

*/
//============================================================================
typedef GSvoid (__stdcall *CBRegServerRcv_LobbyServerMemberUpdateInfo) (
		const GSchar* szMember, const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize );

/*! @} end of group Callbacks */


//******************************************************************************
// The Functions
//******************************************************************************

/*! @defgroup LibraryFunctions Library's main functions
	\brief List of the Library's functions who do the library running.


	Functions who do the library running.

    @{
*/

//============================================================================
// Function RegServerLibrary_Initialize
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:59 PM
/*!
 \brief	Initialize the libary.
 \par       Description:
 Initialize the library, this function must be called before all other library
 functions.
 
 \return  The function succed

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure


*/
//============================================================================
GSbool __stdcall RegServerLibrary_Initialize();


//============================================================================
// Function RegServerLibrary_Uninitialize
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:55 PM
/*!
 \brief	 Uninitialize the library.
 \par       Description:
 Unload the library and free it's resources.

 \return	Status of the function call 

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall RegServerLibrary_Uninitialize();


//============================================================================
// Function RegServer_Engine
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:52 PM
/*!
 \brief	 Do the library work.
 \par       Description: 
 Updates the connection between the client and the server, and handles
 the delivery of queued up messages and reception of messages.
 This function must be called regularly to ensure that the library will
 run smoothly.

*/
//============================================================================
GSvoid __stdcall RegServer_Engine();

/*! @} end of group LibraryFunctions */

/*! @defgroup RouterFunctions Router functions 
	\brief List of the Library's functions who manage the router connection and router requests.


	List of the Library's functions who manage the router connection and router requests.

    @{
*/

//============================================================================
// Function RegServerSend_RouterConnect
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:48 PM
/*!
 \brief	Connect to the router. 
 \par       Description:  
 This function is used to connect to the router.
 \return    Status of the function call

 \retval	GS_TRUE		Connection established
 \retval	GS_FALSE	Failed to established connection
 
 \param	szAddress Router's IP address
 \param	usPort	Router's RegServer port

*/
//============================================================================
GSbool __stdcall RegServerSend_RouterConnect(const GSchar* szAddress,
		GSushort usPort);


//============================================================================
// Function RegServerSend_LoginRouter
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:45 PM
/*!
 \brief	 Send the login info to the router.
 \par       Description:
 Send the user name and the password as registered in ubi.com database.
 For anonymous login use "Ubi_Guest" as szUsername

 \par Callbacks:
 ::CBRegServerRcv_LoginRouterResult
 
 \return  The function succeed ( The message was sent correctly )

 \retval	GS_TRUE
 \retval	GS_FALSE

 \param	szUsername	The username of the Game Server that wants to log in
 \param	szPassword	the password ( size PASSWORDLENGTH )
 \param	szVersion	The version of the Game Server (usually PC1.0)
 ( size VERSIONLENGTH )

*/
//============================================================================
GSbool __stdcall RegServerSend_LoginRouter(const GSchar* szUsername,
		const GSchar* szPassword, const GSchar* szVersion);


//============================================================================
// Function RegServerSend_RouterDisconnect
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:39 PM
/*!
 \brief	Disconnect the Router. 
 \par       Description:
	Close the connection with the Router. 

 \par Callbacks:
 ::CBRegServerRcv_RouterDisconnection
 
 \return  The function succeed 

 \retval	GS_TRUE
 \retval	GS_FALSE ( you are alredy disconnected )


*/
//============================================================================
GSbool __stdcall RegServerSend_RouterDisconnect();


//============================================================================
// Function RegServerSend_RequestParentGroupOnLobby
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:35 PM
/*!
 \brief Get the lobbies of the related game.
 \par       Description:
Request the lobbies that support the game.

 \par Callbacks:
 ::CBRegServerRcv_RequestParentGroup

 \return   The message was sent correctly 

 \retval	GS_TRUE
 \retval	GS_FALSE 

 \param szGameName (size GAMELENGTH )
*/
//============================================================================
GSbool __stdcall RegServerSend_RequestParentGroupOnLobby(
	const GSchar* szGameName);


//============================================================================
// Function RegServerSend_RegisterServerOnLobby
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:28 PM
/*!
 \brief	 Register a room on the lobby-server.
 \par       Description:
Create a room in the chosen lobby.  If you wish to let the library choose the
best Lobby for the room, set the uiLobbyID and iParentID parameters to 0.
You still must call RegServerSend_RequestParentGroupOnLobby() and wait for the
list of Lobbies to arrive.

 \par Callbacks:
 ::CBRegServerRcv_RegisterServer
	

	\return  The function succeed 

 \retval	GS_TRUE
 \retval	GS_FALSE 

 \param	uiLobbyID	the ID of the lobby-server where the id will be created
 \param	iParentID	the ID of the room where the id will be created
 \param	szRoomName	The Name of the room
 \param	szGameName	The name of the game
 \param	uwRoomType	the type of the room	( ROOM_HYBRID_DEDICATED,
 ROOM_UBI_CLIENTHOST_DEDICATED, ROOM_UBI_GAMESERVER_DEDICATED,
 ROOM_DEDICATEDSERVER,
 \param	uiMaxPlayer	 The Maximum Player
 \param	uiMaxSpectator The Maximum Spectator
 \param	szPassword The password of the room
 \param	pstGroupInfo	The pointer of the info of the group.
 \param	iGroupInfoSize	The buffer size of the info of the group
 \param	pstAltGroupInfo  The pointer of the alternate info of the group.
 \param	iAltGroupInfoSize The buffer size of the alternate info of the group
 \param	pstGameData	The connection info to connect to the game 
 (ex Directplay GUID)
 \param	iGameDataSize	The buffer size of game info
 \param usGamePort The Port of ther Game Server
 \param	szGameVersion	The Version of the Game 
 \param	szGSVersion	The version of given to the game by ubi.com
 \param	bScoreSubmission Allow the score submission; dont't allow: GS_FALSE,
 allow: GS_TRUE
 \param bDedicatedServer This room represent a Dedicated Server; not Dedicated
 Server: GS_FALSE, Dedicated Server: GS_TRUE
 
*/
//============================================================================
GSbool __stdcall RegServerSend_RegisterServerOnLobby(
		GSuint uiLobbyID, 
		GSint iParentID, 
		const GSchar* szRoomName, 
		const GSchar* szGameName,
		GSushort uwRoomType, 
		GSuint uiMaxPlayer, 
		GSuint uiMaxSpectator, 
		const GSchar* szPassword,
		const GSvoid* pstGroupInfo, 
		GSint iGroupInfoSize, 
		const GSvoid* pstAltGroupInfo, 
		GSint iAltGroupInfoSize, 
		const GSvoid* pstGameData, 
		GSint iGameDataSize,
		GSushort usGamePort,
		const GSchar* szGameVersion, 
		const GSchar* szGSVersion, 
		GSbool bScoreSubmission,
		GSbool bDedicatedServer );
/*! @} end of group RouterFunctions */

/*! @defgroup LobbyServerFunctions Lobby-Server functions 
	\brief List of the Library's functions who manage the lobby-server connection
	and  lobby-server requests.


	List of the Library's functions who manage the lobby-server connection and
	router requests.

    @{
*/

//============================================================================
// Function RegServerSend_LobbyServerConnection
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:24 PM
/*!
 \brief	Connect to the lobby-server. 
 \par       Description:
	Established a connection to a given lobby-server.
 
 \return   The connection succeed

 \retval	GS_TRUE
 \retval	GS_FALSE	

 \param	szAddress	The address of the lobby server
 \param	usPort	 The port of the lobbyserver
 \param usLocalPort The Port for receiving data ( 0 for random fix )
 \param uiStillAliveDelay The delay than a probe message is sent to keep alive
 the connection
 \param uiDisconnectionDelay The delay that took to close the connection since
 no more messages are exchange.
 
*/
//============================================================================
GSbool __stdcall RegServerSend_LobbyServerConnection( const GSchar* szAddress,
		GSushort usPort, GSushort usLocalPort = 0, GSuint uiStillAliveDelay = 25,
		GSuint uiDisconnectionDelay = 120 );

//============================================================================
// Function RegServerSend_LobbyServerClose
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:17 PM
/*!
 \brief	 Close the lobby-server connection.
 \par       Description:
 Close the lobby-server connection.
 \par Callbacks:
 ::CBRegServerRcv_LobbyServerDisconnection 

 \return   The function succeed

 \retval	GS_TRUE
 \retval	GS_FALSE

*/
//============================================================================
GSbool __stdcall RegServerSend_LobbyServerClose();

//============================================================================
// Function RegServerSend_LobbyServerLogin
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:10 PM
/*!
 \brief	Log on the lobby-server. 
 \par       Description:
 Sends the informations needed by the lobby-server to accomplish the 
 registration.
 
 \par Callbacks:
 ::CBRegServerRcv_LobbyServerLogin

 \return  The message was sent correctly 

 \retval	GS_TRUE
 \retval	GS_FALSE

 \param	szUsername	The Ubi.com username
 \param	iGroupID	The group ID of the resgisted group given by the router after a room register

*/
//============================================================================
GSbool __stdcall RegServerSend_LobbyServerLogin( const GSchar* szUsername,
		GSint iGroupID );

//============================================================================
// Function RegServerSend_UpdateGroupSettings
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:08 PM
/*!
 \brief	 Modify group parameters.
 \par       Description:
This function modifies the parameters a of a room. Modifiactions are done on
a parameter only when the value is not the default value.
Example: To Modify ONLY the number of allowed players to 8, you set the 
variable uiMaxPlayers and you set other variable to the default value.
 
 \return  The message was sent correctly   

 \retval	GS_TRUE	the message was sent to the Lobbby Server
 \retval	GS_FALSE the function was not able to send the message

 \param	iGroupID	The ID of the Group
 \param bOpen Close the acces to the group; close: 0, open: 1, Default Value: -1
 \param	bScoreSubmission Allow the score submission; dont't allow: 0, allow: 1, Default Value: -1
 \param bDedicatedServer This room represent a Dedicated Server; not Dedicated Server: 0, Dedicated Server: 1, Default Value: -1
 \param	uiMaxPlayers	The maximum player of the group, Default Value -1
 \param	uiMaxSpectator	The maximum spectaor of the group, Default Value -1
 \param	szPassword	The password of the group, Default Value NULL ( "" value remove the password )
 \param	pucGroupInfo The varible info of the group, Default Value NULL
 \param	iGroupInfoSize	The buffer size of the info of the group, Default value -1
 \param	pucAltGroupInfo The varible alternate info of the group, Default value NULL
 \param	iAltGroupInfoSize	The buffer size of the alternate info of the group, Default value -1
 \param	pucGameData	The connection info to connect to the game, Default value NULL
 \param	iGameDataSize	The buffer size of game info, Default value -1
 \param usGamePort The Port of the Game Server needed by other client to connect to. Default Value 0

*/
//============================================================================
GSbool __stdcall RegServerSend_UpdateGroupSettings(
		GSint iGroupID,		
		GSbyte bOpen,
		GSbyte bScoreSubmission,
		GSbyte bDedicatedServer,
		GSint uiMaxPlayers,
		GSint uiMaxSpectator,
		const GSchar* szPassword,
		const GSvoid* pucGroupInfo,
		GSint iGroupInfoSize,
		const GSvoid* pucAltGroupInfo,
		GSint iAltGroupInfoSize,
		const GSvoid* pucGameData,
		GSint iGameDataSize,
		GSushort usGamePort);

//============================================================================
// Function RegServerSend_LobbyServerMemberJoin
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/8/2002 6:17:43 PM
/*!

 \brief	 Send to the lobby-server than a player join the game server.  
 \par       Description:
Send to the lobby-sever than a player joined game server.  

 \return  The message was sent correctly   

 \retval	GS_TRUE	the message was sent to the lobby-server
 \retval	GS_FALSE the function was not able to send the message

 \param	szUsername	The name of the player who was just joined
*/
//============================================================================
GSbool __stdcall RegServerSend_LobbyServerMemberJoin( const GSchar* szUsername );


//============================================================================
// Function RegServerSend_LobbyServerMemberLeave
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/8/2002 6:17:47 PM
/*!
 \brief	 Send to the lobby-server than a player left the game server.  
 \par       Description:
Send to the lobby-sever than a player left or disconnected from the game server.

 \return  The message was sent correctly   

 \retval	GS_TRUE	the message was sent to the lobby-server
 \retval	GS_FALSE the library was not able to send the message

 \param	szUsername	The name of the player who was just left

*/
//============================================================================
GSbool __stdcall RegServerSend_LobbyServerMemberLeave( const GSchar* szUsername );


//============================================================================
// Function RegServerSend_MatchStart
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/11/2002 4:45:34 PM
/*!
 \brief	 Send to the lobby-server than the Server wants to start a match.  
 \par       Description:
Send to the lobby-server than the Server wants to start a match. 

 \return  The message was sent correctly   

 \retval	GS_TRUE	the message was sent to the lobby-server
 \retval	GS_FALSE the library was not able to send the message

 \param uiMode  The mode of the match as define for the score system
 ( ex. captuer the flag, Team dead match ... )
 
*/
//============================================================================
GSbool __stdcall RegServerSend_MatchStart( GSuint uiMode = 0 );


//============================================================================
// Function RegServerSend_MatchFinish
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:37 AM
/*!
 \brief	 Force The Lobby-Server to finish the match
 \par       Description: 
 Force The Lobby-Server to finish the match. The match can be finished before
 than the server send a start match if all the members send a
 Player_Match_Finish to the lobby-server.

 \return  The message was sent correctly.
 
 \retval	GS_TRUE	the message was sent to the lobby-server
 \retval	GS_FALSE the library was not able to send the message
*/
//============================================================================
GSbool __stdcall RegServerSend_MatchFinish( );

/*! @} end of group LobbyServerFunctions */


/*! @defgroup FixFunctions Fix library's callbacks functions
\brief Functions who set the library's callbacks.
Those functions take a client function as pararamters./

The Client need to fix the regserver library' callbacks to receving the information 
from the library. It's not mandatory to fix all the callbacks.

    @{
*/

//============================================================================
// Function RegServerFix_LoginRouterResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:15 PM
/*!
 \brief	 Fix the CBRegServerRcv_LoginRouterResult Callback
 \par       Description:
 

 \param	fLoginRouterResult The Callback's called function.	

*/
//============================================================================
GSvoid __stdcall RegServerFix_LoginRouterResult(
		CBRegServerRcv_LoginRouterResult fLoginRouterResult);


//============================================================================
// Function RegServerFix_RouterDisconnection
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:26 PM
/*!
 \brief	Fix the CBRegServerRcv_RouterDisconnection Callback


 \param	fRouterDisconnection	The Callback's called function.	

*/
//============================================================================
GSvoid __stdcall RegServerFix_RouterDisconnection(
		CBRegServerRcv_RouterDisconnection fRouterDisconnection);


//============================================================================
// Function RegServerFix_RegisterServerResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:31 PM
/*!
 \brief	 Fix the CBRegServerRcv_RegisterServer Callback
 

 \param	fRegisterServer	The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_RegisterServerResult(
		CBRegServerRcv_RegisterServer fRegisterServer);


//============================================================================
// Function RegServerFix_RequestParentGroupResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:37:32 PM
/*!
 \brief	 Fix the CBRegServerRcv_RequestParentGroup Callback

 \param	 fRequestParentGroup The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_RequestParentGroupResult(
		CBRegServerRcv_RequestParentGroup fRequestParentGroup);

//============================================================================
// Function RegServerFix_LobbyServerLoginResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:37 PM
/*!
 \brief	 Fix the CBRegServerRcv_LobbyServerLogin Callback

 \param	fLobbyServerLogging	The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerLoginResult(
		CBRegServerRcv_LobbyServerLogin fLobbyServerLogging );

//============================================================================
// Function RegServerFix_LobbyServerUpdateGroupSettingsResult
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:46:54 PM
/*!
 \brief	Fix the CBRegServerRcv_LobbyServerUpdateGroupSettings Callback

 \param	fLobbyServerUpdateGroupSettings	The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerUpdateGroupSettingsResult(
		CBRegServerRcv_LobbyServerUpdateGroupSettings fLobbyServerUpdateGroupSettings );

//============================================================================
// Function RegServerFix_LobbyServerDisconnection
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/3/2002 2:47:00 PM
/*!
 \brief	Fix CBRegServerRcv_LobbyServerDisconnection Callback


 \param	fLobbyServerDisconnection		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerDisconnection(
		CBRegServerRcv_LobbyServerDisconnection fLobbyServerDisconnection );


//============================================================================
// Function RegServerFix_LobbyServerNewMember
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		5/8/2002 6:17:34 PM
/*!
 \brief	 Fix the CBRegServerRcv_LobbyServerNewMember Clallback

 \param	fLobbyServerMemberNew The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMemberNew(
		CBRegServerRcv_LobbyServerMemberNew fLobbyServerMemberNew );


//============================================================================
// Function RegServerFix_LobbyServerMemberLeft
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:20 AM
/*!
 \brief	Fix the CBRegServerRcv_LobbyServerMemberLeft Clallback.

 \param	fLobbyServerMemberLeft		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMemberLeft(
		CBRegServerRcv_LobbyServerMemberLeft fLobbyServerMemberLeft );


//============================================================================
// Function RegServerFix_LobbyServerMatchStartReply
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:17 AM
/*!
 \brief	  Fix the CBRegServerRcv_LobbyServerMatchStartReply Clallback.

 \param	fLobbyServerMatchStartReply	The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMatchStartReply(
		CBRegServerRcv_LobbyServerMatchStartReply fLobbyServerMatchStartReply );


//============================================================================
// Function RegServerFix_LobbyServerMatchFinishReply
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:13 AM
/*!
 \brief	Fix the CBRegServerRcv_LobbyServerMatchFinishReply Clallback.

 \param	fLobbyServerMatchFinishReply		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMatchFinishReply(
		CBRegServerRcv_LobbyServerMatchFinishReply fLobbyServerMatchFinishReply );


//============================================================================
// Function RegServerFix_LobbyServerGroupConfigUpdate
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:09 AM
/*!
 \brief Fix the  CBRegServerRcv_LobbyServerGroupConfigUpdate Clallback.

 \param	fLobbyServerGroupConfigUpdate 		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerGroupConfigUpdate(
		CBRegServerRcv_LobbyServerGroupConfigUpdate fLobbyServerGroupConfigUpdate );


//============================================================================
// Function RegServerFix_LobbyServerMemberUpdateStatus
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:14:04 AM
/*!
 \brief	Fix the CBRegServerRcv_LobbyServerMemberUpdateStatus Clallback.


 \param	fLobbyServerMemberUpdateStatus		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMemberUpdateStatus(
		CBRegServerRcv_LobbyServerMemberUpdateStatus fLobbyServerMemberUpdateStatus );


//============================================================================
// Function RegServerFix_LobbyServerNewGroup
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 11:13:57 AM
/*!
 \brief	Fix the CBRegServerRcv_LobbyServerNewUpdateGroup Clallback.

 \param	fLobbyServerNewUpdateGroup The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerNewUpdateGroup(
		CBRegServerRcv_LobbyServerNewUpdateGroup fLobbyServerNewUpdateGroup );


//============================================================================
// Function RegServerFix_LobbyServerMemberUpdateInfo
// Author:		Pierre-Luc Rigaux plrigaux@ubisoft.com
// Date:		7/23/2002 5:52:44 PM
/*!
 \brief	 Fix the CBRegServerRcv_LobbyServerMemberUpdateInfo Clallback.

 \param	fLobbyServerMemberUpdateInfo		The Callback's called function.

*/
//============================================================================
GSvoid __stdcall RegServerFix_LobbyServerMemberUpdateInfo(
		CBRegServerRcv_LobbyServerMemberUpdateInfo fLobbyServerMemberUpdateInfo );

}; //extern C
/*! @} end of group1 */

#endif //_REGSERVERLIBRARY_H_
