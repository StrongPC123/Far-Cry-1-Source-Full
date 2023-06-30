#ifndef _MSCLIENTCLASS_H_
#define _MSCLIENTCLASS_H_

#include "GSTypes.h"
#include "GSClientClass.h"
//#include "GSLobbyCB.h"
//#include "GSLoginCB.h"
#include "define.h"

class LobbyInfo;
class GameServerMap;
class LobbyInfoList;

#define NOSTATE 0
#define LOGIN_MASK (1L<<0)
#define LOBBY_MASK (1L<<1)
#define JOIN_MASK (1L<<2)
#define REFRESH_MASK (1L<<3)
#define ALTINFO_MASK (1L<<4)

class clMSClientClass : private clGLClient
{
	public:
		clMSClientClass();
		~clMSClientClass();

		GSbool Initialize(const GSchar *szMasterServerIP,
				GSushort usMasterServerPort,const GSchar *szUserName,
				const GSchar *szPassword, const GSchar *szVersion);
		GSbool Uninitialize();
		GSbool RequestGameServers(const GSchar *szGameName);
		GSbool RefreshGameServer(GSint iLobbyID,GSint iRoomID);
		GSbool RequestAlternateInfo(GSint iLobbyID,GSint iRoomID);
		GSbool JoinGameServer(GSint iLobbyID,GSint iRoomID,const GSchar *szPassword,
				const GSchar *szVersion, const GSchar *szGameName,
				const GSvoid *pvPlayerInfo, GSint iPlayerInfoSize);
		GSbool LeaveGameServer(GSint iLobbyID,GSint iRoomID);
		GSbool GameServerConnected(GSint iLobbyID,GSint iRoomID);
		
		GSbool CreateAccount(const GSchar *szMasterServerIP,
				GSushort usMasterServerPort,const GSchar* szVersion,
				const GSchar* szNickName,const GSchar* szPassword,
				const GSchar* szFirstName,const GSchar* szLastName,
				const GSchar* szEmail,const GSchar* szCountry);
		GSbool ModifyAccount(const GSchar* szPassword, const GSchar* szFirstName,
				const GSchar* szLastName, const GSchar* szEmail,
				const GSchar* szCountry);

		GSbool InitMatchResult(GSuint uiMatchID);
		GSbool SetMatchResult(GSchar* szAlias,GSuint uiFieldID,GSint iFieldValue);
		GSbool SubmitMatchResult(GSint iLobbyID,GSint iRoomID);
		GSbool UninitMatchResult();
		GSbool MatchStarted(GSint iLobbyID,GSint iRoomID);
		GSbool MatchFinished(GSint iLobbyID,GSint iRoomID);

		GSbool RequestMOTD(const GSchar* szLanguage);

		GSbool Engine(GSuint uiMaxPostingDelay = 500,
				GSuint uiMaxsOperationalDelay = 800);

		//Callbacks
		virtual GSvoid GameServerCB(GSint iLobbyID,GSint iRoomID,GSshort siGroupType,
				GSchar *szGroupName,GSint iConfig,GSchar *szMaster,GSchar *szAllowedGames,
				GSchar *szGames,GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,
				GSint iSize,GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
				GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
				GSint iEventID) = 0;
		virtual GSvoid ErrorCB(GSint iReason,GSint iLobbyID,GSint iRoomID) = 0;
		virtual GSvoid InitFinishedCB(GSubyte ucType,GSint iError,GSchar *szUserName) = 0;
		virtual GSvoid LoginDisconnectCB() = 0;
		virtual GSvoid LobbyDisconnectCB() = 0;
		virtual GSvoid RequestFinishedCB() = 0;
		virtual GSvoid JoinFinishedCB(GSint iLobbyID,GSint iRoomID,
				GSvoid *vpGameData,GSint iSize,GSchar *szIPAddress,
				GSchar *szAltIPAddress,GSushort usPort) = 0;
		virtual GSvoid AlternateInfoCB(GSint iLobbyID,GSint iRoomID,
				const GSvoid* pcAltGroupInfo,GSint iAltGroupInfoSize) = 0;

		
		virtual GSvoid AccountCreationCB(GSubyte ucType, GSint iReason) = 0;
		virtual GSvoid ModifyAccountCB(GSubyte ucType, GSint iReason) = 0;

		virtual GSvoid MatchStartedCB(GSint iLobbyID,GSint iRoomID,GSuint uiMatchID) = 0;
		virtual GSvoid SubmitMatchCB(GSubyte ucType,GSint iReason,
				GSuint uiMatchID) = 0;

		virtual GSvoid RequestMOTDCB(GSubyte ubType, GSchar *szUbiMOTD,
			GSchar *szGameMOTD, GSint iReason)=0;

	//Ignore the rest of these functions
	public:
		GSvoid LobbyRcv_LoginReply(GSubyte ucType, GSint iReason);
		GSvoid LobbyRcv_LobbyDisconnection(GSint iLobbySrvID);
		GSvoid LobbyRcv_LobbyDisconnectAll();
		GSvoid LobbyRcv_CreateRoomReply(GSubyte ucType, GSint iReason,
				GSchar *pszRoom, GSint iGroupID,GSint iLobbySrvID){};
		GSvoid LobbyRcv_JoinLobbyReply(GSubyte ucType, GSint iReason,
				GSchar *szReason,GSint iGroupID,GSint iLobbySrvID);
		GSvoid LobbyRcv_JoinRoomReply(GSubyte ucType, GSint iReason,
				GSchar *szReason,GSint iGroupID,GSint iLobbySrvID);
		GSvoid LobbyRcv_NewRoom(GSshort siGroupType,GSchar *pszGroupName,
						GSint iGroupID,GSint iLobbySrvID,GSint iParentID,
						GSint iConfig, GSshort siGroupLevel,
						GSchar *pszMaster, GSchar *pszAllowedGames,
						GSchar *pszGames,GSchar *pszGameVersion,GSchar *pszGSVersion,
						GSvoid *vpInfo,GSint iSize,
						GSuint usMaxPlayer, GSuint usNbrPlayer,
						GSuint usMaxVisitor, GSuint usNbrVisitor,
						GSchar *szIPAddress,GSchar *szAltIPAddress,GSint iEventID);
		GSvoid LobbyRcv_NewLobby(GSshort siGroupType,GSchar *pszGroupName,
						GSint iGroupID,GSint iLobbySrvID,GSint iParentID,
						GSint iConfig, GSshort siGroupLevel,
						GSchar *pszMaster, GSchar *pszAllowedGames,
						GSchar *pszGames,GSvoid *vpInfo,GSint iSize,
						GSuint usMaxMember, GSuint usNbrMember,GSint iEventID);
		GSvoid LobbyRcv_RoomInfo(GSshort siGroupType,GSchar *pszGroupName,
						GSint iGroupID,GSint iLobbySrvID,GSint iParentID,
						GSint iConfig, GSshort siGroupLevel,
						GSchar *pszMaster, GSchar *pszAllowedGames,
						GSchar *pszGames,GSchar *pszGameVersion,GSchar *pszGSVersion,
						GSvoid *vpInfo,GSint iSize,
						GSuint usMaxPlayer, GSuint usNbrPlayer,
						GSuint usMaxVisitor, GSuint usNbrVisitor,
						GSchar *szIPAddress,GSchar *szAltIPAddress,GSint iEventID);
		GSvoid LobbyRcv_LobbyInfo(GSshort siGroupType,GSchar *pszGroupName,
						GSint iGroupID,GSint iLobbySrvID,GSint iParentID,
						GSint iConfig, GSshort siGroupLevel,
						GSchar *pszMaster, GSchar *pszAllowedGames,
						GSchar *pszGames,GSvoid *vpInfo,GSint iSize,
						GSuint usMaxMember, GSuint usNbrMember,GSint iEventID);
		GSvoid LobbyRcv_GroupInfoGet(GSubyte ucType, GSint iLobbyID,
				GSint iRoomID);

		GSvoid LobbyRcv_GroupRemove(GSint iGroupID,GSint iLobbySrvID){};

		GSvoid LobbyRcv_MemberJoined(GSchar *szAlias,GSbool bVisitor,
				GSint* iGroupID,GSushort usNbGroup,GSint iLobbySrvID,
				GSchar *szIPAddress,GSchar *szAltIPAddress,GSushort usPing,
				GSvoid *vpPlayerData,GSint iDataSize,GSbool bJoin,
				GSushort usPlayerStatus){};
		GSvoid LobbyRcv_MemberLeave(GSchar *szAlias,GSint iGroupID,
				GSint iLobbySrvID){};

		GSvoid LobbyRcv_StartMatchReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID){};

		GSvoid LobbyRcv_MasterNewReply(GSubyte ucType, GSint iReason,
				GSchar *szAlias,GSint iGroupID,GSint iLobbyServerID){};
		GSvoid LobbyRcv_MasterChanged(GSint iGroupID,GSint iLobbySrvID ,
				GSchar *szAlias,GSchar *szIPAddress,GSchar *szAltIPAddress){};

		GSvoid LobbyRcv_MatchFinishReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID){};

    GSvoid LobbyRcv_KickOut(GSint iGroupID,GSint iLobbySrvID,GSchar *szReason){};
	 	GSvoid LobbyRcv_PlayerKickReply(GSubyte ucType, GSint iReason,
				GSchar *szAlias,GSint iGroupID,GSint iLobbyServerID){};
		GSvoid LobbyRcv_ParentGroupIDReply(GSubyte ucType, GSint iReason,
			GSint iGroupID,GSint iLobbySrvID, GSint iParentGroupID){};
		GSvoid LobbyRcv_GroupLeaveReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID){};
		GSvoid LobbyRcv_GroupConfigUpdate(GSint iGroupID,GSint iLobbySrvID,
				GSint iFlags){};
		GSvoid LobbyRcv_MatchStarted(GSint iGroupID,GSint iLobbyServerID,
				GSuint uiMatchID);
		GSvoid LobbyRcv_GroupConfigUpdateReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID){};
		GSvoid LobbyRcv_SubmitMatchResultReply(GSubyte ucType, GSint iReason,
				GSint iMatchID);
		GSvoid LobbyRcv_UpdatePing(GSint iGroupID,GSint iLobbySrvID,GSchar *szAlias,
				GSushort usPing){};

		GSvoid LobbyRcv_StartGameReply(GSubyte ucType, GSint iReason,GSint iGroupID,
				GSint iLobbySrvID){};
		GSvoid LobbyRcv_GameReadyReply(GSubyte ucType, GSint iReason,GSint iGroupID,
				GSint iLobbySrvID){};
		GSvoid LobbyRcv_GameStarted(GSint iGroupID,GSint iLobbyServerID,
										   GSvoid *vpGameData,GSint iSize,GSchar *szIPAddress,
										   GSchar *szAltIPAddress,GSushort usPort);
		GSvoid LobbyRcv_NewGameMember(GSint iGroupID,GSint iLobbyServerID,
				GSchar *szAlias,GSbool bVisitor){};

		GSvoid LobbyRcv_UpdateGameInfoReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID){};
		GSvoid LobbyRcv_PlayerBanReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID,GSchar *szAlias){};
		GSvoid LobbyRcv_PlayerUnBanReply(GSubyte ucType, GSint iReason,
				GSint iGroupID,GSint iLobbySrvID,GSchar *szAlias){};
		GSvoid LobbyRcv_PlayerBanList(GSint iGroupID,GSint iLobbySrvID,
				GSchar *szAlias){};
		GSvoid LobbyRcv_PlayerBanned(GSint iGroupID,GSint iLobbySrvID,
				GSchar *szReason){};
		GSvoid LobbyRcv_MatchReady(GSint iGroupID,GSint iLobbySrvID){};
		GSvoid LobbyRcv_InfoRefresh(GSint iLobbySrvID){};

		GSvoid LobbyRcv_SetPlayerInfoReply(GSubyte ucType, GSint iReason){};
		GSvoid LobbyRcv_PlayerInfoUpdate(GSchar *szAlias,GSvoid *vpPlayerData,
				GSint iPlayerDataSize){};
		GSvoid LobbyRcv_PlayerGroupList(GSchar *szAlias,GSint iGroupID,
				GSint iLobbySrvID){};
		GSvoid LobbyRcv_PlayerUpdateStatus(GSchar* szMember,
				GSushort usPlayerStatus){};
		GSvoid LobbyRcv_FinalMatchResults(GSuint uiMatchId, GSubyte ucType, GSint iReason, const LADDER_ROW *pResults, GSuint uiNumResult){};
		
		GSvoid LobbyRcv_GetAlternateGroupInfoReply(GSubyte ucType, GSint iReason,
				const GSvoid* pcAltGroupInfo,GSint iAltGroupInfoSize, GSint iGroupID,
				GSint iLobbyServerID );

	public:

		GSvoid LoginRcv_PlayerInfo(GSubyte ucType, GSchar * pszNickName,
			GSchar * pszSurName, GSchar * pszFirstName, GSchar * pszCountry,
			GSchar * pszEmail, GSchar * szIRCID, GSchar * szIPAddress,GSint lReason );
		GSvoid LoginRcv_JoinWaitModuleResult(GSubyte ucType, GSchar * pszAddress,
				GSushort lPort, GSint lReason);
		GSvoid LoginRcv_LoginRouterResult(GSubyte ucType, GSint lReason);
		GSvoid LoginRcv_LoginWaitModuleResult(GSubyte ucType, GSint lReason);
		GSvoid LoginRcv_SystemPage(GSint lSubType, GSchar * pszText);
		GSvoid LoginRcv_LoginDisconnection();
		GSvoid LoginRcv_AccountCreationResult(GSubyte ucType, GSint lReason);
		GSvoid LoginRcv_ModifyUserResult(GSubyte ucType, GSint lReason);
		
		GSvoid LoginRcv_RequestMOTD(GSubyte ubType, GSchar *szUbiMOTD,GSchar *szGameMOTD, GSint iReason);
		
	public:
		GSvoid FriendsRcv_LoginResult(GSubyte ucType, GSint lReason){};
		GSvoid FriendsRcv_AddFriend(GSubyte ucType, GSint lReason,
				GSchar* pszFriend){};
		GSvoid FriendsRcv_DelFriend(GSubyte ucType, GSint lReason,
				GSchar* pszFriend){};
		GSvoid FriendsRcv_GetSession(GSubyte ucType, GSint lReason,
				GSchar* pszPlayer,GSchar* pszSession, GSint lGroupID, GSint lParentID,
				GSint lMaxPlayers, GSint lMaxVisitors,GSint lNbPlayers,
				GSint lNbVisitors,GSchar* szMaster, GSint lConfig, GSchar* szInfo,
				GSchar* szGame){};
		GSvoid FriendsRcv_GetWebBased(GSchar* pszPlayer,
				GSchar* pszWebBasedURL){};
		GSvoid FriendsRcv_Page(GSchar* pszAlias, GSchar* pszMessage,
				GSchar* pszTimeStamp){};
		GSvoid FriendsRcv_PagePlayer(GSubyte ucType, GSint lReason,
				GSchar* pszReceiver){};
		GSvoid FriendsRcv_PeerMsg(GSchar* pszAlias, GSvoid* p_Buffer,
				GSuint uiLength){};
		GSvoid FriendsRcv_PeerPlayer(GSubyte ucType, GSint lReason,
				GSchar* pszReceiver){};
		GSvoid FriendsRcv_ChangeFriend(GSubyte ucType, GSint lReason){};
		GSvoid FriendsRcv_StatusChange(GSubyte ucType, GSint lReason){};
		GSvoid FriendsRcv_UpdateFriend(GSchar* pszFriend, GSint lStatus,
				GSchar* pszGroup, GSint lMood, GSint lOptions, GSchar *szGameName){};
		GSvoid FriendsRcv_SearchPlayer(GSubyte ucType, GSint lReason,
				GSchar* pszAlias, GSint lStatus, GSchar *szGameName){};
		GSvoid FriendsRcv_ScoreCard(GSubyte ucType, GSint lReason,
				GSchar* pszPlayer,GSchar* pszGame, GSchar* pszScore){};
		GSvoid FriendsRcv_IgnorePlayer(GSRESULT rCode,const GSchar * szPlayer){};
		GSvoid FriendsRcv_UnignorePlayer(GSRESULT rCode,const GSchar * szPlayer){};
		GSvoid FriendsRcv_IgnoredPlayer( const GSchar * szPlayer ){};
		GSvoid FriendsRcv_ListIgnoredPlayers( GSRESULT rCode ){};
		
        public:
        GSvoid PSRcv_LoginResult(GSubyte, GSint) {}
        GSvoid PSRcv_Disconnection() {}
        GSvoid PSRcv_GetDataReply(GSubyte, GSint, GSuint, GSvoid *, GSint) {}
        GSvoid PSRcv_SetDataReply(GSubyte, GSint, GSuint) {}
        
		public:
		GSvoid LadderQueryRcv_RequestReply(GSubyte ucType, GSint  iReason,
				GSuint uiRequestId){};
		
	private:

		GSbool StartLogin();
		GSbool LobbyDisconnect();
		GSbool LobbyConnect();

		GSchar m_szUsername[NICKNAMELENGTH];
		GSchar m_szPassword[PASSWORDLENGTH];
		GSchar m_szVersion[VERSIONLENGTH];
		GSchar m_szRouterIP[IPADDRESSLENGTH];
		GSushort m_usPort;

		GSchar m_szGameType[GAMELENGTH];

		//GSint m_iState;
		GSsize_t m_uiTimeOut;

		LobbyInfo *m_pstTargetLobby;
		GSvoid *m_pvPlayerInfo;
		GSint m_iPlayerInfoSize;

		//LobbyInfoList *m_pstRequestedRefresh;
		//LobbyInfoList *m_pstRefreshList;

		//LobbyInfoList *m_pstRequestedAltInfo;
		//LobbyInfoList *m_pstAltInfoList;
		
		LobbyInfoList *m_pstBasicGroups;

		//GameServerMap *m_pstGameServers;
		//GSuint m_uiUniqueID;

		//GSbool m_bJoinLobbyOnce;
		//GSbool m_bJoinRoomOnce;
		
		GSbool m_bReqestingServers;
		//GSbool m_bRefreshingServers;
		GSbool m_bJoinedServer;
		GSbool m_bJoiningServer;
		
		GSbool m_bLobbyConnected;
};
#endif //_MSCLIENTCLASS_H_
