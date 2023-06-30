//////////////////////////////////////////////////////////////////////
//
//	Game source code
//
//	File: Game.h
//
//	History:
//	-August 02,2001: Create by Alberto Demichelis
//	-Sep 24,2001		 Modified by Petar Kotevski
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_GAME_H
#define GAME_GAME_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////
#ifdef FARCRYDLL_EXPORTS
	#define GAMEAPI __declspec(dllexport)
#else
	#define GAMEAPI __declspec(dllimport)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Version of the game
#define GAME_MAIN_VERSION						1						//!< [0..255]
#define GAME_SUB_VERSION						3						//!< [0..255] patch version number, shown in menu
#define GAME_PATCH_VERSION					3						//!< [0..256*256[
#define SERVERINFO_FORMAT_VERSION		87				  //!< [0..255] bump if server info format changes (old version won't show up any more)
#define NETWORK_FORMAT_VERSION			5						//!< [0..2^32] bump if netcode stream format changes
 
#define SAVEVERSION									23					// [Petar] Do not bump this value anymore it shows the release version of the savegame - it will always be supported
#define PATCH1_SAVEVERSION					24					// [Kirill] Do not bump this value anymore it shows the Patch 1 version of the savegame - it will always be supported
#define PATCH2_SAVEVERSION					36					//!< bump this if the savegame format changes and we are still working on patch 2

#define GAME_VERSION_SUFIX					"F"				//!< A - alpha, B - beta, RC - release candidate, F - final


#define MAKE_GAME_VERSION(m,s,p)		(((m)<<24)|((s)<<16)|(p))
#define GAME_VERSION								MAKE_GAME_VERSION(GAME_MAIN_VERSION,GAME_SUB_VERSION,GAME_PATCH_VERSION)

#define ENTITYTYPE_PLAYER				0x00000001
#define ENTITYTYPE_WAYPOINT			0x00000002
#define ENTITYTYPE_OWNTEAM			0x00000004

#define SAVEMAGIC "CRYLEVELSAVE"

// game states
enum { CGS_INPROGRESS=0, CGS_COUNTDOWN=1, CGS_PREWAR=2, CGS_INTERMISSION=3 };

#include "XNetwork.h"

#include "BitStream_Base.h"						// CBitStream_Base
#include "BitStream_Compressed.h"			// CBitStream_Compressed

//forward declarations
//////////////////////////////////////////////////////////////////////
class		CScriptObjectGame;
class		CScriptObjectInput;
class		CScriptObjectBoids;
class		CScriptObjectGUI;
struct	ISystem;
struct	I3DEngine;
class		CXServer;
class		CXClient;
class		CUIHud;
class		CXServerRules;
class		CWeaponSystemEx;
class		CVehicleSystem;
class		CPlayerSystem;
class		CFlockManager;
class		CTagPoint;
struct	ITagPoint;
class		CScriptObjectAI;
class		CIngameDialogMgr;
class		CViewLayer;
class		CScriptTimerMgr;
class		CTeamMgr;
class		CPlayer;
class		CMovieUser;
class		CTimeDemoRecorder;
class		CUISystem;
class		CXGame;
class		CGameMods;


enum EventType { EVENT_MOVECMD=0,EVENT_EXPLOSION=1,EVENT_IMPULSE=2,EVENT_VEHDAMAGE=3 };
enum ActionType { ACTIONTYPE_MOVEMENT = 1, ACTIONTYPE_COMBAT, ACTIONTYPE_GAME, ACTIONTYPE_MULTIPLAYER, ACTIONTYPE_DEBUG};

struct BaseEvent
{
	int nRefCount;
	virtual EventType GetType() = 0;
	virtual void Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream ) = 0;
	virtual void Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream ) = 0;
	virtual void Execute(CXGame *pGame) = 0;
};

struct GameEvent
{
	GameEvent() {}
	GameEvent(const GameEvent &src)
	{
		iPhysTime = src.iPhysTime;
		idx = src.idx;
		(pEvent = src.pEvent)->nRefCount++;
	}
	~GameEvent()
	{
		if (--pEvent->nRefCount<=0)
			delete pEvent;
	}
	GameEvent& operator=(const GameEvent &src)
	{
		iPhysTime = src.iPhysTime;
		idx = src.idx;
		(pEvent = src.pEvent)->nRefCount++;
		return *this;
	}
	int iPhysTime;
	int idx;
	BaseEvent *pEvent;
};

#include "XEntityProcessingCmd.h"

struct EventPlayerCmd : BaseEvent
{
	EntityId								idEntity;
	CXEntityProcessingCmd		cmd;

	virtual EventType GetType() { return EVENT_MOVECMD; }
	void Write(CStream &stm, int iPhysicalTime, IBitStream *pBitStream );
	void Read(CStream &stm, int &iPhysicalTime, IBitStream *pBitStream );
	void Execute(CXGame *pGame);
};

struct EventExplosion : BaseEvent
{
	Vec3						pos;
	float						damage;
	float						rmin,rmax,radius;
	float 					impulsivePressure;
	int 						nTerrainDecalId;
	int32 					nShooterSSID;											//!< clientID - neeeded for MP score calculations, -1 if unknown
	uint16 					iImpactForceMul;
	uint16 					iImpactForceMulFinal;
	uint16 					iImpactForceMulFinalTorso;
	EntityId 				idShooter;
	EntityId 				idWeapon;
	uint8 					shakeFactor;								//!< 0..1
	uint8 					deafnessRadius;							//!< 0..20
	uint8 					deafnessTime;								//!< *0.25 to get float
	uint8 					rminOcc;										//!< 0..1
	uint8 					nOccRes;
	uint8 					nGrow;
	uint8 					terrainDefSize;							//!< 0..20

	virtual EventType GetType() { return EVENT_EXPLOSION; }
	void Write(CStream &stm, int iPhysicalTime, IBitStream *pBitStream );
	void Read(CStream &stm, int &iPhysicalTime, IBitStream *pBitStream );
	void Execute(CXGame *pGame);
};

struct EventPhysImpulse : BaseEvent
{
	int							idPhysEnt;
	Vec3						impulse;
	Vec3						pt;
	Vec3						momentum;
	bool						bHasPt;
	bool						bHasMomentum;

	virtual EventType GetType() { return EVENT_IMPULSE; }
	virtual void Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream );
	virtual void Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream );
	void Execute(CXGame *pGame);
};

struct EventVehicleDamage : BaseEvent
{
	EntityId				idVehicle;
	uint8						damage;

	virtual EventType GetType() { return EVENT_VEHDAMAGE; }
	virtual void Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream );
	virtual void Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream );
	void Execute(CXGame *pGame);
};


//#define _INTERNET_SIMULATOR

//memory stats
class		ICrySizer;

typedef std::vector<string> Vec2Str;
typedef Vec2Str::iterator Vec2StrIt;
typedef std::list<CPlayer*> ListOfPlayers;

#include "IInput.h"		// XActionActivationMode


struct ActionInfo
{
	int nId;
	string sDesc;
	bool bConfigurable;
	XActionActivationMode ActivationMode;
	Vec2Str vecSetToActionMap;	// if it is configured via "BindActionMultipleMaps" it will set the key-bindings to all action-maps in this array and leaves the others untouched
	int nType;
};

typedef std::map<string,ActionInfo> ActionsEnumMap;
typedef ActionsEnumMap::iterator ActionsEnumMapItor;
typedef std::multimap<string, CTagPoint *> TagPointMap;

typedef std::map<CIPAddress, SXServerInfos>	ServerInfosMap;
typedef ServerInfosMap::iterator ServerInfosVecItor;

struct PreviewMapParams
{
	PreviewMapParams()
	{
		bRound=false;
		SectorsX=0;
		SectorsY=0;
	}
	bool bRound;
	int SectorsX, SectorsY;
};

//////////////////////////////////////////////////////////////////////
#include "IGame.h"
#include <IScriptSystem.h>
#include <queue>
#include <set>
#include "EntityClassRegistry.h"
#include "StringTableMgr.h"
#include "XSurfaceMgr.h"
#include "XDemoMgr.h"
#include "XArea.h"

//////////////////////////////////////////////////////////////////////

class CXGame;
class CScriptObjectStream;

//////////////////////////////////////////////////////////////////////
typedef std::queue<string> StringQueue;
typedef std::set<string> StringSet;

class CXGame :
	public IXGame,
	public IServerSnooperSink,
	public INETServerSnooperSink,
	public IPhysicsStreamer,
	public IPhysicsEventClient
{
public:
	CXGame();
	virtual ~CXGame();
	void Reset();
	void SoftReset();

	// interface IGame ----------------------------------------------------------

	bool Init(struct ISystem *pSystem,bool bDedicatedSrv,bool bInEditor,const char *szGameMod);
	bool InitClassRegistry();
	bool Update();
	bool Run(bool &bRelaunch);
	void Release() { delete this; }
	const char *IsMODLoaded();
	IGameMods* GetModsInterface();

	void AllowQuicksave(bool bAllow) {m_bAllowQuicksave = bAllow;};
	bool IsQuicksaveAllowed(void) {return m_bAllowQuicksave;}

	//! \return 0 before was initialized, otherwise point to the GameMods
	CGameMods	*GetModsList() { return(m_pGameMods); }

	bool IsSoundPotentiallyHearable(Vec3d &SoundPos, float fClipRadius);
	void SetTerrainSurface(const char *sMaterial,int nSurfaceID){m_XSurfaceMgr.SetTerrainSurface(sMaterial,nSurfaceID);}
	//! load level for the editor
	bool LoadLevelForEditor(const char *pszLevelDirectory,const char *pszMissionName);
	bool GetLevelMissions( const char *szLevelDir,std::vector<string> &missions );

	virtual void UpdateDuringLoading();

	IScriptObject *GetScriptObject();

    // load level for the game
    void LoadLevelCS(bool reconnect, const char *szMapName, const char *szMissionName, bool listen);

	IEntity *GetMyPlayer();

	// tagpoint management functions
	ITagPoint *CreateTagPoint( const string &name, const Vec3 &pos, const Vec3 &angles);
	virtual ITagPoint *GetTagPoint(const string &name);
	void RemoveTagPoint(ITagPoint *pPoint);
	bool RenameTagPoint(const string &oldname, const string &newname);

	//real internal load level
	//return true if is loading a saved singleplayer level(this should disappear)
	bool IsLoadingLevelFromFile(){	return m_bIsLoadingLevelFromFile;	}
	ISystem *GetSystem() { return m_pSystem; }
	IScriptSystem *GetScriptSystem(){return m_pScriptSystem;}
	virtual IEntityClassRegistry *GetClassRegistry() { return &m_EntityClassRegistry;}

	CUIHud*	GetHud() const { return (CUIHud*)m_pUIHud; }
	CXServerRules* GetRules() const;
	IActionMapManager *GetActionMapManager(){ return m_pIActionMapManager; }
	IXSystem *GetXSystem();
	bool IsOK() { return m_bOK; }

	void OnSetVar(ICVar *pVar);
	// Servers management
	bool	StartupServer(bool listen, const char *szName = NULL);
	void	ShutdownServer();

	// Client management
	bool StartupClient();
	bool StartupLocalClient();
	void ShutdownClient();
	void MarkClientForDestruct();

	bool IsSynchronizing() { return m_bSynchronizing; }
	void AdvanceReceivedEntities(int iPhysicalWorldTime);
	bool HasScheduledEvents();
	void ScheduleEvent(int iPhysTime, BaseEvent *pEvent);
	void ScheduleEvent(IEntity *pEnt, CXEntityProcessingCmd &cmd);

	//! \param shooterSSID clientID 0..255 or -1 if unknown
	void ScheduleEvent(int iPhysTime, const Vec3& pos,float fDamage,float rmin,float rmax,float radius,float fImpulsivePressure,
										 float fShakeFactor,float fDeafnessRadius,float fDeafnessTime,float fImpactForceMul,
										 float fImpactForceMulFinal,float fImpactForceMulFinalTorso,
										 float rMinOcc,int nOccRes,int nOccGrow, IEntity *pShooter, int shooterSSID, IEntity *pWeapon,
										 float fTerrainDefSize,int nTerrainDecalId);
	void ScheduleEvent(int iPhysTime, IEntity *pVehicle,float fDamage);
	void ScheduleEvent(int iPhysTime, IPhysicalEntity *pPhysEnt,pe_action_impulse *pai);
	virtual void ExecuteScheduledEvents();
	void WriteScheduledEvents(CStream &stm, int &iLastEventWritten, int iTimeDelta=0);
	void ReadScheduledEvents(CStream &stm);
	bool UseFixedStep() { return m_iFixedStep2TimeGran!=0; }
	int GetiFixedStep() { return m_iFixedStep2TimeGran; }
	float GetFixedStep() { return g_MP_fixed_timestep->GetFVal(); }
	int QuantizeTime(float fTime)
	{
		return (int)(fTime*m_frTimeGran+0.5f*sgn(fTime));
	}
	int SnapTime(float fTime,float fOffset=0.5f)
	{
		return m_iFixedStep2TimeGran ?
			(int)(fTime*m_frFixedStep+0.5f*sgn(fTime))*m_iFixedStep2TimeGran : (int)(fTime*m_frTimeGran+0.5f*sgn(fTime));
	}
	int SnapTime(int iTime,float fOffset=0.5f)
	{
		return m_iFixedStep2TimeGran ?
			(int)(iTime*m_fTimeGran2FixedStep+0.5f*sgn(iTime))*m_iFixedStep2TimeGran : iTime;
	}

	//! \param shooterSSID clientID 0..255 or -1 if unknown
	void CreateExplosion(const Vec3& pos,float fDamage,float rmin,float rmax,float radius,float fImpulsivePressure,
											 float fShakeFactor,float fDeafnessRadius,float fDeafnessTime,
											 float fImpactForceMul,float fImpactForceMulFinal,float fImpactForceMulFinalTorso,
											 float rMinOcc,int nOccRes,int nOccGrow, IEntity *pShooter, int shooterSSID, IEntity *pWeapon,
											 float fTerrainDefSize,int nTerrainDecalId, bool bScheduled=false);

	//! IPhysicsStreamer (physics-on-demand) callback functions
	int CreatePhysicalEntity(void *pForeignData,int iForeignData,int iForeignFlags);
	int DestroyPhysicalEntity(IPhysicalEntity *pent);
	const char *GetForeignName(void *pForeignData,int iForeignData,int iForeignFlags);

	//////////////////////////////////////////////////////////////////////////
	//! physical events callback functions
	void OnBBoxOverlap(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData,
		IPhysicalEntity *pCollider, void *pColliderForeignData,int iColliderForeignData);
	void OnStateChange(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, int iOldSimClass,int iNewSimClass);
	void OnCollision(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, coll_history_item *pCollision);
	int OnImpulse(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, pe_action_impulse *action);
	void OnPostStep(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, float fTimeInterval);

	virtual IPhysicsStreamer *GetPhysicsStreamer() { return this; }
	virtual IPhysicsEventClient *GetPhysicsEventClient() { return this; }

	bool ConstrainToSandbox(IEntity *pEntity);

	// Ingame Dialogs
	CIngameDialogMgr *GetIngameDialogManager() { return m_pIngameDialogMgr; }

	void BindAction(const char *sAction,const char *sKeys,const char *sActionMap=NULL, int iKeyPos = -1);
	void BindActionMultipleMaps(const char *sAction,const char *sKeys, int iKeyPos = -1);
	bool CheckForAction(const char *sAction);
	void ClearAction(const char *sAction);
	// Stuffs...
	void	ProcessPMessages(const char *szMsg);

	// Return the version of the game
	const char *GetVersion();


	//! functions to know if the current terminal is a server and/or a client
	//@{
	bool	IsServer()	{	return m_pServer!=NULL;	}
	bool	IsClient();
	bool  IsMultiplayer();   // can be used for disabling cheats, or disabling features which cannot be synchronised over a network game
	bool	IsDevModeEnable();
	//@}
	//! save/laod game

	bool SaveToStream(CStream &stm, Vec3 *pos, Vec3 *angles,string sFilename);
	bool LoadFromStream(CStream &stm, bool isdemo);
	bool LoadFromStream_RELEASEVERSION(CStream &str, bool isdemo, CScriptObjectStream &scriptStream);
	bool LoadFromStream_PATCH_1(CStream &str, bool isdemo, CScriptObjectStream &scriptStream);

	void Save(string sFileName, Vec3 *pos, Vec3 *angles,bool bFirstCheckpoint=false );
	bool Load(string sFileName);
	void LoadConfiguration(const string &sSystemCfg,const string &sGameCfg);
	void SaveConfiguration( const char *sSystemCfg,const char *sGameCfg,const char *sProfile);
	void RemoveConfiguration(string &sSystemCfg,string &sGameCfg,const char *sProfile);

  void LoadLatest();

	virtual CXServer* GetServer() { return m_pServer; }

	CXClient* GetClient() { return m_pClient; }

	void SetViewAngles(const Vec3 &angles);

	CWeaponSystemEx *GetWeaponSystemEx(){ return m_pWeaponSystemEx; }
	CUISystem *GetUISystem(){ return m_pUISystem; };
	void ReloadWeaponScripts();
	CVehicleSystem *GetVehicleSystem(){ return m_pVehicleSystem; }
	CPlayerSystem *GetPlayerSystem(){ return m_pPlayerSystem; }

	IClient *CreateClient(IClientSink *pSink,bool bLocal=false)
	{
		return m_pNetwork->CreateClient(pSink,bLocal);
	}
	IServer *CreateServer(IServerSlotFactory *pSink,WORD nPort, bool listen){return m_pNetwork->CreateServer(pSink,nPort,listen);}

	bool GetPreviewMapPosition(float &x, float &y, float mapx, float mapy, float sizex, float sizey, float zoom, float center_x, float center_y, bool bRound);
	int GetSector(int nSectorsX, int nSectorsY, float x, float y);
	void DrawMapPreview(float mapx, float mapy, float sizex, float sizey, float zoom, float center_x, float center_y, float alpha, struct PreviewMapParams *pParams=NULL);
  void DrawRadar(float x, float y, float w, float h, float fRange, INT_PTR *pRadarTextures, _SmartScriptObject *pEntities, char *pRadarObjective);

	// Demo recording stuff ------------------------------------------------

	bool StartRecording(const char *sFileName);
	void StopRecording();
	bool AddDemoChunk(CStream &stm);
	void StartDemoPlay(const char *sFileName);
	void StopDemoPlay();
	void PlaybackChunk();

	// Weapons -------------------------------------------------------------

	INameIterator * GetAvailableWeaponNames();
	INameIterator * GetAvailableProjectileNames();
	bool AddWeapon(const char *pszName);
	bool RemoveWeapon(const char *pszName);
	void RemoveAllWeapons();
	bool AddEquipPack(XDOM::IXMLDOMNode *pPack);
	bool AddEquipPack(const char *pszXML);
	void SetPlayerEquipPackName(const char *pszPackName);
	void RestoreWeaponPacks();

	// Network -------------------------------------------------------------

	//!
	void RefreshServerList();
	//!
	void OnServerFound(CIPAddress &ip, const string &szServerInfoString, int ping);
	//!
	void OnNETServerFound(const CIPAddress &ip, const string &szServerInfoString, int ping);
	//!
	void OnNETServerTimeout(const CIPAddress &ip);
	//! \return might be 0
	INETServerSnooper *GetNETSnooper() { return m_pNETServerSnooper; };
	//! \return might be 0
	IRConSystem *GetIRConSystem() { return m_pRConSystem; };

	void SendMessage(const char *str){
		m_qMessages.push(str);
	}
	bool ExecuteScript(const char *sPath,bool bForceReload=false);

	void EnableUIOverlay(bool bEnable, bool bExclusiveInput);
	bool IsUIOverlay();

	bool	IsInMenu() { return m_bMenuOverlay; };		//!< checks if we are in menu or not
	void	GotoMenu(bool bTriggerOnSwitch = false);	//!< arranges the message queue so that the game goes to the menu
	void	GotoGame(bool bTriggerOnSwitch = false);	//!< arranges the message queue so that the game goes out of the menu
	void	MenuOn();																	//!< enables menu instantly (no message)
	void	MenuOff();																//!< disables menu instantly (no message)
	void	DeleteMessage(const char *szMessage);			//!< deletes all occurrences of a specified message from the message queue

	const char *GetLevelName() { return m_currentLevel.c_str(); }

	void  ResetInputMap();
	string GetLevelsFolder() const;

protected:
	void SetConfigToActionMap(const char *pszActionName, ...);
	//bool LoadMaterials(string sFolder);
	void	InitInputMap();
	void	InitConsoleCommands();
	void	InitConsoleVars();
	bool	IsInPause(IProcess *pProcess);
	//everything related to vehicle will be in another file
	//Marco's NOTE: should be the same for other cvars, code and includes,
	void	InitVehicleCvars();

	//set the common key bindings for the specified action map.
	//it reduces code redundancy and makes things more clear.
	void SetCommonKeyBindings(IActionMap *pActionMap);
	void OnCollectUserData(INT_PTR nValue,int nCookie);		//AMD Port

public:
	void ClearTagPoints();
	void SetCurrentUI(CUIHud *pUI);

	vector2f GetSubtitleSize(const string &szMessage, float sizex, float sizey, const string &szFontName = "default", const string &szFontEffect = "default");
	void WriteSubtitle(const string &szMessage, float x, float y, float sizex, float sizey, const color4f &cColor, const string &szFontName = "default", const string &szFontEffect = "default");

	bool											m_bIsLoadingLevelFromFile;//!<
	struct IActionMapManager*	m_pIActionMapManager;			//!<
	bool											m_bDedicatedServer;				//!<
	bool											m_bOK;										//!<
	bool											m_bUpdateRet;							//!<
	bool											m_bRelaunch;							//!<
	bool											m_bMovieSystemPaused;

	ISystem	*									m_pSystem;								//!< The system interface

	ILog *										m_pLog;										//!<
	I3DEngine	*								m_p3DEngine;							//!< The 3D engine interface
	CXServer *								m_pServer;								//!< The server of this computer
	CXClient *								m_pClient;								//!< The client of this computer


	int m_nDEBUG_TIMING;
	float m_fDEBUG_STARTTIMER;

	//!	The dummy client of this computer, required to get the list of servers if
	//! theres not a real client actually connected and playing

	IServerSnooper	*					m_pServerSnooper;					//!< used for LAN Multiplayer, to remove control servers
	INETServerSnooper	*				m_pNETServerSnooper;			//!< used for Internet Multiplayer, to remove control servers
	IRConSystem *							m_pRConSystem;						//!< used for Multiplayer, to remote control servers
	string										m_szLastAddress;
	bool											m_bLastDoLateSwitch;
	bool											m_bLastCDAuthentication;

	CUIHud *									m_pUIHud;									//!< Hud
	CUIHud *									m_pCurrentUI;							//!< for the current ui

	string										m_currentLevel;						//!< Name of current level.
	string										m_currentMission;					//!< Name of current mission.
	string										m_currentLevelFolder;			//!< Folder of the current level.


	// console variables -----------------------------------------------------------

#ifdef _INTERNET_SIMULATOR
	ICVar* g_internet_simulator_minping;
	ICVar* g_internet_simulator_maxping;
	ICVar* g_internet_simulator_packetloss;
#endif
	ICVar* g_MP_fixed_timestep;
	ICVar* g_maxfps;
	ICVar* cl_ThirdPersonRange;
	ICVar* cl_ThirdPersonRangeBoat;
	ICVar* cl_ThirdPersonAngle;

	ICVar* cl_ThirdPersonOffs;
	ICVar* cl_ThirdPersonOffsAngHor;
	ICVar* cl_ThirdPersonOffsAngVert;

	ICVar* cl_scope_flare;
	ICVar* cl_lazy_weapon;
	ICVar* cl_weapon_fx;
	ICVar* cl_projectile_light;
	ICVar* cl_weapon_light;
	ICVar* w_underwaterbubbles;

	ICVar* cl_ViewFace;
  ICVar* cl_display_hud;
  ICVar* cl_motiontracker;
  ICVar* cl_msg_notification;
	ICVar* cl_hud_name;
  ICVar* cl_hud_pickup_icons;
	ICVar* ai_num_of_bots;

	ICVar* p_name;
	ICVar* p_color;
	ICVar* p_model;
	ICVar* mp_model;
	ICVar* p_always_run;
	ICVar* g_language;
	ICVar* g_playerprofile;
	ICVar* g_serverprofile;
	ICVar* g_Render;
	ICVar* g_GC_Frequence;
	ICVar* g_GameType;
	ICVar* g_LeftHanded;
	ICVar* g_Gore;
	ICVar* g_InstallerVersion;


	ICVar* p_speed_run;
	ICVar* p_sprint_scale;
	ICVar* p_sprint_decoy;
	ICVar* p_sprint_restore_run;	// restore rate when normal run
	ICVar* p_sprint_restore_idle;	// restore rate whwen not running
	ICVar* p_speed_walk;
	ICVar* p_speed_crouch;
	ICVar* p_speed_prone;
	ICVar* p_jump_force;
	ICVar* p_jump_walk_time;
	ICVar* p_jump_run_time;
	ICVar* p_jump_walk_d;
	ICVar* p_jump_walk_h;
	ICVar* p_jump_run_d;
	ICVar* p_jump_run_h;
	ICVar* p_gravity_modifier;
//[KIRILL]	this console variable makes lean global - it has to be player's per-instance property
// to fix bug in MP with leaning
//	ICVar* p_lean;			// lean angle
	ICVar* p_lean_offset;
	ICVar* p_bob_pitch;
	ICVar* p_bob_roll;
	ICVar* p_bob_length;
	ICVar* p_bob_weapon;
	ICVar* p_bob_fcoeff;
	ICVar* p_waterbob_pitch;
	ICVar* p_waterbob_pitchspeed;
	ICVar* p_waterbob_roll;
	ICVar* p_waterbob_rollspeed;
	ICVar* p_waterbob_mindepth;
	ICVar* p_weapon_switch;
	ICVar* p_deathtime;
	ICVar* p_restorehalfhealth;

	ICVar* p_ai_runspeedmult;			//combat stance
	ICVar* p_ai_crouchspeedmult;
	ICVar* p_ai_pronespeedmult;
	ICVar* p_ai_rrunspeedmult;		//relaxed stance
	ICVar* p_ai_rwalkspeedmult;
	ICVar* p_ai_xrunspeedmult;		//stealth stance
	ICVar* p_ai_xwalkspeedmult;

	ICVar* p_lightrange;
	ICVar* p_lightfrustum;

	// player animation control parametrs
	ICVar* pa_leg_velmoving;
	ICVar* pa_leg_velidle;
	ICVar* pa_leg_idletime;
	ICVar* pa_leg_limitaim;
	ICVar* pa_leg_limitidle;
	ICVar* pa_blend0;
	ICVar* pa_blend1;
	ICVar* pa_blend2;
	ICVar* pa_forcerelax;
	ICVar* pa_spine;
	ICVar* pa_spine1;

	ICVar*	m_pCVarCheatMode;

	// limping state

	ICVar* p_limp;

	// flashlight stuff
	ICVar* pl_force;
	ICVar* pl_dist;
	ICVar* pl_intensity;
	ICVar* pl_fadescale;
	ICVar* pl_head;

	//mutants jump parameters
	ICVar* m_jump_vel;
	ICVar* m_jump_arc;

// boat
//*
	ICVar* b_dump;		//angular velocity - waiving
	ICVar* b_dumpRot;	//angular velocity - turning
	ICVar* b_dumpV;		//velocity
	ICVar* b_dumpVH;		//velocity
	ICVar* b_stand;
	ICVar* b_turn;
	ICVar* b_tilt;
	ICVar* b_speedV;
	ICVar* b_accelerationV;
	ICVar* b_float;
	ICVar* b_speedMinTurn;
	//	water waves param
	ICVar* b_wscale;
	ICVar* b_wscalew;
	ICVar* b_wmomentum;


	//	camera mode
	ICVar* b_camera;
/*
//fire/burnable
	ICVar* f_update;
	ICVar* f_draw;
	ICVar* f_drawDbg;
*/
	ICVar* g_LevelName;
	ICVar* g_StartLevel;
	ICVar* g_StartMission;

	ICVar *sv_port;
	ICVar *sv_mapcyclefile;
	ICVar *sv_cheater_kick;
	ICVar *sv_cheater_ban;

	ICVar *sv_timeout;
	ICVar	*cl_timeout;
	ICVar *cl_loadtimeout;
	ICVar	*cl_snooptimeout;
	ICVar	*cl_snoopretries;
	ICVar	*cl_snoopcount;

	ICVar* p_CameraSmoothVLimit;
	ICVar* p_CameraSmoothTime;
	ICVar* p_CameraSmoothScale;

	ICVar* p_LeaveVehicleImpuls;
	ICVar* p_LeaveVehicleBrake;
	ICVar* p_LeaveVehicleBrakeDelay;

	ICVar* p_AutoCenterDelay;
	ICVar* p_AutoCenterSpeed;

	ICVar* p_HitImpulse;
	ICVar* p_DeadBody;
	ICVar* p_RotateHead;
	ICVar* p_RotateMove;
	ICVar* p_HeadCamera;
	ICVar* p_EyeFire;
	ICVar* a_DrawArea;
	ICVar* a_LogArea;
	ICVar* cv_game_Difficulty;
	ICVar* cv_game_Aggression;
	ICVar* cv_game_Accuracy;
	ICVar* cv_game_Health;
	ICVar* cv_game_AllowAIMovement;
	ICVar* cv_game_AllAIInvulnerable;
	ICVar* cv_game_GliderGravity;
	ICVar* cv_game_GliderBackImpulse;
	ICVar* cv_game_GliderDamping;
	ICVar* cv_game_GliderStartGravity;
	ICVar* cv_game_physics_quality;

	ICVar* cv_game_subtitles;

	ICVar* g_timedemo_file;

	ICVar* h_drawbelow;	// show bboxes for static objects below helicopter

	ICVar* pl_JumpNegativeImpulse; //!< this represent the downward impulse power applied when the player reach the max height of the jump, 0 means no impulse.

	ICVar* e_deformable_terrain; //!< the Cvar is created in cry3dengine, this is just a pointer

	float w_recoil_speed_up;
	float w_recoil_speed_down;
	float w_recoil_max_degree;
	float w_accuracy_decay_speed;
	float w_accuracy_gain_scale;
	int w_recoil_vertical_only;

	CStringTableMgr m_StringTableMgr;
	CXSurfaceMgr m_XSurfaceMgr;
	CScriptTimerMgr *m_pScriptTimerMgr;

//	IXArea *CreateArea( const Vec3 *vPoints, const int count, const char** names, const int ncount, const int type=0, const float width=0.0f);
	IXArea *CreateArea( const Vec3 *vPoints, const int count, const std::vector<string>	&names,
											const int type=0, const int groupId=-1, const float width=0.0f, const float height=0.0f );
	IXArea *CreateArea( const Vec3& min, const Vec3& max, const Matrix44& TM, const std::vector<string>	&names,
											const int type=0, const int groupId=-1, const float width=0.0f);
	IXArea *CreateArea( const Vec3& center, const float radius, const std::vector<string>	&names,
											const int type=0, const int groupId=-1, const float width=0.0f);
	void DeleteArea( const IXArea *pArea );
	IXArea *GetArea( const Vec3 &point );

	//! detect areas the listener is in before system update
	void CheckSoundVisAreas();
	//! retrigger them if necessary after system update
	void RetriggerAreas();

	ILipSync* CreateLipSync();

	//! plays a cutscene sequence
	void PlaySequence(const char *pszName,bool bResetFX);
	//! stops the current cutscene
	void StopCurrentCutscene();


	ActionsEnumMap& GetActionsEnumMap() { return m_mapActionsEnum; }

	CXAreaMgr									m_XAreaMgr;
	ServerInfosMap						m_ServersInfos;							//!< Infos about the avaible servers
  string										m_strLastSaveGame;
	bool											m_bEditor;
	tPlayerPersistentData			m_tPlayerPersistentData;

	//! Rendering callback, called at render of frame.
	static void OnRenderCallback( void *pGame );

private: // ------------------------------------------------------------

	bool ParseLevelName(const char *szLevelName,char *szLevel,char *szMission);

	float										m_fFadingStartTime;
	char										m_szLoadMsg[512];
	bool										m_bAllowQuicksave;

	CEntityClassRegistry		m_EntityClassRegistry;
	IScriptSystem *					m_pScriptSystem;
	IRenderer *							m_pRenderer;
	IEntitySystem *					m_pEntitySystem;
	CScriptObjectGame *			m_pScriptObjectGame;
	CScriptObjectInput *		m_pScriptObjectInput;

	CScriptObjectBoids *		m_pScriptObjectBoids;
	CScriptObjectLanguage *	m_pScriptObjectLanguage;
	CScriptObjectAI *				m_pScriptObjectAI;
	CWeaponSystemEx *				m_pWeaponSystemEx;
	CVehicleSystem *				m_pVehicleSystem;
	CPlayerSystem *					m_pPlayerSystem;
	CIngameDialogMgr *			m_pIngameDialogMgr;
	CFlockManager *					m_pFlockManager;

	CMovieUser *						m_pMovieUser;
	CTimeDemoRecorder *			m_pTimeDemoRecorder;

	int											m_nPlayerIconTexId;
	int											m_nVehicleIconTexId;
	int											m_nBuildingIconTexId;
	int											m_nUnknownIconTexId;

	ActionsEnumMap					m_mapActionsEnum;				//!< Input Stuff(is for the client only but must be here)
	TagPointMap							m_mapTagPoints;					//!< Map of tag points by name

	INetwork *							m_pNetwork;
	CXDemoMgr								m_XDemoMgr;
	StringQueue							m_qMessages;
	bool										m_bMenuInitialized;

	std::vector<GameEvent>	m_lstEvents;
	float										m_fTimeGran;
	float										m_fFixedStep;
	float										m_fTimeGran2FixedStep;
	float										m_frFixedStep;
	float										m_frTimeGran;
	int											m_iFixedStep2TimeGran;
	int											m_iLastCmdIdx;
	bool										m_bSynchronizing;
	CBitStream_Base					m_BitStreamBase;						//!< for little compressed readwrite operation with CStreams (savegame,singleplayer,exported games,demo recording)
	CBitStream_Compressed		m_BitStreamCompressed;			//!< for compressed readwrite operation with CStreams (multiplayer)

	//! Name of the last saved checkpoint.
	string									m_sLastSavedCheckpointFilename;

	/*
	//! Time in seconds left to save thumbnail for the last saved checkpoint.
	//! Only used when saving first checkpoint, when at time of checkpoint save nothing is yet visibly on screen.
	float   m_fTimeToSaveThumbnail;
	*/

	CGameMods *							m_pGameMods;								//!< might be 0 (before game init)

public:

	void LoadingError(const char *szError);
	bool GetCDPath(string &szCDPath);

	//////////////////////////////////////////////////////////////////////////
	// DevMode.
	//////////////////////////////////////////////////////////////////////////
	void DevModeInit();
	void DevModeUpdate();
	void DevMode_SavePlayerPos( int index,const char *sTagName=NULL,const char *sDescription=NULL );
	void DevMode_LoadPlayerPos( int index,const char *sTagName=NULL );
	//////////////////////////////////////////////////////////////////////////

	// interface IGame ---------------------------------------------------------

	virtual void GetMemoryStatistics(ICrySizer *pSizer);
	virtual void SetViewMode(bool bThirdPerson);
	//Timur[10/2/2002]void SetScriptGlobalVariables(void);
	virtual void AddRespawnPoint(ITagPoint *pPoint);
	virtual void RemoveRespawnPoint(ITagPoint *pPoint);
	virtual void ResetState(void);
	virtual void HideLocalPlayer( bool hide,bool bEditor );
	virtual void ReloadScripts();
	virtual bool GoreOn() const;
	virtual IBitStream *GetIBitStream();

	//! sets a timer for a generic script object table
	int		AddTimer(IScriptObject *pTable,unsigned int nStartTimer,unsigned int nTimer,IScriptObject *pUserData,bool bUpdateDuringPause);
	void	PlaySubtitle(ISound * pSound);
	bool	OpenPacks(const char *szFolder);
	bool	ClosePacks(const char *szFolder);

	ListOfPlayers				m_PlayersWithLighs;						//!<

	bool								m_bHideLocalPlayer;						//!<
	CUISystem *					m_pUISystem;
	bool								m_bMenuOverlay;								//!<
	bool								m_bUIOverlay;									//!<
	bool								m_bUIExclusiveInput;					//!<
	bool								m_bMapLoadedFromCheckpoint;		//!<

	ListOfPlayers				m_DeadPlayers;

	virtual IXAreaMgr* GetAreaManager() { return &m_XAreaMgr; };
	virtual ITagPointManager* GetTagPointManager();

	ITagPointManager *m_pTagPointManager;
	string m_sGameName;

	virtual int GetInterfaceVersion() { return 1; };
	virtual string& GetName() { return m_sGameName; };

	virtual bool GetModuleState( EGameCapability eCap )
	{
		switch ( eCap )
		{
		case EGameMultiplayer:
			return IsMultiplayer();
		case EGameClient:
			return IsClient();
		case EGameServer:
			return IsServer();
		case EGameDevMode:
			return IsDevModeEnable();
		default:
			return false;
		}
	};
};

#endif // GAME_GAME_H