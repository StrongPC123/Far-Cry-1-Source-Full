
//****************************************************************************
//*   Author:  Guillaume Plante  <gsdevteam@ubisoft.com>
//*   Date:	  2001-09-20
 /*!  \file   GSLobbyCB.h
  *   \brief  Callback functions for the <b><i>lobby service</i></b>.
  *
  *   This file contains all callback functions declaration for the lobby
  *   service.
  */
//****************************************************************************

#ifndef _GSLOBBYCB_H_
#define _GSLOBBYCB_H_

#include "GSTypes.h"
#include "LadderDefines.h"

#ifdef __cplusplus
class clLobbyCallbacks
{
	public:

		virtual GSvoid LobbyRcv_LoginReply(GSubyte ubType, GSint iReason) = 0;
		virtual GSvoid LobbyRcv_LobbyDisconnection(GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_LobbyDisconnectAll() = 0;
		virtual GSvoid LobbyRcv_CreateRoomReply(GSubyte ubType, GSint iReason,
				GSchar *szRoom, GSint iGroupID, GSint iLobbySrvID)=0;
		virtual GSvoid LobbyRcv_JoinLobbyReply(GSubyte ubType, GSint iReason,
				GSchar *szReason, GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_JoinRoomReply(GSubyte ubType, GSint iReason,
				GSchar *szReason, GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_NewRoom(GSshort siGroupType, GSchar *szGroupName,
				GSint iGroupID, GSint iLobbySrvID, GSint iParentID,GSint iConfig,
				GSshort siGroupLevel,GSchar *szMaster, GSchar *szAllowedGames,
				GSchar *szGames, GSchar *szGameVersion, GSchar *szGSVersion,
				GSvoid *vpInfo, GSint iSize,GSuint uiMaxPlayer, GSuint uiNbrPlayer,
				GSuint uiMaxVisitor, GSuint uiNbrVisitor, GSchar *szIPAddress,
				GSchar *szAltIPAddress, GSint iEventID) = 0;
		virtual GSvoid LobbyRcv_NewLobby(GSshort siGroupType, GSchar *szGroupName,
				GSint iGroupID, GSint iLobbySrvID, GSint iParentID, GSint iConfig,
				GSshort sGroupLevel, GSchar *szMaster, GSchar *szAllowedGames,
				GSchar *szGames, GSvoid *vpInfo, GSint iSize, GSuint uiMaxMember,
				GSuint uiNbrMember, GSint iEventID) = 0;
		virtual GSvoid LobbyRcv_RoomInfo(GSshort siGroupType, GSchar *szGroupName,
				GSint iGroupID, GSint iLobbySrvID, GSint iParentID, GSint iConfig,
				GSshort sGroupLevel, GSchar *szMaster, GSchar *szAllowedGames,
				GSchar *szGames, GSchar *szGameVersion, GSchar *szGSVersion,
				GSvoid *vpInfo, GSint iSize, GSuint uiMaxPlayer, GSuint uiNbrPlayer,
				GSuint uiMaxVisitor, GSuint uiNbrVisitor, GSchar *szIPAddress,
				GSchar *szAltIPAddress, GSint iEventID) = 0;
		virtual GSvoid LobbyRcv_LobbyInfo(GSshort siGroupType, GSchar *szGroupName,
				GSint iGroupID, GSint iLobbySrvID, GSint iParentID, GSint iConfig,
				GSshort sGroupLevel, GSchar *szMaster, GSchar *szAllowedGames,
				GSchar *szGames, GSvoid *vpInfo, GSint iSize, GSuint uiMaxMember,
				GSuint uiNbrMember, GSint iEventID) = 0;

		virtual GSvoid LobbyRcv_GroupInfoGet(GSubyte ubType, GSint iLobbyID,
				GSint iRoomID)=0;

		virtual GSvoid LobbyRcv_GroupRemove(GSint iGroupID, GSint iLobbySrvID) = 0;

		virtual GSvoid LobbyRcv_MemberJoined(GSchar *szUsername, GSbool bVisitor,
				GSint* piGroupID, GSushort usNbGroup, GSint iLobbySrvID,
				GSchar *szIPAddress, GSchar *szAltIPAddress, GSushort usPing,
				GSvoid *vpPlayerData, GSint iDataSize, GSbool bJoin,
				GSushort usPlayerStatus ) = 0;
		virtual GSvoid LobbyRcv_MemberLeave(GSchar *szUsername, GSint iGroupID,
				GSint iLobbySrvID) = 0;

		virtual GSvoid LobbyRcv_StartMatchReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;

		virtual GSvoid LobbyRcv_MasterNewReply(GSubyte ubType, GSint iReason,
				GSchar *szUsername, GSint iGroupID, GSint iLobbyServerID) = 0;
		virtual GSvoid LobbyRcv_MasterChanged(GSint iGroupID, GSint iLobbySrvID,
				GSchar *szUsername, GSchar *szIPAddress, GSchar *szAltIPAddress) = 0;

		virtual GSvoid LobbyRcv_MatchFinishReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;

		virtual GSvoid LobbyRcv_KickOut(GSint iGroupID, GSint iLobbySrvID,
				GSchar *szReason) = 0;
	 	virtual GSvoid LobbyRcv_PlayerKickReply(GSubyte ubType, GSint iReason,
				GSchar *szUsername, GSint iGroupID, GSint iLobbyServerID) = 0;
		virtual GSvoid LobbyRcv_ParentGroupIDReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID, GSint iParentGroupID) = 0;
		virtual GSvoid LobbyRcv_GetAlternateGroupInfoReply( GSubyte ubType,
				GSint iReason, const GSvoid* pcAltGroupInfo,
				GSint iAltGroupInfoSize, GSint iGroupID,  GSint iLobbyServerID ) = 0;
		virtual GSvoid LobbyRcv_GroupLeaveReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_GroupConfigUpdate(GSint iGroupID, GSint iLobbySrvID,
				GSint iFlags) = 0;
		virtual GSvoid LobbyRcv_MatchStarted(GSint iGroupID, GSint iLobbyServerID,
				GSuint uiMatchID) = 0;
		virtual GSvoid LobbyRcv_GroupConfigUpdateReply(GSubyte ubType,
				GSint iReason, GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_SubmitMatchResultReply(GSubyte ubType,
				GSint iReason, GSint iMatchID) = 0;
		virtual GSvoid LobbyRcv_UpdatePing(GSint iGroupID, GSint iLobbySrvID,
				GSchar *szUsername, GSushort usPing) = 0;

		virtual GSvoid LobbyRcv_StartGameReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_GameReadyReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_GameStarted(GSint iGroupID, GSint iLobbyServerID,
				GSvoid *vpGameData, GSint iSize, GSchar *szIPAddress,
				GSchar *szAltIPAddress, GSushort usPort) = 0;
		virtual GSvoid LobbyRcv_NewGameMember(GSint iGroupID, GSint iLobbyServerID,
				GSchar *szUsername, GSbool bVisitor) = 0;

		virtual GSvoid LobbyRcv_UpdateGameInfoReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_PlayerBanReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID, GSchar *szUsername) = 0;
		virtual GSvoid LobbyRcv_PlayerUnBanReply(GSubyte ubType, GSint iReason,
				GSint iGroupID, GSint iLobbySrvID, GSchar *szUsername) = 0;
		virtual GSvoid LobbyRcv_PlayerBanList(GSint iGroupID, GSint iLobbySrvID,
				GSchar *szUsername) = 0;
		virtual GSvoid LobbyRcv_PlayerBanned(GSint iGroupID, GSint iLobbySrvID,
				GSchar *szReason) = 0;
		virtual GSvoid LobbyRcv_MatchReady(GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_InfoRefresh(GSint iLobbySrvID) = 0;

		virtual GSvoid LobbyRcv_SetPlayerInfoReply(GSubyte ubType,
				GSint iReason) = 0;
		virtual GSvoid LobbyRcv_PlayerInfoUpdate(GSchar *szUsername,
				GSvoid *vpPlayerData, GSint iPlayerDataSize) = 0;
		virtual GSvoid LobbyRcv_PlayerGroupList(GSchar *szUsername,
				GSint iGroupID, GSint iLobbySrvID) = 0;
		virtual GSvoid LobbyRcv_PlayerUpdateStatus( GSchar* szMember,
				GSushort usPlayerStatus ) = 0;
		virtual GSvoid LobbyRcv_FinalMatchResults(GSuint uiMatchId, GSubyte ubType,
				GSint iReason, const LADDER_ROW *pResults, GSuint uiNumResults) = 0;
};

#endif //__cplusplus

/*! @addtogroup group_LobbyCB
    @{
*/

//============================================================================
// Callback CBLobbyRcv_LoginReply
/*!
 \brief	 Receive status of the login request
 \par    Description:
  This callback will be called when the client receive a response from the router
  after asking to join the lobby server
  \par Related Function:
	LobbySend_Login()

 \param	ubType	The status of the message received back (GSSUCCESS or GSFAIL)
 \param	lReason	The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_LoginReply)(GSubyte ubType,
		GSint iReason);

//============================================================================
// Callback CBLobbyRcv_LobbyDisconnection
/*!
 \brief	 Client as been disconnected from lobby server
 \par    Description:
  This callback will be called when the client has been disconnected from
  a specific lobby server
  \par Related Function:
	LobbySend_Disconnection()

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_LobbyDisconnection)(GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_LobbyDisconnectAll
/*!
 \brief	 Client as been disconnected from lobby server
 \par    Description:
  This callback will be called when the client has been disconnected from all
  the available lobby server
  \par Related Function:
	LobbySend_DisconnectAll()

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_LobbyDisconnectAll)();

//============================================================================
// Callback CBLobbyRcv_CreateRoomReply
/*!
 \brief	 Receive status of the create room request
 \par    Description:
  This callback will be called when the client receive a response from
  the lobby server after asking to reate a new room
  \par Related Function:
	LobbySend_CreateRoom()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The Parent group doesn't exist.<br>
	ERRORLOBBYSRV_NOTINGROUP: The player isn't in the parent group.<br>
	ERRORLOBBYSRV_INVALIDGROUPNAME: The room name isn't valid.<br>
	ERRORLOBBYSRV_GROUPALREADYEXIST: The room already exists.<br>
	ERRORLOBBYSRV_GAMENOTALLOWED: The gamename is not allowed in the parent
			lobby.<br>
	ERRORLOBBYSRV_NOMOREPLAYERS: The usMaxPlayers was to big.<br>
	ERRORLOBBYSRV_SPECTATORNOTALLOWED: Spectators are not allowed.<br>
	ERRORLOBBYSRV_NOMORESPECTATORS: The usMaxVisitors was to big.<br>
	ERRORLOBBYSRV_WRONGGROUPTYPE: The usRoomType was the wrong type.<br>


 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param szRoom		The name of the newly created room
 \param iGroupID	The group id of the newly created room
 \param iLobbySrvID	The id of the server on which the specified group is
 located


*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_CreateRoomReply)(GSubyte ubType,
		GSint iReason, GSchar* szRoom, GSint iGroupID, GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_NewRoom
/*!
 \brief	 Receive a message informing of a newly created room
 \par    Description:
  This callback will be called when the client receive a message from the
  lobby server informing of a newly created room.
  \par Related Function:
	LoginSend_CreateRoom()

 \param	sGroupType		The type of room. (ROOM_DIRECTPLAY, ROOM_GAMEMODULE,
						ROOM_P2P, ROOM_CLIENTHOST)
 \param	szGroupName	The name of the room
 \param iGroupID		The id of the room.
 \param iLobbySrvID		The id of the server on which the specified room is
 located
 \param iParentID		The id of the parent group.
 \param	iConfig			The room configuration flag
 \param	szMaster		The name of the master of the room.
 \param szAllowedGames	The games allowed in this room.
 \param szGames		The games that can be played in the room.
 \param szGameVersion	The version of the game (information only)
 \param szGSVersion	The version of the gs-game (important, this correct version
                        must be used to join the room)
 \param vpInfo			A pointer to the game data
 \param iSize			The size of the game data structure
 \param	usMaxPlayer		The maximum number of players allowed in that room
 \param usNbrPlayer		The number of players currently in that room
 \param usMaxVisitor	The maximum number of visitors allowed in that room
 \param usNbrVisitor	The number of visitors currently in that room
 \param szIPAddress		The ip address of the host (master) of the room
 \param szAltIPAddress	The alternate ip address of the host (master) of the room
 \param usNbrMember		The event id for that room

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_NewRoom)(GSshort sGroupType,
		GSchar *szGroupName, GSint iGroupID, GSint iLobbySrvID, GSint iParentID,
		GSint iConfig, GSshort sGroupLevel, GSchar *szMaster,
		GSchar *szAllowedGames, GSchar *szGames, GSchar *szGameVersion,
		GSchar *szGSVersion, GSvoid *vpInfo, GSint iSize, GSuint uiMaxPlayer,
		GSuint uiNbrPlayer, GSuint uiMaxVisitor, GSuint uiNbrVisitor,
		GSchar *szIPAddress, GSchar *szAltIPAddress, GSint iEventID);

//============================================================================
// Callback CBLobbyRcv_NewLobby
/*!
 \brief	 Receive a message informing of a newly created lobby
 \par    Description:
  This callback will be called when the client receive a message from the
  lobby server informing of a newly created lobby.
  \par Related Function:
	LoginSend_CreateLobby()

 \param	sGroupType		The type of lobby.
 \param	szGroupName	The name of the lobby.
 \param iGroupID		The id of the lobby.
 \param iLobbySrvID		The id of the server on which the specified lobby is located
 \param iParentID		The id of the parent group.
 \param	iConfig			The lobby configuration flag.
 \param	szMaster		The name of the master of the lobby.
 \param szAllowedGames	The games allowed in this lobby and in the child groups.
 \param szGames		The games that can be played in the lobby.
 \param vpInfo			A pointer to the game data.
 \param iSize			The size of the game data structure.
 \param	usMaxMember		The maximum number of members allowed in that lobby.
 \param usNbrMember		The number of member currently in that lobby.
 \param iEventID		The event id for that lobby and its childs groups

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_NewLobby)(GSshort sGroupType,
		GSchar *szGroupName, GSint iGroupID, GSint iLobbySrvID, GSint iParentID,
		GSint iConfig, GSshort siGroupLevel, GSchar *szMaster,
		GSchar *szAllowedGames, GSchar *szGames, GSvoid *vpInfo, GSint iSize,
		GSuint uiMaxMember, GSuint uiNbrMember, GSint iEventID);



//============================================================================
// Callback CBLobbyRcv_RoomInfo
/*!
 \brief	 Receive a message about updated room infomations.
 \par    Description:
  This callback will be called when the client receive a message from the
  lobby server updating room information.
  \par Related Function:
	LoginSend_CreateRoom()

 \param	siGroupType		The type of room. (ROOM_DIRECTPLAY, ROOM_GAMEMODULE,
						ROOM_P2P, ROOM_CLIENTHOST)
 \param	szGroupName	The name of the room
 \param iGroupID		The id of the room.
 \param iLobbySrvID		The id of the server on which the specified room is
 located
 \param iParentID		The id of the parent group.
 \param	iConfig			The room configuration flag
 \param	szMaster		The name of the master of the room.
 \param szAllowedGames	The games allowed in this room.
 \param szGames		The games that can be played in the room
 \param szGameVersion	The version of the game (information only)
 \param szGSVersion	The version of the gs-game (important, this correct version
		must be used to join the room)
 \param vpInfo			A pointer to the game data
 \param iSize			The size of the game data structure
 \param	usMaxPlayer		The maximum number of players allowed in that room
 \param usNbrPlayer		The number of players currently in that room
 \param usMaxVisitor	The maximum number of visitors allowed in that room
 \param usNbrVisitor	The number of visitors currently in that room
 \param szIPAddress		The ip address of the host (master) of the room
 \param szAltIPAddress	The alternate ip address of the host (master) of the
 room
 \param usNbrMember		The event id for that room
*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_RoomInfo)(GSshort sGroupType,
		GSchar *szGroupName, GSint iGroupID, GSint iLobbySrvID, GSint iParentID,
		GSint iConfig, GSshort sGroupLevel, GSchar *szMaster,
		GSchar *szAllowedGames,GSchar *szGames, GSchar *szGameVersion,
		GSchar *szGSVersion, GSvoid *vpInfo, GSint iSize, GSuint uiMaxPlayer,
		GSuint uiNbrPlayer, GSuint uiMaxVisitor, GSuint uiNbrVisitor,
		GSchar *szIPAddress, GSchar *szAltIPAddress, GSint iEventID);



//============================================================================
// Callback CBLobbyRcv_LobbyInfo
/*!
 \brief	 Receive a message about updated room infomations.
 \par    Description:
  This callback will be called when the client receive a message from the
  lobby server updating lobby information.
  \par Related Function:
	LoginSend_CreateLobby()

 \param	sGroupType		The type of lobby.
 \param	szGroupName	The name of the lobby.
 \param iGroupID		The id of the lobby.
 \param iLobbySrvID		The id of the server on which the specified lobby is
 located
 \param iParentID		The id of the parent group.
 \param	iConfig			The lobby configuration flag.
 \param	szMaster		The name of the master of the lobby.
 \param szAllowedGames	The games allowed in this lobby and in the child groups.
 \param szGames		The games that can be played in the lobby.
 \param vpInfo			A pointer to the game data.
 \param iSize			The size of the game data structure.
 \param	uiMaxMember		The maximum number of members allowed in that lobby.
 \param uiNbrMember		The number of member currently in that lobby.
 \param iEventID		The event id for that lobby and its childs groups

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_LobbyInfo)(GSshort siGroupType,
		GSchar *szGroupName, GSint iGroupID, GSint iLobbySrvID, GSint iParentID,
		GSint iConfig, GSshort siGroupLevel, GSchar *szMaster,
		GSchar *szAllowedGames, GSchar *szGames, GSvoid *vpInfo, GSint iSize,
		GSuint uiMaxMember, GSuint uiNbrMember, GSint iEventID);


//============================================================================
// Callback CBLobbyRcv_GroupInfoGet
/*!
 \brief	 Received when a call to LobbySend_GroupInfoGet fails
 \par    Description:
  This callback will be called when the client tried to get the group info
	on a group that doesn't exist.
  \par Related Function:
	LobbySend_GroupInfoGet()

 \param	ubType	Always GSFAIL
 \param iLobbyID	The id of the server on which the specified group is located
 \param iRoomID	The id of the Grou

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GroupInfoGet)(GSubyte ubType,
		GSint iLobbyID,GSint iRoomID);

//============================================================================
// Callback CBLobbyRcv_GroupRemove
/*!
 \brief	 Receive a indication that a group as been removed
 \par    Description:
  This callback will be called when the client receive a message indicating
  that a group has been removed
  \par Related Function:
	LoginSend_JoinRoom()
	LoginSend_JoinLobby()

 \param	iGroupID	The id of the group.
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GroupRemove)(GSint iGroupID,
		GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_JoinRoomReply

/*!
 \brief	 Receive status of the join room request
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after asking to join a room.
  \par Related Function:
	LoginSend_JoinRoom()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The lobby doesn't exist.<br>
	ERRORLOBBYSRV_NOMOREMEMBERS: The room is full.<br>
	ERRORLOBBYSRV_PASSWORDNOTCORRECT: The password is not correct.<br>
	ERRORLOBBYSRV_ALREADYINGROUP: The player is already in the room.<br>
	ERRORLOBBYSRV_MEMBERBANNED: The player is banned from the room.<br>
	ERRORLOBBYSRV_GROUPCLOSE: The room is closed.<br>
	ERRORLOBBYSRV_GAMEINPROGRESS: The game has already started.
	ERRORLOBBYSRV_NOMORESPECTATORS: The maximum number of visitors has been
			reached.<br>
	ERRORLOBBYSRV_NOMOREPLAYERS: The maximum number of players has been
			reached.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	szReason	The reason the player was given if he was banned of that group
 \param	iGroupID	The id of the room the client has (or tried to) joined.
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_JoinRoomReply)(GSubyte ubType,
		GSint iReason, GSchar *szReason, GSint iGroupID, GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_JoinLobbyResult

/*!
 \brief	 Receive status of the join lobby request
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after asking to join a lobby.
  \par Related Function:
	LoginSend_JoinLobby()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The lobby doesn't exist.<br>
	ERRORLOBBYSRV_NOMOREMEMBERS: The lobby is full.<br>
	ERRORLOBBYSRV_PASSWORDNOTCORRECT: The password is not correct.<br>
	ERRORLOBBYSRV_ALREADYINGROUP: The player is already in the lobby.<br>
	ERRORLOBBYSRV_MEMBERBANNED: The player is banned from the lobby.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	szReason	The reason the player was given if he was banned of that group
 \param	iGroupID	The id of the lobby the client has (or tried to) joined.
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_JoinLobbyReply)(GSubyte ubType,
		GSint iReason, GSchar *szReason, GSint iGroupID, GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_MemberJoined

/*!
 \brief	 Receive a message informaing of a new member
 \par    Description:
  This callback will be called when the client receive a
  message from the lobby server informing of a new member in the group

 \param	szUsername		The alias of the player that has joined the group
 \param	bVisitor	Vistor flag; if the member joined as a visitor, it is true.
 \param	piGroupID	The List of the group id were the player is.
 ( NB the variables bVisitor, usPlayerStatus, bJoin, usPing, vpPlayerData are
 related to the first group id of the list	)
 \param usNbGroups The Number of GroupId.
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	szIPAddress	The local ip address detected by the client itself
 \param szAltIPAddress	The local ip address detected by the client itself
 \param	usPing		The ping of the player, 0xFFFF is the value returned in case
			of error or if the ping is not available.
 \param vpPlayerData Pointer to the player specific data buffer.
 \param iPlayerDataSize Size of the player specific data buffer.
 \param bJoin This flag tells the client if the player has just joined the group
			(TRUE) or if he was already member of the group when the client joined
			(FALSE)
 \param usPlayerStatus The status of the player as describe in LobbyDefine.h

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MemberJoined)(GSchar* szUsername,
		GSbool bVisitor, GSint* piGroupID, GSushort usNbGroups, GSint iLobbySrvID,
		GSchar *szIPAddress, GSchar *szAltIPAddress, GSushort usPing,
		GSvoid *vpPlayerData, GSint iPlayerDataSize, GSbool bJoin,
		GSushort usPlayerStatus );

//============================================================================
// Callback CBLobbyRcv_MemberLeave

/*!
 \brief	 Receive a message informaing member leaving a group
 \par    Description:
  This callback will be called when the client receive a message from the lobby
	server informing that a member has left the group

 \param	szUsername		The alias of the player that has left the group
 \param	iGroupID	The id of the group the member has left
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MemberLeave)(GSchar *szUsername,
		GSint iGroupID, GSint iLobbySrvID);


//============================================================================
// Callback CBLobbyRcv_StartMatchReply

/*!
 \brief	 Receive status of the start match request
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the start match message
  \par Related Function:
	LoginSend_StartMatch()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master of the room.<br>
	ERRORLOBBYSRV_GAMENOTINITIATED: The game has not started.<br>
	ERRORLOBBYSRV_MATCHNOTFINISHED: The match has already been started.<br>
	ERRORLOBBYSRV_MATCHSCORESSUBMISSIONEVENTFAIL: The match could not be
			created.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the concerned group
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_StartMatchReply)(GSubyte ubType,
		GSint iReason, GSint  iGroupID, GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_MasterChanged

/*!
 \brief	 Receive a member join message
 \par    Description:
  This callback will be called when the client receive a message
  from the lobby server indicating that a new master has been
  nominated in the specified group.

  \par Related Function:
	LoginSend_MasterNew()

 \param	iGroupID	The id of the group where the new master is.
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	szUsername		The alias of the new master.
 \param szIPAddress	The IP address of the host
 \param szAltIPAddress	The alternate ip address of the new master, this will
			often be the internal network address of the player. If one can't connect
			to the first ip address, this on should be used.


*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MasterChanged)(GSint  iGroupID,
		GSint iLobbySrvID, GSchar *szUsername, GSchar *szIPAddress,
		GSchar *szAltIPAddress);

//============================================================================
// Callback CBLobbyRcv_MasterNewReply

/*!
 \brief	 Receive a member join message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the MasterNew message
  \par Related Function:
	LoginSend_MasterNew()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master of the room.<br>
	ERRORLOBBYSRV_MEMBERNOTFOUND: The new master isn't in the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	szUsername		The alias of the player that is now master of the
			specified group
 \param	iGroupID	The id of the concerned group
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MasterNewReply)(GSubyte ubType,
		GSint iReason, GSchar *szUsername, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_MatchFinishReply

/*!
 \brief	 Receive a member join message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the MatchFinish message
  \par Related Function:
	LoginSend_MatchFinish()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The group in which the match took place
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MatchFinishReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_PlayerKickReply

/*!
 \brief	 Receive a PlayerKick message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the PlayerKick message
  \par Related Function:
	LoginSend_PlayerKick()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master.<br>
	ERRORLOBBYSRV_MEMBERNOTFOUND: The kicked player isn't in the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	szUsername		Alias of the kicked player
 \param iGroupID	The id of the group the player has been kicked of
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerKickReply)(GSubyte ubType,
		GSint iReason, GSchar *szUsername, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_KickOut

/*!
 \brief	 Receive a KickOut message
 \par    Description:
  This callback will be called when the client receive a message from
  the lobby server telling him that he as been kicked out of a group.
  \par Related Function:
	LoginSend_PlayerKick()

 \param	iGroupID	The id of the group the player was kicked out from.
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	szUsername	The reason given by the group master for kicking the player.

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_KickOut)(GSint iGroupID,
		GSint iLobbySrvID, GSchar *szReason);

//============================================================================
// Callback CBLobbyRcv_ParentGroupIDReply

/*!
 \brief	 Receive a ParentGroupID message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the ParentGroupID message.

  \par Related Function:
	LoginSend_GetParentGroupID()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group that was queried
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	iParentGroupID	The id of the parent group.

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_ParentGroupIDReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbySrvID, GSint iParentGroupID);

//============================================================================
// Callback CBLobbyRcv_GetAlternateGroupInfoReply

/*!
 \brief	 Receive a ParentGroupID message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending the ParentGroupID message.
  \par Related Function:
	LoginSend_GetParentGroupID()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group that was queried
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param pcAltGroupInfo Pointer to the alternate group info buffer.
 \param iAltGroupInfoSize Size of the alternate group info buffer.
*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GetAlternateGroupInfoReply)(
		GSubyte ubType, GSint iReason, const GSvoid* pcAltGroupInfo,
		GSint iAltGroupInfoSize, GSint iGroupID,  GSint iLobbyServerID );

//============================================================================
// Callback CBLobbyRcv_GroupLeaveReply

/*!
 \brief	 Receive a GroupLeave message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending a request to leave a group.
  \par Related Function:
	LoginSend_LeaveGroup()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The Group doesn't exist.<br>
	ERRORLOBBYSRV_NOTINGROUP: The player is not in the group.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group the player left
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GroupLeaveReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbySrvID);


//============================================================================
// Callback CBLobbyRcv_GroupConfigUpdate

/*!
 \brief	 Receive a GroupConfigUpdate message
 \par    Description:
  This callback will be called when the client receive a message from
  the lobby server telling him that a group of which he is a member has
  been through configuration changes.

 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param iFlags		The new configuration flags for the specified group

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GroupConfigUpdate)(GSint iGroupID,
		GSint iLobbySrvID, GSint iFlags);


//============================================================================
// Callback CBLobbyRcv_MatchStarted

/*!
 \brief	 Receive a MatchStarted message
 \par    Description:
 This is sent after the lobby server receive a Match Start message from the
 master or the game server.  The Lobby Server tells eveyones in the Group than
 the Match has sarted and gives the Match Id used for the score submition.


 \param	iGroupID	The id of the group in which a game as started
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param uiMatchID	The unique id for the started match ( used for the score
 submission )

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MatchStarted)(GSint iGroupID,
		GSint iLobbyServerID, GSuint uiMatchID);

//============================================================================
// Callback CBLobbyRcv_UpdateRoomConfigReply

/*!
 \brief	 Receive a GroupConfigUpdateResult message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending a request to change group config flags.
  \par Related Function:
	LoginSend_UpdateGroupConfig()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player isn't the master of the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_UpdateRoomConfigReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_SubmitMatchResultReply

/*!
 \brief	 Receive a SubmitMatchResult message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after sending a request to submit the score results.

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_MATCHNOTEXIST: The match doesn't exist.<br>
	ERRORLOBBYSRV_MATCHALREADYFINISHEDFORYOU: The match was already finished.<br>
	ERRORLOBBYSRV_MATCHSCORESSUBMISSIONALREDYSENT: The player has already submited
			their scores.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iMatchID	The unique match id that was used to submit the scores.

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_SubmitMatchResultReply)(GSubyte ubType,
		GSint iReason, GSint iMatchID);

//============================================================================
// Callback CBLobbyRcv_UpdatePing

/*!
 \brief	 Receive a UpdatePing message
 \par    Description:
  This callback will be called when the client receive a message from the lobby
	server about a new player ping. This tell the client to refresh the ping value
	(ping between client and the host of the game server) of the specific player.

 \param	iGroupID	The id of the group which has been updated with result
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	szUsername		The alias of the concerned player
 \param	usPing		The new ping of the player

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_UpdatePing)(GSint iGroupID,
		GSint iLobbyServerID, GSchar *szUsername, GSushort usPing);



//============================================================================
// Callback CBLobbyRcv_StartGame

/*!
 \brief	 Receive a StartGame message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a Start Game message
  \par Related Function:
	LoginSend_StartGame()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master of the room.<br>
	ERRORLOBBYSRV_BEGINALREADYDONE: The game has already been started.<br>
	ERRORLOBBYSRV_GAMENOTFINISHED: The game has not finished.<br>
	ERRORLOBBYSRV_MINPLAYERSNOTREACH: The minimum number of players hasn't been
			reached.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_StartGameReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_GameReadyReply

/*!
 \brief	 Receive a GameReady message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a GameReady message
  \par Related Function:
	LoginSend_StartGame()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_GAMENOTINITIATED: The game has not been started.<br>
	ERRORLOBBYSRV_NOTMASTER: The player is not the master.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GameReadyReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_GameStarted

/*!
 \brief	 Receive a GameStarted message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a GameReady message
  \par Related Function:
	LoginSend_StartGame()

 \param	iGroupID	The id of the group in wich the game has started.
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	vpGameData	The id of the group in wich the game has started.
 \param iSize		The id of the server on which the specified group is located
 \param	szIPAddress	The ip address of the host.
 \param szAltIPAddress	The alternate ip address of the host if no connection
 is successful on the first that one should be used.
 \param usPort		The port to connect to on the host.


*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_GameStarted)(GSint iGroupID,
		GSint iLobbyServerID, GSvoid *vpGameData, GSint iSize, GSchar *szIPAddress,
		GSchar *szAltIPAddress, GSushort usPort);

//============================================================================
// Callback CBLobbyRcv_NewGameMember

/*!
 \brief	 Receive a NewGameMember message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a GameReady message
  \par Related Function:
	LoginSend_StartGame()

 \param	iGroupID	The group id in wich the specified player will play the game
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param	szUsername		The alias of the player
 \param	bVisitor	The visitor flag, true if the player is a visitor, false else.

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_NewGameMember)(GSint iGroupID,
		GSint iLobbyServerID, GSchar *szUsername, GSbool bVisitor);

//============================================================================
// Callback CBLobbyRcv_UpdateGameInfoReply

/*!
 \brief	 Receive a UpdateGameInfoReply message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a UpdateGameInfoReply message
  \par Related Function:
	LoginSend_UpdateGameInfo()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTINGROUP: The player isn't in the room.<br>
	ERRORLOBBYSRV_NOTMASTER: The player isn't the master of the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_UpdateGameInfoReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID);

//============================================================================
// Callback CBLobbyRcv_PlayerBanReply

/*!
 \brief	 Receive a PlayerBanReply message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a PlayerBan message
  \par Related Function:
	LoginSend_PlayerBan()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player isn't the master of the room.<br>
	ERRORLOBBYSRV_MEMBERNOTFOUND: The banned player isn't in the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param szUsername		The alias of the player that was banned

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerBanReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID, GSchar *szUsername);

//============================================================================
// Callback CBLobbyRcv_PlayerUnBanReply

/*!
 \brief	 Receive a PlayerUnBanReply message
 \par    Description:
  This callback will be called when the client receive a response from the
  lobby server after a PlayerUnBan message
  \par Related Function:
	LoginSend_PlayerUnBan()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>
	ERRORLOBBYSRV_NOTMASTER: The player isn't the master of the room.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param	iGroupID	The id of the group which has been updated
 \param iLobbySrvID	The id of the server on which the specified group is located
 \param szUsername		The alias of the player that was un-banned

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerUnBanReply)(GSubyte ubType,
		GSint iReason, GSint iGroupID, GSint iLobbyServerID, GSchar *szUsername);

//============================================================================
// Callback CBLobbyRcv_PlayerBanList

/*!
 \brief	 Receive a PlayerUnBanReply message
 \par    Description:
  This callback will be called for each player that was banned for a group,
  it is the response from the lobby server after having called
	LobbySendPlayerBanList()
  \par Related Function:
	LobbySend_PlayerBanList()

	\par Errors:
	ERRORLOBBYSRV_GROUPNOTEXIST: The group doesn't exist.<br>

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param szUsername		The alias of the player that was banned

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerBanList)(GSint iGroupID,
		GSint iLobbyServerID, GSchar *szUsername);

//============================================================================
// Callback CBLobbyRcv_PlayerBanned

/*!
 \brief	 Receive a PlayerBan message
 \par    Description:
  This callback will be called when a player gets banned from a group.
  \par Related Function:
	LobbySend_PlayerBan()

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL
 \param szReason	The reason the player was banned.

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerBanned)(GSint iGroupID,
		GSint iLobbySrvID, GSchar *szReason);

//============================================================================
// Callback CBLobbyRcv_MatchReady
/*!
 \brief	 Receive a MatchReady message
 \par    Description:


 \param	iGroupID	The id of the group where the match takes place
 \param iLobbySrvID	The id of the server on which the specified group is located

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_MatchReady)(GSint iGroupID,
		GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_InfoRefresh
/*!
 \brief	 Receive a InfoRefresh message telling that it would be good to refresh
         all group and player related information.
 \par    Description:

 \param iLobbySrvID	The id of the lobby server from wich we received the message

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_InfoRefresh)(GSint iLobbySrvID);

//============================================================================
// Callback CBLobbyRcv_SetPlayerInfoReply
/*!
 \brief	 Receive a reply after having sent a request to change player information
 \par    Description:

 \param	ubType		The status of the message received back (GSSUCCESS or GSFAIL)
 \param	iReason		The reason of failure if ubType is GSFAIL

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_SetPlayerInfoReply)(GSubyte ubType,
		GSint iReason);

//============================================================================
// Callback CBLobbyRcv_PlayerInfoUpdate

/*!
 \brief	 Receive a reply after having sent a request to change player information
 \par    Description:

 \param	szUsername The alias of the player who's personal data has changed
 \param	vpPlayerData Pointer to the player data buffer
 \param iPlayerDataSize The data buffer size

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerInfoUpdate)(GSchar *szUsername,
		GSvoid *vpPlayerData, GSint iPlayerDataSize);

//============================================================================
// Callback CBLobbyRcv_PlayerInfoUpdate

/*!
 \brief	 Receive an update of the player's status
 \par    Description:
  Receive an update of the player's status

 \param	szUsername The alias of the player who's personal status has changed
 \param usPlayerStatus The Player's status

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerStatusUpdate)( GSchar* szUsername,
		GSushort usPlayerStatus );

//============================================================================
// Callback CBLobbyRcv_PlayerGroupList

/*!
 \brief	 Receive a message telling that a player is member of a specific group
 \par    Description:
  Receive a message telling that a player is member of a specific group
 \param	szUsername The alias of the player
 \param iGroupID The group id
 \param iLobbySrvID	The id of the lobby server the group is hosted

*/
//============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_PlayerGroupList)(GSchar *szUsername,
		GSint iGroupID, GSint iLobbySrvID);

//===============================================================================
// Callback CBLobbyRcv_FinalMatchResults

/*!
\brief Official results of a match
\par   Description:
       This callback is called once the submitted match results have been
       submitted, validated, and stored in the ladder database. It contains
       the official results of the match or the error that occured during
       the submission process.

\param      uiMatchId   The match to who the results apply
\param      ubType      The type of notification - GSSUCCESS or GSFAIL
\param      iReason     The reason of a GSFAIL. The possible values are:
                        <UL>
                        <LI>ERRSS_BADFORMAT
                        <BR>Internal error. The messages between the parts of
                        the service where corrupted.
                        <LI>ERRSS_DBFAILURE
                        <BR>An error occured on the DB. The ubi.com server log
                        will contain the exact nature of the error.
                        <LI>ERRSS_SUBMISSIONFAILED
                        <BR>The results could not be submitted successfully.
                        The exact nature of the error will be logged in the
                        database along with the submitted values
                        <LI>ERRSS_VALIDATIONFAILED
                        <BR>The results could not be validated. The exact nature
                        of the error will be logged in the database along with
                        the submitted values.
                        </UL>
\param      pResults    An array of data rows containing the results values.
                        You do not have ownership of this array. The memory will
                        be freed when the callback function returns.
\param      uiNumResult The number of rows in the result array
*/
//===============================================================================
typedef GSvoid (__stdcall *CBLobbyRcv_FinalMatchResults)(GSuint uiMatchId,
		GSubyte ubType, GSint iReason, const LADDER_ROW *pResults,
		GSuint uiNumResult);

/*! @} end of group_LobbyCB */

#endif //_GSLOBBYCB_H_
