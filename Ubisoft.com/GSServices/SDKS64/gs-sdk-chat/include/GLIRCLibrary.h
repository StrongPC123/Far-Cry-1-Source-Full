//****************************************************************************
//*   Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
//*   Date:    5/15/01 10:13:04 AM
 /*!  \file   GLIRCLibrary.h
  *   \brief  Available functions for client applications
  *
  *   This files provided all the functionalities of the GS Chat Library
  *   for 3rd parties to use to create easy chat management as an add-on
  *   to the session management system.
  */
//****************************************************************************

/*!
\mainpage gs-sdk-chat
\section intro Introduction
  ubi.com chatting system interface

\section description Description
 This SDK provides functionality for game developers to add chatting capability
 to their match-making menus and game (although limited). This services will be
 compatible with the ubi.com Game Service client chat.

 The chat system is an optional addon to our matchmaking libraries. The use of the
 gs-sdk-base is mandatory if you wish to use the gs-sdk-chat. The ubi.com chat
 is based on IRCU server-base.

 This SDK features:
 - Connecting and disconnecting from chat server;
 - Joining and leaving chat rooms;
 - Sending public or private message and actions;
 - AWAY and SILENCING options
 - Font settings communication.

\section usage Getting Started
 This section should give you enough information to get started in using this SDK.
 It is recommended that you connect to the chat server when you make your initial
 connections with the other services of ubi.com. You must start the usage of this
 library by calling IRC_Initialisation() before calling any other functions. You
 may provide this function with the ubi.com user name or call IRC_SetUbiComUserName().

\subsection step1 Step 1 - Getting Online
 The initial step to getting a user to chat is to have its game client connect and
 login to the chat server. This is achieved by calling the IRC_Connect() and 
 IRC_SendLogin() functions successively.

 The IP address and port of the chat server can be retrieved in the same config file
 that contains the rest of the ubi.com server locations. 

 The nickname needed by IRC_SendLogin() is the IRC identifier that can be
 retrieved by the gs-sdk-base function LoginSend_PlayerInfo(). It is necessary to
 use this identifier as it insures us that the user will be unique on the chat server.
 The username argument should be fixed for all users of your game and should be provided
 to you by ubi.com. The last argument, realname, should be the ubi.com user name of the
 player.

 A user who has successfully logged in will have its 'Welcome' callback called.

\subsection step2 Step 2 - Joining A Room
 Before being able to chat publically, a user needs to join a chat room (also called
 channel). This can be achieved by calling the JoinChannel() function if your 
 game is supported by ubi.com arena services or by the JoinChannel2() function if
 it is supported by the ubi.com lobby servers. The necessary information to pass as 
 arguments to these functions are available through the gs-sdk-base interface.

\subsection step3 Step 3 - Let The Chatting Begin
 To chat publically, you need to call the PublicMessage() function associated with
 the service supporting your game (see Step 2). It is also possible to send 'actions'
 publically. This is a special line of text that will be displayed as if the user
 was executing an action.

 It is also possible to chat privately. However, you will first need to obtain the IRC
 identifier of the target of that chat by calling the LoginSend_PlayerInfo() function
 available in gs-sdk-base.

\subsection step4 Step 4 - Until Next Time...
 To end a chat session, you will need to call the IRCSend_Quit() function followed by
 the IRC_Disconnect() function.

\subsection other Special Options
 There are two options available to chat user: the away mode and silencing. You can set and
 unset your away mode by respectivaly calling IRC_SetAway() and IRC_RemoveAway()
 functions. In this mode, any user sending you a private message will automatically receive
 the message passed to the SetAway function. Silencing allows for ignoring private message
 from other users. It currently does not work with public messages.

\subsection notes Notes:
1. In-game chatting<br>
It is possible to use the same messaging functions while in-game to chat with the other 
players. There is however no method to send messages only to a subgroup of the players
of the game session.

2. Font settings<br>
The fontifying functions are a mean to personalize the way a chat line will appear. Its
use is totally optional. You must be aware of the possibility that users may try to use
this feature to abuse the system (i.e. with HUGE fonts).

*/


#ifndef __GLIRCLIBRARY_H__
#ifndef DOX_SKIP_THIS
#	define __GLIRCLIBRARY_H__
#endif // DOX_SKIP_THIS

#include "GSTypes.h"
#include "GLIRCCallbacks.h"

#ifndef DOX_SKIP_THIS
#	define GL_IRC_FONTSTYLE_REGULAR 0x00000000
#	define GL_IRC_FONTSTYLE_BOLD    0x00000001
#	define GL_IRC_FONTSTYLE_ITALIC  0x00000002
#	define GL_IRC_FONTSTYLE_STRIKE  0x00000003

#	define GL_IRC_FONTSET_ANSI    0
#	define GL_IRC_FONTSET_DEFAULT 1
#	define GL_IRC_FONTSET_SYMBOL  2
#endif // DOX_SKIP_THIS


extern "C" {

/*! @defgroup group_function_gen Functions: Generalities
\brief General functions

These functions are used to initialize, uninitialize, and run the engine of the
library.

@{
*/

//============================================================================
// Function IRC_Initialisation
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:15:30 AM
/*!
 \par       Description:
			Initialises the communication ressources. Must be call prior to any
			other functions. Giving the username is optional. You will however
			need to provide it by the IRCSetUbiComUserName function is you do not 
			pass it through here.

 \return    Result of the initialization

 \retval	GS_FALSE	Initialization fails
 \retval	GS_TRUE	Initialization was succesfull

 \param		szUserName	ubi.com username of the client
*/
//============================================================================
GSbool __stdcall IRC_Initialisation(const GSchar * szUserName = "");


//============================================================================
// Function IRC_Uninitialisation
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:27:28 AM
/*!
 \par       Description:
			Call this when you're done using the library, use in pair with
			IRC_Initialisation.

 \return    None
*/
//============================================================================
GSvoid __stdcall IRC_Uninitialisation();


//============================================================================
// Function IRC_Engine
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:29:05 AM
/*!
 \par       Description:
			Updates the connection between the client and the server, and handles
			the delivery of queued up messages and reception of messages. This
			function should be called regularly to ensure that the application
			will run smoothly.

 \return    State of the communication

 \retval	GS_FALSE	Indicates a problem with the communication. Client is disconnected
 \retval	GS_TRUE	No problems encountered


*/
//============================================================================
GSbool __stdcall IRC_Engine();

/*! @} end of group_function_gen */

/*! @defgroup group_function_conn Functions: Connection
\brief Connection functions

These functions handle the connection process of the system. They enable a player
to connect, login, quit, and disconnect to/from a chat server.

@{
*/

//============================================================================
// Function IRC_Connect
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:41:31 AM
/*!
 \par       Description:
			Makes a connection to the given chat server unless already connected
			to it. If connected to another chat server, it automatically disconnect
			from it first.

 \return    Result of the operation

 \retval	GS_FALSE	Connection failed
 \retval	GS_TRUE	Connection was successfull

 \param		szIPAddress		IP of the chat server in dot notation
 \param		wPort			Port of the chat server

*/
//============================================================================
GSbool __stdcall IRC_Connect(const GSchar * szIPAddress, GSshort wPort);


//============================================================================
// Function IRC_Disconnect
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:50:18 AM
/*!
 \par       Description:
			Disconnects from the chat server.

 \return    Result of the operation

 \retval	GS_TRUE	Always
*/
//============================================================================
GSbool __stdcall IRC_Disconnect();


//============================================================================
// Function IRCSend_Login
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 10:51:28 AM
/*!
 \par       Description:
			Registers the client to the chat server with the given IRC nickname,
			username, and realname. 

 \return    Result of the operation

 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		szNickName	IRC nickname as given by the gs-sdk-base
 \param		szUserName	Static string related to the application using this sdk
 \param		szRealName	GS username of the client
*/
//============================================================================
GSbool __stdcall IRCSend_Login(const GSchar * szNickName, const GSchar * szUserName, const GSchar * szRealName);


//============================================================================
// Function IRCSend_Quit
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:00:01 AM
/*!
 \par       Description:
			Leaves the chat server sending an optional quit messages and 
			disconnects the sockets.

 \return    Result of the operation

 \retval	GS_TRUE	Always

 \param		szQuitMsg	Message to display to the other chat participant when
						user quits.
*/
//============================================================================
GSbool __stdcall IRCSend_Quit(const GSchar * szQuitMsg);

/*! @} end of group_function_conn */

/*! @defgroup group_function_chan Functions: Channel
\brief Chat Room Functions

These functions handle the interaction with a chat room enabling a player to 
join and leave a room.

@{
*/

//============================================================================
// Function IRCSend_JoinChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:01:50 AM
/*!
 \par       Description:
			Ask the chat server to add you in the channel of the given session
			group id. All channels are password protected using information from
			the session it is associated with. For use with gsarena session.

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE		Message was sent

 \param		lGroupID		ID of the session to join chat. Available from the
							gs-sdk-base.
 \param		szGameName		"FOLDER" for a basicgroupd or the ugly name of the 
							game associated to the session. Available from the
							gs-sdk-base.
 \param		szBranchGames	Allowed games in a "FOLDER", empty string otherwise.
							Available from the gs-sdk-base.
*/
//============================================================================
GSbool __stdcall IRCSend_JoinChannel(GSint lGroupID, const GSchar * szGameName, const GSchar * szBranchGames);

//============================================================================
// Function IRCSend_JoinChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    12/4/01 5:02:14 PM
/*!
 \par       Description:
			Ask the chat server to add you in the channel of the given room of
			a given lobbyserver.

 \return    Result of the operation
  
 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE		Message was sent
	
 \param		iRoomID		ID of the room to join. Available from the gs-sdk-base
 \param		iLobbyID	ID of the lobbyserver where that room reside. 
						Available form the gs-sdk-base
*/
//============================================================================
GSbool __stdcall IRCSend_JoinChannel2(GSint iRoomID, GSint iLobbyID);


#ifndef DOX_SKIP_THIS
//============================================================================
// Function IRCSend_JoinPrivateChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:08:04 AM
/*!
 \par       Description:
			Same than IRCSend_JoinChannel except that we join a private channel
			unrelated to the gs network session.

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent

 \param		lUniqueId	ID to identify the channel.
 \param		szPassword	Password to protect channel. Can be empty.
*/
//============================================================================
#ifdef GSCLIENT_PRIVATECHAT
GSbool __stdcall IRCSend_JoinPrivateChannel(GSint lUniqueId, const GSchar * szPassword);
#endif // GSCLIENT_PRIVATECHAT
#endif // DOX_SKIP_THIS


//============================================================================
// Function IRCSend_LeaveChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:15:29 AM
/*!
 \par       Description:
			Ask the chat server to remove you from the channel of to the given
			session group id. An optional message can be given and will be sent
			to the member of that channel when you leave.

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent

 \param	lGroupID	Session ID of the chat to leave
 \param	szLeaveMsg	Leave message to display the participant of this room when
					user leaves
*/
//============================================================================
GSbool __stdcall IRCSend_LeaveChannel(GSint lGroupID, const GSchar * szLeaveMsg);

//============================================================================
// Function IRCSend_LeaveChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    12/4/01 5:05:43 PM
/*!
 \par       Description:
			Ask the chat server to remove you from the channel of to the given
			room and lobbyserver id. An optional message can be given and will
			be sent to the members of that channel when you leave.
			
 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE		Message was sent

 \param		iRoomID		ID of the room to leave.
 \param		iLobbyID	ID of the lobbyserver where that room reside.
 \param		szLeaveMsg	Leave message to display to the other participants
*/
//============================================================================
GSbool __stdcall IRCSend_LeaveChannel2(GSint iRoomID, GSint iLobbyID, const GSchar * szLeaveMsg);


#ifndef DOX_SKIP_THIS
//============================================================================
// Function IRCSend_LeavePrivateChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:18:30 AM
/*!
 \par       Description:
			Similar to IRCSend_LeaveChannel except that this function is called
			in order to leave a channel joined with IRCSend_JoinPrivateChannel

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent

 \param	lUniqueId	Channel ID to leave
 \param	szLeaveMsg	Leave message to display the participant of this room when
					user leaves
*/
//============================================================================
#ifdef GSCLIENT_PRIVATECHAT
GSbool __stdcall IRCSend_LeavePrivateChannel(GSint lUniqueId, const GSchar * szLeaveMsg);
#endif // GSCLIENT_PRIVATECHAT
#endif // DOX_SKIP_THIS

/*! @} end of group_function_chan */

/*! @defgroup group_function_msg Functions: Messaging
\brief Messaging Functions

These functions handle the interaction between players enabling them to send
public and private messages and actions.

@{
*/

//============================================================================
// Function IRCSend_PublicMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:21:44 AM
/*!
 \par       Description:
			Sends the given message to the channel of the given session group
			id.

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent

 \param		lGroupID	ID of the session
 \param		szMsg		Message to send
*/
//============================================================================
GSbool __stdcall IRCSend_PublicMessage(GSint lGroupID, const GSchar * szMsg);

//============================================================================
// Function IRCSend_PublicMessage2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    12/4/01 5:08:27 PM
/*!
 \par       Description:
			Sends the given message to the channel of the given room and lobby
			server ID.

 \return    Result of the operation
  
 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE		Message was sent
	
 \param		iRoomID		ID of the room
 \param		iLobbyID	ID of the lobbyserver
 \param		szMsg		Message to send
*/
//============================================================================
GSbool __stdcall IRCSend_PublicMessage2(GSint iRoomID, GSint iLobbyID, const GSchar * szMsg);


#ifndef DOX_SKIP_THIS
//============================================================================
// Function IRCSend_PrivateChannelMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:23:06 AM
/*!
 \par       Description:
			Similar to IRCSend_PublicMessage except that this function is used
			to communicate inside a channel joined with IRCSend_JoinPrivateChannel.

 \return    Result of the operation

 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent

 \param		lUniqueId	Channel ID to send the message to
 \param		szMsg		Message to send
*/
//============================================================================
#ifdef GSCLIENT_PRIVATECHAT
GSbool __stdcall IRCSend_PrivateChannelMessage(GSint lUniqueId, const GSchar * szMsg);
#endif // GSCLIENT_PRIVATECHAT
#endif // DOX_SKIP_THIS


//============================================================================
// Function IRCSend_PrivateMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:11 AM
/*!
 \par       Description:
			Sends a message to the given IRC nickname

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		szTarget	IRC nickname of the recipient. Available from the 
						gs-sdk-base
 \param		szMsg		Message to send
*/
//============================================================================
GSbool __stdcall IRCSend_PrivateMessage(GSchar * szTarget, GSchar * szMsg);


//============================================================================
// Function IRCSend_PublicAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:16 AM
/*!
 \par       Description:
			Sends the given message to the channel of the given session group
			id. Will display the message as an action (ie Joe ate an apple 
			if szAction was 'eats an apple')

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		lGroupID	Session ID to send action to
 \param		szAction	Action to send
*/
//============================================================================
GSbool __stdcall IRCSend_PublicAction(GSint lGroupID, const GSchar * szAction);

//============================================================================
// Function IRCSend_PublicAction2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    12/4/01 5:09:57 PM
/*!
 \par       Description:
			Sends the given message to the channel of the given room and lobby
			server id. Will display the message as an action (ie 'Joe eats an 
			apple' if szAction was 'eats an apple')
  
 \return    Result of the operation
	
 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE		Message was sent
	  
 \param		iRoomID		ID of the room
 \param		iLobbyID	ID of the lobbyserver
 \param		szAction	Message to send
*/
//============================================================================
GSbool __stdcall IRCSend_PublicAction2(GSint iRoomID, GSint iLobbyID, const GSchar * szAction);


#ifndef DOX_SKIP_THIS
//============================================================================
// Function IRCSend_PrivateChannelAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:47:08 AM
/*!
 \par       Description:
			Similar to IRCSend_PublicAction except that this function is used
			to communicate inside a channel joined with IRCSend_JoinPrivateChannel.

 \return    Result of the operation
  
 \retval	GS_FALSE	Could not send message
 \retval	GS_TRUE	Message was sent
	
 \param		lUniqueId	Channel ID to send the message to
 \param		szAction	Action to send
*/
//============================================================================
#ifdef GSCLIENT_PRIVATECHAT
GSbool __stdcall IRCSend_PrivateChannelAction(GSint lUniqueId, const GSchar * szAction);
#endif // GSCLIENT_PRIVATECHAT
#endif // DOX_SKIP_THIS


//============================================================================
// Function IRCSend_PrivateAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:20 AM
/*!
 \par       Description:
			Sends a message as an action to the given IRC nickname

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param	szTarget	IRC nickname of the recipient. Available from the 
					gs-sdk-base
 \param	szAction	Action to send
*/
//============================================================================
GSbool __stdcall IRCSend_PrivateAction(const GSchar * szTarget, const GSchar * szAction);

/*! @} end of group_function_msg */

/*! @defgroup group_function_misc Functions: Miscellaneous
\brief Miscellaneous Functions

These functions provides miscellaneous options available to the players.

@{
*/

//============================================================================
// Function IRCSend_SilenceUser
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:23 AM
/*!
 \par       Description:
			Ask the server to silently ignore all private messages from the
			given IRC nickname. No confirmation of your request will be sent
			back. This does not block public messages.

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		szTarget	IRC nickname of the user to block. Available from the
						gs-sdk-base
*/
//============================================================================
GSbool __stdcall IRCSend_SilenceUser(const GSchar * szTarget);


//============================================================================
// Function IRCSend_UnSilenceUser
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:26 AM
/*!
 \par       Description:
			Ask the server to remove the given IRC nickname from your ignore list.
			No confirmation of your request will be sent back from the server

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		szTarget	IRC nickname of the user to unblock. Available from the
						gs-sdk-base
*/
//============================================================================
GSbool __stdcall IRCSend_UnSilenceUser(const GSchar * szTarget);


//============================================================================
// Function IRCSend_SetAway
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:29 AM
/*!
 \par       Description:
			Ask the server to put yourself in away mode. With such a status,
			each message sent to you privately will automatically be answered
			by the given away message. 

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent

 \param		szMsg	Away message
*/
//============================================================================
GSbool __stdcall IRCSend_SetAway(const GSchar * szMsg);


//============================================================================
// Function IRCSend_RemoveAway
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:33:32 AM
/*!
 \par       Description:
			Ask the server to remove yourself from away mode.

 \return    Result of the operation
  
 \retval	GS_FALSE	Message could not be sent
 \retval	GS_TRUE	Message was successfully sent
*/
//============================================================================
GSbool __stdcall IRCSend_RemoveAway();

/*! @} end of group_function_misc */

/*! @defgroup group_function_fix Functions: Registration
\brief Callback Registration Functions

These functions handle the registration of the available callbacks.

@{
*/

//============================================================================
// Function IRCFix_Welcome
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 11:57:46 AM
/*!
 \par       Description:
			Register the function that will be called when the server acknowledge
			your registration

 \return    None

 \param		fWelcome	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_Welcome(CBIRCRcv_Empty fWelcome);


//============================================================================
// Function IRCFix_SetAwayResult
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:07:48 PM
/*!
 \par       Description:
			Register the function that will be called when the server acknowledge
			that your SetAway request

 \return    None

 \param		fSetAwayResult	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_SetAwayResult(CBIRCRcv_Empty fSetAwayResult);


//============================================================================
// Function IRCFix_RemoveAwayResult
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:08:36 PM
/*!
 \par       Description:
			Register the function that will be called when the server acknowledge
			that your RemoveAway request

 \return    None

 \param		fRemoveAwayResult	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_RemoveAwayResult(CBIRCRcv_Empty fRemoveAwayResult);


//============================================================================
// Function IRCFix_PlayerQuit
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:09:42 PM
/*!
 \par       Description:
			Registers the function that will be called when your client receives
			the message that someone has quit the chat server (that person must
			be in at least one chat room with you).

 \return    None

 \param		fPlayerQuit	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerQuit(CBIRCRcv_PlayerQuit fPlayerQuit);


//============================================================================
// Function IRCFix_PlayerJoinChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:33:13 PM
/*!
 \par       Description:
			Registers the function that will be called when your client receives
			the message that someone has joined a chat room that you are in.

 \return    None

 \param		fPlayerJoinChannel	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerJoinChannel(CBIRCRcv_PlayerJoinChannel fPlayerJoinChannel);


//============================================================================
// Function IRCFix_PlayerJoinChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:33:13 PM
/*!
 \par       Description:
			Registers the function that will be called when your client receives
			the message that someone has joined a chat room that you are in.

 \return    None
  
 \param		fPlayerJoinChannel	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerJoinChannel2(CBIRCRcv_PlayerJoinChannel2 fPlayerJoinChannel);


//============================================================================
// Function IRCFix_PlayerLeaveChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:33:51 PM
/*!
 \par       Description:
			Registers the funtion that will be called when your client receives
			the message that someone had left a chat room that you are in.

 \return    None

 \param		fPlayerLeaveChannel		Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerLeaveChannel(CBIRCRcv_PlayerLeaveChannel fPlayerLeaveChannel);


//============================================================================
// Function IRCFix_PlayerLeaveChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:33:51 PM
/*!
 \par       Description:
			Registers the funtion that will be called when your client receives
			the message that someone had left a chat room that you are in.

 \return    None
  
 \param		fPlayerLeaveChannel		Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerLeaveChannel2(CBIRCRcv_PlayerLeaveChannel2 fPlayerLeaveChannel);


//============================================================================
// Function IRCFix_ChannelNameList
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:34:33 PM
/*!
 \par       Description:
			Registers the function that will be called after you have joined a
			channel and the server informs you who was there at the moment you
			joined. (NB. The client's name will be part of that list and he will
			also receive a PlayerJoinChannel message for himself)

 \return    None

 \param		fChannelNameList	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_ChannelNameList(CBIRCRcv_ChannelNameList fChannelNameList);


//============================================================================
// Function IRCFix_ChannelNameList2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:34:33 PM
/*!
 \par       Description:
			Registers the function that will be called after you have joined a
			channel and the server informs you who was there at the moment you
			joined. (NB. The client's name will be part of that list and he will
			also receive a PlayerJoinChannel message for himself)

 \return    None
  
 \param		fChannelNameList	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_ChannelNameList2(CBIRCRcv_ChannelNameList2 fChannelNameList);


//============================================================================
// Function IRCFix_PublicMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:35:45 PM
/*!
 \par       Description:
			Registers the function to be called when receiving public
			message. (NB. There is no echo when you send messages)

 \return    None

 \param		fPublicMessage	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PublicMessage(CBIRCRcv_PublicMessage fPublicMessage);


//============================================================================
// Function IRCFix_PublicMessage2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:35:45 PM
/*!
 \par       Description:
			Registers the function to be called when receiving public
			message. (NB. There is no echo when you send messages)

 \return    None
  
 \param		fPublicMessage	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PublicMessage2(CBIRCRcv_PublicMessage2 fPublicMessage);


//============================================================================
// Function IRCFix_PrivateMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:36:24 PM
/*!
 \par       Description:
			Registers the function to be called when receiving private
			message. (NB. There is no echo when you send messages)

 \return    None

 \param		fPrivateMessage		Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PrivateMessage(CBIRCRcv_PrivateMessage fPrivateMessage);


//============================================================================
// Function IRCFix_PublicAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:12 PM
/*!
 \par       Description:
			Registers the function to be called when receiving public
			action. (NB. There is no echo when you send messages)

 \return    None

 \param		fPublicAction	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PublicAction(CBIRCRcv_PublicAction fPublicAction);


//============================================================================
// Function IRCFix_PublicAction2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:12 PM
/*!
 \par       Description:
			Registers the function to be called when receiving public
			action. (NB. There is no echo when you send messages)

 \return    None
  
 \param		fPublicAction	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PublicAction2(CBIRCRcv_PublicAction2 fPublicAction);


//============================================================================
// Function IRCFix_PrivateAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:15 PM
/*!
 \par       Description:
			Registers the function to be called when receiving private
			action. (NB. There is no echo when you send messages)

 \return    None

 \param		fPrivateAction	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PrivateAction(CBIRCRcv_PrivateAction fPrivateAction);


//============================================================================
// Function IRCFix_PlayerAway
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:17 PM
/*!
 \par       Description:
			Registers the function to be called when the server notifies you
			that your private recipient is away.

 \return    None

 \param		fPlayerAway		Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_PlayerAway(CBIRCRcv_PlayerAway fPlayerAway);


//============================================================================
// Function IRCFix_OnError
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:22 PM
/*!
 \par       Description:
			Registers the function to be called upon receiving an ERROR message
			from the server. Mainly for debug info, no assumption should be made
			from receiving those messages

 \return    None

 \param		fOnError	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_OnError(CBIRCRcv_Error fOnError);


//============================================================================
// Function IRCFix_Debug
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:39:26 PM
/*!
 \par       Description:
			Registers a function to be called for each message received
			from the server. This is for debugging purpose only. This
			function will be called in addition to the regular callback.

 \return    None

 \param		fDebug	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_Debug(CBIRCRcv_Error fDebug);


//============================================================================
// Function IRCFix_Default
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    11/20/01 9:38:36 AM
/*!
 \par       Description:
			Registers a function to be called for each message received
			from the server. This is for debugging purpose only. This
			function will be called in addition to the regular callback.

 \return    None

 \param		fDefault	Function to callback
*/
//============================================================================
GSvoid __stdcall IRCFix_Default(CBIRCRcv_Error fDefault);

/*! @} end of group_function_fix */

/*! @defgroup group_function_set Functions: Settings
\brief Settings Functions

These functions handles the different settings that will be passed when sending
messages to other players on the chat server.

@{
*/

//============================================================================
// Function IRCSetUbiComUserName
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:17 PM
/*!
 \par       Description:
			Sets the ubi.com username to be broadcasted with messages. The use
			of this function is optional if the username was passed through the
			IRC_Initialise function.

 \return    None

 \param		szUserName	User's ubi.com username
*/
//============================================================================
GSvoid __stdcall IRCSetUbiComUserName(const GSchar * szUserName);


//============================================================================
// Function IRCSetFontColor
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:20 PM
/*!
 \par       Description:
			Sets the font color to be broadcasted with messages. The use of this
			function is optional if no action are to be taken with the font.

 \return    None

 \param		lColor	An 0x00RRGGBB colour value
*/
//============================================================================
GSvoid __stdcall IRCSetFontColor(GSint lColor);


//============================================================================
// Function IRCSetFontSize
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:23 PM
/*!
 \par       Description:
			Sets the font size to be broadcasted with messages. The use of this
			function is optional if no action are to be taken with the font.

 \return    None


 \param		iSize	An integer representing the font point
*/
//============================================================================
GSvoid __stdcall IRCSetFontSize(GSint iSize);


//============================================================================
// Function IRCSetFontStyle
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:28 PM
/*!
 \par       Description:
			Sets the font style to be broadcasted with messages. The use of this
			function is optional if no action are to be taken with the font.

 \return    None

 \param		lStyle	GL_IRC_FONTSTYLE_REGULAR, GL_IRC_FONTSTYLE_BOLD, 
					GL_IRC_FONTSTYLE_ITALIC, GL_IRC_FONTSTYLE_STRIKE
*/
//============================================================================
GSvoid __stdcall IRCSetFontStyle(GSint lStyle);


//============================================================================
// Function IRCSetFontSet
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:30 PM
/*!
 \par       Description:
			Sets the font set to be broadcasted with messages. The use of this
			function is optional if no action are to be taken with the font.

 \return    None

 \param		iSet	GL_IRC_FONTSET_ANSI, GL_IRC_FONTSET_NORMAL, 
					GL_IRC_FONTSET_SYMBOL
*/
//============================================================================
GSvoid __stdcall IRCSetFontSet(GSint iSet);


//============================================================================
// Function IRCSetFontFamily
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:33 PM
/*!
 \par       Description:
			Sets the font family to be broadcasted with messages. The use of this
			function is optional if no action are to be taken with the font.

 \return    None

 \param		szFont	Name of the font family
*/
//============================================================================
GSvoid __stdcall IRCSetFontFamily(const GSchar * szFont);


//============================================================================
// Function  IRCGetUbiComUserName
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:36 PM
/*!
 \par       Description:
			Returns the value set as ubi.com username

 \return    GS username

 \param     szUserNameOut   A buffer of size NICKNAMELENGTH
*/
//============================================================================
GSchar * __stdcall IRCGetUbiComUserName(GSchar * szUserNameOut);


//============================================================================
// Function IRCGetFontColor
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:39 PM
/*!
 \par       Description:
			Returns the value used as the font color

 \return    Font colour as 0x00RRGGBB
*/
//============================================================================
GSint __stdcall IRCGetFontColor();


//============================================================================
// Function IRCGetFontSize
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:43 PM
/*!
 \par       Description:
			Returns the value used as the font size

 \return    Font size value
*/
//============================================================================
GSint __stdcall IRCGetFontSize();


//============================================================================
// Function IRCGetFontStyle
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:46 PM
/*!
 \par       Description:
			Returns the value used as the font style

 \return    Font style

 \retval	GL_IRC_FONTSTYLE_REGULAR
 \retval	GL_IRC_FONTSTYLE_BOLD
 \retval	GL_IRC_FONTSTYLE_ITALIC
 \retval	GL_IRC_FONTSTYLE_STRIKE
*/
//============================================================================
GSint __stdcall IRCGetFontStyle();


//============================================================================
// Function IRCGetFontSet
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:51 PM
/*!
 \par       Description:
			Returns the value used as the font set

 \return    Font set

 \retval	GL_IRC_FONTSET_ANSI
 \retval	GL_IRC_FONTSET_DEFAULT
 \retval	GL_IRC_FONTSET_SYMBOL
*/
//============================================================================
GSint __stdcall IRCGetFontSet();


//============================================================================
// Function  IRCGetFontFamily
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 1:52:54 PM
/*!
 \par       Description:
			Returns the value used as the font family

 \return    Name of the font
*/
//============================================================================
GSchar * __stdcall IRCGetFontFamily();

/*! @} end of group_function_set */

} // extern "C"

#endif
