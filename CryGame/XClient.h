//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XClient.h
//  Description: XClient class.
//
//  History:
//  - August 3, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XCLIENT_H
#define GAME_XCLIENT_H

#if _MSC_VER > 1000
# pragma once
#endif


#include <vector>
#include <string>
#include <map>

#include <IInput.h>

#include "ScriptObjectClient.h"
#include "XClientSnapshot.h"
#include "XEntityProcessingCmd.h"
#include "XNetworkStats.h"						// CXNetworkStats
#include "ScriptObjectVector.h"						// CXNetworkStats

struct SCameraParams;
struct IScriptSystem;
struct SXGameContext;

struct SSoundInfo
{
	int nEntityId;			//!<
	Vec3 Pos;						//!<
	float fRadius;			//!<
	float fThread;			//!<
	float fDistance2;		//!< squared distance from player at the time of occurance
	float fTimeout;			//!< Time until this soundinfo is removed from list
};

/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<EntityId> EntityIdList;
typedef EntityIdList::iterator EntityIdListItor;

typedef std::vector<IEntity*> EntityList;
typedef EntityList::iterator EntityListItor;

typedef std::vector<SSoundInfo>	TSoundList;
typedef TSoundList::iterator TSoundListIt;

/////////////////////////////////////////////////////////////////////////////////////////////
// CXClient class.
class CXClient :  
public IClientSink, 
public IActionMapSink,
public IEntitySystemSink
{
public:
	//! constructor
	CXClient();

private:										// to make sure the client is only released in one place
	//! destructor
	virtual ~CXClient();

public:
	//!
	bool Init(CXGame *pGame,bool bLocal=false);
	//!
	void Reset();
	//!
	void DrawNetStats();

	// interface IClientSink -------------------------------------------------

	virtual void OnXConnect();
	virtual void OnXClientDisconnect(const char *szCause);
	virtual void OnXContextSetup(CStream &stmContex);
	virtual void OnXData(CStream &stm);
	virtual void OnXServerTimeout();
	virtual void OnXServerRessurect();
	virtual unsigned int GetTimeoutCompensation();
	virtual void MarkForDestruct();
	virtual bool DestructIfMarked();

	// interface IEntitySystemSink -------------------------------------------

	void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity );
	void OnSpawn(IEntity *ent,CEntityDesc & ed);
	void OnRemove(IEntity *ent);
	void OnBind(EntityId id,EntityId child,unsigned char param){}
	void OnUnbind(EntityId id,EntityId child,unsigned char param){}

	IClient* GetNetworkClient() const { return m_pIClient; };

	// -----------------------------------------------------------------------

	// console variables
	bool CreateConsoleVariables();

	TSoundList& GetSoundEventList() { return m_lstSounds; }
	
	// Connection management functions
	//! \param szAddr
	//! \param inbDoLateSwitch
	//! \param inbCDAuthorization true=request CDKeyAuthorization - needed for joining WAN Servers, false=Send fake CDKeyAuthorization
	void XConnect( const char *szAddr, const bool inbDoLateSwitch=false, const bool inbCDAuthorization=false );
	//!
	void DemoConnect();
	//!
	void XDisconnect(const char *szCause);
	//!
	void Update();
	//!
	void UpdateClientNetwork();

#ifdef PS2
	unsigned int GetCurrentTime()
	{
		return (unsigned int)(m_pTimer->GetCurrTime()*1000.f);
	}
#else	
	inline unsigned int GetCurrentTime()
	{
		return (unsigned int)(m_pTimer->GetCurrTime()*1000.f);
	}
#endif

	void SoundEvent(EntityId idSrc,Vec3 &pos,float fRadius,float fThreat);
	// Overload of the IClient interface
	void SendReliable(CStream &stm);
	void SendUnreliable(CStream &stm);

	//! \return size in bits of the whole packet
	size_t SendReliableMsg(XCLIENTMSG msg, CStream &stm);
	//! \return size in bits of the whole packet
	size_t SendUnreliableMsg(XCLIENTMSG msg, CStream &stm, const bool bWithSize=false );	
	//!
	bool IsReady();
	//!
	void SendTextMessage(TextMessage &tm);
	//!
	void SendCommand(const char *sCmd);
	//!
	void SendScriptHashResponse( const unsigned int dwHash );
	//!
	void SetBitsPerSecond( const unsigned int dwBitsPerSecond );
	//!
	void SetUpdateRate( const unsigned int dwUpdatesPerSec );
	//!
	void AddHudMessage(const char *sMessage,float lifetime,bool bHighPriority=false);
  //!
  void AddHudSubtitle(const char *sMessage, float lifetime);
  //!
  void ResetSubtitles(void);
	//!
  void OnMapChanged();
	//!
  void OnMapChangedReally();
	//!
	void SetEntityCamera( const SCameraParams &CameraParams );
      
	// player management functions

	//!
	EntityId GetPlayerId() const;
	//!
	void SetPlayerID( EntityId wPlayerID );
	//!
	int GetPing()
	{
		return m_pIClient->GetPing();
	}

	bool IsConnected() { return m_bConnected; };
	//!
	void LazyChannelAcknowledge();
	//!
	bool GetLazyChannelState();

	// Triggers function
	void TriggerMoveLeft(float fValue,XActivationEvent ae);
	void TriggerMoveRight(float fValue,XActivationEvent ae);
	void TriggerMoveForward(float fValue,XActivationEvent ae);
	void TriggerMoveBackward(float fValue,XActivationEvent ae);
	void TriggerJump(float fValue,XActivationEvent ae);
	void TriggerMoveMode(float fValue,XActivationEvent ae);
	void TriggerMoveMode2(float fValue,XActivationEvent ae);
	void TriggerLeanLeft(float fValue,XActivationEvent ae);
	void TriggerLeanRight(float fValue,XActivationEvent ae);
	void TriggerHoldBreath(float fValue,XActivationEvent ae);
	void TriggerFireMode(float fValue,XActivationEvent ae);
	void TriggerFire0(float fValue,XActivationEvent ae);
	void TriggerFireCancel(float fValue,XActivationEvent ae);
	void TriggerFireGrenade(float fValue,XActivationEvent ae);
	void TriggerVehicleBoost(float fValue,XActivationEvent ae);
	void TriggerReload(float fValue,XActivationEvent ae);
	void TriggerUse(float fValue,XActivationEvent ae);
	void TriggerTurnLR(float fValue,XActivationEvent ae);
	void TriggerTurnUD(float fValue,XActivationEvent ae);
	void TriggerWalk(float fValue,XActivationEvent ae);
	void TriggerRunSprint(float fValue,XActivationEvent ae);
	void TriggerFlashlight(float fValue,XActivationEvent ae);
	void TriggerChangeView(float fValue,XActivationEvent ae);
	//weapons.
	void TriggerNextWeapon(float fValue,XActivationEvent ae);
	void TriggerPrevWeapon(float fValue,XActivationEvent ae);
	void TriggerWeapon0(float fValue,XActivationEvent ae);
	void TriggerWeapon1(float fValue,XActivationEvent ae);
	void TriggerWeapon2(float fValue,XActivationEvent ae);
	void TriggerWeapon3(float fValue,XActivationEvent ae);
	void TriggerWeapon4(float fValue,XActivationEvent ae);
	void TriggerWeapon5(float fValue,XActivationEvent ae);
	void TriggerWeapon6(float fValue,XActivationEvent ae);
	void TriggerWeapon7(float fValue,XActivationEvent ae);
	void TriggerWeapon8(float fValue,XActivationEvent ae);
	void TriggerWeapon9(float fValue,XActivationEvent ae);
	void TriggerWeapon10(float fValue,XActivationEvent ae);
	void TriggerWeapon11(float fValue,XActivationEvent ae);
	void TriggerWeapon12(float fValue,XActivationEvent ae);
	void TriggerWeapon13(float fValue,XActivationEvent ae);
	void TriggerWeapon14(float fValue,XActivationEvent ae);
	void TriggerDropWeapon(float fValue,XActivationEvent ae);
	void CycleGrenade(float fValue,XActivationEvent ae);
	
	//client side

	void TriggerItem0(float fValue,XActivationEvent ae);
	void TriggerItem1(float fValue,XActivationEvent ae);
	void TriggerItem2(float fValue,XActivationEvent ae);
	void TriggerItem3(float fValue,XActivationEvent ae);
	void TriggerZoomToggle(float fValue,XActivationEvent ae);
	void TriggerZoomIn(float fValue,XActivationEvent ae);
	void TriggerZoomOut(float fValue,XActivationEvent ae);
	void TriggerConcentration(float fValue,XActivationEvent ae);
	void TriggerQuickLoad(float fValue,XActivationEvent ae);
	void TriggerQuickSave(float fValue,XActivationEvent ae);
	void TriggerMessageMode(float fValue,XActivationEvent ae);
	void TriggerMessageMode2(float fValue,XActivationEvent ae);
	void TriggerScreenshot(float fValue, XActivationEvent ae);

	void TriggerMoveModeToggle(float fValue, XActivationEvent ae);
	void TriggerAimToggle(float fValue, XActivationEvent ae);

	BEGIN_INPUTACTIONMAP()
		REGISTER_INPUTACTIONMAP(ACTION_MOVE_LEFT, TriggerMoveLeft)
		REGISTER_INPUTACTIONMAP(ACTION_MOVE_RIGHT, TriggerMoveRight)
		REGISTER_INPUTACTIONMAP(ACTION_MOVE_FORWARD, TriggerMoveForward)
		REGISTER_INPUTACTIONMAP(ACTION_MOVE_BACKWARD, TriggerMoveBackward)
		REGISTER_INPUTACTIONMAP(ACTION_JUMP, TriggerJump)
		REGISTER_INPUTACTIONMAP(ACTION_WALK, TriggerWalk)
		REGISTER_INPUTACTIONMAP(ACTION_RUNSPRINT, TriggerRunSprint)
		REGISTER_INPUTACTIONMAP(ACTION_MOVEMODE, TriggerMoveMode)
		REGISTER_INPUTACTIONMAP(ACTION_MOVEMODE2, TriggerMoveMode2)
		REGISTER_INPUTACTIONMAP(ACTION_LEANLEFT, TriggerLeanLeft)
		REGISTER_INPUTACTIONMAP(ACTION_LEANRIGHT, TriggerLeanRight)
		REGISTER_INPUTACTIONMAP(ACTION_HOLDBREATH, TriggerHoldBreath)
		REGISTER_INPUTACTIONMAP(ACTION_FIREMODE, TriggerFireMode)
		REGISTER_INPUTACTIONMAP(ACTION_FIRE0, TriggerFire0)
		REGISTER_INPUTACTIONMAP(ACTION_FIRECANCEL, TriggerFireCancel)
		REGISTER_INPUTACTIONMAP(ACTION_FIRE_GRENADE, TriggerFireGrenade)
		REGISTER_INPUTACTIONMAP(ACTION_VEHICLE_BOOST, TriggerVehicleBoost)
		REGISTER_INPUTACTIONMAP(ACTION_RELOAD, TriggerReload)
		REGISTER_INPUTACTIONMAP(ACTION_USE, TriggerUse)
		REGISTER_INPUTACTIONMAP(ACTION_TURNLR, TriggerTurnLR)
		REGISTER_INPUTACTIONMAP(ACTION_TURNUD, TriggerTurnUD)
//		REGISTER_INPUTACTIONMAP(ACTION_SAVEPOS, TriggerSavePos)
	//	REGISTER_INPUTACTIONMAP(ACTION_LOADPOS, TriggerLoadPos)
		REGISTER_INPUTACTIONMAP(ACTION_FLASHLIGHT, TriggerFlashlight)
		REGISTER_INPUTACTIONMAP(ACTION_CHANGE_VIEW, TriggerChangeView)
		//weapons.
		REGISTER_INPUTACTIONMAP(ACTION_NEXT_WEAPON, TriggerNextWeapon)
		REGISTER_INPUTACTIONMAP(ACTION_PREV_WEAPON, TriggerPrevWeapon)
		REGISTER_INPUTACTIONMAP(ACTION_WEAPON_0, TriggerWeapon0)
		REGISTER_INPUTACTIONMAP(ACTION_WEAPON_1, TriggerWeapon1)
		REGISTER_INPUTACTIONMAP(ACTION_WEAPON_2, TriggerWeapon2)
		REGISTER_INPUTACTIONMAP(ACTION_WEAPON_3, TriggerWeapon3)
		REGISTER_INPUTACTIONMAP(ACTION_WEAPON_4, TriggerWeapon4)
		REGISTER_INPUTACTIONMAP(ACTION_CYCLE_GRENADE, CycleGrenade)

		REGISTER_INPUTACTIONMAP(ACTION_DROPWEAPON, TriggerDropWeapon)
//client side only(for script)
		REGISTER_INPUTACTIONMAP(ACTION_ITEM_0, TriggerItem0)
		REGISTER_INPUTACTIONMAP(ACTION_ITEM_1, TriggerItem1)
		REGISTER_INPUTACTIONMAP(ACTION_ZOOM_TOGGLE, TriggerZoomToggle)
		REGISTER_INPUTACTIONMAP(ACTION_ZOOM_IN, TriggerZoomIn)
		REGISTER_INPUTACTIONMAP(ACTION_ZOOM_OUT, TriggerZoomOut)
		REGISTER_INPUTACTIONMAP(ACTION_CONCENTRATION, TriggerConcentration);
		//REGISTER_INPUTACTIONMAP(ACTION_QUICKLOAD,TriggerQuickLoad);
		//REGISTER_INPUTACTIONMAP(ACTION_QUICKSAVE,TriggerQuickSave);

		REGISTER_INPUTACTIONMAP(ACTION_MESSAGEMODE, TriggerMessageMode)
		REGISTER_INPUTACTIONMAP(ACTION_MESSAGEMODE2, TriggerMessageMode2)
		REGISTER_INPUTACTIONMAP(ACTION_TAKESCREENSHOT, TriggerScreenshot)

		REGISTER_INPUTACTIONMAP(ACTION_MOVEMODE_TOGGLE, TriggerMoveModeToggle)
		REGISTER_INPUTACTIONMAP(ACTION_AIM_TOGGLE, TriggerAimToggle)

	END_INPUTACTIONMAP()
		
private: // -------------------------------------------------------------

	void UpdateISystem();
	void LoadPlayerDesc();
	//packet parsers
	bool ParseIncomingStream(CStream &stm);
	bool OnServerMsgSetPlayer(CStream &stm);
	bool OnServerMsgText(CStream &stm);
	bool OnServerMsgSetTeam(CStream &stm);
	bool OnServerMsgAddTeam(CStream &stm);
	bool OnServerMsgRemoveTeam(CStream &stm);
	bool OnServerMsgAddEntity(CStream &stm);
	bool OnServerMsgRemoveEntity(CStream &stm);
	bool OnServerMsgUpdateEntity(CStream &stm);
	bool OnServerMsgTimeStamp(CStream &stm);
	bool OnServerMsgStuff(CStream &stm);
	bool OnServerMsgClientString(CStream &stm);
	bool OnServerMsgSetEntityName(CStream &stm);
	bool OnServerMsgScoreBoard(CStream &stm);
	bool OnServerMsgSetTeamScore(CStream &stm);
	bool OnServerMsgSetTeamFlags(CStream &stm);
	bool OnServerMsgSetPlayerScore(CStream &stm);
	bool OnServerMsgGameState(CStream &stm);
	bool OnServerMsgSetEntityState(CStream &stm);
	bool OnServerMsgBindEntity(CStream &stm);
//  bool OnServerMsgObituary(CStream &stm);
	bool OnServerMsgCmd(CStream &stm);
	bool OnServerMsgSyncVar(CStream &stm);
	bool OnServerMsgEventSchedule(CStream &stm);
	bool OnServerMsgRequestScriptHash(CStream &stm);
	bool OnServerMsgSyncAIState(CStream &stm);

/*
	void OnCommandSay(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
	void OnCommandGo(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
	void OnCommandAttack(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
	void OnCommandDefend(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
	void OnCommandCover(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
	void OnCommandBarrageFire(IEntity *pSender, CStream &stm, _SmartScriptObject &pTable);
*/

	void LoadingError(const char *szError);

private: // -------------------------------------------------------------

	EntityId						m_wPlayerID;							//!<
	int									m_iPhysicalWorldTime;			//!<
	bool								m_bIgnoreSnapshot;				//!<
	bool								bDoSwitch;								//!<

public: // -------------------------------------------------------------

	float								m_fFrontSound;						//!<
	float								m_fBackSound;							//!<
	float								m_fLeftSound;							//!<
	float								m_fRightSound;						//!<
	TSoundList					m_lstSounds;							//!< occured sounds to be used for the radar

	bool								m_bDisplayHud;						//!<
	IClient	*						m_pIClient;								//!<
	CStream	*						m_pSavedConsoleVars;			//!< saved console variable state (to restore the VF_REQUIRE_NET_SYNC marked vars), 0 when not used

	bool								m_bLocalHost;							//!< this client is the local host ?
	class CXGame *			m_pGame;									//!< The game
	bool								m_bLinkListenerToCamera;	//!<
	struct IXSystem *		m_pISystem;								//!< The system interface
	IScriptSystem *			m_pScriptSystem;					//!<
	IEntitySystem *			m_pEntitySystem;					//!<
	ILog *							m_pLog;										//!<
	bool								m_bLazyChannelState;			//!< used for sending ordered reliable data over the unreliable connection (slow but never stalls, used for scoreboard)


	// Player
	CXEntityProcessingCmd m_PlayerProcessingCmd;
	CXEntityProcessingCmd m_PrevPlayerProcessingCmd;

	// The current game context
	SXGameContext				m_GameContext;
	
	// Action map
	struct IActionMapManager*	m_pIActionMapManager;

	//used to discard old unreliable packets
	float								m_fLastRemoteAsyncCurrTime;

	float								m_fLastScoreBoardTime;
	float								m_fLastClientStringTime;

	SCameraParams *			m_CameraParams;						//!<
	
	// gamestate stuff
	char								m_nGameState;							//!< one of the above. hardcoded in Lua
	short								m_nGameLastTime;					//!< in seconds
	float								m_fGameLastTimeReceived;	//!<

	unsigned int				m_nDiscardedPackets;			//!<
	
	CXClientSnapshot		m_Snapshot;								//!< Snapshot
	bool								m_bSelfDestruct;					//!< usually false, to make sure the client is only released in one place

	// Client console variables
	ICVar* cl_sound_event_timeout;
	ICVar* cl_sound_event_radius;
	ICVar* cl_runroll;
	ICVar* cl_runpitch;
	ICVar* cl_runheight;
	ICVar* cl_runheightspeed;
	ICVar* cl_playerclassid;
/*
	ICVar* cl_explShakeDCoef;
	ICVar* cl_explShakeAmplH;
	ICVar* cl_explShakeAmplV;
	ICVar* cl_explShakeFreq;
	ICVar* cl_explShakeTime;
*/
	float cl_explShakeDCoef;
	float cl_explShakeAmplH;
	float cl_explShakeAmplV;
	float cl_explShakeFreq;
	float cl_explShakeTime;

	ICVar* cl_sound_detection_max_distance;
	ICVar* cl_sound_detection_min_distance;
	ICVar* cl_netstats;
	ICVar* cl_cmdrate;

  bool										m_bMapConnecting;				//!<
	EntityIdList						m_lstGarbageEntities;		//!<
	EntityList							m_lstUpdatedEntities;		//!<
	ITimer *								m_pTimer;								//!<
	bool										m_bRecordingDemo;				//!< used for demo recording (use network stream)
	bool										m_bPlaybackDemo;				//!< used for demo recording (use network stream)
	CScriptObjectClient *		m_pScriptObjectClient;	//!<
	IScriptObject *					m_pClientStuff;					//!< connection to the associated scriptobject

private: // --------------------------------------------------------------------------------

	string									m_sClientString;				//!< XSERVERMSG_CLIENTSTRING
	CXNetworkStats					m_NetStats;							//!< for network statistics (count and size per packet type)
	CStream									m_stmVehicle;						//!<
	bool										m_bConnected;						//!< is the client fully connected to the game ? (connection established, map loaded, setup complete, ...)

	CScriptObjectVector			m_sopMsgNormal;					//!<
	CScriptObjectVector			m_sopMsgPos;						//!<

	//!
	static const char *GetMsgName( XSERVERMSG inValue );
	//! is called by Update()
	void UpdateSound( const float fFrameTime );
	//!
	void SendInputToServer( const bool bTimeToSend );
	//! restore VF_REQUIRE_NET_SYNC marked console vars
	void RestoreServerSyncedVars();
};

#endif // GAME_XCLIENT_H
