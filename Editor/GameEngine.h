////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gameengine.h
//  Version:     v1.00
//  Created:     13/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __gameengine_h__
#define __gameengine_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** This class serves as a high-level wrapper arround Far Cry game engine.
*/
class CGameEngine
{
public:
	CGameEngine();
	~CGameEngine(void);

	/** Initialize System.
			@return true if Initializion succeded, false overwise.
	*/
	bool Init( bool bPreviewMode,bool bTestMode,const char *sCmdLine );

	/** Initialize game.
			@return true if Initializion succeded, false overwise.
	*/
	bool InitGame( const char *sGameDLL );
	
	/** Load new terrain level into 3d engine.
			Also load AI triangulation for this level.
	*/
	bool LoadLevel( const CString &levelPath,const CString &mission,bool bDeleteAIGraph,bool bReleaseResources );

	/** Reload level if it was already loaded.
	*/
	bool ReloadLevel();

	/** Loads AI triangulation.
	*/
	bool LoadAI( const CString &levelName,const CString &missionName );

	/** Load new mission.
	*/
	bool LoadMission( const CString &mission );
	
	/** Reload environment settings in currently loaded level.
	*/
	bool ReloadEnvironment();

	/** Switch In/Out of game mode.
			@param inGame When true editor switch to game mode.
	*/
	void SetGameMode( bool inGame );

	/** Switch In/Out of AI and Physics simulation mode.
			@param enabled When true editor switch to simulation mode.
	*/
	void SetSimulationMode( bool enabled );

	/** Get current simulation mode.
	*/
	bool GetSimulationMode() const { return m_simulationMode; };

	/** Returns true if level is loaded.
	*/
	bool IsLevelLoaded() const { return m_bLevelLoaded; };

	/** Assign new level path name.
	*/
	void SetLevelPath( const CString &path );

	/** Assign new current mission name.
	*/
	void SetMissionName( const CString &mission );

	/** Return name of currently loaded level.
	*/
	const CString& GetLevelName() const { return m_levelName; };
	
	/** Return name of currently active mission.
	*/
	const CString& GetMissionName() const { return m_missionName; };

	/** Get fully specified level path.
	*/
	const CString& GetLevelPath() const { return m_levelPath; };

	/** Query if engine is in game mode.
	*/
	bool IsInGameMode() const { return m_inGameMode; };

	/** Force level loaded variable to true.
	*/
	void SetLevelLoaded( bool bLoaded ) { m_bLevelLoaded = bLoaded; }

	/** Generate AI Triangulation of currently loaded data.
	*/
	void GenerateAiTriangulation();

	/** Forces all entities to be reregistered in sectors again.
	*/
	void ForceRegisterEntitiesInSectors();

	/** Put surface types from Editor to game.
	*/
	void PutSurfaceTypesToGame( bool bExportToEngine=false );

	/** Sets equipment pack current used by player.
	*/
	void SetPlayerEquipPack( const char *sEqipPackName );

	/** Called when Game resource file must be reloaded.
	*/
	void ReloadResourceFile( const CString &filename );

	/** Query ISystem interface.
	*/
	ISystem* GetSystem() { return m_ISystem; };

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//! Set player position in game.
	//! @param bEyePos If set then givven position is position of player eyes.
	void SetPlayerPos( const Vec3 &pos,bool bEyePos=true );
	//! Set player angles in game.
	void SetPlayerAngles( const Vec3 &angles );
	
	//! When set, player in game will be every frame synchronized with editor camera.
	void SyncPlayerPosition( bool bEnable );
	bool IsSyncPlayerPosition() const { return m_syncPlayerPosition; };

	//////////////////////////////////////////////////////////////////////////
	// Game MOD support.
	//////////////////////////////////////////////////////////////////////////
	//! Set game's current Mod name.
	void SetCurrentMOD( const char *sMod );
	//! Returns game's current Mod name.
	const char* GetCurrentMOD() const;

	//! Called every frame.
	void Update();

	//////////////////////////////////////////////////////////////////////////
private:
	/** Completly Reset Game and Editor rendering resources for next level.
	*/
	void ResetResources();
  
private:
	CString m_levelName;
	CString m_missionName;
	CString m_levelPath;
	CString m_MOD;

	bool m_bLevelLoaded;
	bool m_inGameMode;
	bool m_simulationMode;
	bool m_syncPlayerPosition;

	ISystem *m_ISystem;
	I3DEngine *m_I3DEngine;
	IAISystem *m_IAISystem;
	IEntitySystem *m_IEntitySystem;
	IGame* m_IGame;
	bool m_bGameInitialized;

	Vec3 m_playerPos;
	Vec3 m_playerAngles;

	struct SSystemUserCallback* m_pSystemUserCallback;

	HMODULE m_hSystemHandle;
};


#endif // __gameengine_h__