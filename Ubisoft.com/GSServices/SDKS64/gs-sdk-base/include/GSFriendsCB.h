//****************************************************************************
//*   Author:  Guillaume Plante  <gsdevteam@ubisoft.com>
//*   Date:	  5/16/01 9:43:49 AM
 /*!  \file   GSFriendsCB.h
  *   \brief  Callback functions for the <b><i>friends service</i></b>.
  *
  *   This file contains all callback functions declaration for the friend
  *   service.
  */
//****************************************************************************


#ifndef _GSFRIENDSCB_H_
#define _GSFRIENDSCB_H_

#include "GSTypes.h"
#include "GSErrors.h"

#ifdef __cplusplus
class clFriendsCallbacks
{
	public:
	virtual GSvoid FriendsRcv_LoginResult(GSubyte ubType, GSint iReason) = 0;
	virtual GSvoid FriendsRcv_AddFriend(GSubyte ubType, GSint iReason,
			GSchar* szFriend) = 0;
	virtual GSvoid FriendsRcv_DelFriend(GSubyte ubType, GSint iReason,
			GSchar* szFriend) = 0;
    virtual GSvoid FriendsRcv_IgnorePlayer( GSRESULT rCode,
                                            const GSchar * szPlayer ) = 0;
    virtual GSvoid FriendsRcv_UnignorePlayer( GSRESULT rCode,
                                              const GSchar * szPlayer ) = 0;
    virtual GSvoid FriendsRcv_ListIgnoredPlayers( GSRESULT rCode ) = 0;
    virtual GSvoid FriendsRcv_IgnoredPlayer( const GSchar * szPlayer ) = 0;
	virtual GSvoid FriendsRcv_Page(GSchar* szUsername, GSchar* szMessage,
			GSchar* szTimeStamp) = 0;
	virtual GSvoid FriendsRcv_PagePlayer(GSubyte ubType, GSint iReason,
			GSchar* szUsername) = 0;
	virtual GSvoid FriendsRcv_PeerMsg(GSchar* szUsername, GSvoid* p_Buffer,
			GSuint uiLength) = 0;
	virtual GSvoid FriendsRcv_PeerPlayer(GSubyte ubType, GSint iReason,
			GSchar* szUsername) = 0;
	virtual GSvoid FriendsRcv_ChangeFriend(GSubyte ubType, GSint iReason) = 0;
	virtual GSvoid FriendsRcv_StatusChange(GSubyte ubType, GSint iReason) = 0;
	virtual GSvoid FriendsRcv_UpdateFriend(GSchar* szUsername, GSint iReason,
			GSchar* szGroup, GSint iMood, GSint iOptions, GSchar *szGameName) = 0;
	virtual GSvoid FriendsRcv_SearchPlayer(GSubyte ubType, GSint iReason,
			GSchar* szUsername, GSint iStatus, GSchar *szGameName) = 0;
	virtual GSvoid FriendsRcv_ScoreCard(GSubyte ubType, GSint iReason,
			GSchar* szPlayer,GSchar* szGame, GSchar* szScore) = 0;
};
#endif //__cplusplus


/*! @addtogroup group_FriendCB
    @{
*/

//============================================================================
// Callback CBFriendsRcv_LoginResult
/*!
 \brief	 Receive friends service login result
 \par       Description:
  This callback will be called when the client receives response from the router
  after asking to log into the friend service
  
	\par Related Function:
	FriendsSend_Login()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_LoginResult)(GSubyte ubType,
		GSint iReason);

//============================================================================
// Callback CBFriendsRcv_AddFriend
/*!
 \brief	 Receive the status of the friend addition request
 \par       Description:
  This callback will be called when the client receives response from the router
  after adding a friend to his friend list
	
  \par Related Function:
	FriendsSend_AddFriend()
	
	\par Errors:
	ERRORROUTER_DBPROBLEM: There is a problem with the database<br>
	ERRORFRIENDS_FRIENDNOTEXIST: The username didn't exist.

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szFriend	The alias of the newly added friend

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_AddFriend)(GSubyte ubType,
		GSint iReason, GSchar* szUsername);

//============================================================================
// Callback CBFriendsRcv_DelFriend
/*!
 \brief	 Receive the status of the remove friend request
 \par       Description:
  This callback will be called when the client receives response from the router
  after removing a friend from the friend list.
	
  \par Related Function:
	FriendsSend_DelFriend()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szUsername	The alias of the newly deleted friend

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_DelFriend)(GSubyte ubType,
		GSint iReason, GSchar* szUsername);

//============================================================================
// Callback CBFriendsRcv_IgnorePlayer

/*!
\brief  Result of an ignore player request
\par    Description:
        This callback will be called with the results of a previous ignore
        player request.

        <BR>
        Related function : FriendsSend_IgnorePlayer()

\param  szPlayer  The ubi.com username of the player that was ignored
\param  rCode     Result code of the request. Possible values are:
                  <UL>
                      <LI>GSS_OK<BR>
                      There was no error
                      <LI>GSE_INVALIDUSER<BR>
                      The username to ignore is not a valid ubi.com user.
                      <LI>GSE_DBFAILURE<BR>
                      An error occured on the DB while processing
                      the request.
                      <LI>GSE_FAIL<BR>
                      An unsuspected error occured most likely due to a bug
                      on ubi.com
                  </UL>
*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_IgnorePlayer)( GSRESULT rCode,
                                                       const GSchar * szPlayer );

//============================================================================
// Callback CBFriendsRcv_UnignorePlayer

/*!
\brief  Result of an unignore player request
\par    Description:
        This callback will be called with the results of a previous unignore
        player request.

        <BR>
        Related function : FriendsSend_UnignorePlayer()

\param  szPlayer  The ubi.com username of the player that was removed from
                  the ignore-list
\param  rCode     Result code of the request. Possible values are:
                  <UL>
                      <LI>GSS_OK<BR>
                      There was no error
                      <LI>GSE_DBFAILURE<BR>
                      An error occured on the DB while processing
                      the request.
                      <LI>GSE_FAIL<BR>
                      An unsuspected error occured most likely due to a bug
                      on ubi.com
                  </UL>
*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_UnignorePlayer)(
    GSRESULT rCode, const GSchar * szPlayer );

//============================================================================
// Callback CBFriendsRcv_ListIgnoredPlayers

/*!
\brief  Result of a request to get the ignore-list
\par    Description:
        This callback will be called with the results of a previous ignore-list
        retrieval request.

        <BR>
        Related function : FriendsSend_ListIgnoredPlayers()

\param  rCode     Result code of the request. Possible values are:
                  <UL>
                      <LI>GSS_OK<BR>
                      There was no error
                      <LI>GSE_DBFAILURE<BR>
                      An error occured on the DB while processing
                      the request.
                      <LI>GSE_FAIL<BR>
                      An unsuspected error occured most likely due to a bug
                      on ubi.com
                  </UL>
*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_ListIgnoredPlayers)( GSRESULT rCode );

//============================================================================
// Callback CBFriendsRcv_IgnoredPlayer

/*!
\brief  Enumeration of the ignored players
\par    Description:
        This callback will be called for each player in a ignore-list following
        a ignore-list retrieval request. The end of list is notified by the
        CBFriendsRcv_ListIgnoredPlayers callback.

        <BR>
        Related function : FriendsSend_ListIgnoredPlayers()

\param  szPlayer  The ubi.com username of a ignored player
*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_IgnoredPlayer)(
    const GSchar * szPlayer );

//============================================================================
// Callback CBFriendsRcv_Page
/*!
 \brief	 Receive a page message
 \par       Description:
  This callback will be called when the client receives a page message
  from another player
	
  \par Related Function:
	FriendsSend_PagePlayer()

 \param	szUsername	the username of the sender
 \param	szMessage	Message
 \param	szTimeStamp	The server timestamp of when the page message was sent
 

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_Page)(GSchar* szUsername,
		GSchar* pszMessage, GSchar* szTimeStamp);

//============================================================================
// Callback CBFriendsRcv_PagePlayer
/*!
 \brief	 Receive the status of send page request
 \par       Description:
  This callback will be called when the client receives response from the router
  after sending a page to another player
	
  \par Related Function:
	FriendsSend_PagePlayer()
	
	\par Errors:
	ERRORROUTER_UNKNOWNERROR: The page message was not sent.
	ERRORFRIENDS_PLAYERSTATUSCOREONLINE: The player can't receive page messages.
	ERRORFRIENDS_PLAYERIGNORE: The player has ignored the sender

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szUsername	The username of the recipient

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_PagePlayer)(GSubyte ubType,
		GSint iReason, GSchar* szUsername);

//============================================================================
// Callback CBFriendsRcv_PeerMsg
/*!
 \brief	 Receive a peer message
 \par       Description:
  This callback will be called when the client receives a peer message from
	another player
	
  \par Related Function:
	FriendsSend_PeerPlayer()
 
 \param	szUsername	Username of the sender
 \param	p_Buffer	Data buffer
 \param	uiLength	Length of the buffer

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_PeerMsg)(GSchar* szUsername,
		GSvoid* p_Buffer, GSuint uiLength);


//============================================================================
// Callback CBFriendsRcv_PeerPlayer
/*!
 \brief	 Receive the status of the send peer message request
 \par       Description:
  This callback will be called when the client receives response from the router
  after sending a peer player message
	
  \par Related Function:
	FriendsSend_PeerPlayer()

	\par Errors:
	ERRORFRIENDS_PLAYERSTATUSCOREONLINE: The player can't receive peer messages.
	ERRORROUTER_CLIENTINCOMPATIBLE: The other player doesn't have the same client
			version as you.
	ERRORFRIENDS_PLAYERNOTONLINE: The player is not online.
    ERRORFRIENDS_PLAYERIGNORE: The player has ignored the sender

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szUsername	Username of the receiver of the message

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_PeerPlayer)(GSubyte ubType,
		GSint iReason, GSchar* szUsername);

//============================================================================
// Callback CBFriendsRcv_ChangeFriend
/*!
 \brief	 Receive the status of change friend request
 \par       Description:
  This callback will be called when the client receives response from the
	router after sending a change friend request.
	
  \par Related Function:
	FriendsSend_StatusChange()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_ChangeFriend)(GSubyte ubType,
		GSint iReason);

//============================================================================
// Callback CBFriendsRcv_StatusChange
/*!
 \brief	 Receive the status of a change status request
 \par       Description:
  This callback will be called when the client receives response from the router
  after the player as changed his status
	
  \par Related Function:
	FriendsSend_StatusChange()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_StatusChange)(GSubyte ubType,
	GSint iReason);

//============================================================================
// Callback CBFriendsRcv_UpdateFriend
/*!
 \brief	 Receive information about a friend
 \par       Description:
  This callback will be called when the client receives information about 
  a friend that is in his friend list. This will be called on friend service
  loging and each time a friend is added to the friend list.
	
  \par Related Function:
	FriendsSend_ChangeFriend()
  FriendsSend_Login()

 \param	szUsername	The username of the friend
 \param	iStatus	Current status of the friend
 \param	szGroup	Group of the friend
 \param	iMood	Current mood of the friend
 \param	iOptions	Friend options
 \param szGameName The GameName when the status is PLAYERINLOBBY,
 		PLAYERINROOMOPEN,PLAYERINROOMCLOSE can be up to GAMELENGTH*4 in size.

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_UpdateFriend)(GSchar* szUsername,
		GSint iStatus, GSchar* szGroup, GSint iMood, GSint iOptions,
		GSchar *szGameName);

//============================================================================
// Callback CBFriendsRcv_SearchPlayer
/*!
 \brief	 Receive the status of search player request
 \par       Description:
  This callback will be called when the client receives response from the router
  after calling for a search of all the player that match a pattern. For each
  player found, this will be called.
	
  \par Related Function:
	FriendsSend_SearchPlayer()
	
	\par Errors:
	ERRORFRIENDS_FRIENDNOTEXIST: No more players found.

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szUsername	Username of the player that was found
 \param	iStatus	Status of the player that was found
 \param szGameName The GameName when the status is PLAYERINLOBBY,
 		PLAYERINROOMOPEN,PLAYERINROOMCLOSE can be up to GAMELENGTH*4 in size

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_SearchPlayer)(GSubyte ubType,
		GSint iReason,GSchar* szUsername, GSint iStatus, GSchar *szGameName);

//============================================================================
// Callback CBFriendsRcv_ScoreCard
/*!
 \brief	 Receive score card information for a player
 \par       Description:
	DEPRECATED: This callback has been replaced by the Ladder Query Service
	\par Related Function:
	FriendsSend_GetPlayerScores()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason	The reason of failure if ubType is GSFAIL
 \param	szUsername	the username of the player
 \param	szGame	Game name (ugly)
 \param	szScore	Scoree of the player

*/
//============================================================================
typedef GSvoid (__stdcall *CBFriendsRcv_ScoreCard)(GSubyte ubType,
		GSint iReason,GSchar* szUsername,GSchar* szGame, GSchar* szScore);

/*! @} end of group_FriendCB */



#endif //_GSFRIENDSCB_H_
