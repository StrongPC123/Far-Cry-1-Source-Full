
//****************************************************************************
//*   Author:  Scott Schmeisser  gsdevteam@ubisoft.com
//*   Date:    4/19/02 1:44:25 PM
 /*!  \file   MSClientLibrary.h
  *   \brief  The header file for the Master Server Client Library
  *
  *	  This interface provides game client functionality to retrieve a list of
  *   running game servers registered on Ubi.com (Master server list style).
  */
//****************************************************************************

#ifndef _MSCLIENTLIBRARY_H_
#define _MSCLIENTLIBRARY_H_

#include "GSTypes.h"

/*!
\mainpage gs-sdk-msclient
\section intro Introduction
 Ubi.com master server list interface


\section description Description
 This sdk provides functionalities for a game developper to add game server listing
 mechanism in his project. This system works by connecting on all Ubi.com lobbies
 and retrieving the list of game servers for the game requested.

*/

/*! @defgroup group1 Callback definitions
\brief Callback definitions

These callbacks definitions are used by the game to process response to queries sent to Ubi.com.
    @{
*/


//============================================================================
// CallBack MSClient_GameServerCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 1:44:16 PM
/*!
 \brief	 Receivec information on a game server.
 \par       Description:
 This callback will be called whenever a game server is found for the request the client has made.

 \param	iLobbyID	The id of the lobby server
 \param	iRoomID	The Id of the room.
 \param	siGroupType	The type of game server. (ROOM_DIRECTPLAY,ROOM_GAMEMODULE,ROOM_P2P,ROOM_CLIENTHOST)
 \param	szGroupName	The name of the game server
 \param	iConfig	The game server configuration flag 
 \param	szMaster	The name of the master of the room.
 \param	szAllowedGames	The games allowed in this room.
 \param	szGames	The games that can be played in the room.
 \param	szGameVersion	The version of the game (information only)
 \param	szGSVersion	The version of the gs-game
 \param	vpInfo	A pointer to the game data
 \param	iSize	The size of the game data structure
 \param	uiMaxPlayer	The maximum number of players allowed in that room
 \param	uiNbrPlayer	The number of players currently in that room
 \param	uiMaxVisitor	The maximum number of visitors allowed in that room
 \param	uiNbrVisitor	The number of visitors currently in that room
 \param	szIPAddress	The ip address of the host (master) of the room
 \param	szAltIPAddress	The alternate ip address of the host (master) of the room
 \param	iEventID	The event id for that room

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_GameServerCB)(GSint iLobbyID,GSint iRoomID,
		GSshort siGroupType,GSchar *szGroupName, GSint iConfig,
		GSchar *szMaster,GSchar *szAllowedGames,GSchar *szGames,
		GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,GSint iSize,
		GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
		GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSint iEventID);

//============================================================================
// CallBack MSClient_AlternateInfoCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		8/19/02 1:44:16 PM
/*!
 \brief	 The Alternate Info of a Game Server
 \par       Description:
 This called when alternate information on a Game Server is received.

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.
 \param	pcAltGroupInfo	The alternate information
 \param	iAltInfoSize	The size of the alternate info

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_AlternateInfoCB)(GSint iLobbyID,GSint iRoomID,
		const GSvoid* pcAlternateInfo, GSint iAltInfoSize);

//============================================================================
// CallBack MSClient_ErrorCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:35:34 PM
/*!
 \brief	 An error occured
 \par       Description:
 Called when a error occurs.  The reason can be looked up in LobbyDefine.h

 \param	iReason	The reason for the error
 \param iLobbyID	The ID of the lobby.  0 if the error doesn't involve a Lobby
 \param iRoomID	The ID of the room.  0 if the error doesn't involve a Room

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_ErrorCB)(GSint iReason,GSint iLobbyID,
		GSint iRoomID);

//============================================================================
// CallBack MSClient_InitFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:36:34 PM
/*!
 \brief	 The initialization has finished
 \par       Description:
 The library has finished initializing and other functions can be called.
 If an error happened while initializing, the type will be GSFAIL and iError
 will contain the reason for the error.  This value can be looked up in
 define.h

 \param ucType  GSSUCCESS or GSFAIL
 \param	iError	The reason for the GSFAIL
 \param szUserName	The correct case of the players username.
*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_InitFinishedCB)(GSubyte ucType, GSint iError,
		GSchar *szUserName);

//============================================================================
// CallBack MSClient_LoginDisconnectCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		8/9/02 5:29:53 PM
/*!
 \brief	 The library has disconnected from the Router
 \par       Description:
 The library has become disconnected from the Router.  The client should
 uninitialize and then re-initialize the library.  If the player was in a Game
 they will have to call MSClient_JoinGameServerByID and rejoin the GameServer.
 They do not have to disconnect from the game server, this should not
 interrupt the player while they play the game.

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_LoginDisconnectCB)();


//============================================================================
// CallBack MSClient_LobbyDisconnectCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		8/9/02 5:33:26 PM
/*!
 \brief	 The library has disconnected from the Lobby
 \par       Description:
 The library has become disconnected from the Lobby.  The client does not need
 to uninitialize and re-initialize the library.  If the player was in a Game
 they will have to call MSClient_JoinGameServerByID and rejoin the GameServer.
 They do not have to disconnect from the game server, this should not
 interrupt the player while they play the game.


*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_LobbyDisconnectCB)();
//============================================================================
// CallBack MSClient_RequestFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:37:41 PM
/*!
 \brief	 All the Game Servers have been downloaded
 \par       Description:
 Called when all the Game Servers have been downloaded.  This is currently
 3 seconds after the last Game Server has been received.


*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_RequestFinishedCB)();

//============================================================================
// CallBack MSClient_JoinFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:41:32 PM
/*!
 \brief	 Finished joining the game server
 \par       Description:
 The library has finished joining the game server on the lobby.

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.
 \param	vpGameData	The Game data
 \param	iSize	The size of the Game Data
 \param	szIPAddress	The IP address of the game server
 \param	szAltIPAddress The Alternate address
 \param	usPort The Port of the game server
*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_JoinFinishedCB)(GSint iLobbyID,GSint iRoomID,
		GSvoid *vpGameData,GSint iSize,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSushort usPort);

//============================================================================
// CallBack MSClient_AccountCreationCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/02 11:03:32 AM
/*!
 \brief	 Account Creatation callback
 \par       Description:
 Tells the client if the account creation was successfull.

 \param	ucType	GSSUCCESS or GSFAIL
 \param	iReason	The reason for a GSFAIL.
*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_AccountCreationCB)(GSubyte ucType,
		GSint iReason);

//============================================================================
// CallBack MSClient_ModifyAccountCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/02 11:03:32 AM
/*!
 \brief	 Account Creatation callback
 \par       Description:
 Tells the client if the account modification was successfull.

 \param	ucType	GSSUCCESS or GSFAIL
 \param	iReason	The reason for a GSFAIL.
*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_ModifyAccountCB)(GSubyte ucType,
		GSint iReason);

//============================================================================
// CallBack MSClient_MatchStartedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 10:39:51 AM
/*!
 \brief	 The match has started
 \par       Description:
 The game server has started the match

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.
 \param	uiMatchID	The id of the started match

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_MatchStartedCB)(GSint iLobbyID,GSint iRoomID,
		GSuint uiMatchID);


//============================================================================
// CallBack MSClient_SubmitMatchCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 11:16:34 AM
/*!
 \brief	 The replay to MSClient_SubmitMatchResult
 \par       Description:
 Informs the program if the SubmitMatch was successful or not

 \param	ucType	GSSUCCESS or GSFAIL
 \param	iReason	if usType is GSFAIL this is the reason.  See LobbyDefines.h
 \param	iMatchID	The id of the match

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_SubmitMatchCB)(GSubyte ucType,
		GSint iReason,GSuint iMatchID);


//============================================================================
// Callback MSClient_RequestMOTDCB

/*!
 \brief	 Receive the message of the day
 \par       Description:
  This callback will be called when the client receives the MOTDs from the
	server.  The messages will never be greater the MOTDLENGTH.
	
  \par Related function:
	MSClient_RequestMOTD()
	
	\par Errors:
	ERRORROUTER_DBPROBLEM: There is a problem with the database.<br>

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param szUbiMOTD The message of the day for the Ubi.com Game Service
 \param szGameMOTD The message of the day for the game.
 \param	iReason	The reason for the failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *MSClient_RequestMOTDCB)(GSubyte ubType,
		GSchar *szUbiMOTD, GSchar *szGameMOTD, GSint iReason);

/*! @} end of group1 */


extern "C" {

	/*! @defgroup group2 Master server client functionalities
\brief Master server client API

API for the master server client library.
    @{
*/

//============================================================================
// Function MSClient_Initialize
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:48:31 PM
/*!
 \brief	 Initialize the library
 \par       Description:
 Initializes the library. This function will connect to and log on the Ubi.com network.
 Since this could take some time, the function returns and MSClient_InitFinishedCB is called to say
 when the login has finished.

 \return    The success of the Function

 \param	szMasterServerIP	The IP of the GSRouter
 \param	usMasterServerPort	The port of the GSRotuer
 \param	szUserName	The players username
 \param	szPassword	The players password
 \param	szVersion	The client version

*/
//============================================================================
GSbool __stdcall MSClient_Initialize(const GSchar *szMasterServerIP,
		GSushort usMasterServerPort,const GSchar *szUserName,
		const GSchar *szPassword, const GSchar *szVersion);


//============================================================================
// Function MSClient_Uninitialize
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:58:14 PM
/*!
 \brief	 Uninitialize the library
 \par       Description:
 Uninitializes the library and frees all allocated memory

 \return    the success of the Function

*/
//============================================================================
GSbool __stdcall MSClient_Uninitialize();


//============================================================================
// Function MSClient_Engine
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 2:59:11 PM
/*!
 \brief	 Runs the Library
 \par       Description:
 Sends and receives library messages

 \return    the success of the Function

 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine to
  decode message in the queue and calling appropriate callback. (Milliseconds)

*/
//============================================================================
GSbool __stdcall MSClient_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);


//============================================================================
// Function MSClient_RequestMOTD
/*!
 \brief		Request the message of the day
 \par       Description:
 This function asks the server to send the message of the day based the client
 version and the requested language.  If the requested language is not available
 it will default to english.

	\par Callbacks:
	::MSClient_RequestMOTDCB

 \return    Status of the function call

 \retval	GS_TRUE	Account created successfully
 \retval	GS_FALSE	Failure to create account

 \param	szLanguage	The language to receive the MOTD in.

*/
//============================================================================
GSbool __stdcall MSClient_RequestMOTD(const GSchar *szLanguage);

//============================================================================
// Function MSClient_RequestGameServers
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:00:24 PM
/*!
 \brief	 Request a new list of game servers
 \par       Description:
 Request a list of Game Servers based on the ugly game name.
 MSClient_GameServerCB is called for each Game Server received.
 MSClient_RequestFinishedCB is called when the list is finished.  You can call
 MSClient_JoinGameServer before the list is finished.

 \return    the success of the Function

 \param	szGameName	The game name of the servers to request

*/
//============================================================================
GSbool __stdcall MSClient_RequestGameServers(const GSchar *szGameName);


//============================================================================
// Function MSClient_RefreshGameServers
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		6/12/02 5:25:28 PM
/*!
 \brief	 Refresh certain Game Servers
 \par       Description:
 Refresh the given Game Server. MSClient_GameServerCB is called for
 the Game Server refreshed.

 \return    the success of the Function

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_RefreshGameServer(GSint iLobbyID,GSint iRoomID);

//============================================================================
// Function MSClient_RequestAlternateInfo
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		6/12/02 5:25:28 PM
/*!
 \brief	 Request the alternate info of a game server.
 \par       Description:
 Request the alternate information for the given Game Server.
 MSClient_AlternateInfoCB is called for when the request is received.

 \return    The success of the Function.

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_RequestAlternateInfo(GSint iLobbyID,GSint iRoomID);

//============================================================================
// Function MSClient_JoinGameServer
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:24:58 PM
/*!
 \brief	 Tells the library to join a Game Server
 \par       Description:
 This lets the Lobby Server and GSRouter know what Game Server a player is on.
 MSClient_JoinFinishedCB is called when the Game Server has been joined.

 \return    The success of the Function

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.
 \param	szPassword	The password of the Game Server to join
 \param szGSVersion The GSVersion of the Game Server.
 \param szGameName The Game Name of the Ggame Server.
 \param pvPlayerInfo A buffer to send to the Game Server.
 \param iPlayerInfoSize The size of the pvPlayerInfo buffer in bytes.

*/
//============================================================================
GSbool __stdcall MSClient_JoinGameServer(GSint iLobbyID,GSint iRoomID,
		const GSchar *szPassword,const GSchar *szGSVersion,const GSchar *szGameName,
		const GSvoid *pvPlayerInfo, GSint iPlayerInfoSize);

//============================================================================
// Function MSClient_LeaveGameServer
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:31:07 PM
/*!
 \brief	 Leaves the Game Server
 \par       Description:
 Tells the Lobby Server and GSRouter that the player has left the GameServer.

 \return     The success of the Function

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_LeaveGameServer(GSint iLobbyID,GSint iRoomID);

//============================================================================
// Function MSClient_GameServerConnected
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		11/20/02 3:31:07 PM
/*!
 \brief	 Leaves the Game Server
 \par       Description:
 Tells the Lobby Server and that the player has connected to the GameServer.

 \return     The success of the Function

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_GameServerConnected(GSint iLobbyID,GSint iRoomID);


//============================================================================
// Function MSClient_CreateAccount
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/02 11:31:07 AM
/*!
 \brief	 Create an account
 \par       Description:
 Create an account on the Game Service.  This function has to be called before
 calling MSClient_Initialize() since that function requires a valid account.
 You must call MSClient_Engine() until you receive the callback
 MSClient_AccountCreationCB.  Then you can call MSClient_Uninitialize() followed
 by MSClient_Initialize() with the created accounts username and password.

 \return     The success of the Function

 \param	szMasterServerIP	The IP of the GSRouter
 \param	usMasterServerPort	The port of the GSRotuer
 \param	szVersion	Version of the player's client
 \param	szNickName	Alias of the player
 \param	szPassword	Password of the new player
 \param	szFirstName	Player's first name
 \param	szLastName	Player's last name
 \param	szEmail		Player's email
 \param	szCountry	Player's country

*/
//============================================================================
GSbool __stdcall MSClient_CreateAccount(const GSchar *szMasterServerIP,
		GSushort usMasterServerPort,const GSchar* szVersion,
		const GSchar* szNickName,const GSchar* szPassword,const GSchar* szFirstName,
		const GSchar* szLastName,const GSchar* szEmail,const GSchar* szCountry);

//============================================================================
// Function MSClient_ModifyAccount
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/02 11:31:07 AM
/*!
 \brief	 Create an account
 \par       Description:
 Modifies an account on the Game Service.  The success of the modification is
 received by the MSClient_ModifyAccountCB callback.

 \return     The success of the Function

 \param	szPassword	Password of the new player
 \param	szFirstName	Player's first name
 \param	szLastName	Player's last name
 \param	szEmail		Player's email
 \param	szCountry	Player's country

*/
//============================================================================
GSbool __stdcall MSClient_ModifyAccount(const GSchar* szPassword,
		const GSchar* szFirstName,const GSchar* szLastName, const GSchar* szEmail,
		const GSchar* szCountry);


//============================================================================
// Function MSClient_MatchStarted
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 2:00:49 PM
/*!
 \brief	 Tell the lobby server that the player is starting a match
 \par       Description:
Tell the lobby server that the match you are starting a match
This should be called after receiving the MSClient_MatchStartedCB callback.
Everyone in the room who will be submiting scores for the match has to call
this function.

 \return    Status of the function call

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_MatchStarted(GSint iLobbyID,GSint iRoomID);		
		
//============================================================================
// Function MSClient_InitMatchResult
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 1:54:52 PM
/*!
 \brief	 Initialize the librairie score submission system
 \par       Description:
 Initialize the librairie score submission system, this has to be called before
doing any other score submission-related function call. Note that you must
initialise the score submission system prior to any match that will be submitted.
This should be call after receiving the MatchStarted message, and the scores
should be cleared after being submitted to the lobby server with
LobbySend_SubmitMatchResult.

 \return  The success of the Function

 \param	uiMatchID	The match unique id as returned by CBLobbyRcv_MatchStarted

*/
//============================================================================
GSbool __stdcall MSClient_InitMatchResult(GSuint uiMatchID);


//============================================================================
// Function MSClient_SetMatchResult
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 1:56:18 PM
/*!
 \brief	 Set results for a player in a match
 \par       Description:
Set the result for a player in a match in the library's score submission
system. Each player should set the result for each other player that where
in the match including himself. This will insure validity of scores submitted
to the LobbyServer.

 \return    The success of the Function

 \param	szAlias		The alias of the player associated with the results
 \param	uiFieldID	The result field id
 \param	iFieldValue	The actual value that will be set for the specified field

*/
//============================================================================
GSbool __stdcall MSClient_SetMatchResult(GSchar* szAlias,
		GSuint uiFieldID,GSint iFieldValue);


//============================================================================
// Function MSClient_SubmitMatchResult
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 1:56:56 PM
/*!
 \brief	 Send the compiled scores of a match to the lobby server
 \par       Description:
Send the compiled scores of a match to the lobby server for archiving,
this will send a message to the lobby server and set the match result
on the server-side. You will receive a confirmation of the message sent
with the MSClient_SubmitMatchCB callback.

 \return    Status of the function call

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_SubmitMatchResult(GSint iLobbyID,GSint iRoomID);


//============================================================================
// Function MSClient_UninitMatchResult
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 1:58:22 PM
/*!
 \brief	 Unload the internal score submission system
 \par       Description:
Unload the internal score submission system previously initialized for a specific
match. Must be called after scores has been submitted to the lobby server.

 \return    Status of the function call

*/
//============================================================================
GSbool __stdcall MSClient_UninitMatchResult();


//============================================================================
// Function MSClient_MatchFinished
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 2:00:49 PM
/*!
 \brief	 Tell the lobby server that a match is finished
 \par       Description:
Tell the lobby server that the match you were playing has Finished
Everyone in the room has to call this function to confirm the end of the match after
submiting the scores

 \return    Status of the function call

 \param	iLobbyID	The ID of the Lobby Server
 \param	iRoomID	The ID of the Room.

*/
//============================================================================
GSbool __stdcall MSClient_MatchFinished(GSint iLobbyID,GSint iRoomID);

/*! @} end of group2 */

/*! @defgroup group3 Callback registration
\brief Register the callbacks function names.

Theses functions are used to register callbacks of the master server client API.
    @{
*/

//============================================================================
// Function MSClient_FixRequestMOTD
/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fFunction	Name of a function of type 
 		::MSClient_RequestMOTDCB

*/
//============================================================================
GSbool __stdcall MSClient_FixRequestMOTD(MSClient_RequestMOTDCB fFunction);

//============================================================================
// Function MSClient_FixGameServerCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:32:14 PM
/*!
 \brief	 Fix the GameServer Callback
 \par       Description:
 Register the callback function that will be called when information about a game server 
 is received.

 \return     Status of the function call

 \param	fFunction	The function to call.

*/
//============================================================================
GSbool __stdcall MSClient_FixGameServerCB(MSClient_GameServerCB fFunction);

//============================================================================
// Function MSClient_FixAlternateInfoCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:32:14 PM
/*!
 \brief	 Fix the AlternateInfo Callback
 \par       Description:
 Register the callback function that will be called when extrended information about a game server 
 is received.

 \return     The success of the Function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixAlternateInfoCB(MSClient_AlternateInfoCB fFunction);

//============================================================================
// Function MSClient_FixErrorCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:33:20 PM
/*!
 \brief	 Fix the Error Callback
 \par       Description:
 Tells the library what function to call when an Error is received

 \return    The success of the Function

 \param	fFunction	The Function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixErrorCB(MSClient_ErrorCB fFunction);


//============================================================================
// Function MSClient_FixInitFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:37:05 PM
/*!
 \brief	 Fix the InitFinished callback
 \par       Description:
 Tells the library what function to call when initialization has finish.

 \return    The success of the Function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixInitFinishedCB(MSClient_InitFinishedCB fFunction);

//============================================================================
// Function MSClient_FixLoginDisconnectCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		8/19/02 3:37:05 PM
/*!
 \brief	 Fix the LoginDisconnect callback
 \par       Description:
 Register the callback function that will be called when a disconnection
 from Ubi.com authentication servers is detected.

 \return    The success of the Function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixLoginDisconnectCB(
		MSClient_LoginDisconnectCB fFunction);

//============================================================================
// Function MSClient_FixLoginDisconnectCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		8/19/02 3:37:05 PM
/*!
 \brief	 Fix the LobbyDisconnect callback
 \par       Description:
 Register the callback function that will be called when a disconnection
 from Ubi.com matchmaking servers is detected.

 \return    The success of the Function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixLobbyDisconnectCB(
		MSClient_LobbyDisconnectCB fFunction);

//============================================================================
// Function MSClient_FixRequestFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:40:32 PM
/*!
 \brief	 Fix the RequestFinished callback
 \par       Description:
 Register the callback function that will be called when a disconnection
 from Ubi.com matchmaking servers is detected.

 \return    The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixRequestFinishedCB(
		MSClient_RequestFinishedCB fFunction);


//============================================================================
// Function MSClient_FixJoinFinishedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		4/19/02 3:42:19 PM
/*!
 \brief	 Fix the JoinFinished callback
 \par       Description:
 Tells the library what function to call when the Game Server has been joined.

 \return    The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixJoinFinishedCB(MSClient_JoinFinishedCB fFunction);

//============================================================================
// Function MSClient_AccountCreationCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/03 11:15:19 AM
/*!
 \brief	 Fix the AccountCreation callback
 \par       Description:
 Tells the library what function to call when the Account Creation has finished.

 \return    The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixAccountCreationCB(MSClient_AccountCreationCB fFunction);

//============================================================================
// Function MSClient_ModifyAccountCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		5/30/03 11:15:19 AM
/*!
 \brief	 Fix the ModifyUser callback
 \par       Description:
 Tells the library what function to call when the Account Modification has
 finished.

 \return    The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixModifyAccountCB(MSClient_ModifyAccountCB fFunction);

//============================================================================
// Function MSClient_FixMatchStartedCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 2:02:01 PM
/*!
 \brief	 Fix the Match Started callback
 \par       Description:
Tells the library what function to call when a match has been started.

 \return    The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixMatchStartedCB(MSClient_MatchStartedCB fFunction);


//============================================================================
// Function MSClient_FixSubmitMatchCB
// Author:		Scott Schmeisser  gsdevteam@ubisoft.com
// Date:		7/25/02 2:03:13 PM
/*!
 \brief	 Fix the SubmitMatch callback
 \par       Description:
Tells the library what function to call when a SubmitMatch responce has been
received

 \return   The success of the function

 \param	fFunction	The function to call

*/
//============================================================================
GSbool __stdcall MSClient_FixSubmitMatchCB(MSClient_SubmitMatchCB fFunction);

/*! @} end of group3 */


}
#endif _MSCLIENTLIBRARY_H_
