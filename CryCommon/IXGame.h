#ifndef __IXGAME_H__
#define __IXGAME_H__

#include "IGame.h"

/*	This interface exposes the basic functionality
to initialize and run the game.
*/
//////////////////////////////////////////////////////////////////////////
struct IXGame : public IGame
{
	/* Initialize game.
	@return true on success, false otherwise
	*/
	virtual bool Init(struct ISystem *pSystem,bool bDedicatedSrv,bool bInEditor,const char *szGameMod) = 0;
	/*Upadate the module and all subsystems
	@return false to stop the main loop 
	*/
	virtual bool Update() = 0;
	/*run the main loop until another subsystem force the exit
	@return false to stop the main loop 
	*/
	virtual bool Run(bool &bRelaunch) = 0;

	/* Returns current MOD
	NULL if FarCry, name of MOD otherwise
	*/
	virtual const char *IsMODLoaded() = 0;

	/* Returns interface to access Game Mod functionality.
	*/
	virtual IGameMods* GetModsInterface() = 0;

	/*Executes scheduled events, called by system before executing each fixed time step in multiplayer
	*/
	virtual void ExecuteScheduledEvents() = 0;
	/*Tells whether fixed timestep physics in multiplayer is on
	*/
	virtual bool UseFixedStep() = 0;
	/*Snaps to to fixed step 
	*/
	virtual int SnapTime(float fTime,float fOffset=0.5f) = 0;
	/*Snaps to to fixed step 
	*/
	virtual int SnapTime(int iTime,float fOffset=0.5f) = 0;
	/*returns fixed MP step in physworld time granularity
	*/
	virtual int GetiFixedStep() = 0;
	/*returns fixed MP step
	*/
	virtual float GetFixedStep() = 0;
	/*Shutdown and destroy the module (delete this)
	*/
	virtual void Release() = 0;
	/*Load level [level editor only]
	@param pszLevelDirectory level directory
	*/
	virtual bool LoadLevelForEditor(const char *pszLevelDirectory, const char *pszMissionName = 0) = 0;
	/* Check if a sound is potentially hearable (used to check if loading a dialog is needed)
	*/
	virtual bool IsSoundPotentiallyHearable(Vec3d &SoundPos, float fClipRadius)=0;
	/*Assign a material to a tarrain layer
	*/
	virtual void SetTerrainSurface(const char *sMaterial,int nLayerID)=0;
	/*Get the local player entity[editor only]
	*/
	virtual IEntity *GetMyPlayer() = 0;
	/*Get the entity class regitry
	*/
	virtual IEntityClassRegistry *GetClassRegistry() = 0;

	/*Set the angles of the view camera of the game
	*/
	virtual void SetViewAngles(const Vec3 &angles) = 0;

	/*Retrieves the player-system
	*/
	virtual class CPlayerSystem *GetPlayerSystem() = 0;

	/* This function creates a tag point in the game world
	*/
	virtual ITagPoint *CreateTagPoint(const string &name, const Vec3 &pos, const Vec3 &angles) = 0;

	/* Retieves a tag point by name
	*/
	virtual ITagPoint *GetTagPoint(const string &name) =0;

	/*	Deletes a tag point from the game
	*/
	virtual void RemoveTagPoint(ITagPoint *pPoint) = 0;

	// shape
	virtual IXArea *CreateArea( const Vec3 *vPoints, const int count, const std::vector<string>	&names, 
		const int type=0, const int groupId=-1, const float width=0.0f, const float height=0.0f) = 0;
	// box
	virtual IXArea *CreateArea( const Vec3& min, const Vec3& max, const Matrix44& TM, const std::vector<string>	&names, 
		const int type=0, const int groupId=-1, const float width=0.0f) = 0;
	// sphere
	virtual IXArea *CreateArea( const Vec3& center, const float radius, const std::vector<string>	&names, 
		const int type=0, const int groupId=-1, const float width=0.0f) = 0;
	//		const char* const className, const int type=0, const float width=0.0f) = 0;
	virtual void		DeleteArea( const IXArea *pArea ) = 0;
	virtual IXArea *GetArea( const Vec3 &point ) = 0;

	// detect areas the listener is in before system update
	virtual void		CheckSoundVisAreas()=0;
	// retrigger them if necessary after system update
	virtual void		RetriggerAreas()=0;

	/* Returns an enumeration of the currently available weapons
	*/
	virtual INameIterator * GetAvailableWeaponNames() = 0;
	virtual INameIterator * GetAvailableProjectileNames() = 0;

	/* Add a weapon and load it
	*/
	virtual bool AddWeapon(const char *pszName) = 0;

	/* Remove a loaded weapon by name
	*/
	virtual bool RemoveWeapon(const char *pszName) = 0;

	/* Remove all loaded weapons
	*/
	virtual void RemoveAllWeapons() = 0;

	virtual bool AddEquipPack(const char *pszXML) = 0;

	virtual void RestoreWeaponPacks() = 0;

	virtual void SetPlayerEquipPackName(const char *pszPackName) = 0;

	virtual void SetViewMode(bool bThirdPerson) = 0;

	virtual void AddRespawnPoint(ITagPoint *pPoint) = 0;
	virtual void RemoveRespawnPoint(ITagPoint *pPoint) = 0;
	virtual void OnSetVar(ICVar *pVar)=0;
	virtual void SendMessage(const char *s)=0;
	virtual void ResetState() = 0;
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;
	virtual void HideLocalPlayer( bool hide,bool bEditor ) = 0;

	// saves player configuration
	virtual void SaveConfiguration( const char *sSystemCfg,const char *sGameCfg,const char *sProfile)=0;

	/* This is used by editor for changing properties from scripts (no restart).
	*/
	virtual void ReloadScripts()=0;

	// sets a timer for a generic script object table
	virtual int AddTimer(IScriptObject *pTable,unsigned int nStartTimer,unsigned int nTimer,IScriptObject *pUserData,bool bUpdateDuringPause)=0;

	virtual CXServer *GetServer() = 0;

	// functions to know if the current terminal is a server and/or a client
	virtual bool IsServer() = 0;
	virtual bool IsClient() = 0;
	virtual bool IsMultiplayer()=0;   // can be used for disabling cheats, or disabling features which cannot be synchronised over a network game
	virtual bool IsDevModeEnable()=0;

	virtual void EnableUIOverlay(bool bEnable, bool bExclusiveInput) = 0;
	virtual bool IsUIOverlay() = 0;
	virtual bool IsInMenu() = 0;
	virtual void GotoMenu(bool bTriggerOnSwitch = false) = 0;
	virtual void GotoGame(bool bTriggerOnSwitch = false) = 0;

	// functions return callback sinks for the physics
	virtual IPhysicsStreamer *GetPhysicsStreamer() = 0;
	virtual IPhysicsEventClient *GetPhysicsEventClient() = 0;

	// checks if gore enabled in the game
	virtual bool GoreOn() const = 0;

	// for compressed readwrite operation with CStreams 
	// /return you can assume the returned pointer is always valid
	virtual IBitStream *GetIBitStream()=0;

	// is called from time to time during loading (usually network updates)
	// currently only called for server map loading
	virtual void UpdateDuringLoading()=0;

	virtual bool GetModuleState( EGameCapability eCap ) = 0;

	virtual string& GetName() = 0;
	virtual int GetInterfaceVersion() = 0;
};

//DOC-IGNORE-BEGIN
//## Macros used to access FarCry specific elements from it's previous IGame interface

#define CheckFarCryIGame_Begin( pGame ) \
{ \
	if ( pGame != NULL && pGame->GetInterfaceVersion() == 1 ) \
	{

#define CheckFarCryIGame_End() \
	}\
}

#define GetIXGame( pGame )   ((IXGame*)pGame)

//DOC-IGNORE-END

#endif