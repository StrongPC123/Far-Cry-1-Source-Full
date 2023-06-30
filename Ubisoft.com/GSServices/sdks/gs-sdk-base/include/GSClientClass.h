#ifndef _GSCLIENCLASS_H_
#define _GSCLIENCLASS_H_

#include "GSErrors.h"
#include "GSLoginCB.h"
#include "GSFriendsCB.h"
#include "GSLobbyCB.h"
#include "GSPersistentCB.h"
#include "GSLadderQueryCB.h"
#include "GSRemoteAlgorithmCB.h"
#include "LadderDefines.h"
#include "RemoteAlgorithmDefines.h"

class CGLClientLIB;

class clGLClient : public clLoginCallbacks, public clFriendsCallbacks,
                   public clLobbyCallbacks, public clPersistentCallbacks,
                   public clLadderQueryCallbacks
{
	public:
		clGLClient();
		virtual ~clGLClient();

		GSbool GL_AddRouterPing(GSint lID, GSchar *szIPAddress, GSint lPort);
		GSbool GL_GetFirstPing(GSint *plID);
		GSbool GL_ClearRouterPing();

		//Login Messages
		GSbool Login_Engine(GSuint uiMaxPostingDelay = 500, GSuint uiMaxsOperationalDelay = 800);
		GSbool LoginSend_Connect(const GSchar* szAddress, GSushort lPort);
		GSbool LoginSend_Disconnect();
		GSbool LoginSend_LoginRouter(const GSchar* szAlias, const GSchar* szPassword, 
			const GSchar* szVersion, GSbool bPublicIP = GS_TRUE);
		GSbool LoginSend_LoginWaitModule(const GSchar* szAlias);
		GSbool LoginSend_JoinWaitModule();
		GSbool LoginSend_PlayerInfo(const GSchar* szAlias);
		GSbool LoginSend_Sleep();
		GSbool LoginSend_WakeUp();
		GSbool LoginSend_ModifyAccount(const GSchar* szPassword, const GSchar* szFirstName,
								 const GSchar* szLastName, const GSchar* szEmail, const GSchar* szCountry);
		GSbool LoginSend_CreateAccount(const GSchar* szVersion, const GSchar* szNickName, const GSchar* szPassword, const GSchar* szFirstName,
								const GSchar* szLastName, const GSchar* szEmail, const GSchar* szCountry);
		GSbool LoginSend_RequestMOTD(const GSchar *szLanguage);

		//Friends Messages
		GSbool Friends_Engine(GSuint uiMaxPostingDelay = 500, GSuint uiMaxsOperationalDelay = 800);
		GSbool FriendsSend_Connect();
		GSbool FriendsSend_Disconnect();
		GSbool FriendsSend_Login(GSint lStatus = 0, GSint lMood = 0);
		GSbool FriendsSend_AddFriend(const GSchar* szFriend, const GSchar* szGroup, GSint lOptions);
		GSbool FriendsSend_DelFriend (const GSchar* szFriend);
		GSbool FriendsSend_FriendList();
        
        GSRESULT FriendsSend_IgnorePlayer( const GSchar * szPlayer );
        GSRESULT FriendsSend_UnignorePlayer( const GSchar * szPlayer);
        GSRESULT FriendsSend_ListIgnoredPlayers();
        
		GSbool FriendsSend_PagePlayer(const GSchar* pszAlias, const GSchar* szMessage);
		GSbool FriendsSend_PeerPlayer(const GSchar* pszAlias, GSvoid *p_Buffer, GSuint uiLength);
		GSbool FriendsSend_StatusChange(GSint lStatus, GSint lMood);
		GSbool FriendsSend_ChangeFriend(const GSchar* szFriend, const GSchar* szGroup, GSint lOptions);
		GSbool FriendsSend_SearchPlayer(const GSchar* pszAlias, const GSchar* pszSurName, const GSchar* pszFirstName, const GSchar* pszCountry, const GSchar* pszEmail, GSint lSex, const GSchar* pszGame);
		GSbool FriendsSend_GetPlayerScores(const GSchar* pszAlias);

		//Lobby messages
		GSbool Lobby_Engine(GSuint uiMaxPostingDelay = 500, GSuint uiMaxsOperationalDelay = 800);
		GSbool LobbySend_Connect();
		GSbool LobbySend_Login(const GSchar* szGames, GSbool bPublicIP = GS_TRUE, GSushort usPlayerStatus = 0 );
		GSbool LobbySend_Disconnect(GSint iLobbyServerID);
		GSbool LobbySend_DisconnectAll();

		GSbool LobbySend_CreateRoom(GSint iParentGroupID, GSint iLobbyServerID, const GSchar *szRoomName,
			const GSchar *szGames, GSushort usRoomType, GSushort usMaxPlayers, GSushort usMaxVisitors,
			const GSvoid *vpData, GSint iSize, const GSvoid* vpAltGroupInfo, GSint iAltGroupInfoSize,
			const GSchar *szPassword, const GSchar *szGameVersion, const GSchar *szGSVersion);

		GSbool LobbySend_JoinLobby(GSint iGroupID, GSint iLobbyServerID,
			const GSchar *szPassword, GSint iConfig = 0);
		GSbool LobbySend_JoinRoom(GSint iGroupID, GSint iLobbyServerID, const GSchar *szPassword,
			GSbool bVisitor, const GSchar *szVersion, GSint iConfig = 0);
		GSbool LobbySend_LeaveGroup(GSint iGroupID, GSint iLobbyServerID);

		GSbool LobbySend_GetGroupInfo(GSint iGroupID, GSint iLobbyServerID, GSint iconfig);
		GSbool LobbySend_GetAlternateGroupInfo(GSint iGroupID, GSint iLobbyServerID );

		GSbool LobbySend_InitMatchResults(GSuint uiMatchID);
		GSbool LobbySend_SetMatchResult(GSchar *szAlias, GSuint uiFieldID, GSint iFieldValue);
		GSbool LobbySend_SubmitMatchResult(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_ClearMatchResult();
		
		GSbool LobbySend_StartMatch(GSint iGroupID, GSint iLobbyServerID, GSuint uiMode = 0 );
        GSbool LobbySend_MatchFinish(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_PlayerMatchStarted(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_PlayerMatchFinished(GSint iGroupID, GSint iLobbyServerID);

		GSbool LobbySend_NewMaster(GSint iGroupID, GSint iLobbyServerID, GSchar *szAlias);
		GSbool LobbySend_PlayerKick(GSint iGroupID, GSint iLobbyServerID, GSchar *szAlias, GSchar *szReason);
		GSbool LobbySend_GetParentGroupID(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_UpdateRoomConfig(GSint iGroupID, GSint iLobbyServerID, GSbyte bDedicatedServer,
							GSbyte bOpen, GSbyte bScore_Submission, GSint iMaxPlayers,
							GSint iMaxVisitors, const GSchar* szPassword, const GSvoid* vpData, 
							GSint iSize, const GSvoid* vpAltGroupInfo, GSint iGroupInfoSize );

		GSbool LobbySend_GameStart(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_GameReady(GSint iGroupID, GSint iLobbyServerID, GSvoid* vpGameData, GSint iSize,
					GSushort usPort = 0, GSchar *szIPAddress = 0);
		GSbool LobbySend_GameFinish(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_GameConnected(GSint iGroupID, GSint iLobbyServerID);

		GSbool LobbySend_UpdateGameInfo(GSint iGroupID, GSint iLobbyServerID, GSvoid* vpGameData, GSint iSize,
									 GSushort usPort = 0, GSchar *szIPAddress = 0);
		GSbool LobbySend_PlayerBan(GSint iGroupID, GSint iLobbyServerID, GSchar *szAlias, GSchar *szReason);
		GSbool LobbySend_PlayerUnBan(GSint iGroupID, GSint iLobbyServerID, GSchar *szAlias);
		GSbool LobbySend_GetPlayerBannedList(GSint iGroupID, GSint iLobbyServerID);
		GSbool LobbySend_SetPlayerInfo(GSvoid *vpPlayerData, GSint iPlayerDataSize);
		GSbool LobbySend_GetPlayerGroups(GSchar *szAlias);
		GSbool LobbySend_ChangeRequestedLobbies(const GSchar* szGames);
		
		GSbool PS_Engine(GSuint uiMaxPostingDelay = 500, GSuint uiMaxsOperationalDelay = 800);
		GSbool PSSend_Login();
		GSbool PSSend_Disconnect();
		GSuint PSSend_GetPrivateData(const GSchar *szGame, GSint iEventID,
			GSint iRecordID);
		GSuint PSSend_SetPrivateData(const GSchar *szGame, GSint iEventID,
			GSint iRecordID, const GSvoid *vpData, GSint iSize);
		GSuint PSSend_GetPublicData(const GSchar *szGame, GSint iEventID,
			GSint iRecordID, const GSchar *szPlayerAlias = 0);
		GSuint PSSend_SetPublicData(const GSchar *szGame, GSint iEventID,
			GSint iRecordID, const GSvoid *vpData, GSint iSize);

        GSbool LadderQuery_Initialize(GSchar *szLocale);
		GSbool LadderQuery_Uninitialize();
		GSbool LadderQuery_Engine(GSuint uiMaxPostingDelay = 500,GSuint uiMaxsOperationalDelay = 800);
		GSuint LadderQuery_SendRequest();
		
		GSbool LadderQuery_CreateRequest(GSchar *szGameName, GSuint uiEvent = 0, GSuint uiMode = 0);
		GSbool LadderQuery_RequestPivotUser(const GSchar *pszEntry, GSuint uiNumberOfEntries);
		GSbool LadderQuery_RequestPivotRow(GSuint  uiEntry, GSuint uiNumberOfEntries);
		GSbool LadderQuery_RequestSet(const GSchar *pszEntries[], GSuint uiNumberOfEntries);
		GSbool LadderQuery_RequestOrderedList(GSuint uiFirstEntry, GSuint uiNumberOfEntries);
		
		GSbool LadderQuery_AddSortConstraint(const GSchar *pszField, GSbool bSortDirection);
		GSbool LadderQuery_AddFilterConstraint(const LADDER_FILTER *pLadderFilter[], GSuint uiListOfFilters);
		GSbool LadderQuery_AddDisplayConstraint(const GSchar *pszFields[], GSuint uiNumberOfFields);
		
		GSbool LadderQuery_GetResultSearchCount(GSuint uiRequestID, GSuint & uiCount);
		GSbool LadderQuery_GetResultEntryCount(GSuint uiRequestID, GSuint & uiCount);
		GSbool LadderQuery_GetResultFieldCaption(GSuint uiRequestID, const GSchar *pszField, GSchar * pszCaption);
		GSbool LadderQuery_StartResultEntryEnumeration(GSuint uiRequestID, GSuint uiStartEntry);
		GSbool LadderQuery_NextResultEntry(GSuint uiRequestID);
		GSbool LadderQuery_GetCurrentEntryField(GSuint uiRequestID, const GSchar *pszField, GSint & iValue);
		GSbool LadderQuery_GetCurrentEntryFieldAsString(GSuint uiRequestID, const GSchar *pszField, GSchar * pszValue);
		GSvoid LadderQuery_ReleaseResult(GSuint uiRequestID);	

        GSRESULT RemoteAlgorithm_Initialise(const GSchar * szGameName);
        GSRESULT RemoteAlgorithm_Uninitialise();
        GSRESULT RemoteAlgorithm_Engine(GSuint uiMaxPostingDelay = 500,
                                        GSuint uiMaxsOperationalDelay = 800);
        GSRESULT RemoteAlgorithm_Execute(GSuint uiAlgoId,
                                         RAE_VALUE * pInput, GSuint uiNumInput,
                                         RemoteAlgorithm_OutputCB fOutput,
                                         GSvoid * pData, GSuint & uiRequest);
        
		private:
			CGLClientLIB *m_pClientLIB;

};

#endif //_GSCLIENCLASS_H_
