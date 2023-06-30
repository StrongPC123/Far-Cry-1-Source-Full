//****************************************************************************
//*   Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
//*   Date:    5/15/01 3:36:04 PM
 /*!  \file   GLIRCCallbacks.h
  *   \brief  Defines the callback types used by the library
  *
  *   This file describes the callback types when by the library.
  */
//****************************************************************************

#ifndef __GLIRCCALLBACKS_H__
#ifndef DOX_SKIP_THIS
#	define __GLIRCCALLBACKS_H__
#endif // DOX_SKIP_THIS

#include "GSTypes.h"

/*! @defgroup group_callback_misc Callbacks: Miscellaneous
\brief Miscellaneous callbacks

These callbacks are not classified and may be used by several functions or as
for debugging purposes.

@{
*/

//============================================================================
// CallBack CBIRCRcv_Empty
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:44:14 PM
/*!
 \par       Description:
			Type of callback used by message with no parameters. The callbacks
			for _Welcome, _SetAwayResult, and _RemoveAwayResult are of this 
			type.
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_Empty)(GSvoid);


//============================================================================
// CallBack CBIRCRcv_Error
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:46:40 PM
/*!
 \par       Description:
			Type of callback used mainly for debugging. The callbacks for
			_OnError and _Debug are of this type. 

 \param		szErrorMsg	Error message description
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_Error)(GSchar *szErrorMsg);

/*! @} enf og group_callback_misc */

/*! @defgroup group_callback_player Callbacks: Player
\brief Player Related Callbacks

These callbacks are used by functions reporting information about other players
status in joined channels.

@{
*/

//============================================================================
// CallBack CBIRCRcv_PlayerQuit
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:48:00 PM
/*!
 \par       Description:
			Type of callback called when the user sees another quit IRC.

 \param		szPlayer	The IRC nickname of the user that quits
 \param		szQuitMsg	The quit message
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerQuit)(GSchar * szPlayer, GSchar * szQuitMsg);


//============================================================================
// CallBack CBIRCRcv_PlayerJoinChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:10 PM
/*!
 \par       Description:
			Type of callback called when the user seers himself or another
			enter a channel.

 \param		szPlayer	The IRC nickname of the joining player
 \param		lGroupID	The session ID chat being joined
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerJoinChannel)(GSchar * szPlayer, GSint lGroupID);


//============================================================================
// CallBack CBIRCRcv_PlayerJoinChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    12/10/01 9:52:09 AM
/*!
 \par       Description:
			Type of callback called when the user seers himself or another
			enter a channel.
 
 \param		szPlayer	The IRC nickname of the joining player
 \param		iGroupID	The group ID chat being joined
 \param		iLobbyID	The ID of the lobby of the chat group
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerJoinChannel2)(GSchar * szPlayer, GSint iLobbyID, GSint iGroupID);


//============================================================================
// CallBack CBIRCRcv_PlayerLeaveChannel
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:13 PM
/*!
 \par       Description:
			Type of callback called when the user sees himself or another
			leave a channel.

 \param		szPlayer	The IRC nickname of the leaving player
 \param		lGroupID	The session ID chat being left
 \param		szLeaveMsg	Leave message
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerLeaveChannel)(GSchar * szPlayer, GSint lGroupID, GSchar * szLeaveMsg);


//============================================================================
// CallBack CBIRCRcv_PlayerLeaveChannel2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:13 PM
/*!
 \par       Description:
			Type of callback called when the user sees himself or another
			leave a channel.

 \param		szPlayer	The IRC nickname of the leaving player
 \param		lGroupID	The group ID chat being left
 \param		iLobbyID	The ID of the lobby of the chat group
 \param		szLeaveMsg	Leave message
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerLeaveChannel2)(GSchar * szPlayer, GSint iLobbyID, GSint lGroupID, GSchar * szLeaveMsg);


//============================================================================
// CallBack CBIRCRcv_PlayerAway
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:18 PM
/*!
 \par       Description:
			Type of callback called when the user sends a private message to
			a user that is in away mode.

 \param		szPlayer	The IRC nickname of the away player
 \param		szAwayMsg	Away message
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PlayerAway)(GSchar * szPlayer, GSchar * szAwayMsg);


//============================================================================
// CallBack CBIRCRcv_ChannelNameList
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:20 PM
/*!
 \par       Description:
			Type of callback called after a player joins a channel. It is used
			to tell to the user about the users in the channel he joins. This
			list will contain is own name. They may be more than one calls to
			this callback per channel join. The size of the member names is
			IRCIDLENGTH which is defined in define.h

 \param		lGroupID	The session ID chat that was joined
 \param		pszMembers	Array of IRC nickname present in the room
 \param		wCount		The number of element in pszMembers
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_ChannelNameList)(GSint lGroupID, GSchar ** pszMembers, GSshort wCount);


//============================================================================
// CallBack CBIRCRcv_ChannelNameList2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:20 PM
/*!
 \par       Description:
			Type of callback called after a player joins a channel. It is used
			to tell to the user about the users in the channel he joins. This
			list will contain is own name. They may be more than one calls to
			this callback per channel join. The size of the member names is
			IRCIDLENGTH which is defined in define.h

 \param		lGroupID	The group ID chat that was joined
 \param		iLobbyID	The ID of the lobby of the group
 \param		pszMembers	Array of IRC nickname present in the room
 \param		wCount		The number of element in pszMembers
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_ChannelNameList2)(GSint iLobbyID, GSint lGroupID, GSchar ** pszMembers, GSshort wCount);

/*! @} end of group_callback_player */

/*! @defgroup group_callback_message Callbacks: Messaging
\brief Messaging Callbacks

These callbacks are used by messaging functions that are called when users are 
chatting.

@{
*/

//============================================================================
// CallBack CBIRCRcv_PublicMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:25 PM
/*!
 \par       Description:
			Type of callback called when a public message is sent to a channel.
			Note that the user will not receive his own messages back from the
			server.

 \param		szPlayer		The IRC nickname of the source of the message
 \param		szUserName		The GS username of the source of the message
 \param		lGroupID		The session ID when the message was sent
 \param		szMsg			The message that was sent
 \param		lFontColor		Color of the font used by the source
 \param		iFontSize		Size of the font used by the source
 \param		FontStyle		Style of the font used by the source
 \param		iFontSet		Set of the font used by the source
 \param		szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PublicMessage)(GSchar * szPlayer, GSchar * szUserName, GSint lGroupID,
									   GSchar * szMsg, GSint lFontColor, GSint iFontSize,
									   GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);


//============================================================================
// CallBack CBIRCRcv_PublicMessage2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:25 PM
/*!
 \par       Description:
			Type of callback called when a public message is sent to a channel.
			Note that the user will not receive his own messages back from the
			server.

 \param		szPlayer		The IRC nickname of the source of the message
 \param		szUserName		The GS username of the source of the message
 \param		lGroupID		The group ID where the message was sent
 \param		iLobbyID		The ID of the lobby of the group
 \param		szMsg			The message that was sent
 \param		lFontColor		Color of the font used by the source
 \param		iFontSize		Size of the font used by the source
 \param		FontStyle		Style of the font used by the source
 \param		iFontSet		Set of the font used by the source
 \param		szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PublicMessage2)(GSchar * szPlayer, GSchar * szUserName, GSint iLobbyID, GSint lGroupID,
												   GSchar * szMsg, GSint lFontColor, GSint iFontSize,
												   GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);


//============================================================================
// CallBack CBIRCRcv_PrivateMessage
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 3:04:44 PM
/*!
 \par       Description:
			Type of callback called when a private message is sent to the user.

 \param		szPlayer		The IRC nickname of the source of the message
 \param		szUserName		The GS username of the source of the message
 \param		szMsg			The message that was sent
 \param		lFontColor		Color of the font used by the source
 \param		iFontSize		Size of the font used by the source
 \param		FontStyle		Style of the font used by the source
 \param		iFontSet		Set of the font used by the source
 \param		szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PrivateMessage)(GSchar * szPlayer, GSchar * szUserName,
										GSchar * szMsg, GSint lFontColor, GSint iFontSize,
										GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);


//============================================================================
// CallBack CBIRCRcv_PublicAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:43 PM
/*!
 \par       Description:
			Type of callback called when a public action is sent to a channel.
			Note that the user will not receive his own messages back from the
			server.

 \param	szPlayer		The IRC nickname of the source of the message
 \param	szUserName		The GS username of the source of the message
 \param	lGroupID		The session ID when the message was sent
 \param	szAction		The action that was sent
 \param	lFontColor		Color of the font used by the source
 \param	iFontSize		Size of the font used by the source
 \param	FontStyle		Style of the font used by the source
 \param	iFontSet		Set of the font used by the source
 \param	szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PublicAction)(GSchar * szPlayer, GSchar * szUserName, GSint lGroupID,
									 GSchar * szAction, GSint lFontColor, GSint iFontSize,
									 GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);


//============================================================================
// CallBack CBIRCRcv_PublicAction2
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 2:50:43 PM
/*!
 \par       Description:
			Type of callback called when a public action is sent to a channel.
			Note that the user will not receive his own messages back from the
			server.

 \param	szPlayer		The IRC nickname of the source of the message
 \param	szUserName		The GS username of the source of the message
 \param	lGroupID		The group ID when the message was sent
 \param	iLobbyID		The ID of the lobby if the group
 \param	szAction		The action that was sent
 \param	lFontColor		Color of the font used by the source
 \param	iFontSize		Size of the font used by the source
 \param	FontStyle		Style of the font used by the source
 \param	iFontSet		Set of the font used by the source
 \param	szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PublicAction2)(GSchar * szPlayer, GSchar * szUserName, GSint iLobbyID, GSint lGroupID,
												  GSchar * szAction, GSint lFontColor, GSint iFontSize,
												  GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);


//============================================================================
// CallBack CBIRCRcv_PrivateAction
// Author:  Philippe Lalande  [gsdevelopers@ubisoft.com]
// Date:    5/15/01 3:05:58 PM
/*!
 \par       Description:
			Type of callback called when a private action is sent to the user.

 \param	szPlayer		The IRC nickname of the source of the message
 \param	szUserName		The GS username of the source of the message
 \param	szAction		The action that was sent
 \param	lFontColor		Color of the font used by the source
 \param	iFontSize		Size of the font used by the source
 \param	FontStyle		Style of the font used by the source
 \param	iFontSet		Set of the font used by the source
 \param	szFontFamily	Family of the font used by the source
*/
//============================================================================
typedef GSvoid (__stdcall *CBIRCRcv_PrivateAction)(GSchar * szPlayer, GSchar * szUserName,
									   GSchar * szAction, GSint lFontColor, GSint iFontSize,
									   GSint FontStyle, GSint iFontSet, GSchar * szFontFamily);

/*! @} end of group_callback_messaging */

#endif // __GLIRCCALLBACKS_H__
