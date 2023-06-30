
//****************************************************************************
//*   Author:  Guillaume Plante <gsdevteam@ubisoft.com>
 /*!  \file    GSClientLibrary.h
  *   \brief   Available functions for basic ubi.com services usage.
  *   \par     Description:
  *   This file provide all the methods that allow developers to use the basic
  *	  ubi.com services.
  */
//****************************************************************************

/*!
\mainpage gs-sdk-base
\section intro Introduction
ubi.com GameService base SDK


\section description Description
This SDK is the interface to the basic ubi.com services.
 - The <i><b>login service</b></i> allow you to connect and authenticate users
 		on the ubi.com network. This service also provide the functionalities to
		create and manage ubi.com accounts.
 - The <i><b>friends service</b></i> allow you to manage a list of friends,
 		send private message (pages) to them, in short it provides buddy-list and
		instant-messaging functionalities. 
 - The <i><b>lobby service</b></i> is the core of the ubi.com match-making
 		functionalities. This service will enable you to join game lobbies,join,
		create and manage rooms.
 - The <i><b>persistent storage service</b></i> allows you to store/retrieve
 		permanent game information on the ubi.com network.
 - The <i><b>ladder query service</b></i> allows you to retieve ladder
 		information that were previously submitted to ubi.com.
 - The <i><b>remote algorithm execution service</b></i> allows you to execute
        custom algorithms stored on ubi.com.

*/

#ifndef __GSCLIENTLIBRARY_H__
#define __GSCLIENTLIBRARY_H__

// Error return code definitions
#include "GSErrors.h"

// login service callbacks definitions
#include "GSLoginCB.h"

// friends service callbacks definitions
#include "GSFriendsCB.h"

// lobby service callbacks definitions
#include "GSLobbyCB.h"

// persistent storage service callbacks definitions
#include "GSPersistentCB.h"

// ladder query service callbacks definitions
#include "GSLadderQueryCB.h"

// remote algorithm execution service callbacks definitions
#include "GSRemoteAlgorithmCB.h"

// ladder query service specific definitions
#include "LadderDefines.h"

// remote algorithm execution service definitions
#include "RemoteAlgorithmDefines.h"

#ifdef __cplusplus

	GSbool __stdcall Login_FixCallbacks(clLoginCallbacks* pLoginCB);
	GSbool __stdcall Friends_FixCallbacks(clFriendsCallbacks* pFriendsCB);
	GSbool __stdcall Lobby_FixCallbacks(clLobbyCallbacks* pLobbyCB);
	GSbool __stdcall PS_FixCallbacks(clPersistentCallbacks* pPSCB);
	GSbool __stdcall LadderQuery_FixCallbacks(
			clLadderQueryCallbacks* pLadderQueryCB);
	
#endif //__cplusplus



extern "C" {

//============================================================================
// Function GSClientLibrary_Initialize

/*!
 \brief		Initialisation function for the client library
 \par       Description:
 Initialize the communication ressources. Must be call prior to any other
 functions.

 \return    Status of the function call

 \retval	GS_TRUE		Initialisation was successfull
 \retval	GS_FALSE	Initialisation failed

*/
//============================================================================
GSbool __stdcall GSClientLibrary_Initialize();


//============================================================================
// Function GSClientLibrary_Uninitialize

/*!
 \brief	 Free memory.
 \par	 Description:
 This function free memory that as been allocated by the library.
 Call this when you're done using the library, use in pair with
 GSClientLibrary_Initialize.

 \return    Status of the function call

 \retval	GS_TRUE		Library deallocation was successfull
 \retval	GS_FALSE	Library deallocation failed
*/
//============================================================================
GSbool __stdcall GSClientLibrary_Uninitialize();


//============================================================================
// Function GSGetLocalIPAddress


/*!
 \brief	 Get local ip address of the client
 \par    Description:
	This function retrieves the local ip address of the client.  Should only be
	called when the connection to the server is open, otherwise it will return
	GS_FALSE.

 \return    Status of the function call

 \retval	GS_TRUE		IP address detection successfull
 \retval	GS_FALSE	IP address detection failed

 \param		szIPAddress	Character string that contains the ip address


*/
//============================================================================
GSbool __stdcall GSGetLocalIPAddress(GSchar* szIPAddress);


/*! @defgroup group1 Login Service
\brief Functions used to connect to the <i><b>login service</b></i>.

These function are used to send login-related
message to the server and to register the callbacks
functions related to login messages
    @{
*/

/*! @defgroup group1_1 Functions
\brief Messages sent to the game service.

These function are used to send login-related
messages to the server
    @{
*/



//============================================================================
// Function Login_Engine


/*!
 \brief		Update connection status-messages handling relating to the login
 		service
 \par       Description:
 Updates the connection between the client and the server, and handles the
 delivery of queued up messages and reception of messages relating to the login
 service.  This function should be called regularly to ensure that the
 application will run smoothly.

 \return    Status of the function call

 \retval	GS_TRUE		The connection is ok and function call was a success
 \retval	GS_FALSE	There has been a communication problem between the client 
 		and the server

 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine
 		to decode message in the queue and calling appropriate callback.
		(Milliseconds)

*/
//============================================================================
GSbool __stdcall Login_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);


//============================================================================
// Function LoginSend_Connect


/*!
 \brief		Connect to the router
 \par       Description:
 This function is opens a connect to the router or wait module.  When this
 function returns successfully LoginSend_LoginRouter() should be called.  When
 connecting to the wait module you should call LoginSend_LoginWaitModule() next.

 \return    Status of the function call

 \retval	GS_TRUE		Connection established
 \retval	GS_FALSE	Failed to established connection

 \param	szAddress	Router's IP address
 \param	usPort		Router's client port number

*/
//============================================================================
GSbool __stdcall LoginSend_Connect(const GSchar* szAddress, GSushort usPort);



//============================================================================
// Function LoginSend_Disconnect


/*!
 \brief		Disconnect from the router
 \par       Description:
 Disconnect from the Server

 \return    Status of the function call

 \retval	GS_TRUE		Disconnection successfull
 \retval	GS_FALSE	Failed to disconnect

*/
//============================================================================
GSbool __stdcall LoginSend_Disconnect();


//============================================================================
// Function LoginSend_LoginRouter


/*!
 \brief		Login to the router
 \par       Description:
 This function is used to login to the router using your username and password
 for your game service account.  The szVersion parameter will is the games
 client version.  This will be given to you by the Game Service developers.

 \par Callbacks:
 ::CBLoginRcv_LoginRouterResult

 \return    Status of the function call

 \retval	GS_TRUE	Login successfull
 \retval	GS_FALSE	Login failed

 \param		szUsername		The username of the player that wants to log in
 \param		szPassword	The password
 \param		szVersion	The version of the client
 \param		bPublicIP	If this flag is enabled, the ip address will be sent to
 		players in your along with others player infos, if not your ip will not be
		sent to other players, thus preventing them to ping you.
*/
//============================================================================
GSbool __stdcall LoginSend_LoginRouter(const GSchar* szUsername,
		const GSchar* szPassword, const GSchar* szVersion,
		GSbool bPublicIP = GS_TRUE);



//============================================================================
// Function LoginSend_JoinWaitModule


/*!
 \brief		Ask to join the wait module
 \par       Description:
  This function is used to ask to join the wait module the router will respond
	with the ip and port of the wait module to join.  This should only be called
	after receving a successfull ::CBLoginRcv_LoginRouterResult

	\par Callbacks:
	::CBLoginRcv_JoinWaitModuleResult

 \return    Status of the function call

 \retval	GS_TRUE		Request successfull
 \retval	GS_FALSE	Request failure

*/
//============================================================================
GSbool __stdcall LoginSend_JoinWaitModule();


//============================================================================
// Function LoginSend_LoginWaitModule


/*!
 \brief		Login to the wait module
 \par       Description:
  This function is used to login to the waitmodule.  It should only be called
	after successfully using LoginSend_Connect() to connect to the wait module.
	
	\par Callbacks:
	::CBLoginRcv_LoginWaitModuleResult


 \return    Status of the function call

 \retval	GS_TRUE		Login successfull
 \retval	GS_FALSE	Login failed

 \param	szUsername	The Username of the player

*/
//============================================================================
GSbool __stdcall LoginSend_LoginWaitModule(const GSchar* szUsername);



//============================================================================
// Function LoginSend_PlayerInfo


/*!
 \brief		Get player account information
 \par       Description:
  This function is used to get information stored for a player.  Inorder to use
	the chat sdk it is important to call this on yourself after sucessfully
	logging in.  This is because you must get your szIRCID before using chat and
	the correct case for the username.
	
	\par Callbacks:
	::CBLoginRcv_PlayerInfo


 \return    Status of the function call

 \retval	GS_TRUE	    Success
 \retval	GS_FALSE	Failure

 \param		szUsername		The username of the the player you want the info of.

*/
//============================================================================
GSbool __stdcall LoginSend_PlayerInfo(const GSchar* szUsername);


//============================================================================
// Function LoginSend_Sleep


/*!
 \brief		Put player in sleep mode
 \par       Description:
  This function is used to put the player in sleep mode, which means he will not
	receive any message from the server until he calls LoginSend_WakeUp(). The
	message are stored when a player is in sleep mode.

 \par Related Function:
 LoginSend_WakeUp()

 \return    Status of the function call

 \retval	GS_TRUE		Successfully put in sleep mode
 \retval	GS_FALSE	Failure to put in sleep mode

*/
//============================================================================
GSbool __stdcall LoginSend_Sleep();


//============================================================================
// Function LoginSend_WakeUp


/*!
 \brief		Put player in awake mode
 \par       Description:
  This function is used to put the player in "awake" mode which means that he
	will start to receive message from the router again.  Any system and page
	messages that were saved while the player was asleep will be sent.

 \par Related Function:
 LoginSend_Sleep()

 \return    Status of the function call

 \retval	GS_TRUE		Successfully put in awake mode
 \retval	GS_FALSE	Failure to put in awake mode

*/
//============================================================================
GSbool __stdcall LoginSend_WakeUp();



//============================================================================
// Function LoginSend_ModifyAccount


/*!
 \brief		Modify player informations
 \par       Description:
  This function is used to modify account information for a player.
	
	\par Callbacks:
	::CBLoginRcv_ModifyUserResult


 \return    Status of the function call

 \retval	GS_TRUE	Successfully modify the account
 \retval	GS_FALSE	Failure to modify the account

 \param	szPassword		the new Password for the player
 \param	szFirstName		the new First name for the player
 \param	szLastName		the new Last name for the player
 \param	szEmail			the new Email for the player
 \param	szCountry		the new Country for the player

*/
//============================================================================
GSbool __stdcall LoginSend_ModifyAccount(const GSchar* szPassword,
		const GSchar* szFirstName, const GSchar* szLastName, const GSchar* szEmail,
		const GSchar* szCountry);



//============================================================================
// Function LoginSend_CreateAccount


/*!
 \brief		Account creation
 \par       Description:
  This function is used to create a new account on the gs network
  <b>NOTE</b>: <i>You have to call this function before login to the
	waitmodule.</i> and after the LoginSend_Connect function.

	\par Callbacks:
	::CBLoginRcv_AccountCreationResult


 \return    Status of the function call

 \retval	GS_TRUE	Account created successfully
 \retval	GS_FALSE	Failure to create account

 \param	szVersion	Version of the player'client
 \param	szUsername	Username of the player
 \param	szPassword	Password of the new player
 \param	szFirstName	Player's first name
 \param	szLastName	Player's last name
 \param	szEmail		Player's email
 \param	szCountry	Player's country

*/
//============================================================================
GSbool __stdcall LoginSend_CreateAccount(const GSchar* szVersion,
		const GSchar* szUsername, const GSchar* szPassword,
		const GSchar* szFirstName, const GSchar* szLastName, const GSchar* szEmail,
		const GSchar* szCountry);



//============================================================================
// Function LoginSend_RequestMOTD


/*!
 \brief		Request the message of the day
 \par       Description:
 This function asks the server to send the message of the day based the client
 version and the requested language.  If the request language is not available
 it will default to english.

	\par Callbacks:
	::CBLoginRcv_RequestMOTD

 \return    Status of the function call

 \retval	GS_TRUE	Account created successfully
 \retval	GS_FALSE	Failure to create account

 \param	szLanguage	The language to receive the MOTD in.

*/
//============================================================================
GSbool __stdcall LoginSend_RequestMOTD(const GSchar *szLanguage);

/*! @} end of group1_1 */

/*! @defgroup group1_2 Callback registration
\brief Register the callbacks function names

Theses functions are used to set the function name
for the callbacks of the <b><i>login service</i></b>.
    @{
*/


//============================================================================
// Function LoginFix_PlayerInfo


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerInfo	Name of a function of type ::CBLoginRcv_PlayerInfo

*/
//============================================================================
GSbool __stdcall LoginFix_PlayerInfo(CBLoginRcv_PlayerInfo fPlayerInfo);


//============================================================================
// Function LoginFix_JoinWaitModuleResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fJoinWaitModuleResult	Name of a function of type
 		::CBLoginRcv_JoinWaitModuleResult

*/
//============================================================================
GSbool __stdcall LoginFix_JoinWaitModuleResult(
		CBLoginRcv_JoinWaitModuleResult fJoinWaitModuleResult);


//============================================================================
// Function LoginFix_LoginRouterResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginRouterResult	Name of a function of type 
 		::CBLoginRcv_LoginRouterResult

*/
//============================================================================
GSbool __stdcall LoginFix_LoginRouterResult(
		CBLoginRcv_LoginRouterResult fLoginRouterResult);


//============================================================================
// Function LoginFix_LoginWaitModuleResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginWaitModuleResult	Name of a function of type 
 		::CBLoginRcv_LoginRouterResult

*/
//============================================================================
GSbool __stdcall LoginFix_LoginWaitModuleResult(
		CBLoginRcv_LoginWaitModuleResult fLoginWaitModuleResult);


//============================================================================
// Function LoginFix_SystemPage


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to to set the function name for the callback.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fSystemPage	Name of a function of type ::CBLoginRcv_SystemPage

*/
//============================================================================
GSbool __stdcall LoginFix_SystemPage(CBLoginRcv_SystemPage fSystemPage);


//============================================================================
// Function LoginFix_LoginDisconnection


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginDisconnection	Name of a function of type 
 		::CBLoginRcv_LoginDisconnection

*/
//============================================================================
GSbool __stdcall LoginFix_LoginDisconnection(
		CBLoginRcv_LoginDisconnection fLoginDisconnection);



//============================================================================
// Function LoginFix_LoginAccountCreationResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginCreateAccount	Name of a function of type 
 		::CBLoginRcv_AccountCreationResult

*/
//============================================================================
GSbool __stdcall LoginFix_LoginAccountCreationResult(
		CBLoginRcv_AccountCreationResult fLoginCreateAccount);

//============================================================================
// Function LoginFix_ModifyUserResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fLoginModifyUser	Name of a function of type 
 		::CBLoginRcv_ModifyUserResult

*/
//============================================================================
GSbool __stdcall LoginFix_ModifyUserResult(
		CBLoginRcv_ModifyUserResult fLoginModifyUser);
		
		
//============================================================================
// Function LoginFix_RequestMOTD


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fRequestMOTD	Name of a function of type 
 		::CBLoginRcv_RequestMOTD

*/
//============================================================================
GSbool __stdcall LoginFix_RequestMOTD(CBLoginRcv_RequestMOTD fRequestMOTD);
/*! @} end of group1_2 */

/*! @defgroup group_LoginCB Callbacks
\brief The login service function callbacks

    @{
		@}
*/

/*! @} end of group1 */




/*! @defgroup group2 Friends Service
\brief Functions used to connect and use the <b><i>friend service</i></b>.

These functions are used to send friend-related
messages to the server and to register the callbacks
    @{
*/

/*! @defgroup group2_1 Functions
\brief Messages sent to the game service.

These function are used to send friend-related
messages to the server
    @{
*/

//============================================================================
// Function Friends_Engine


/*!
 \brief		Update connection status-messages handling relating to the friend
 		service
 \par       Description:
 Updates the connection between the client and the server, and handles
 the delivery of queued up messages and reception of messages relating to the
 friend service.  This function should be called regularly to ensure that the
 application will run smoothly.

 \return    Status of the function call

 \retval	GS_TRUE	The connection is ok and function call was a success
 \retval	GS_FALSE	There has been a problem with the communication of
 messages between the client and the server

 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine
	to decode message in the queue and calling appropriate callback.
	(Milliseconds)

*/
//============================================================================
GSbool __stdcall Friends_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);


//============================================================================
// Function FriendsSend_Connect


/*!
 \brief		This function is deprecated
 \par       Description:
  You don't need to call this function anymore to connect to the friends
  service. See FriendsSend_Login()

 \return    Status of the function call

 \retval	GS_TRUE		Successfully connected to the friends service
 \retval	GS_FALSE	Failure to connect to the friends service

*/
//============================================================================
GSbool __stdcall FriendsSend_Connect();


//============================================================================
// Function FriendsSend_Disconnect


/*!
 \brief		Disconnect from the friends service
 \par       Description:
  This function is used to disconnect from the friends service


 \return    Status of the function call

 \retval	GS_TRUE		Successfully disconnect from the friends service
 \retval	GS_FALSE	Failure to disconnect from the friends service

*/
//============================================================================
GSbool __stdcall FriendsSend_Disconnect();



//============================================================================
// Function FriendsSend_Login


/*!
 \brief		Log into the friends service
 \par       Description:
  This function is used to log into the friends service.  See define.h for the
	list of statuses.

	\par Callbacks:
	::CBFriendsRcv_LoginResult<br>
	::CBFriendsRcv_UpdateFriend for all your friends<br>


 \return    Status of the function call

 \retval	GS_TRUE		Successfully logged into the friends service
 \retval	GS_FALSE	Failure to log into the friends service

 \param	lStatus	Status of the player (default at 0)
 \param	lMood	Mood of the player (default at 0)

*/
//============================================================================
GSbool __stdcall FriendsSend_Login(GSint lStatus = 0, GSint lMood = 0);


//============================================================================
// Function FriendsSend_AddFriend


/*!
 \brief		Add a new friend
 \par       Description:
  This function is used to add a new friend to the player's friend list.  These
	friends are stored on the server so you don't have to be stored locally.

	\par Callbacks:
	::CBFriendsRcv_AddFriend<br>
	::CBFriendsRcv_UpdateFriend for the new friend<br>

 \return    Status of the function call

 \retval	GS_TRUE		Successfully added new friend
 \retval	GS_FALSE	Failure when added new friend

 \param	szUsername	The username of the player that will be added
 \param	szGroup		The group to which the player will be added.  This is just a
 string so that is saved for the friend.  You can have any number of groups.
 \param	iOptions	Friend options.  See define.h

*/
//============================================================================
GSbool __stdcall FriendsSend_AddFriend(const GSchar* szUsername,
		const GSchar* szGroup, GSint iOptions);

//============================================================================
// Function FriendsSend_DelFriend


/*!
 \brief		Remove a friend from friend list
 \par       Description:
  This function is used to remove a friend from the player's friend list
	
	\par Callbacks:
	::CBFriendsRcv_DelFriend


 \return    Status of the function call

 \retval	GS_TRUE		Successfully removed friend
 \retval	GS_FALSE	Failure when removing friend

 \param	szUsername	username of the friend we want to remove

*/
//============================================================================
GSbool __stdcall FriendsSend_DelFriend (const GSchar* szUsername);

//============================================================================
// Function FriendsSend_FriendList


/*!
 \brief		Get all friends in friend list
 \par       Description:
  This function is used to get the list of friends that are in our friend list.
	This normally doesn't have to be called because the server pushes the
	::CBFriendsRcv_UpdateFriend callbacks to the client.  If client for some
	reason forgets the list of friends they have this function can called to force
	an ::CBFriendsRcv_UpdateFriend for all their friends.

	\par Callbacks:
	::CBFriendsRcv_UpdateFriend for every friend


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall FriendsSend_FriendList();

//==============================================================================
// Function FriendsSend_IgnorePlayer
/*!
\brief Adds someone to the player's ignore-list
\par   Description:
       This function adds a ubi.com username to the player's ignore-list. When
       someone is on a player's ignore-list, it cannot send pages or
       peer-messages to the player ignoring it. The ignore-list is stored
       remotely on the servers.

       To use this function successfully, you needs to be logged in to ubi.com
       and to the Friends service.

\par   Callbacks:
       ::CBFriendsRcv_IgnorePlayer<br>

\param      szPlayer    The ubi.com username of the player to ignore
                        (case-insensitive)
                        
\return     Result code of the operation
\retval     GSS_OK              There was no error
\retval     GSE_NOTINITIALIZED  The library needs to be initialised
\retval     GSE_UNEXPECTED      The user is not logged in to ubi.com
\retval     GSE_BADMODE         The user is not logged in to the Friends service
\retval     GSE_BADARG          The szPlayer argument is NULL or empty
*/
//==============================================================================
GSRESULT __stdcall FriendsSend_IgnorePlayer( const GSchar * szPlayer );

//==============================================================================
// Function FriendsSend_UnignorePlayer
/*!
\brief Removes someone to the player's ignore-list
\par   Description:
       This function removes a ubi.com username to the player's ignore-list.

       To use this function successfully, you needs to be logged in to ubi.com
       and to the Friends service.

\par   Callbacks:
       ::CBFriendsRcv_UnignorePlayer<br>

\param      szPlayer    The ubi.com username of the player to remove from the
                        ignore-list (case-insensitive)
                        
\return     Result code of the operation
\retval     GSS_OK              There was no error
\retval     GSE_NOTINITIALIZED  The library needs to be initialised
\retval     GSE_UNEXPECTED      The user is not logged in to ubi.com
\retval     GSE_BADMODE         The user is not logged in to the Friends service
\retval     GSE_BADARG          The szPlayer argument is NULL or empty
*/
//==============================================================================
GSRESULT __stdcall FriendsSend_UnignorePlayer( const GSchar * szPlayer );

//==============================================================================
// Function FriendsSend_ListIgnoredPlayers
/*!
\brief Retrieves the ignore-list of the player
\par   Description:
       This function gets the remotely stored ignore-list of a player

       To use this function successfully, you needs to be logged in to ubi.com
       and to the Friends service.

\par   Callbacks:
       ::CBFriendsRcv_ListIgnoredPlayers<br>
       ::CBFriendsRcv_IgnoredPlayer<br>

\return     Result code of the operation
\retval     GSS_OK              There was no error
\retval     GSE_NOTINITIALIZED  The library needs to be initialised
\retval     GSE_UNEXPECTED      The user is not logged in to ubi.com
\retval     GSE_BADMODE         The user is not logged in to the Friends service
*/
//==============================================================================
GSRESULT __stdcall FriendsSend_ListIgnoredPlayers();

//============================================================================
// Function FriendsSend_PagePlayer

/*!
 \brief		Send a page to a friend
 \par       Description:
  This function is used to send a page to a player.  If the player isn't online
	the message will be saved and sent to them the next time they login to the
	friends service.

	\par Callbacks:
	::CBFriendsRcv_PagePlayer telling you if the message was sent.
	::CBFriendsRcv_Page is sent to the other player

 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	szUsername	The username of the recipient player
 \param	szMessage	The actual message

*/
//============================================================================
GSbool __stdcall FriendsSend_PagePlayer(const GSchar* szUsername,
		const GSchar* szMessage);


//============================================================================
// Function FriendsSend_PeerPlayer


/*!
 \brief		Send data to a friend
 \par       Description:
  This function is used to send binary data to a friend.  You can only send peer
	messages to players that are have the same client version as you.
	\par Callbacks:
	::CBFriendsRcv_PeerPlayer<br>
	::CBFriendsRcv_PeerMsg is sent to the other player<br>


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	szUsername	Username of the recipient player of the data.
 \param	p_Buffer	Data buffer.
 \param	uiLength	Lenght of the buffer.

*/
//============================================================================
GSbool __stdcall FriendsSend_PeerPlayer(const GSchar* szUsername,
		GSvoid* p_Buffer, GSuint uiLength);


//============================================================================
// Function FriendsSend_StatusChange


/*!
 \brief		Change the player status
 \par       Description:
  This function is used to set a new status and mood for the current player.

	\par Callbacks:
	::CBFriendsRcv_StatusChange<br>
	::CBFriendsRcv_UpdateFriend is sent to anyone who has the player as friend.

 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	iStatus		New status of the player
 \param	iMood		New mood of the player

*/
//============================================================================
GSbool __stdcall FriendsSend_StatusChange(GSint iStatus, GSint iMood);


//============================================================================
// Function FriendsSend_ChangeFriend


/*!
 \brief		Change friend properties
 \par       Description:
  This function is used to change friend properties like options or the group
	he is in.

	\par Callbacks:
	::CBFriendsRcv_UpdateFriend will have the updated info.


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szUsername	username of the friend
 \param	szGroup		New group of the player
 \param	iOptions	Friend options

*/
//============================================================================
GSbool __stdcall FriendsSend_ChangeFriend(const GSchar* szUsername,
		const GSchar* szGroup, GSint iOptions);

//============================================================================
// Function FriendsSend_SearchPlayer


/*!
 \brief		Search for a player.
 \par       Description:
  This function is used to search a player in the database.  Any of the
	arguments can be left blank, the server  will return a list of matching player
	that as the search pattern.

	\par Callback:
	::CBFriendsRcv_SearchPlayer


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	szUsername		username of the player
 \param	szSurName		Player last name
 \param	szFirstName	Player first name
 \param	szCountry		Player's country
 \param	szEmail		Player's email
 \param	iSex			Player's gender
 \param	szGame			Player's favorite game

*/
//============================================================================
GSbool __stdcall FriendsSend_SearchPlayer(const GSchar* szUsername,
		const GSchar* szSurName, const GSchar* szFirstName,
		const GSchar* szCountry, const GSchar* szEmail, GSint iSex,
		const GSchar* szGame);

//============================================================================
// Function FriendsSend_GetPlayerScores


/*!
 \brief		Get player score
 \par       Description:
  DEPRECATED: This function has been replaced by the Ladder Query Service

	\par Callback:
	::CBFriendsRcv_ScoreCard


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	szUsername	Username of the player we want to get the score from.

*/
//============================================================================
GSbool __stdcall FriendsSend_GetPlayerScores(const GSchar* szUsername);




/*! @} end of group2_1 */

/*! @defgroup group2_2 Callback registration
\brief Register the callbacks function names

Theses functions are used to set the function name
for the callbacks of the <b><i>friends service</i></b>.
    @{
*/

//============================================================================
// Function FriendsFix_LoginResult


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginResult	Name of a function of type ::CBFriendsRcv_LoginResult

*/
//============================================================================
GSbool __stdcall FriendsFix_LoginResult(CBFriendsRcv_LoginResult fLoginResult);

//============================================================================
// Function FriendsFix_AddFriend


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fAddFriend	Name of a function of type ::CBFriendsRcv_AddFriend

*/
//============================================================================
GSbool __stdcall FriendsFix_AddFriend(CBFriendsRcv_AddFriend fAddFriend);

//============================================================================
// Function FriendsFix_DelFriend


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fDelFriend	Name of a function of type ::CBFriendsRcv_DelFriend

*/
//============================================================================
GSbool __stdcall FriendsFix_DelFriend(CBFriendsRcv_DelFriend fDelFriend);

//============================================================================
// Function FriendsFix_IgnorePlayer


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fIgnorePlayer	Name of a function of type ::CBFriendsRcv_IgnorePlayer

*/
//============================================================================
GSvoid __stdcall FriendsFix_IgnorePlayer( CBFriendsRcv_IgnorePlayer fIgnorePlayer );

//============================================================================
// Function FriendsFix_UnignorePlayer


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fUnignorePlayer	Name of a function of type ::CBFriendsRcv_UnignorePlayer

*/
//============================================================================
GSvoid __stdcall FriendsFix_UnignorePlayer( CBFriendsRcv_UnignorePlayer fUnignorePlayer );

//============================================================================
// Function FriendsFix_ListIgnoredPlayers


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fListIgnoredPlayers	Name of a function of type ::CBFriendsRcv_ListIgnoredPlayers

*/
//============================================================================
GSvoid __stdcall FriendsFix_ListIgnoredPlayers( CBFriendsRcv_ListIgnoredPlayers fListIgnoredPlayers );

//============================================================================
// Function FriendsFix_IgnoredPlayer


/*!
 \brief		Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fIgnoredPlayer	Name of a function of type ::CBFriendsRcv_IgnoredPlayer

*/
//============================================================================
GSvoid __stdcall FriendsFix_IgnoredPlayer( CBFriendsRcv_IgnoredPlayer fIgnoredPlayer );

//============================================================================
// Function FriendsFix_Page


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fPage	Name of a function of type ::CBFriendsRcv_Page

*/
//============================================================================
GSbool __stdcall FriendsFix_Page(CBFriendsRcv_Page fPage);

//============================================================================
// Function FriendsFix_PagePlayer


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fPagePlayer	Name of a function of type ::CBFriendsRcv_PagePlayer

*/
//============================================================================
GSbool __stdcall FriendsFix_PagePlayer(CBFriendsRcv_PagePlayer fPagePlayer);

//============================================================================
// Function FriendsFix_PeerMsg


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fPeerMsg	Name of a function of type ::CBFriendsRcv_PeerMsg

*/
//============================================================================
GSbool __stdcall FriendsFix_PeerMsg(CBFriendsRcv_PeerMsg fPeerMsg);

//============================================================================
// Function FriendsFix_PeerPlayer


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fPeerPlayer	Name of a function of type ::CBFriendsRcv_PeerPlayer

*/
//============================================================================
GSbool __stdcall FriendsFix_PeerPlayer(CBFriendsRcv_PeerPlayer fPeerPlayer);

//============================================================================
// Function FriendsFix_StatusChange


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fStatusChange	Name of a function of type ::CBFriendsRcv_StatusChange

*/
//============================================================================
GSbool __stdcall FriendsFix_StatusChange(
		CBFriendsRcv_StatusChange fStatusChange);

//============================================================================
// Function FriendsFix_ChangeFriend


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fChangeFriend	Name of a function of type ::CBFriendsRcv_ChangeFriend

*/
//============================================================================
GSbool __stdcall FriendsFix_ChangeFriend(
		CBFriendsRcv_ChangeFriend fChangeFriend);

//============================================================================
// Function FriendsFix_UpdateFriend


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fUpdateFriend	Name of a function of type ::CBFriendsRcv_UpdateFriend

*/
//============================================================================
GSbool __stdcall FriendsFix_UpdateFriend(
		CBFriendsRcv_UpdateFriend fUpdateFriend);

//============================================================================
// Function FriendsFix_SearchPlayer


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fSearchPlayer	Name of a function of type ::CBFriendsRcv_SearchPlayer

*/
//============================================================================
GSbool __stdcall FriendsFix_SearchPlayer(
		CBFriendsRcv_SearchPlayer fSearchPlayer);

//============================================================================
// Function FriendsFix_ScoreCard


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to set the function name for the callback


 \return    Status of the function call

 \retval	GS_TRUE	Success
 \retval	GS_FALSE	Failure

 \param	fScoreCard	Name of a function of type ::CBFriendsRcv_ScoreCard

*/
//============================================================================
GSbool __stdcall FriendsFix_ScoreCard(CBFriendsRcv_ScoreCard fScoreCard);


/*! @} end of group2_2 */

/*! @defgroup group_FriendCB Callbacks
\brief The friends service function callbacks

    @{
		@}
*/



/*! @} end of group2 */


/*! @defgroup group4 Lobby Service
\brief Functions used to connect to the <b><i>lobby service</i></b>.

These function are used to send lobby related
message to the server and to register the callbacks
    @{
*/

/*! @defgroup group4_1 Functions
\brief Messages sent to the lobby server.

These function are used to send lobby related
message to the server
    @{
*/


//============================================================================
// Function		Lobby_Engine
/*!
 \brief	 Update connection status-messages handling relating to the lobby
 service

 \par       Description:
 Updates the connection between the client and the server, and handles the
 delivery of queued up messages and reception of messages relating to the lobby
 service. This function should be called regularly to ensure that the
 application will run smoothly.

 \return    Status of the function call

 \retval	GS_TRUE	 The connection is ok and function call was a success.
 \retval	GS_FALSE There has been a communication problem.

 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine
  to decode message in the queue and calling appropriate callback.
	(Milliseconds)

*/
//============================================================================
GSbool __stdcall Lobby_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);



//============================================================================
// Function		LobbySend_Connect


/*!
 \brief	 This function is deprecated
 \par       Description:
 You don't need to call this function anymore to open up a connection
 to a lobby server.

 \return    Status of the function call

 \retval	GS_TRUE	    The connection is ok and function call was a success
 \retval	GS_FALSE	There has been a problem with the communication of
 messages between the client and the server

*/
//============================================================================
GSbool __stdcall LobbySend_Connect();


//============================================================================
// Function LobbySend_Disconnect


/*!
 \brief	 Disconnect from the lobby server
 \par       Description:
 This function will the player from a specific lobby server. After disconnecting
 from a lobby server, you will still receive Lobby-related messages if your are
 connected to another lobby server.

 \par Callbacks:
 ::CBLobbyRcv_GroupRemove for every group on that Lobby Server<br>
 ::CBLobbyRcv_LobbyDisconnection for every Lobby Server<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iLobbyServerID	The id of the lobby server you are disconnecting from.

*/
//============================================================================
GSbool __stdcall LobbySend_Disconnect(GSint iLobbyServerID);

//============================================================================
// Function LobbySend_DisconnectAll


/*!
 \brief	 Disconnect from the lobby server
 \par      Description:
 Disconnect from all the Lobby servers. You will not receive anymore
 lobby-related messages after calling this function.
 
 \par Callbacks:
 ::CBLobbyRcv_GroupRemove for every group<br>
 ::CBLobbyRcv_LobbyDisconnection for every Lobby Server<br>
 ::CBLobbyRcv_LobbyDisconnectAll<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall LobbySend_DisconnectAll();


//============================================================================
// Function LobbySend_Login


/*!
 \brief	 This function is used to log into the lobby service.
 \par       Description:
   This function is used to log into the lobby service.
  On logging to that service, the user will receive a list of all the groups
  on the lobby server. The parameter szGame is used to narrow the list of groups
  received with only those who are running the game specified in the argument
  field. More than one game can be specify, each ugly game name must be
	separated by one of those character: | ; or , 

	\par Callbacks:
	::CBLobbyRcv_LoginReply<br>
	::CBLobbyRcv_NewLobby for every lobby that supports the game names in
			szGames<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szGames	List of game separated by comma or pipe | character (can be
		empty)
 \param	bPublicIP	If this flag is enabled, the ip address will be sent to
  players in your along with others player infos, if not your ip will not be
	sent to other players, thus preventing them to ping you.
 \param usPlayerStatus Set the player's status flag as define in LobbyDefine.h
*/
//============================================================================
GSbool __stdcall LobbySend_Login(const GSchar* szGames,
		GSbool bPublicIP = GS_TRUE, GSushort usPlayerStatus = 0);


//============================================================================
// Function		LobbySend_JoinLobby


/*!
 \brief	 Join a lobby
 \par       Description:
 Join a lobby on the lobby server.
 
 \par Callbacks:
 ::CBLobbyRcv_JoinLobbyReply
 ::CBLobbyRcv_LobbyInfo if LSM_GROUPINFO is set<br>
 ::LobbyRcv_NewRoom if LSM_CHILDGROUPINFO is set<br>
 ::CBLobbyRcv_MemberJoined if LSM_GROUPMEMBERS is set<br>
 

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the lobby to join
 \param iLobbyServerID The id of the server on which the specified group is
 	located
 \param	szPassword	Password to join lobby if needed
 \param	iconfig		Flag of information you want to receive when joining lobby
 LSM_GROUPINFO		to receive information about joined lobby
 LSM_CHILDGROUPINFO	to receive information about child group of the joined lobby
 LSM_GROUPMEMBERS   to receive list of player member of joined lobby
 LSM_ALLINFO        for all previous flags

*/
//============================================================================
GSbool __stdcall LobbySend_JoinLobby(GSint iGroupID, GSint iLobbyServerID,
		const GSchar* szPassword, GSint iconfig = 0);


//============================================================================
// Function LobbySend_JoinRoom


/*!
 \brief	 Join a room
 \par       Description:
 Join a room on the lobby server.
 
 \par Callbacks:
	::CBLobbyRcv_JoinRoomReply<br>
	::CBLobbyRcv_RoomInfo if LSM_GROUPINFO is set.<br>
	::CBLobbyRcv_MemberJoined for every player in the room if LSM_GROUPMEMBERS is
			set.<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the room to join
 \param iLobbyServerID	The id of the server on which the specified group is
 	located
 \param	szPassword	Password to join room if needed
 \param	bVisitor	Join as visitor if true, join as player if false
 \param	szVersion	The version of the game played in the room
 \param	iconfig		Flag of information you want to receive when joining lobby:
 LSM_GROUPINFO		to receive information about the joined room
 LSM_GROUPMEMBERS   to receive list of player member of joined room

*/
//============================================================================
GSbool __stdcall LobbySend_JoinRoom(GSint iGroupID, GSint iLobbyServerID,
		const GSchar* szPassword, GSbool bVisitor, const GSchar* szVersion,
		GSint iconfig = 0);


//============================================================================
// Function LobbySend_LeaveGroup


/*!
 \brief	 Leave a group
 \par       Description:
 This function is used to leave a room or lobby.
 
 \par Callbacks:
 ::CBLobbyRcv_MemberLeave for yourself.<br>
 ::CBLobbyRcv_GroupLeaveReply<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the player want to leave
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_LeaveGroup(GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_CreateRoom


/*!
 \brief		Create a room
 \par       Description:
Create a new room on the lobby server.  After successfully creating the room
you must join if before it shows up.

	\par Callbacks:
	::CBLobbyRcv_CreateRoomReply<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure


 \param	iParentGroupID	ID of the parent group
 \param iLobbyServerID The id of the server on which the room is created
 \param	szRoomName		Name of the lobby
 \param	szGames			Games played in the created lobby
 \param	usRoomType		The type of the game that will be played in the room<br>
  ROOM_CLIENTHOST: Should always be this for ingame clients.
 \param	usMaxPlayers	Maximum number of player in the room
 \param	usMaxVisitors	Maximum number of visitor in the room
 \param	vpData			Group info.  This can be any binary data the client wants.
 \param	iSize			Size of the group info
 \param vpAltGroupInfo Alternate Group Info.  This can be any binary data the
 client wants.  See LobbySend_GetAlternateGroupInfo()
 \param iAltGroupInfoSize Size of the alternate group info
 \param	szPassword		Password to lock the lobby
 \param	szGameVersion		Version of the game played in this room (information
 	purpose only)
 \param	szGSVersion		Version of the game played in this room given by ubi.com
 (gs-game version will be checked upon joining that room)

*/
//============================================================================
GSbool __stdcall LobbySend_CreateRoom(GSint iParentGroupID,
		GSint iLobbyServerID, const GSchar* szRoomName, const GSchar* szGames,
		GSushort usRoomType, GSushort usMaxPlayers , GSushort usMaxVisitors,
		const GSvoid* vpData, GSint iSize, const GSvoid* vpAltGroupInfo,
		GSint iAltGroupInfoSize,const GSchar* szPassword,
		const GSchar* szGameVersion, const GSchar* szGSVersion);

//============================================================================
// Function		LobbySend_StartMatch


/*!
 \brief	 Tell the lobby server that you are ready to start the match
 \par       Description:
Tell the lobby server that you are ready to start the match:
the message will be passed allong the member of the room and the match will
start.

	\par Callbacks:
	::CBLobbyRcv_StartMatchReply<br>
	::CBLobbyRcv_MatchStarted is sent to everyone in the room<br>
	::CBLobbyRcv_UpdateRoomConfigReply is sent to everyone in the parent group<br>
	::CBLobbyRcv_PlayerStatusUpdate is sent to everyone in the parent group for
			everyone in the room.<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param uiMode The mode of the Match
 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_StartMatch(GSint iGroupID,
		GSint iLobbyServerID, GSuint uiMode = 0 );

//============================================================================
// Function		LobbySend_MatchFinish
/*!
 \brief	 Tell the lobby server that you are finishing the match
 \par       Description:
Tell the lobby server that you want to stop the match. Be sure before sending
that Message than other players have finished there match be cause they will
no longer be able to submit there scores to the lobby server.

	\par Callbacks:
	::CBLobbyRcv_MatchFinishReply
	::CBLobbyRcv_PlayerStatusUpdate is sent to everyone in the parent group for
			everyone in the room.<br>
	::CBLobbyRcv_UpdateRoomConfigReply is sent to everyone in the parent group<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//===========================================================================
GSbool __stdcall LobbySend_MatchFinish( GSint iGroupID, GSint iLobbyServerID );

//============================================================================
// Function		LobbySend_NewMaster


/*!
 \brief	 Tell the lobby server that you are ready to start the match
 \par       Description:
Tell the lobby server to change the master for the specified group.

	\par Callbacks:
	::CBLobbyRcv_MasterNewReply<br>
	::CBLobbyRcv_MasterChanged is sent to everyone in the parent lobby.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located
 \param szUsername		The Username of the new master in the group.
*/
//============================================================================
GSbool __stdcall LobbySend_NewMaster(GSint iGroupID, GSint iLobbyServerID,
		GSchar* szUsername);

//============================================================================
// Function		LobbySend_PlayerMatchStarted


/*!
 \brief	 Tell the lobby server that a match is finished
 \par       Description:
Tell the lobby server that the match you were playing as started
Everyone in the room has to call this function to confirm the start of the match

	\par Related Function:
	LobbySend_StartMatch()<br>
	LobbySend_PlayerMatchFinished()<br>

	\par Callbacks:
	None


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_PlayerMatchStarted(GSint iGroupID,
		GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_PlayerMatchFinished


/*!
 \brief	 Tell the lobby server that a match is finished
 \par       Description:
Tell the lobby server that the match you were playing as ended
Everyone in the room has to call this function to confirm the end of the match.

	\par Related Function:
	LobbySend_StartMatch()<br>
	LobbySend_PlayerMatchStarted()<br>

	\par Callbacks:
	::CBLobbyRcv_PlayerStatusUpdate is sent to everyone in the parent group for
			everyone in the room.<br>
	

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_PlayerMatchFinished(GSint iGroupID,
		GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_PlayerKick


/*!
 \brief	 Tell the lobby server that to kick a player
 \par       Description:
Tell the lobby server to kick a player out of a specified group
(must be master of that group)

	/par Callbacks:
	::CBLobbyRcv_MemberLeave to all players in parent lobby<br>
	::CBLobbyRcv_PlayerStatusUpdate to all players in parent lobby<br>
	::CBLobbyRcv_PlayerKickReply<br>
	::CBLobbyRcv_KickOut to the kicked player<br>
		
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The id of the group the match will be in.
 \param iLobbyServerID	The id of the server on which the specified group is
 located
 \param szUserName		The username of the player to kick.
 \param szReason	The reason the given to the kicked player for this action.

*/
//============================================================================
GSbool __stdcall LobbySend_PlayerKick(GSint iGroupID, GSint iLobbyServerID,
		GSchar* szUserName, GSchar* szReason);

//============================================================================
// Function		LobbySend_GetParentGroupID


/*!
 \brief	 Tell the lobby server that to get a parent group id.
 \par       Description:
Query the lobby server to get the parent group id of a group.

	\par Callbacks:
	::CBLobbyRcv_ParentGroupIDReply

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID	The ID of the group we want to get info from.
 \param iLobbyServerID	The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_GetParentGroupID(GSint iGroupID,
		GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_UpdateRoomConfig


/*!
 \brief	 Tell the lobby server to update room configuration flags
 \par       Description:
Tell the lobby server to update a specific room configuration flags
(Must be master). Each of those argument can be set to -1 (in case of numeric)
or NULL (in case of a GSchar*) if the argument as not changed.

	\par Callbacks:
	::CBLobbyRcv_UpdateRoomConfigReply<br>
	::CBLobbyRcv_GroupConfigUpdate to everyone in the parent lobby if the config
			flags changed.<br>
	::CBLobbyRcv_RoomInfo if the other settings changed.<br>
	
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the room we want to modify.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param	bDedicatedServer		Set the Dedicated Server flag on the group Default
 Value: -1
 \param	bOpen			Set the open or close flag on the group Default Value: -1
 \param	bScore_Submission	Set the score submission flag on the group Default
 Value: -1
 \param	iMaxPlayers		The maximum number of player Default Value: -1
 \param	iMaxVisitors	The maximum number of visitors Default Value: -1
 \param	szPassword		The password to enter the group (PASSWORDLENGTH) Default
 Value: NULL
 \param	vpGroupInfo The group data      Default Value: NULL
 \param	iGroupInfoSize	The group data size   Default Value: -1 
 \param vpAltGroupInfo The alternate group info  Default Value: NULL
 \param iAltGroupInfoSize The alternate group info buffer size  Default Value:
 -1 

*/
//============================================================================
GSbool __stdcall LobbySend_UpdateRoomConfig(GSint iGroupID,
		GSint iLobbyServerID, GSbyte bDedicatedServer, GSbyte bOpen,
		GSbyte bScore_Submission, GSint iMaxPlayers, GSint iMaxVisitors,
		const GSchar* szPassword , const GSvoid* vpGroupInfo, GSint iGroupInfoSize,
		const GSvoid* vpAltGroupInfo, GSint iAltGroupInfoSize );


//============================================================================
// Function		LobbySend_GetGroupInfo


/*!
 \brief	 Get information about a group
 \par       Description:
Get information about a group

	\par Callbacks:
	::CBLobbyRcv_RoomInfo if LSM_GROUPINFO is set.<br>
	::CBLobbyRcv_MemberJoined if LSM_GROUPMEMBERS is set.<br>
	::CBLobbyRcv_NewRoom if LSM_CHILDGROUPINFO is set.<br>
	
 \return    Status of the function call

 \param	iGroupID		The id of the group the player is in
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param	iconfig		Flag of information you want to receive about the group
 LSM_GROUPINFO		to receive information about the group.
 LSM_CHILDGROUPINFO	to receive information about child group
 LSM_GROUPMEMBERS   to receive list of player member of the specified group
 LSM_ALLINFO        for all previous flags

*/
//============================================================================
GSbool __stdcall LobbySend_GetGroupInfo(GSint iGroupID, GSint iLobbyServerID,
		GSint iconfig);


//============================================================================
// Function		LobbySend_GetAlternateGroupInfo
/*!
 \brief	 Get information about a group
 \par       Description:
Get the alternate information about a group.

 \par Callbacks:
 ::CBLobbyRcv_GetAlternateGroupInfoReply
 
 \return    Status of the function call.

 \param	iGroupID		The id of the group the player is in.
 \param iLobbyServerID		The id of the server on which the specified group is
 located.

*/
//============================================================================
GSbool __stdcall LobbySend_GetAlternateGroupInfo(GSint iGroupID,
		GSint iLobbyServerID );

//============================================================================
// Function		LobbySend_InitMatchResults


/*!
 \brief	 Initialize the library score submission system
 \par       Description:
Initialize the library score submission system, this has to be called before
doing any other score submission-related function call. Note that you must
initialize the score submission system prior to any match that will be submitted.
This should be call after receiving the MatchStarted message, and the scores
should be cleared after being submitted to the lobby server with
LobbySend_SubmitMatchResult.
  \par Related Function:
	LobbySend_StartMatch()<br>
	LobbySend_SubmitMatchResult()<br>
	LobbySend_ClearMatchResult()<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	uiMatchID	The match unique id as returned by CBLobbyRcv_MatchStarted

*/
//============================================================================
GSbool __stdcall LobbySend_InitMatchResults(GSuint uiMatchID);

//============================================================================
// Function		LobbySend_SetMatchResult


/*!
 \brief	 Set results for a player in a match
 \par       Description:
Set the result for a player in a match in the library's score submission
system. Each player should set the result for each other player that where
in the match including himself. This will insure validity of scores submitted
to the LobbyServer.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szUserName		The username of the player associated with the results
 \param	uiFieldID	The result field id
 \param	iFieldValue	The actual value that will be set for the specified field

*/
//============================================================================
GSbool __stdcall LobbySend_SetMatchResult(const GSchar* szUserName, GSuint uiFieldID,
		GSint iFieldValue);

//============================================================================
// Function		LobbySend_SubmitMatchResult


/*!
 \brief	 Send the compiled scores of a match to the lobby server
 \par       Description:
Send the compiled scores of a match to the lobby server for archiving, 
this will send a message to the lobby server and set the match result
on the server-side. You will receive a confirmation of the message sent
with the CBLobbyRcv_SubmitMatchResult callback.

  \par Related Function:
	LobbyFix_SubmitMatchResultReply()

	\par Callbacks:
	::CBLobbyRcv_SubmitMatchResultReply

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the match took place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_SubmitMatchResult(GSint iGroupID,
		GSint iLobbyServerID);


//============================================================================
// Function		LobbySend_ClearMatchResult


/*!
 \brief	 Unload the internal score submission system
 \par       Description:
Unload the internal score submission system previously initialized for a
specific match. Must be called after scores has been submitted to the lobby
server.

  \par Related Function:
	LobbySend_InitMatchResults()<br>
	LobbyFix_SubmitMatchResultReply()<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall LobbySend_ClearMatchResult();


//============================================================================
// Function		LobbySend_UpdatePing


/*!
 \brief	 Send a new ping value to the server
 \par       Description:
Update the ping value for a player. When the client ping the host of a game
server, he can tell his ping to the other players by calling this functions
which will update his ping value on the server. The server will then forward
this information to the other players.

  \par Related Function:
	LobbyFix_UpdatePing()

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group the player is in
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param usPing			The new ping of the player

*/
//============================================================================
GSbool __stdcall LobbySend_UpdatePing(GSint iGroupID, GSint iLobbyServerID,
		GSushort usPing);

//============================================================================
// Function		LobbySend_GameStart


/*!
 \brief	 Send a message to inform the lobby server that the game is started.
 \par       Description:
Tell the lobby server that the host have started his game. This call should be
followed by the LobbySend_GameReady() function once the host is ready to
accept connection.

  \par Related Function:
	LobbySend_GameReady()<br>
	LobbyFix_GameStartReply()<br>

	\par Callbacks:
	::CBLobbyRcv_StartGameReply<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_GameStart(GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_GameConnected


/*!
 \brief	 Send a message to inform the lobby server that you are connected
 \par       Description:
Inform the lobby server that you are connected to the game server.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_GameConnected(GSint iGroupID, GSint iLobbyServerID);


//============================================================================
// Function		LobbySend_GameFinish


/*!
 \brief	 Send a message to inform the lobby server about the end of the game.
 \par       Description:
Inform the lobby server that the game you were playing is now finished

	\par Callbacks:
	::CBLobbyRcv_GroupConfigUpdate to everyone in parent lobby<br>
	::CBLobbyRcv_PlayerStatusUpdate to everyone in parent lobby<br>
	::CBLobbyRcv_InfoRefresh<br>
	
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located

*/
//============================================================================
GSbool __stdcall LobbySend_GameFinish(GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_GameReady


/*!
 \brief	 Send a message to inform the lobby server about connection readiness
 \par       Description:
Called by the master, this function inform the lobby server that the host
is ready to accept connection. It will set the basic game data that will
be distributed among other players.

	\par Callbacks:
	::CBLobbyRcv_GameReadyReply<br>
	::CBLobbyRcv_GroupConfigUpdate to everyone in parent lobby<br>
	::CBLobbyRcv_GameStarted to everyone in room<br>
	::CBLobbyRcv_PlayerStatusUpdate to everyone in parent lobby<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param vpGameData		Specific game data (i.e. DirectPlay structure)
 \param iSize			The size of the game data.
 \param usPort			The port of the game server
 \param szIPAddress		IP address of the game server.

*/
//============================================================================
GSbool __stdcall LobbySend_GameReady(GSint iGroupID, GSint iLobbyServerID,
		GSvoid* vpGameData, GSint iSize, GSushort usPort = 0,
		GSchar* szIPAddress = 0);

//============================================================================
// Function		LobbySend_UpdateGameInfo


/*!
 \brief	 Tell the lobby server about new game information.
 \par       Description:
Tell the lobby server about new game information; this is mostly used in the
host migration process, the player becoming the new master can send new game
data to the lobby server which will then redistribute this data among the other
players.

 \par Callbacks:
 ::CBLobbyRcv_UpdateGameInfoReply<br>
 
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param vpGameData		Specific game data (i.e. DirectPlay structure)
 \param iSize			The size of the game data.
 \param usPort			The port of the game server
 \param szIPAddress		IP address of the game server.
*/
//============================================================================
GSbool __stdcall LobbySend_UpdateGameInfo(GSint iGroupID, GSint iLobbyServerID,
		GSvoid* vpGameData, GSint iSize, GSushort usPort = 0,
		GSchar* szIPAddress = 0);

//============================================================================
// Function		LobbySend_PlayerBan


/*!
 \brief	 Ban a player from a group
 \par       Description:
Tell the lobby server to ban a player from a lobby or room. The player won't be
able to joined a group from which he was banned, it will also be impossible to
join any child group of the banned group.

	\par Callbacks:
	::CBLobbyRcv_MemberLeave to all players for the player that was banned<br>
	::CBLobbyRcv_PlayerBanReply<br>
	::CBLobbyRcv_PlayerBanned is sent to the banned player<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param szUserName			Username of the player that has to be banned.
 \param szReason		The reason the player was banned.
*/
//============================================================================
GSbool __stdcall LobbySend_PlayerBan(GSint iGroupID, GSint iLobbyServerID,
		GSchar* szUserName, GSchar* szReason);


//============================================================================
// Function		LobbySend_PlayerUnBan


/*!
 \brief	 Un-Ban a player from a group
 \par       Description:
Tell the lobby server to un-ban a player that was previously banned from a lobby
or room.

	\par Callbacks:
	::CBLobbyRcv_PlayerUnBanReply<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
 \param szUserName			Username of the player that has to be banned.
*/
//============================================================================
GSbool __stdcall LobbySend_PlayerUnBan(GSint iGroupID, GSint iLobbyServerID,
		GSchar* szUserName);

//============================================================================
// Function		LobbySend_GetPlayerBannedList


/*!
 \brief	 Get banned player list.
 \par       Description:
Ask the lobby server about all banned players for a specific group.

	\par Callbacks:
	::CBLobbyRcv_PlayerBanList<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	iGroupID		The id of the group where the game is taking place.
 \param iLobbyServerID		The id of the server on which the specified group is
 located
*/
//============================================================================
GSbool __stdcall LobbySend_GetPlayerBannedList(GSint iGroupID,
		GSint iLobbyServerID);

//============================================================================
// Function		LobbySend_SetPlayerInfo


/*!
 \brief	 Set player specific information.
 \par       Description:
Set player specific information stored on the server

	\par Callback:
	::CBLobbyRcv_SetPlayerInfoReply<br>
	::CBLobbyRcv_PlayerInfoUpdate is sent to all players<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	vpPlayerInfo		Pointer to player info data buffer
 \param iPlayerInfoSize of the player info data buffer
*/
//============================================================================
GSbool __stdcall LobbySend_SetPlayerInfo(GSvoid* vpPlayerInfo,
		GSint iPlayerInfoSize);

//============================================================================
// Function		LobbySend_GetPlayerGroups


/*!
 \brief	 Set player specific information.
 \par       Description:
Get the groups that a player is member of

	\par Callbacks:
	::CBLobbyRcv_PlayerGroupList for every group the player is in.<br>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szUsername The username of the player we want to get info from
*/
//============================================================================
GSbool __stdcall LobbySend_GetPlayerGroups(GSchar* szUsername);


//============================================================================
// Function LobbySend_ChangeRequestedLobbies


/*!
 \brief	 This function is used to get a new list of available lobbies for a game
 \par       Description:
   This function is used to get a new list of available lobbies for a game
  The parameter szGame is used to narrow the list of groups received with only
	those who are running the game specified in the argument field.  More than one
	game can be specify, each ugly game name must be separated by one of those
	character: | ; or , 

	\par Callbacks:
	::CBLobbyRcv_NewLobby for every lobby supporting the new game name

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szGames	List of game separated by comma or pipe | character (can be
 empty)
 
*/
//============================================================================
GSbool __stdcall LobbySend_ChangeRequestedLobbies(const GSchar* szGames);

/*! @} end of group4_1 */

/*! @defgroup group4_2 Callback registration
\brief Register the callbacks function names

Theses functions are used to set the function name
for the callbacks of the <b><i>lobby service</i></b>.
    @{
*/


//============================================================================
// Function LobbyFix_LoginReply


/*!
 \brief	Set the function name for te callback.
 \par       Description:
  This function is used to register the LoginResult callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLoginReply Name of a function of type CBLobbyRcv_LoginReply

*/
//============================================================================
GSbool __stdcall LobbyFix_LoginReply(CBLobbyRcv_LoginReply fLoginReply);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the LobbyDisconnection callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLobbyDisconnection	Name of a function of type 
 ::CBLobbyRcv_LobbyDisconnection

*/
//============================================================================
GSbool __stdcall LobbyFix_LobbyDisconnection(
		CBLobbyRcv_LobbyDisconnection fLobbyDisconnection);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the LobbyDisconnection callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLobbyDisconnectAll	Name of a function of type ::
 CBLobbyRcv_LobbyDisconnectAll

*/
//============================================================================
GSbool __stdcall LobbyFix_LobbyDisconnectAll(
		CBLobbyRcv_LobbyDisconnectAll fLobbyDisconnectAll);

//============================================================================
// Function LobbyFix_CreateRoomReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the CreateRoom callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fCreateRoomReply Name of a function of type ::CBLobbyRcv_CreateRoomReply

*/
//============================================================================
GSbool __stdcall LobbyFix_CreateRoomReply(
		CBLobbyRcv_CreateRoomReply fCreateRoomReply);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the NewRoom callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fNewRoom	Name of a function of type ::CBLobbyRcv_NewRoom

*/
//============================================================================
GSbool __stdcall LobbyFix_NewRoom(CBLobbyRcv_NewRoom fNewRoom);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the NewLobby callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fNewLobby	Name of a function of type ::CBLobbyRcv_NewLobby

*/
//============================================================================
GSbool __stdcall LobbyFix_NewLobby(CBLobbyRcv_NewLobby fNewLobby);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the RoomInfo callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fRoomInfo	Name of a function of type CBLobbyRcv_RoomInfo

*/
//============================================================================
GSbool __stdcall LobbyFix_RoomInfo(CBLobbyRcv_RoomInfo fRoomInfo);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the LobbyInfo callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fLobbyInfo	Name of a function of type CBLobbyRcv_LobbyInfo

*/
//============================================================================
GSbool __stdcall LobbyFix_LobbyInfo(CBLobbyRcv_LobbyInfo fLobbyInfo);


//============================================================================
// Function LobbyFix_GroupInfoGet

/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GroupInfoGet callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGroupInfoGet	Name of a function of type ::CBLobbyRcv_GroupInfoGet

*/
//============================================================================
GSbool __stdcall LobbyFix_GroupInfoGet(CBLobbyRcv_GroupInfoGet fGroupInfoGet);

//============================================================================
// Function LobbyFix_JoinRoomReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the JoinRoomResult callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fJoinRoomReply Name of a function of type ::CBLobbyRcv_JoinRoomReply

*/
//============================================================================
GSbool __stdcall LobbyFix_JoinRoomReply(CBLobbyRcv_JoinRoomReply fJoinRoomReply);

//============================================================================
// Function LobbyFix_JoinLobbyReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the JoinLobbyResult callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fJoinLobbyReply Name of a function of type ::CBLobbyRcv_JoinLobbyReply

*/
//============================================================================
GSbool __stdcall LobbyFix_JoinLobbyReply(
		CBLobbyRcv_JoinLobbyReply fJoinLobbyReply);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MemberJoined callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMemberJoined	Name of a function of type ::CBLobbyRcv_MemberJoined

*/
//============================================================================
GSbool __stdcall LobbyFix_MemberJoined(CBLobbyRcv_MemberJoined fMemberJoined);

//============================================================================
// Function LobbyFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MemberLeave callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMemberLeave	Name of a function of type ::CBLobbyRcv_MemberLeave

*/
//============================================================================
GSbool __stdcall LobbyFix_MemberLeave(CBLobbyRcv_MemberLeave fMemberLeave);


//============================================================================
// Function LobbyFix_StartMatch


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the StartMatch callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fStartMatchReply Name of a function of type ::CBLobbyRcv_StartMatchReply

*/
//============================================================================
GSbool __stdcall LobbyFix_StartMatchReply(
		CBLobbyRcv_StartMatchReply fStartMatchReply);

//============================================================================
// Function LobbyFix_GroupRemove


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GroupRemove callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGroupRemove	Name of a function of type ::CBLobbyRcv_GroupRemove

*/
//============================================================================
GSbool __stdcall LobbyFix_GroupRemove(CBLobbyRcv_GroupRemove fGroupRemove);

//============================================================================
// Function LobbyFix_MasterNewReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MasterNew callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	 fMasterNewReply Name of a function of type ::CBLobbyRcv_MasterNewReply

*/
//============================================================================
GSbool __stdcall LobbyFix_MasterNewReply(
		CBLobbyRcv_MasterNewReply fMasterNewReply);

//============================================================================
// Function LobbyFix_MasterChanged


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MasterChanged callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMasterChanged	Name of a function of type ::CBLobbyRcv_MasterChange

*/
//============================================================================
GSbool __stdcall LobbyFix_MasterChanged(CBLobbyRcv_MasterChanged fMasterChanged);

//============================================================================
// Function LobbyFix_MatchFinishReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MatchFinish callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMatchFinishReply Name of a function of type 
 ::CBLobbyRcv_MatchFinishReply

*/
//============================================================================
GSbool __stdcall LobbyFix_MatchFinishReply(
		CBLobbyRcv_MatchFinishReply fMatchFinishReply);

//============================================================================
// Function LobbyFix_PlayerKickReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the PlayerKick callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerKickReply Name of a function of type ::CBLobbyRcv_PlayerKickReply

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerKickReply(
		CBLobbyRcv_PlayerKickReply fPlayerKickReply);

//============================================================================
// Function LobbyFix_KickOut


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the KickOut callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fKickOut	Name of a function of type ::CBLobbyRcv_KickOut

*/
//============================================================================
GSbool __stdcall LobbyFix_KickOut(CBLobbyRcv_KickOut fKickOut);

//============================================================================
// Function LobbyFix_ParentGroupID


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the ParentGroupID callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fParentGroupIDReply	Name of a function of type 
 ::CBLobbyRcv_ParentGroupID

*/
//============================================================================
GSbool __stdcall LobbyFix_ParentGroupIDReply(
		CBLobbyRcv_ParentGroupIDReply fParentGroupIDReply);

//============================================================================
// Function LobbyFix_GetAlternateGroupInfoReply

/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GetAlternateGroupInfoReply callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGetAlternateGroupInfoReply	Name of a function of type 
 ::CBLobbyRcv_GetAlternateGroupInfoReply

*/
//============================================================================
GSbool __stdcall LobbyFix_GetAlternateGroupInfoReply(
		CBLobbyRcv_GetAlternateGroupInfoReply fGetAlternateGroupInfoReply);

//============================================================================
// Function LobbyFix_GroupLeaveReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GroupLeave callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGroupLeaveReply Name of a function of type 
 ::CBLobbyRcv_GroupLeaveReply

*/
//============================================================================
GSbool __stdcall LobbyFix_GroupLeaveReply(
		CBLobbyRcv_GroupLeaveReply fGroupLeaveReply);

//============================================================================
// Function LobbyFix_GroupConfigUpdate


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GroupConfigUpdate callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGroupConfigUpdate	Name of a function of type ::CBLobbyRcv_GroupLeave

*/
//============================================================================
GSbool __stdcall LobbyFix_GroupConfigUpdate(
		CBLobbyRcv_GroupConfigUpdate fGroupConfigUpdate);

//============================================================================
// Function LobbyFix_GroupConfigUpdate


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MatchStarted callback


 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMatchStarted	Name of a function of type ::CBLobbyRcv_GroupLeave

*/
//============================================================================
GSbool __stdcall LobbyFix_MatchStarted(CBLobbyRcv_MatchStarted fMatchStarted);


//============================================================================
// Function LobbyFix_GroupConfigUpdateResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdateRoomConfigReply callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fUpdateRoomConfigReply Name of a function of type
  ::CBLobbyRcv_UpdateRoomConfigReply

*/
//============================================================================
GSbool __stdcall LobbyFix_UpdateRoomConfigReply(
		CBLobbyRcv_UpdateRoomConfigReply fUpdateRoomConfigReply);

//============================================================================
// Function LobbyFix_SubmitMatchResultReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the SubmitMatchResult callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	 fSubmitMatchResultReply Name of a function of type 
 ::CBLobbyRcv_SubmitMatchResultReply

*/
//============================================================================
GSbool __stdcall LobbyFix_SubmitMatchResultReply(
		CBLobbyRcv_SubmitMatchResultReply fSubmitMatchResultReply);

//============================================================================
// Function LobbyFix_UpdatePing


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the SubmitMatchResult callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fUpdatePing Name of a function of type ::CBLobbyRcv_UpdatePing

*/
//============================================================================
GSbool __stdcall LobbyFix_UpdatePing(CBLobbyRcv_UpdatePing fUpdatePing);

//============================================================================
// Function LobbyFix_GameStartReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fStartGameReply Name of a function of type ::CBLobbyRcv_StartGameReply

*/
//============================================================================
GSbool __stdcall LobbyFix_GameStartReply(
		CBLobbyRcv_StartGameReply fStartGameReply);

//============================================================================
// Function LobbyFix_GameReadyReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGameReadyReply Name of a function of type ::CBLobbyRcv_GameReadyReply

*/
//============================================================================
GSbool __stdcall LobbyFix_GameReadyReply(
		CBLobbyRcv_GameReadyReply fGameReadyReply);

//============================================================================
// Function LobbyFix_GameStarted


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGameStarted Name of a function of type ::CBLobbyRcv_GameStarted

*/
//============================================================================
GSbool __stdcall LobbyFix_GameStarted(CBLobbyRcv_GameStarted fGameStarted);

//============================================================================
// Function LobbyFix_NewGameMember


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the NewGameMember callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fNewGameMember Name of a function of type ::CBLobbyRcv_NewGameMember

*/
//============================================================================
GSbool __stdcall LobbyFix_NewGameMember(CBLobbyRcv_NewGameMember fNewGameMember);

//============================================================================
// Function LobbyFix_UpdateGameInfoReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fUpdateGameInfoReplyCB Name of a function of type 
 ::CBLobbyRcv_UpdateGameInfoReply

*/
//============================================================================
GSbool __stdcall LobbyFix_UpdateGameInfoReply(
		CBLobbyRcv_UpdateGameInfoReply fUpdateGameInfoReplyCB);

//============================================================================
// Function LobbyFix_PlayerBanReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerBanReplyCB Name of a function of type 
 ::CBLobbyRcv_PlayerBanReply

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerBanReply(
		CBLobbyRcv_PlayerBanReply fPlayerBanReplyCB);

 //============================================================================
// Function LobbyFix_PlayerUnBanReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerUnBanReplyCB Name of a function of type 
 ::CBLobbyRcv_PlayerUnBanReply

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerUnBanReply(
		CBLobbyRcv_PlayerUnBanReply fPlayerUnBanReplyCB);

//============================================================================
// Function LobbyFix_PlayerBanList


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerBanListCB Name of a function of type ::CBLobbyRcv_PlayerBanList

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerBanList(
		CBLobbyRcv_PlayerBanList fPlayerBanListCB);

//============================================================================
// Function LobbyFix_PlayerBanned


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the UpdatePing callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerBannedCB Name of a function of type ::CBLobbyRcv_PlayerBanned

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerBanned(CBLobbyRcv_PlayerBanned fPlayerBannedCB);

//============================================================================
// Function LobbyFix_MatchReady


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the MatchReady callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fMatchReadyCB Name of a function of type ::CBLobbyRcv_MatchReady

*/
//============================================================================
GSbool __stdcall LobbyFix_MatchReady(CBLobbyRcv_MatchReady fMatchReadyCB);

//============================================================================
// Function LobbyFix_InfoRefresh


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the InfoRefresh callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fInfoRefresh Name of a function of type ::CBLobbyRcv_InfoRefresh

*/
//============================================================================
GSbool __stdcall LobbyFix_InfoRefresh(CBLobbyRcv_InfoRefresh fInfoRefresh);

//============================================================================
// Function LobbyFix_SetPlayerInfoReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the SetPlayerInfoReply callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fSetPlayerInfoReply Name of a function of type 
 ::CBLobbyRcv_SetPlayerInfoReply

*/
//============================================================================
GSbool __stdcall LobbyFix_SetPlayerInfoReply(
		CBLobbyRcv_SetPlayerInfoReply fSetPlayerInfoReply);

//============================================================================
// Function LobbyFix_PlayerInfoUpdate


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the PlayerInfoUpdate callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerInfoUpdate Name of a function of type 
 ::CBLobbyRcv_PlayerInfoUpdate

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerInfoUpdate(
		CBLobbyRcv_PlayerInfoUpdate fPlayerInfoUpdate);

//============================================================================
// Function LobbyFix_PlayerGroupList


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the PlayerGroupList callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerGroupListCB Name of a function of type 
 ::CBLobbyRcv_PlayerGroupList

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerGroupList(
		CBLobbyRcv_PlayerGroupList fPlayerGroupListCB);


//============================================================================
// Function LobbyFix_PlayerStatusUpdate
/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the PlayerStatusUpdate callback

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPlayerStatusUpdate Name of a function of type 
 ::CBLobbyRcv_PlayerStatusUpdate

*/
//============================================================================
GSbool __stdcall LobbyFix_PlayerStatusUpdate(
		CBLobbyRcv_PlayerStatusUpdate fPlayerStatusUpdate );


GSbool __stdcall LobbyFix_FinalMatchResults(
		CBLobbyRcv_FinalMatchResults fFinalMatchResults);


/*! @} end of group4_2 */


/*! @defgroup group_LobbyCB Callbacks
\brief The lobby service function callbacks
    @{
		@}
*/


/*! @} end of group4 */


/*! @defgroup group5 Persistent Storage Service
\brief Functions used to connect to the <b><i>persistent storage service</i></b>.

These function are used to send persistent storage related
message to the server and to register the callbacks
functions related to this service
    @{
*/

/*! @defgroup group5_1 Functions
\brief Messages sent to the persistent storage service

These function are used to send to
messages to the persistent storage server
    @{
*/

//============================================================================
// Function		PSSend_Login


/*!
 \brief	 Login to the persistent storage service.
 \par       Description:
Login to the persistent storage service. Will return false if unable to connect
true on success

	\par Callbacks:
	::CBPSRcv_LoginResult

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall PSSend_Login();

//============================================================================
// Function		PSSend_Disconnect


/*!
 \brief	 Disconnect from the persistent storage service.
 \par       Description:
Disconnect from the persistent storage service.

	\par Callbacks:
	::CBPSRcv_Disconnection

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall PSSend_Disconnect();

//============================================================================
// Function		PS_Engine


/*!
 \brief		Update connection status-messages handling relating to the
  persistent data storage service
 \par       Description:
 Updates the connection between the client and the server, and handles
 the delivery of queued up messages and reception of messages relating to the
 persistent data storage service service. This function should be called
 regularly to ensure that the application will run smoothly with the service.

 \return    Status of the function call

 \retval	GS_TRUE		The connection is ok and function call was a success
 \retval	GS_FALSE	There has been a communication problem between the client
 		and the server

 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine
 to decode message in the queue and calling appropriate callback. (Milliseconds)

*/
//============================================================================
GSbool __stdcall PS_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);

//============================================================================
// Function		PSSend_GetPrivateData


/*!
 \brief	 Get the private data related to a player.
 \par       Description:
Get the private data related to a player using the persistent data storage
service.

	\par Callbacks:
	::CBPSRcv_GetDataReply
	
 \return    ID of the request

 \param	szGame		The name of the game
 \param	iEventID	The id of the event (this id is available in the lobby info)
 \param	iRecordID	Specific id of the record set.

*/
//============================================================================
GSuint __stdcall PSSend_GetPrivateData(const GSchar* szGame, GSint iEventID,
		GSint iRecordID);

//============================================================================
// Function		PSSend_SetPrivateData


/*!
 \brief	 Set the private data related to a player.
 \par       Description:
Set the private data related to a player using the persistent data storage
service.

	\par Callbacks:
	::CBPSRcv_SetDataReply

 \param	szGame		The name of the game
 \param	iEventID	The id of the event (this id is available in the lobby info)
 \param	iRecordID	Specific id of the record set.
 \param	vpData		The actual data buffer
 \param	iSize		Size of the data buffer

*/
//============================================================================
GSuint __stdcall PSSend_SetPrivateData(const GSchar* szGame, GSint iEventID,
		GSint iRecordID, const GSvoid* vpData, GSint iSize);


//============================================================================
// Function		PSSend_GetPublicData


/*!
 \brief	 Get the public data related to a player.
 \par       Description:
Get the public data related to a player using the persistent data storage
service.

	\par Callbacks:
	::CBPSRcv_SetDataReply

 \return    ID of the request

 \param	szGame		The name of the game
 \param	iEventID	The id of the event (this id is available in the lobby info)
 \param	iRecordID	Specific id of the record set.
 \param	szUsername	The username of the player related with the data 
						(Default to the current username if the argument is not
						passed)

*/
//============================================================================
GSuint __stdcall PSSend_GetPublicData(const GSchar* szGame, GSint iEventID,
		GSint iRecordID, const GSchar* szUsername = 0);

//============================================================================
// Function		PSSend_SetPublicData


/*!
 \brief	 Set the public data related to a player.
 \par       Description:
Set the public data related to a player using the persistent data storage
service.

	\par Callbacks:
	::CBPSRcv_SetDataReply

 \return    ID of the request

 \param	szGame		The name of the game
 \param	iEventID	The id of the event (this id is available in the lobby info)
 \param	iRecordID	Specific id of the record set.
 \param	vpData		The actual data buffer
 \param	iSize		Size of the data buffer

*/
//============================================================================
GSuint __stdcall PSSend_SetPublicData(const GSchar* szGame, GSint iEventID,
		GSint iRecordID, const GSvoid* vpData, GSint iSize);


/*! @} end of group5_1 */

/*! @defgroup group5_2 Callback registration
\brief persistent storage service callbacks

These function are used register
persistent storage service callbacks
    @{
*/


//============================================================================
// Function PSFix_LoginResult


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the LoginResult callback for the
  persistent storage service

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPSLoginResult Name of a function of type ::CBPSRcv_LoginResult

*/
//============================================================================
GSbool __stdcall PSFix_LoginResult(CBPSRcv_LoginResult fPSLoginResult);

//============================================================================
// Function PSFix_Disconnection


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the disconnection callback for the
  persistent storage service

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fPSDisconnection Name of a function of type ::CBPSRcv_Disconnection

*/
//============================================================================
GSbool __stdcall PSFix_Disconnection(CBPSRcv_Disconnection fPSDisconnection);


//============================================================================
// Function PSFix_SetDataReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the SetDataReply callback for the
  persistent storage service

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fSetDataReply Name of a function of type ::CBPSRcv_SetDataReply

*/
//============================================================================
GSbool __stdcall PSFix_SetDataReply(CBPSRcv_SetDataReply fSetDataReply);

//============================================================================
// Function PSFix_GetDataReply


/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the GetDataReply callback for the
  persistent storage service

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fGetDataReply Name of a function of type ::CBPSRcv_GetDataReply

*/
//============================================================================
GSbool __stdcall PSFix_GetDataReply(CBPSRcv_GetDataReply fGetDataReply);
/*! @} end of group5_2 */

/*! @defgroup group_PSCB Callbacks
\brief The persistent storage service function callbacks

    @{
		@}
*/


/*! @} end of group5 */


/*! @defgroup group6 Ladder Query Service
\brief Functions used to use the <b><i>ladder query service</i></b>.

Theses functions are used to query the ubi.com data providers to get
ranking informations on players.
    @{
*/

/*! @defgroup group6_1 Functions
\brief Messages sent to the <b><i>ladder query service</i></b>.

These functions are used to send request to the ubi.com data provider and 
retrieve results when a reply is given by the server.
    @{
*/


//============================================================================
// Function LadderQuery_Initialize
/*!
 \brief	 Initialization function for the ladder query service.
 \par       Description:
 This function takes a 2 character string argument that represent the locale
 in which the string results will be returned from the server.

 The initialisation of the ladder query service can be done after the client
 library initialization (i.g. GSClientLibrary_Initialize())
 
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	szLocale The locale in which the string results will be returned from the
 server.

*/
//============================================================================
GSbool __stdcall LadderQuery_Initialize(GSchar *szLocale);

//============================================================================
// Function LadderQuery_Uninitialize
/*!
 \brief	 Uninitialization function for the ladder query service.
 \par       Description:
 This function uninitialize the ladder query service and free allocated memory.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

*/
//============================================================================
GSbool __stdcall LadderQuery_Uninitialize();

//============================================================================
// Function LadderQuery_Engine
/*!
 \brief		Update connection status related to the ladder query service
 \par       Description:
 Updates the connection between the client and the server, and handles
 the delivery of queued up messages and reception of messages relating to the
 ladder query service service.  This function should be called regularly to
 ensure that the service will run smoothly and properly.

 \return    Status of the function call

 \retval	GS_TRUE		The connection is ok and function call was a success
 \retval	GS_FALSE	There has been a communication problem between the client
 and the server
 
 \param	uiMaxPostingDelay	The maximum time to be spent inside the engine to
  read incomming messages and posting them to the message queue. (Milliseconds)
 \param	uiMaxsOperationalDelay	The maximum time to be spent inside the engine
 to decode message in the queue and calling appropriate callback. (Milliseconds)
*/
//============================================================================
GSbool __stdcall LadderQuery_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxsOperationalDelay = 800);


//============================================================================
// Function LadderQuery_CreateRequest
/*!
 \brief	 Create a basic ladder query request
 \par       Description:
 This function creates a basic ladder query request (ordered list, with no
 constraint) that can be sent over the network to the ubi.com data provider.
 The LadderQuery_CreateRequest() function should be called each time you want to
 make a request. Once the request is created, you can tell which type of request
 you want, and optionally add one or more constraint.
 If you call this function again before sending the request created previously,
 the type and constraint of the previous request are overwritten by the default
 ones.

 <b>NOTE</b>: <i>This function will return GS_FALSE if you have not completed the
 login sequence on the gs-router server.</i>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

  \param	szGameName	The game name associated with the requested ladder data.
  \param	uiEvent		The event id associated with the ladder data.
  \param	uiMode		The game mode.

*/
//============================================================================
GSbool __stdcall LadderQuery_CreateRequest(GSchar *szGameName,
		GSuint uiEvent = 0, GSuint uiMode = 0);

//============================================================================
// Function LadderQuery_SendRequest
/*!
 \brief	 Send a request over the network.
 \par       Description:
 This function send a request over the network to the ubi.com servers.
 This function will return the id of the sent request and in case of
 error it will return 0. The id of the request will be used once we get
 the results so you have to keep it in memory.

	\par Callbacks:
	::CBLadderQueryRcv_RequestReply

 \return    The id of the request that has been sent.

 \retval	0		Internal error
 \retval	else	The id of the request that has been sent.

*/
//============================================================================
GSuint __stdcall LadderQuery_SendRequest();

//============================================================================
// Function LadderQuery_RequestPivotUser
/*!
 \brief	 Requesting an ordered list of entries around a pivot (username)
 \par       Description:
 This function set the request mode to retrieve an ordered list of entries
 around a pivot where the pivot is a user name entry.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	pszEntry	Pointer to a character string representing the username to use
 as a pivot.
 \param	uiNumberOfEntries Total number of entries to retrieve.
*/
//============================================================================
GSbool __stdcall LadderQuery_RequestPivotUser(const GSchar *pszEntry,
		GSuint uiNumberOfEntries);

//============================================================================
// Function LadderQuery_RequestPivotRow
/*!
 \brief	 Requesting an ordered list of entries around a pivot (row number)
 \par       Description:
 This function set the request mode to retrieve an ordered list of entries
 around a pivot where the pivot is a row number in the ladder.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRowNumber	The row number used as a pivot.
 \param	uiNumberOfEntries Total number of entries to retrieve.
*/
//============================================================================
GSbool __stdcall LadderQuery_RequestPivotRow(GSuint  uiRowNumber,
		GSuint uiNumberOfEntries);

//============================================================================
// Function LadderQuery_RequestSet
/*!
 \brief	 Requesting a pre-defined set of entries
 \par       Description:
 This function set the request mode to retrieve a pre-defined set of entries
 supplied by the caller.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	pszEntries	Pointer on an array of string values. Each string values
 should be a username.
 \param	uiNumberOfEntries Total number of entries in the array of values.
*/
//============================================================================
GSbool __stdcall LadderQuery_RequestSet(const GSchar *pszEntries[],
		GSuint uiNumberOfEntries);

//============================================================================
// Function LadderQuery_RequestOrderedList
/*!
 \brief	 Requesting an ordered list of entries from a starting index (row
 number).
 \par       Description:
 This function set the request mode to retrieve an ordered list of entries from
 a starting zero-based index representing a row number.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiFirstEntry	Zero-based index of the first entry to get.
 \param	uiNumberOfEntries Total number of entries in the array of values.
*/
//============================================================================
GSbool __stdcall LadderQuery_RequestOrderedList(GSuint uiFirstEntry,
		GSuint uiNumberOfEntries);

//============================================================================
// Function LadderQuery_AddSortConstraint
/*!
 \brief	 Add a <b>sort</b> constraint to a created request.
 \par       Description:
 This function set a sort constraint to a request that has been created.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	pszField	The field name to sort by.
 \param	bSortDirection The sort direction where GS_TRUE = natural order and
 GS_FALSE = unnatural order.
*/
//============================================================================
GSbool __stdcall LadderQuery_AddSortConstraint(const GSchar *pszField,
		GSbool bSortDirection);

//============================================================================
// Function LadderQuery_AddFilterConstraint
/*!
 \brief	 Add a <b>filter</b> constraint to a created request.
 \par       Description:
 This function set a filter constraint to a request that has been created. 
 Filters are defined using the LADDER_FILTER structure.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	pLadderFilter	A pointer to an array of ladder filter.
 \param	uiListOfFilters The size of the array of ladder filter.
*/
//============================================================================
GSbool __stdcall LadderQuery_AddFilterConstraint(
		const LADDER_FILTER *pLadderFilter[], GSuint uiListOfFilters);

//============================================================================
// Function LadderQuery_AddDisplayConstraint
/*!
 \brief	 Add a <b>display</b> constraint to a created request.
 \par       Description:
 This function set a display constraint to a request that has been created. 
 Display constraint will make the results contain only the supplied list of
 fields.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	pszFields	A pointer to an array of string values (fields).
 \param	uiNumberOfFields The size of the array of string values.
*/
//============================================================================
GSbool __stdcall LadderQuery_AddDisplayConstraint(const GSchar *pszFields[],
		GSuint uiNumberOfFields);

/*! @} end of group6_1 */

/*! @defgroup group6_2 Results fetching
\brief Get the entries requested when a request completed successfully.

These functions are used to fetch results from a request that came back to the
client library.
    @{
*/


//============================================================================
// Function LadderQuery_GetResultSearchCount
/*!
 \brief	 Get the request search count.
 \par       Description:
 This function will return the number of valid entries that were found on server
 for the request that was sent. This number may differ from the number of
 entries returned in the results set. See LadderQuery_GetResultEntryCount().

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param	uiCount     [OUT] The number of entries matching the request
                    specifications.
*/
//============================================================================
GSbool __stdcall LadderQuery_GetResultSearchCount(GSuint uiRequestID,
		GSuint & uiCount);

//============================================================================
// Function LadderQuery_GetResultEntryCount
/*!
 \brief	 Get the request entry count.
 \par       Description:
 This function will return the number of entries that were returned as part of
 the results set.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param	uiCount     [OUT] The number of entries in the results set.
*/
//============================================================================
GSbool __stdcall LadderQuery_GetResultEntryCount(GSuint uiRequestID,
		GSuint & uiCount);

//============================================================================
// Function LadderQuery_GetResultFieldCaption
/*!
 \brief	 Get the pretty name of a field.
 \par       Description:
 This function will get the name of a field like it should be displayed in the
 game (pretty name).  Example: Field name "TIME" = Field caption "Total time
 played"

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param	pszField    Pointer to a character string representing the field
                    name to identify.
 \param	pszCaption  [OUT] Pointer to a character string that will represent
                    the field caption on success.
*/
//============================================================================
GSbool __stdcall LadderQuery_GetResultFieldCaption(GSuint uiRequestID,
		const GSchar *pszField, GSchar * pszCaption);

//============================================================================
// Function LadderQuery_StartResultEntryEnumeration
/*!
 \brief	 Start the entry enumeration.
 \par       Description:
 This function will initialise the enumeration of entries received in the
 results set. This call is important and must be done before any call to
 LadderQuery_NextResultEntry() , LadderQuery_GetCurrentEntryField() ,
 and LadderQuery_GetCurrentEntryFieldAsString() .
 <b>NOTE</b>: <i>Since you can call this function multiple times without
 modifying the results set, you can use this function to get the results in the
 order you want by modifying the uiStartEntry parameter.</i>

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param	uiStartEntry The index of the entry where the enumeration will start.
*/
//============================================================================
GSbool __stdcall LadderQuery_StartResultEntryEnumeration(GSuint uiRequestID,
		GSuint uiStartEntry = 0);

//============================================================================
// Function LadderQuery_NextResultEntry
/*!
 \brief	 Move the results set pointer to the next entry (iteration)
 \par       Description:
 This function will move the results set pointer to the next entry in the list
 of entries received.  It is used in conjonction with
 LadderQuery_GetCurrentEntryField() and LadderQuery_GetCurrentEntryFieldAsString()
 to iterate through the list of entries in the results set and get their values.

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
*/
//============================================================================
GSbool __stdcall LadderQuery_NextResultEntry(GSuint uiRequestID);

//============================================================================
// Function LadderQuery_GetCurrentEntryField
/*!
 \brief	 Get the current numeric field value
 \par       Description:
 This function is used to get the numeric field value currently pointed by the
 results set pointer.  It is used in conjonction with
 LadderQuery_NextResultEntry() to iterate through the list of entries in the
 results set and get their values.
 <b>NOTE</b>: <i>If the entry is a character string, this function will return
 GS_FALSE.</i>
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param pszField    Pointer to a character string representing the field name.
 \param	iValue      [OUT] The numeric field value retrieved.
*/
//============================================================================
GSbool __stdcall LadderQuery_GetCurrentEntryField(GSuint uiRequestID,
		const GSchar *pszField, GSint & iValue);

//============================================================================
// Function LadderQuery_GetCurrentEntryFieldAsString
/*!
 \brief	 Get the current field value in a string format
 \par       Description:
 This function is used to get the field value currently pointed by the results
 set pointer in a string format.  It is used in conjonction with
 LadderQuery_NextResultEntry() to iterate through the list of entries in the
 results set and get their values.
 
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
 \param pszField    Pointer to a character string representing the field name.
 \param	pszValue    [OUT] Pointer to a character string representing the
                    field value.
*/
//============================================================================
GSbool __stdcall LadderQuery_GetCurrentEntryFieldAsString(GSuint uiRequestID,
		const GSchar *pszField, GSchar * pszValue);

//============================================================================
// Function LadderQuery_GetCurrentEntryFieldAsString
/*!
 \brief	 Release the results set from memory
 \par       Description:
 This function is used to release the results received from the server after no
 more operations on the results set need to be done. Each successfull request
 triggers a memory allocation for the results, these results need to be flushed
 when they are not used anymore.
 
 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure
 
 \param	uiRequestID The request identifier supplied in the callback.
*/
//============================================================================
GSvoid __stdcall LadderQuery_ReleaseResult(GSuint uiRequestID);


/*! @} end of group6_2 */

/*! @defgroup group6_3 Callback registration
\brief ladder query service callbacks

These function are used register
ladder query service callbacks
    @{
*/
//============================================================================
// Function LadderQueryFix_RequestReply
/*!
 \brief	Set the function name for the callback.
 \par       Description:
  This function is used to register the LadderQuery RequestReply callback for
	the ladder query service

 \return    Status of the function call

 \retval	GS_TRUE		Success
 \retval	GS_FALSE	Failure

 \param	fQueryReply Name of a function of type ::CBLadderQueryRcv_RequestReply

*/
//============================================================================
GSbool __stdcall LadderQueryFix_RequestReply(
		CBLadderQueryRcv_RequestReply fQueryReply);


/*! @} end of group6_2 */

/*! @defgroup group_LadderQuery Callbacks
\brief The ladder query service function callbacks

    @{
		@}
*/

/*! @} end of group6 */

/*! @defgroup group7 Remote Algorithm Execution Service
\brief Information about the <b><i>remote algorithm execution service</i></b>

These are the functions and structures exported by the service.

@{
*/

/*! @defgroup group7_1 Functions
\brief Functions of the service

There functions are that need to be used in order to access the service
@{
*/

//==============================================================================
// Function RemoteAlgorithm_Initialise
/*!
\brief Initialises the resources required by the service
\par   Description:
       This function initialises the resources that will be required to use the
       remote algorithm execution service. It takes as input the game identifier
       character string supplied to the developers by ubi.com.

       This function can only be called after having called
       GSClientLibrary_Initialize().

\param      szGameName  The game identifier (i.e. ugly name)

\return     Result code of the operation
\retval     GSS_OK      The service is ready to be used
\retval     GSE_ALREADYINITIALIZED      The library was already initialised
\retval     GSE_UNEXPECTED      The client library was not initialised
*/
//==============================================================================
GSRESULT __stdcall RemoteAlgorithm_Initialise(const GSchar * szGameName);

//==============================================================================
// Function RemoteAlgorithm_Uninitialise
/*!
\brief Releases the resources acquired by the service
\par   Description:
       This function releases the resources that were acquired during the
       initialisation.

       This function has no effect if RemoveAlgorithm_Initialise() has not been
       called before.

\return     Result code of the operation
\retval     GSS_OK      No possible error could occur
*/
//==============================================================================
GSRESULT __stdcall RemoteAlgorithm_Uninitialise();

//==============================================================================
// Function RemoteAlgorithm_Engine
/*!
\brief Runs the service
\par   Description:
       This function keeps the service alive by process the outgoing request and
       the incoming replies.

       It is important that this function be called regularly to insure proper
       working of the service.

       This function can only be called after having called
       RemoteAlgorithm_Initialise().

\param      uiMaxPostingDelay       Maximum time (in millisec) to spend reading
                                    messages
\param      uiMaxOperationalDelay   Maximum time (in millisec) to spend
                                    processing read messages (the others will
                                    be put in a queue)

\return     Result code of the operation
\retval     GSS_OK      There was no error
\retval     GSE_NOTINITIALIZED     The service needs to be initialised
*/
//==============================================================================
GSRESULT __stdcall RemoteAlgorithm_Engine(GSuint uiMaxPostingDelay = 500,
		GSuint uiMaxOperationalDelay = 800);

//==============================================================================
// Function RemoteAlgorithm_Execute
/*!
\brief Requests the execution of an algorithm
\par   Description:
       This function sends the request to execute the target algorithm. The call
       to this function is asynchronous. The result of the algorithm will be
       sent to the callback passed as an argument.

       This function can only be called after having called
       RemoteAlgorithm_Initialise(). You also need to be logged in to ubi.com
       (LoginSend_LoginRouter()) for this function to work.

\par Callbacks:
::RemoteAlgorithm_OutputCB<br>

\param      uiAlgoId    The target algorithm identifier
\param      pInput      An array of values to pass as input to the target
                        algorithm
\param      uiNumInput  The number of values in the input array
\param      fOutput     The function to call once the output is ready
\param      pData       Custom data to send back to the callback function
\param      uiRequestId [OUTPUT] The request identifier when the call succeed

\return     Result code of the operation
\retval     GSS_OK      There was no error
\retval     GSE_NOTINITIALIZED     The service needs to be initialised
\retval     GSE_UNEXPECTED      The user is not logged in to ubi.com
*/
//==============================================================================
GSRESULT __stdcall RemoteAlgorithm_Execute(GSuint uiAlgoId,
		const RAE_VALUE * pInput, GSuint uiNumInput,
		const RemoteAlgorithm_OutputCB fOutput, const GSvoid * pData,
		GSuint & uiRequestId);

/*! @} end of group7_1 */

/*! @defgroup group7_2 Type Definition
\brief Definitions of the service's data type

The following definitions are the type of data that are needed to use the
service's interface.
@{
*/

/*! @} end of group7_2 */
		
/*! @defgroup group_RAECB Callbacks
\brief List of the callbacks available in the remote algorithm execution
       service

@{
@}
*/


/*! @} end of group7 */

} //extern C

#endif //__GSCLIENTLIBRARY_H__
