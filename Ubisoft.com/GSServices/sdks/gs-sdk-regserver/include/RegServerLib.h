/***SDOC************************************************************************
 *                                UbiSoft Network Development
 *                                ---------------------------
 *
 * FILE........: RegServerLib.h
 * CREATION....: Sept 2001
 * AUTHOR......: Guillaume Plante
 *
 * DESCRIPTION.: Interface class to login fonctionality
 *
 *******************************************************************************
 *                                         FILE HISTORY
 *******************************************************************************
 *
 * DATE........:
 * AUTHOR......:
 * DESCRIPTION.:
 *
 ***********************************************************************EDOC***/

#ifndef __CLIENTLIB_H__
#define __CLIENTLIB_H__

#include "define.h"

class clDataList;
class clDataBin;
class clSRPGSClient;
class CRegServerConnection;
class clParentGroups;

class CRegisterServer
{
	public:

	CRegisterServer();
	virtual ~CRegisterServer();

	// RegServer public functions
	public:
	GSvoid RegServer_Engine();
	GSbool RegServerSend_RouterConnect( const GSchar* szAddress, GSint lPort );
	GSbool RegServerSend_RouterDisconnect();
	GSbool RegServerSend_LoginRouter( const GSchar* szAlias,
			const GSchar* szPassword, const GSchar* szVersion );
	GSbool RegServerSend_RequestParentGroupOnLobby( const GSchar* szGameName );

	GSbool RegServerSend_RegisterServerOnLobby(
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


	//////////////////////////////
	// Lobby Server
	//////////////////////////////
	GSbool RegServerSend_LobbyServerConnection( const GSchar* szAddress, GSushort usPort,
			GSushort usLocalPort = 0, GSuint uiStillAliveDelay = 25, GSuint uiDisconnectionDelay = 120 );
	GSbool RegServerSend_LobbyServerClose();
	GSbool RegServerSend_LobbyServerLogin( const GSchar* szAlias,GSint iGroupID );
	GSbool RegServerSend_LobbyServerMemberJoin( const GSchar* szMember );
	GSbool RegServerSend_LobbyServerMemberLeave( const GSchar* szMember );
	GSbool RegServerSend_UpdateGroupSettings(
			GSint iGroupID,
			GSbyte bOpen,
			GSbyte bScoreSubmission,
			GSbyte bDedicatedServer,
			GSint iMaxPlayers,
			GSint iMaxSpectator,
			const GSchar* szPassword,
			const GSvoid* pucGroupInfo,
			GSint iGroupInfoSize,
			const GSvoid* pucAltGroupInfo,
			GSint iAltGroupInfoSize,
			const GSvoid* pucGameData,
			GSint iGameDataSize,
			GSushort usGamePort );
	
	GSbool RegServerSend_MatchStart( GSuint uiMode = 0 );
	GSbool RegServerSend_MatchFinish( );

	//**************************************************
	//virtual Function used for the CallBack
	//**************************************************
	virtual GSvoid RegServerRcv_LoginRouterResult( GSubyte ucType, GSint lReason,
			const GSchar* szIPAddress ) = 0;
	virtual GSvoid RegServerRcv_RouterDisconnection() = 0;
	virtual GSvoid RegServerRcv_RegisterServerResult( GSubyte pucType,
			GSint plReason,
			GSint iGroupID,
			const GSchar* szAddress,
			GSushort usPort,
			const GSchar* szSessionName ) = 0;
	virtual GSvoid RegServerRcv_RequestParentGroupResult( GSubyte ucType,
			GSint lReason, GSint iServerID,GSint iGroupID, const GSchar* szGroupName,
			GSuint uiNbPlayers, GSuint uiMaxPlayers ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerLoginResults( GSubyte ucType,
			GSint iReason, GSint iLobbyServerID, GSint iGroupID ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerUpdateGroupSettingsResults(
			GSubyte ucType, GSint iReason, GSint iGroupID ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerDisconnection() = 0;
	virtual GSvoid RegServerRcv_LobbyServerMemberNew( const GSchar* szMember,
			GSbool bSpectator,
		const GSchar* szIPAddress, const GSchar* szAltIPAddress,
			const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize,
			GSushort usPlayerStatus ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerMatchStartReply( GSubyte ucType,
			GSint iReason, GSint iGroupID ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerMemberLeft(const GSchar* szMember ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerMatchFinishReply( GSubyte ucType,
			GSint iReason, GSint iGroupID ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerGroupConfigUpdate(
			GSuint uiGroupConfig, GSint iGroupID ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerMemberUpdateStatus(
			const GSchar* szPlayer, GSushort usPlayerStatus ) = 0;
	virtual	GSvoid RegServerRcv_LobbyServerNewGroup (GSushort usRoomType,
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
		GSuint GroupInfoSize,
		GSuint uiMatchEventID,
		GSuint uiMaxPlayers,
		GSuint uiNbPlayers,
		GSuint uiMaxSpectators,
		GSuint uiNbSpectators,
		const GSchar* szGameVersion,
		const GSchar* szGSGameVersion,
		const GSchar* szIPAddress,
		const GSchar* szAltIPAddress ) = 0;
	virtual GSvoid RegServerRcv_LobbyServerMemberUpdateInfo(
			const GSchar* szMember, const GSvoid* pPlayerInfo,
			GSuint uiPlayerInfoSize ) = 0;

	private:

	GSvoid RegServer_UnInitRouterConnection();
	GSvoid RegServer_UnInitLobbyServerConnection();
	GSvoid CheckDisconnection();

	GSbool RegServerRcv_LoginRouterResult( GSushort SFType,clDataList* pDataList);
	GSbool RegServerRcv_RegisterServerResult( GSushort usFSType,
			clDataList* pDataList );
	GSbool RegServerRcv_RequestParentGroupResult( GSushort SFType,
			clDataList* pDataList );

	GSbool CheckLobbyServerConnection();
	GSbool ManageLobbyServerConnection();
	GSvoid CheckTheLobbyMessages();
	GSvoid ReadLobbySeverAnswer( GSubyte ucType, clDataList* pDataList );
	GSvoid RegServerRcv_LobbyServerLoginResults( GSuchar ucType,
			clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerUpdateGroupSettingsResults( GSuchar ucType,
			clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerNewMembers( clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerMatchStartReply( GSuchar ucType,
			clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerMemberLeft( clDataList* pDataList );
	GSvoid RegServerRcv_LobbyServerMatchFinishReply( GSuchar ucType,
			clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerGroupConfigUpdate( clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerMemberUpdateStatus( clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerNewGroup ( clDataList* pDataMessage );
	GSvoid RegServerRcv_LobbyServerMemberUpdateInfo(  clDataList* pDataMessage );


	clParentGroups *m_pParentGroups;

	GSchar m_szPlayerAlias[NICKNAMELENGTH];
	GSchar m_szAddress[IPADDRESSLENGTH];
	GSbool m_bServerRegistered;

	GSbool m_bLobbyConnected;
	GSbool m_bRouterConnected;

	GSchar m_szLocalIP[IPADDRESSLENGTH];
	CRegServerConnection* m_RouterConnection;
	clSRPGSClient* m_pSRPGSClient;
	GSint m_iLobbyServerID;
	GSint m_iGroupID;

};

#endif //__CLIENTLIB_H__
