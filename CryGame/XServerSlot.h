//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServerSlot.h
//  Description: XServerSlot class.
//
//  History:
//  - 08/03/2001: Created by Alberto Demichelis
//  - 12/09/2003: MartinM cleaned up
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XSERVERSLOT_H
#define GAME_XSERVERSLOT_H

#if _MSC_VER > 1000
# pragma once
#endif

#include "XEntityProcessingCmd.h"
#define MAX_ENT_DIST_FACTOR 100	
class CScriptObjectServerSlot;

#include "XNetworkStats.h"									// CXNetworkStats

class CXSnapshot;
class CXServer;


struct SlotNetStats
{
	unsigned int								ping;										//!<
	unsigned int								packetslost;						//!<
	unsigned int								upacketslost;						//!<
	string											name;										//!<
	unsigned int								maxsnapshotbitsize;			//!<
	unsigned int								lastsnapshotbitsize;		//!<
};

//////////////////////////////////////////////////////////////////////
/*! this class represent a client on the server-side.This mean that 
for every connected client a respective CXServerSlot exists.
@see IServerSlot
@see IServerSlotSink
@see IServer
*/
class CXServerSlot : public IServerSlotSink
{
public:
	//! constructor
	CXServerSlot(CXServer *pParent, IServerSlot *pSlot);
	//! destructor
	virtual ~CXServerSlot();
	//!
	void GetNetStats(SlotNetStats &ss);
	//!
	void Disconnect(const char *sCause);

	//!< from the serverslot internals
	void GetBandwidthStats( SServerSlotBandwidthStats &out );
	//!< from the serverslot internals
	void ResetBandwidthStats();

	// interface IServerSlotSink ----------------------------------

	virtual void OnXServerSlotConnect(const BYTE *pbAuthorizationID, unsigned int uiAuthorizationSize);
	virtual void OnXServerSlotDisconnect(const char *szCause);
	virtual void OnContextReady(CStream &stm);
	virtual void OnData(CStream &stm);
	virtual void OnXPlayerAuthorization( bool bAllow, const char *szError, const BYTE *pGlobalID, unsigned int uiGlobalIDSize );

	// ------------------------------------------------------------

	//! this function is only used to optimize the network code
	void OnSpawnEntity(CEntityDesc &ed,IEntity *pEntity,bool bSend);
	//! this function is only used to optimize the network code
	void OnRemoveEntity(IEntity *pEntity);

	//!
	void BanByID();
	//!
	void BanByIP();

	// attributes -------------------------------------------------

	//! \return slot id
	BYTE GetID();
	//!
	bool IsXServerSlotGarbage();
	//!
	bool IsLocalHost();
	//! \return pin in milliseconds
	unsigned int GetPing();
	//! \param idPlayer
	void SetPlayerID(EntityId idPlayer);
	//! \return 
	EntityId GetPlayerId() const;

	//! \return client requested name
	const char *GetName();
	//! \return client requested player model
	const char *GetModel();
	//! \return client requested player color in non team base multiplayer mods (string from the client)
	const char *GetColor();

	//! \param state
	//! \param time absolute time since the state started on the server (not used in SP)
	void SetGameState(int state, int time=0);
	//!
	CXServer *GetServer();
	//!
	void SendReliable(CStream &stm,bool bSecondaryChannel);
	//!
	void SendUnreliable(CStream &stm);
	//! \return size in bits of the whole packet
	size_t SendReliableMsg(XSERVERMSG msg, CStream &stm,bool bSecondaryChannel, const char *inszName="Unknown" );
	//! \return size in bits of the whole packet
	size_t SendUnreliableMsg(XSERVERMSG msg, CStream &stm, const char *inszName="Unknown", const bool bWithSize=false );		
	//!
	void SendText(const char *sText,float fLifeTime=DEFAULT_TEXT_LIFETIME);
	//!
	void SendCommand(const char *sCmd);
	//!
	void SendCommand(const char *sCmd, const Vec3 &invPos, const Vec3 &invNormal, 
		const EntityId inId, const unsigned char incUserByte );
	//!
	bool IsReady();
	//!
	bool CanSpawn(){return m_bCanSpawn;}
	//!
	float GetPlayerWriteStepBack()
	{
		if(m_iLastCommandServerPhysTime!=0)
		{
			return (m_pPhysicalWorld->GetiPhysicsTime()-m_iLastCommandServerPhysTime)*m_pPhysicalWorld->GetPhysVars()->timeGranularity;
		}
		return 0.0f;
	}
	//!
	unsigned int GetCommandClientPhysTime() 
	{
		return m_iLastCommandClientPhysTime ? m_iLastCommandClientPhysTime : m_pPhysicalWorld->GetiPhysicsTime();
	}
	//!
	unsigned int GetClientWorldPhysTime()
	{
		return m_pPhysicalWorld->GetiPhysicsTime()+m_iClientWorldPhysTimeDelta;
	}
	//!
	void SendTextMessage(TextMessage &tm,bool bSecondaryChannel);
	//!
	IScriptObject *GetScriptObject(){return m_ScriptObjectServerSlot.GetScriptObject();}
	//! update the server slot stuff
	void Update(bool send_snap, bool send_events);

	//! context setup
	void ContextSetup();
	//!
	void CleanupSnapshot()
	{
		m_Snapshot.Cleanup();
	} 
	//!
	bool IsContextReady();

	//return if this is the first snapshot(so you have to send the full snapshot for all entities)

	void ResetPlayTime(){m_fPlayTime=m_pTimer->GetCurrTime();}
	//!
	float GetPlayTime(){return m_pTimer->GetCurrTime()-m_fPlayTime;}
	//! \return amount of bytes allocated by this instance and it's childs 
	unsigned MemStats();
	//!
	int GetClientTimeDelta() { return m_iClientWorldPhysTimeDelta; }
	//!
	void MarkEntityOffSync(EntityId id);
	//!
	bool IsEntityOffSync(EntityId id);
	//!
	bool IsClientSideEntity(IEntity *pEnt);
	//!
	IServerSlot* GetIServerSlot() { return m_pISSlot; }
	//! \return true=channel is now occupied you can send your data through the lazy channel, false=channel is not ready yet
	bool OccupyLazyChannel();
	//! 
	bool ShouldSendOverLazyChannel();
	//!
	bool GetServerLazyChannelState();

	//static void ConvertToValidPlayerName( const char *szName, char outName[65] );
	static void ConvertToValidPlayerName( const char *szName, char* outName, size_t sizeOfOutName );

	// ---------------------------------------------------------------------

	CXEntityProcessingCmd		m_PlayerProcessingCmd;				//!<
	bool										m_bForceScoreBoard;						//!<
	float										m_fLastScoreBoardTime;				//!<
	CXSnapshot							m_Snapshot;										//!< snapshot

private: // --------------------------------------------------------------

	bool ParseIncomingStream(CStream &stm);
	void OnClientMsgPlayerProcessingCmd(CStream &stm);
	void OnClientMsgJoinTeamRequest(CStream &stm);
	void OnClientMsgCallVote(CStream &stm);
	void OnClientMsgVote(CStream &stm);
	void OnClientMsgKill(CStream &stm);
	void OnClientMsgName(CStream &stm);
	void OnClientMsgCmd(CStream &stm);
	void OnClientMsgRate(CStream &stm);
	void OnClientOffSyncEntityList(CStream &stm);
	void OnClientReturnScriptHash(CStream &stm);
	void OnClientMsgAIState(CStream &stm);
	void SendScoreBoard();
	void ValidateName();
	void ClientString(const char *s);
	void FinishOnContextReady();

	// ---------------------------------------------------------------------

	string										m_strPlayerName;									//!< client requested player name
	string										m_strPlayerModel;									//!< client requested player model
	string										m_strClientColor;									//!< client requested player color in non team base multiplayer mods

	EntityId									m_wPlayerId;											//!<
	float											m_fPlayTime;											//!< absolute time when ResetPlayTime() was called
	bool											m_bXServerSlotGarbage;						//!<
	bool											m_bLocalHost;											//!<
	bool											m_bCanSpawn;											//!<
	bool											m_bWaitingForContextReady;				//!<
	bool											m_bContextIsReady;								//!< Gametype on client is syncronized (ClientStuff is available)
	IServerSlot *							m_pISSlot;												//!< might be 0
	CXServer *								m_pParent;												//!<

	CScriptObjectServerSlot		m_ScriptObjectServerSlot;
	ILog *										m_pLog;
	ITimer *									m_pTimer;
	IPhysicalWorld *					m_pPhysicalWorld;
	int												m_nState; 	

	EntityClassId							m_ClassId;												//!<
	int												m_ClientRequestedClassId;					//!< do we need this?
	bool											m_bReady;													//!<

	//?? hack
	CEntityDesc								m_ed;

	int												m_iLastCommandServerPhysTime;
	int												m_iLastCommandClientPhysTime;
	int												m_iClientWorldPhysTimeDelta;
	int												m_nDesyncFrames;
	std::map<EntityId,int>		m_mapOffSyncEnts;
	int												m_iLastEventSent;

	float											m_fLastClientStringTime;
	string										m_sClientString;									//!< XSERVERMSG_CLIENTSTRING
	EntityId									m_idClientVehicle;
	float											m_fClientVehicleSimTime;

	BYTE											m_vGlobalID[64];
	unsigned char							m_bGlobalIDSize;
	BYTE											m_vAuthID[64];
	unsigned char							m_bAuthIDSize;
	bool											m_bServerLazyChannelState;				//!< used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)
	bool											m_bClientLazyChannelState;				//!< used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)
	uint32										m_dwUpdatesSinceLastLazySend;			//!< update cylces we wait for response (for resending), 0=it wasn't set at all

	friend class CScriptObjectServerSlot;
	friend class CXSnapshot;
};

#endif // GAME_XSERVERSLOT_H
