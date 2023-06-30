//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: IGame.h
//  Description: Game Interface.
//
//  History:
//  - 08/08/2001: Created by Alberto Demichelis
//  - 09/24/2001: Modified by Petar Kotevski
//  - 12/14/2003: MartinM made ClassID from unsigned char(8bit) to EntityClassId (16bit))
//  - 27/04/2004: First cleanup by Mathieu Pinard
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_IGAME_H
#define GAME_IGAME_H
#if _MSC_VER > 1000
# pragma once
#endif

#ifdef WIN32 
#ifdef CRYGAME_EXPORTS
#define CRYGAME_API __declspec(dllexport)
#else
#define CRYGAME_API __declspec(dllimport)
#endif
#else	//WIN32
#define CRYGAME_API
#endif

#if defined(LINUX)
#	include "platform.h"
#	include "Cry_Math.h"
#	include <vector>
#endif

//#include "IEntitySystem.h"				// EntityClassId
#include <CryVersion.h>

#define PLAYER_CLASS_ID						1
#define ADVCAMSYSTEM_CLASS_ID			97			// is this the right place to put that define?
#define SPECTATOR_CLASS_ID				98			//
#define SYNCHED2DTABLE_CLASS_ID		205			//

//////////////////////////////////////////////////////////////////////////
typedef unsigned short EntityClassId;			//< unique identifier for the entity class (defined in ClassRegistry.lua)

//////////////////////////////////////////////////////////////////////////
struct	I3DEngine;
struct	ISystem;
struct	ITagPoint;
struct	IXArea;
class   CXArea;
struct	IXAreaMgr;
class	CXAreaMgr;
struct	ILipSync;
struct	ICVar;
class		CXServer;
struct	IBitStream;
class		ICrySizer; 
struct	ISound;
struct  IScriptObject;
struct	IEntity;

/*
This structure stores the informations to identify an entity type
@see CEntityClassRegistry
*/
//////////////////////////////////////////////////////////////////////////
struct EntityClass
{
	// type id
	EntityClassId				ClassId;
	// class name inside the script file
	string							strClassName;
	// script relative file path
	string							strScriptFile;
	// script fully specified file path (Relative to root folder).
	string							strFullScriptFile;
	// Game type of this entity (Ex. Weapon,Player).
	string							strGameType;
	//specify that this class is not level dependent and is not loaded from LevelData.xml
	bool								bReserved;
	//
	bool								bLoaded;

	EntityClass() { ClassId = 0;bLoaded=false; }
	// Copy constrctor required by STL containers.
	EntityClass( const EntityClass &ec ) { *this = ec; }
	// Copy operator required by STL containers.
	EntityClass& operator=( const EntityClass &ec )
	{
		bReserved=ec.bReserved;
		ClassId = ec.ClassId;
		strClassName = ec.strClassName;
		strScriptFile = ec.strScriptFile;
		strFullScriptFile = ec.strFullScriptFile;
		strGameType = ec.strGameType;
		bLoaded=ec.bLoaded;
		return *this;
	}
};


/* This interface allow to load or create new entity types
@see CEntityClassRegistry
*/
struct IEntityClassRegistry
{
	/*Retrieves an entity class by name
	@param str entity name
	@return EntityClass ptr if succeded, NULL if failed
	*/
	virtual EntityClass *GetByClass(const char *sClassName,bool bAutoLoadScript=true)= 0;
	//virtual EntityClass *GetByClass(const string &str)= 0;
	/*Retrieves an entity class by ClassId
	@param ClassId class id
	@return EntityClass ptr if succeded, NULL if failed
	*/
	virtual EntityClass *GetByClassId(const EntityClassId ClassId,bool bAutoLoadScript=true)= 0;
	/*Adds a class type into the registry
	@param ClassId class id
	@param sClassName class name(into the script file)
	@param sScriptFile script file path
	@param pLog pointer to the log interface
	@param bForceReload if set to true force script to be eloaded for already registered class.
	@return true if added, false if failed
	*/
	virtual bool AddClass(const EntityClassId ClassId,const char* sClassName,const char* sScriptFile,bool bReserved=false,bool bForceReload=false) = 0;

	/*move the iterator to the begin of the registry
	*/
	virtual void MoveFirst() = 0;
	/*get the next entity class into the registry
	@return a pointer to the next entityclass, or NULL if is the end
	*/
	virtual EntityClass *Next() = 0;
	/*return the count of the entity classes
	@return the count of the entity classes
	*/
	virtual int Count() = 0;

	virtual bool LoadRegistryEntry(EntityClass *pClass, bool bForceReload=false) = 0;
	// debug to OutputDebugString()
	virtual void Debug()=0;
};

struct INameIterator
{
	virtual void Release() = 0;
	virtual void MoveFirst() = 0;
	virtual bool MoveNext() = 0;
	virtual bool Get(char *pszBuffer, INT *pSize) = 0;
};

class IPhysicsStreamer;
class IPhysicsEventClient;

//////////////////////////////////////////////////////////////////////////
// MOD related

// flags
#define MOD_NEWGAMEDLL	1L<<1 //tells if the MOD contains a replacement of CryGame.dll

// Description of the Game MOD.
//////////////////////////////////////////////////////////////////////////
struct SGameModDescription
{
	// Constructor.
	SGameModDescription() 
	{
		dwFlags=0;
	};

	// Mod's name.
	string sName;
	// Mod's title.
	string sTitle;
	// Folder where this mod is located.
	string sFolder;
	// Mod's author.
	string sAuthor;
	// Mod Version.
	SFileVersion version;
	// Mod description.
	string sDescription;
	// Website.
	string sWebsite;
	// Mod flags
	int	dwFlags;
};

// Interface to access Game modifications parameters.
//////////////////////////////////////////////////////////////////////////
struct IGameMods
{
	// Returns description of the currently active game mode.
	// @returns NULL if the game mod is not found.
	virtual const SGameModDescription* GetModDescription( const char *sModName ) const = 0;
	// @returns name of the mod currently active, never returns 0
	virtual const char* GetCurrentMod() const = 0;
	// Sets the currently active game mod.
	// @returns true if Mod is successfully set, false if Mod set failed.
	virtual bool SetCurrentMod( const char *sModName,bool bNeedsRestart=false ) = 0;
	// Returns modified path for the currently active mod/tc (if any)
	// @returns true if there is an active mod, false otherwise
	virtual const char* GetModPath(const char *szSource)= 0;
};

struct ITagPointManager
{
	// This function creates a tag point in the game world
	virtual ITagPoint *CreateTagPoint(const string &name, const Vec3 &pos, const Vec3 &angles) = 0;

	// Retrieves a tag point by name
	virtual ITagPoint *GetTagPoint(const string &name) =0;

	// Deletes a tag point from the game
	virtual void RemoveTagPoint(ITagPoint *pPoint) = 0;

	virtual void AddRespawnPoint(ITagPoint *pPoint) = 0;
	virtual void RemoveRespawnPoint(ITagPoint *pPoint) = 0;
};

enum EGameCapability
{
	EGameMultiplayer = 1,
	EGameClient,
	EGameServer,
	EGameDevMode,
};

//	Exposes the basic functionality to initialize and run the game.
struct IGame
{
	//########################################################################
	//## EXTREMELY IMPORTANT: Do not modify anything below, else the binary 
	//##                      compatibility with the gold version of Far Cry 
	//##                      will be broken.

	// Summary: Initialize game.
	// Returns: true on success, false otherwise
	virtual bool Init( struct ISystem *pSystem, bool bDedicatedSrv, bool bInEditor, const char *szGameMod ) = 0;

	// Summary: Update the module and all subsystems
	// Returns: false to stop the main loop 
	virtual bool Update() = 0;

	// Summary: Run the main loop until another subsystem force the exit
	// Returns: false to stop the main loop 
	virtual bool Run( bool &bRelaunch ) = 0;

	// Summary: Determines if a MOD is currently loaded
	// Returns: A string holding the name of the MOD if one is loaded, else 
	//          NULL will be returned if only Far Cry is loaded.
	virtual const char *IsMODLoaded() = 0;

	// Returns interface to access Game Mod functionality.
	virtual IGameMods* GetModsInterface() = 0;

	//## EXTREMELY IMPORTANT: Do not modify anything above, else the binary 
	//##                      compatibility with the gold version of Far Cry 
	//##                      will be broken.
	//########################################################################

	//Shutdown and destroy the module (delete this)
	virtual void Release() = 0;

	// Executes scheduled events, called by system before executing each fixed time step in multiplayer
	virtual void ExecuteScheduledEvents() = 0;
	
	// Tells whether fixed timestep physics in multiplayer is on
	virtual bool UseFixedStep() = 0;
	
	// Snaps to to fixed step 
	virtual int SnapTime(float fTime,float fOffset=0.5f) = 0;

	// Snaps to to fixed step 
	virtual int SnapTime(int iTime,float fOffset=0.5f) = 0;
	
	// returns fixed MP step in physworld time granularity
	virtual int GetiFixedStep() = 0;
	
	// returns fixed MP step
	virtual float GetFixedStep() = 0;
	
	// Load level [level editor only]
	// @param pszLevelDirectory level directory
	virtual bool LoadLevelForEditor(const char *pszLevelDirectory, const char *pszMissionName = 0) = 0;

	// Get the entity class regitry
	virtual IEntityClassRegistry *GetClassRegistry() = 0;

	virtual void OnSetVar(ICVar *pVar)=0;
	virtual void SendMessage(const char *s)=0;
	virtual void ResetState() = 0;
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;

	// saves player configuration
	virtual void SaveConfiguration( const char *sSystemCfg,const char *sGameCfg,const char *sProfile)=0;

	// This is used by editor for changing properties from scripts (no restart).
	virtual void ReloadScripts()=0;

	virtual bool GetModuleState( EGameCapability eCap ) = 0;

	// functions return callback sinks for the physics
	virtual IPhysicsStreamer *GetPhysicsStreamer() = 0;
	virtual IPhysicsEventClient *GetPhysicsEventClient() = 0;

	// is called from time to time during loading (usually network updates)
	// currently only called for server map loading
	virtual void UpdateDuringLoading()=0;

	//virtual ITagPointManager* GetTagPointManager();
	virtual IXAreaMgr* GetAreaManager() = 0;
	virtual ITagPointManager* GetTagPointManager() = 0;
};

#ifdef GAME_IS_FARCRY
	#include "IXGame.h"
#endif

typedef IGame* (*PFNCREATEGAMEINSTANCE)();
// interface of the DLL
extern "C" CRYGAME_API IGame* CreateGameInstance();



#endif // GAME_IGAME_H
