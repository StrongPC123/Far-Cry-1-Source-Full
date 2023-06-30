////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gameengine.cpp
//  Version:     v1.00
//  Created:     13/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "gameengine.h"

#include "CryEditDoc.h"
#include "Objects\ObjectManager.h"
#include "Objects\EntityScript.h"
#include "Mission.h"
#include "GameExporter.h"
#include "SurfaceType.h"
#include "VegetationMap.h"
#include "Heightmap.h"
#include "TerrainGrid.h"
#include "EdMesh.h"

#include "AI\AIManager.h"
#include "Material\MaterialManager.h"
#include "Particles\ParticleManager.h"

#include <IAgent.h>
#include <I3DEngine.h>
#include <IAISystem.h>
#include <IEntitySystem.h>
#include <EntityDesc.h>
#include <IMovieSystem.h>
#include <IGame.h>
#include <IInput.h>
#include <IScriptSystem.h>
#include <ICryPak.h>

#include <IDataProbe.h>


//////////////////////////////////////////////////////////////////////////
// Implementation of System Callback structure.
//////////////////////////////////////////////////////////////////////////
struct SSystemUserCallback : public ISystemUserCallback
{
	bool OnError( const char *szErrorString )
	{
		if (szErrorString)
			Log( szErrorString );
		if (GetIEditor()->IsInTestMode())
		{
			abort();
		}
		char str[4096];
		if (szErrorString)
			sprintf( str,"%s\r\nSave Level Before Exiting the Editor?",szErrorString );
		else
			sprintf( str,"Unknown Error\r\nSave Level Before Exiting the Editor?" );
		int res = MessageBox( NULL,str,"Engine Error",MB_YESNOCANCEL|MB_ICONERROR|MB_TOPMOST|MB_APPLMODAL|MB_DEFAULT_DESKTOP_ONLY);
		if (res == IDYES || res == IDNO)
		{
			if (res == IDYES)
			{
				if (GetIEditor()->SaveDocument())
					MessageBox( NULL,"Level has been sucessfully saved!\r\nPress Ok to terminate Editor.","Save",MB_OK );
			}
		}
		return true;
	}
	void OnSaveDocument()
	{
		GetIEditor()->SaveDocument();
	}
	void OnProcessSwitch()
	{
		if (GetIEditor()->IsInGameMode())
			GetIEditor()->SetInGameMode(false);
	}
};

//////////////////////////////////////////////////////////////////////////
// Implments EntitySystemSink for InGame mode.
//////////////////////////////////////////////////////////////////////////
struct SInGameEntitySystemListener  : public IEntitySystemSink
{
	SInGameEntitySystemListener() {}
	~SInGameEntitySystemListener()
	{
		// Remove all remaining entities from entity system.
		IEntitySystem *pEntitySystem = GetIEditor()->GetSystem()->GetIEntitySystem();
		for (std::set<int>::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
		{
			pEntitySystem->RemoveEntity(*it, true);
		}
	}

	virtual void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity) {};
	virtual void OnSpawn( IEntity *e, CEntityDesc &ed  )
	{
		if(ed.ClassId!=0 && ed.ClassId!=PLAYER_CLASS_ID) // Ignore MainPlayer
		{
			m_entities.insert(e->GetId());
		}
	}
	virtual void OnRemove( IEntity *e )
	{
		m_entities.erase(e->GetId());
	}
	virtual void OnBind(EntityId id,EntityId child,unsigned char param){}
	virtual void OnUnbind(EntityId id,EntityId child,unsigned char param){}
	// Ids of all spawned entities.
	std::set<int> m_entities;
};

namespace
{
	SInGameEntitySystemListener* s_InGameEntityListener = 0;
};

//////////////////////////////////////////////////////////////////////////
// Timur.
// This is FarCry.exe authentication function, this code is not for public release!!
//////////////////////////////////////////////////////////////////////////
static void GameSystemAuthCheckFunction( void *data )
{
	// src and trg can be the same pointer (in place encryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit:  int key[4] = {n1,n2,n3,n4};
	// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) {\
	register unsigned int y=v[0],z=v[1],n=32,sum=0; \
	while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

	// src and trg can be the same pointer (in place decryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit: int key[4] = {n1,n2,n3,n4};
	// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) { \
	register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32; \
	while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

	// Data assumed to be 32 bytes.
	int key1[4] = {1178362782,223786232,371615531,90884141};
	TEA_DECODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key1 );
	int key2[4] = {89158165, 1389745433,971685123,785741042};
	TEA_ENCODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key2 );
}

//////////////////////////////////////////////////////////////////////////
CGameEngine::CGameEngine()
{
	m_ISystem = NULL;
	m_IGame = NULL;

	m_I3DEngine = 0;
	m_IAISystem = 0;
	m_IEntitySystem = 0;

	m_bLevelLoaded = false;
	m_inGameMode = false;
	m_simulationMode = false;
	m_syncPlayerPosition = true;
	m_bGameInitialized = false;

	m_hSystemHandle = 0;

	m_pSystemUserCallback = new SSystemUserCallback;

	m_levelName = "Untitled";
}

//////////////////////////////////////////////////////////////////////////
CGameEngine::~CGameEngine()
{
	// Release all EdMEshes.
	CEdMesh::ReleaseAll();

	delete m_pSystemUserCallback;

	// Disable callback to editor.
	m_ISystem->GetIMovieSystem()->SetCallback( NULL );
	
	if (m_ISystem)
		m_ISystem->Release();
	if (m_hSystemHandle)
		FreeLibrary(m_hSystemHandle);

	//////////////////////////////////////////////////////////////////////////
	// Delete entity registry.
	//////////////////////////////////////////////////////////////////////////
	CEntityScriptRegistry::Instance()->Release();;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::Init( bool bPreviewMode,bool bTestMode, const char *sInCmdLine )
{
	m_hSystemHandle = LoadLibrary( "CrySystem.dll" );
	if (!m_hSystemHandle)
	{
		Error( "CrySystem.DLL Loading Failed" );
		return false;
	}

	PFNCREATESYSTEMINTERFACE pfnCreateSystemInterface = 
		(PFNCREATESYSTEMINTERFACE)::GetProcAddress( m_hSystemHandle,"CreateSystemInterface" );


	SSystemInitParams sip;
	sip.bEditor = true;
	sip.bPreview = bPreviewMode;
	sip.bTestMode = bTestMode;
	sip.hInstance = AfxGetInstanceHandle();
	if (AfxGetMainWnd())
		sip.hWnd = AfxGetMainWnd()->GetSafeHwnd();
	sip.pLog = new CLogFile;
	sip.pUserCallback = m_pSystemUserCallback;
	sip.pValidator = GetIEditor()->GetErrorReport(); // Assign validator from Editor.
	if (sInCmdLine)
		strcpy( sip.szSystemCmdLine,sInCmdLine );
	sip.pCheckFunc = &GameSystemAuthCheckFunction;

	m_ISystem = pfnCreateSystemInterface( sip );
	if (!m_ISystem)
	{
		Error( "CreateSystemInterface Failed" );
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::InitGame( const char *sGameDLL )
{
	m_I3DEngine = m_ISystem->GetI3DEngine();
	m_IAISystem = m_ISystem->GetAISystem();
	m_IEntitySystem = m_ISystem->GetIEntitySystem();


	SGameInitParams ip;
	ip.sGameDLL = sGameDLL;	

	if (!m_ISystem->CreateGame( ip ))
	{
		Error( "CreateGame Failed %s",sGameDLL );
		return false;
	}
	
	m_IGame = m_ISystem->GetIGame();
	/*
	m_IGame->LoadLevel( "Editor" );

	m_ISystem->IgnoreUpdates( true );
	
	//@HACK: Update game several times to insure client connection.
	for (int i = 0; i < 10; i++)
	{
		m_IGame->Update();
	}
	m_ISystem->IgnoreUpdates( false );
	*/

	/*
	// Make main console font use realpixels.
	IFFont *pFont = GetIEditor()->GetSystem()->GetICryFont()->GetFont("Default");
	if (pFont)
	{
		pFont->UseRealPixels(true);
	}
	*/

	// Enable displaying of labels.
	//@HACK!: @FIXME
	ICVar* pCvar = m_ISystem->GetIConsole()->GetCVar("r_DisplayInfo");
	if (pCvar) pCvar->Set("1");

	// Now initialize our AI.
	GetIEditor()->GetAI()->Init( m_ISystem );
	//! Remove indoor nodes from loaded graph.
//	GetAISystem()->GetNodeGraph()->RemoveIndoorNodes();

	//////////////////////////////////////////////////////////////////////////
	// Execute Editor.lua ovrride file.
	IScriptSystem *pScriptSystem = m_ISystem->GetIScriptSystem();
	pScriptSystem->ExecuteFile("Editor.Lua",false);
	//////////////////////////////////////////////////////////////////////////

	CEntityScriptRegistry::Instance()->LoadScripts();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetLevelPath( const CString &path )
{
	CString relPath = GetIEditor()->GetRelativePath(path);
	if (relPath.IsEmpty())
		relPath = path;

	m_levelPath = Path::RemoveBackslash(relPath);
	m_levelName = Path::RemoveBackslash(relPath);

	if (m_I3DEngine)
		m_I3DEngine->SetLevelPath( m_levelPath );
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetMissionName( const CString &mission )
{
	m_missionName = mission;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::LoadLevel( const CString &levelPath,const CString &mission,bool bDeleteAIGraph,bool bReleaseResources )
{
	// In case 3d engine was not been initialized before.
	m_I3DEngine = m_ISystem->GetI3DEngine();

	////////////////////////////////////////////////////////////////////////
	// Load a map inside the engine
	////////////////////////////////////////////////////////////////////////
	SetLevelPath( levelPath );
	m_missionName = mission;

	CLogFile::FormatLine("Loading map '%s' into engine...", (const char*)m_levelPath );

	if (bReleaseResources)
		ResetResources();

	// Switch the current directory back to the Master CD folder first.
	// The engine might have trouble to find some files when the current
	// directory is wrong
	SetCurrentDirectory( GetIEditor()->GetMasterCDFolder() );

	CString pakFile = m_levelPath + "/*.pak";
	// Open Pak file for this level.
	if (!m_ISystem->GetIPak()->OpenPacks( pakFile ))
	{
		// Pak1 not found.
		//CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,"Level Pack File %s Not Found",sPak1.c_str() );
	}

	// Initialize physics grid.
	if (bReleaseResources)
	{
		if (m_ISystem->GetIPhysicalWorld())
		{
			float fCellSize = 4.0f;
			SSectorInfo si;
			GetIEditor()->GetHeightmap()->GetSectorsInfo( si );
			float terrainSize = si.sectorSize * si.numSectors;
			m_ISystem->GetIPhysicalWorld()->SetupEntityGrid( 2,Vec3(0,0,0),terrainSize/fCellSize,terrainSize/fCellSize,fCellSize,fCellSize );
		}
	}

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Load level in 3d engine.
	//////////////////////////////////////////////////////////////////////////
	if (!m_I3DEngine->LoadLevel( m_levelPath,m_missionName,true))
	{
		CLogFile::WriteLine("ERROR: Can't load level !");
		AfxMessageBox( "ERROR: Can't load level !" );
		return false;
	}
	m_bLevelLoaded = true;

	CHeightmap *pHeightmap = GetIEditor()->GetHeightmap();
	if (pHeightmap)
	{
		// Set to heightmap size of currently loaded texture.
		/*
		int texSize = m_I3DEngine->GetTerrainTextureDim();
		if (texSize > 0)
		pHeightmap->SetSurfaceTextureSize(texSize,texSize);
		*/

		// Initialize terrain grid.
		SSectorInfo si;
		pHeightmap->GetSectorsInfo(si);
		pHeightmap->GetTerrainGrid()->InitSectorGrid( si.numSectors );
	}


	HEAP_CHECK
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_REFRESH );
	HEAP_CHECK
		//GetIEditor()->GetSystem()->GetI3DEngine()->RemoveAllStaticObjects();

		CVegetationMap *vegMap = GetIEditor()->GetVegetationMap();
	if (vegMap)
		vegMap->PlaceObjectsOnTerrain();

	//GetIEditor()->GetStatObjMap()->PlaceObjectsOnTerrain();
	if (bDeleteAIGraph)
	{
		m_IAISystem->FlushSystem();
		m_IAISystem->LoadTriangulation( 0,0 );
		//if (!LoadAI( m_levelName,m_missionName ))
		//return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::ReloadLevel()
{
	if (!IsLevelLoaded())
		return false;
	if (!LoadLevel( GetLevelPath(),GetMissionName(),false,false ))
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::LoadAI( const CString &levelName,const CString &missionName )
{
	if (!m_IAISystem)
		return false;
	if (!IsLevelLoaded())
		return false;

	CLogFile::FormatLine( "Loading AI Triangulation %s, %s",(const char*)levelName,(const char*)missionName );

	m_IAISystem->FlushSystem();
	GetIEditor()->GetObjectManager()->SendEvent( EVENT_CLEAR_AIGRAPH );
	m_IAISystem->LoadTriangulation( levelName,missionName );
	m_IAISystem->GetNodeGraph()->RemoveIndoorNodes();

	/*
	// Call OnReset for all entities.
	IEntityIt *entityIt = m_IEntitySystem->GetEntityIterator();
	if (entityIt)
	{
		for (IEntity *entity = entityIt->Next(); entity != 0; entity = entityIt->Next())
		{
			CLogFile::FormatLine( "Reseting: %s, Class: %s",entity->GetName(),entity->GetEntityClassName() );
			entity->Reset();
		}
	}
	*/
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::LoadMission( const CString &mission )
{
	if (!IsLevelLoaded())
		return false;
	if (mission != m_missionName)
	{
		m_missionName = mission;
		m_I3DEngine->LoadEnvironmentSettingsFromXML( m_missionName, true );
	}

	/*
	CLogFile::WriteLine( "Initializing AI system" );
	char szLevel[1024];
	_splitpath( GetIEditor()->GetDocument()->GetPathName(),0,0,szLevel,0 );
	CString missionName = mission->GetName();
	GetIEditor()->GetSystem()->GetAISystem()->Init( GetIEditor()->GetSystem(),szLevel,missionName );
	*/

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameEngine::ReloadEnvironment()
{
	if (!m_I3DEngine)
		return false;
	if (!IsLevelLoaded())
		return false;

	//char szLevelPath[_MAX_PATH];
	//strcpy( szLevelPath,GetLevelPath() );
	//PathAddBackslash( szLevelPath );
	//CGameExporter gameExporter( GetIEditor()->GetSystem() );
	//gameExporter.ExportLevelData( szLevelPath,false );

	XmlNodeRef env = new CXmlNode("Environment");
	CXmlTemplate::SetValues( GetIEditor()->GetDocument()->GetEnvironmentTemplate(),env );

	// Notify mission that environment may be changed.
	GetIEditor()->GetDocument()->GetCurrentMission()->OnEnvironmentChange();

	//! Add lighting node to environment settings.
	GetIEditor()->GetDocument()->GetCurrentMission()->GetLighting()->Serialize( env,false );

	CString xmlStr = env->getXML();

	// Reload level data in engine.
	m_I3DEngine->LoadEnvironmentSettingsFromXML( GetMissionName(),true,xmlStr );
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetGameMode( bool inGame )
{
	if (m_inGameMode == inGame)
		return;

	IGame *pGame = GetIEditor()->GetGame();

	//////////////////////////////////////////////////////////////////////////
	if (m_I3DEngine)
	{
		// Reset some 3d engine effects.
		m_I3DEngine->ResetScreenFx();
		m_I3DEngine->ResetParticlesAndDecals();
	}

	m_inGameMode = inGame;
	if (inGame)
	{
//		m_IGame->HideLocalPlayer(false);
		// Go to game mode.
		IEntity *myPlayer;
#ifndef _ISNOTFARCRY
			myPlayer = GetIXGame( pGame )->GetMyPlayer();
#endif
		if (myPlayer)
		{
			pe_player_dimensions dim;
			dim.heightEye = 0;
			if (myPlayer->GetPhysics())
				myPlayer->GetPhysics()->GetParams( &dim );

			myPlayer->SetPos( GetIEditor()->GetViewerPos() - Vec3(0,0,dim.heightEye) );
			myPlayer->SetAngles( GetIEditor()->GetViewerAngles() );
		}
		
#ifndef _ISNOTFARCRY	
			GetIXGame( GetIEditor()->GetGame() )->SetViewAngles( GetIEditor()->GetViewerAngles() );
#endif
		
		// Disable accelerators.
		GetIEditor()->EnableAcceleratos( false );

		// Reset physics state before switching to game.
		m_ISystem->GetIPhysicalWorld()->ResetDynamicEntities();

		// Reset Equipment
#ifndef _ISNOTFARCRY
            GetIXGame( GetIEditor()->GetGame() )->RestoreWeaponPacks();
#endif

		// Reset mission script.
		GetIEditor()->GetDocument()->GetCurrentMission()->ResetScript();

		// reset all agents in aisystem
		m_ISystem->GetAISystem()->Reset();

		// [Anton] - order changed, see comments for CGameEngine::SetSimulationMode
		if (m_IGame)
			m_IGame->ResetState();

		//! Send event to switch into game.
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_INGAME );

		// Reset movie system.
		GetIEditor()->GetAnimation()->ResetAnimations(true);

		// Switch to first person mode.
		//GetIEditor()->GetGame()->SetViewMode(false);
		// drawing of character.
#ifndef _ISNOTFARCRY
			GetIXGame( m_IGame )->HideLocalPlayer( false, true );
#endif
		
		SetPlayerPos( m_playerPos );
		SetPlayerAngles( m_playerAngles );

		// Register in game entitysystem listener.
		s_InGameEntityListener = new SInGameEntitySystemListener;
		m_IEntitySystem->SetSink(s_InGameEntityListener);
	}
	else
	{
		// reset all agents in aisystem
		m_ISystem->GetAISystem()->Reset();

		// Out of game in Editor mode.
		GetIEditor()->SetViewerPos( m_ISystem->GetViewCamera().GetPos() );
		GetIEditor()->SetViewerAngles( m_ISystem->GetViewCamera().GetAngles() );

		// Unregister ingame entitysystem listener, and kill all remaining entities.
		m_IEntitySystem->RemoveSink(s_InGameEntityListener);
		delete s_InGameEntityListener;
		s_InGameEntityListener = 0;

		// Enable accelerators.
		GetIEditor()->EnableAcceleratos( true );

		// Reset mission script.
		GetIEditor()->GetDocument()->GetCurrentMission()->ResetScript();

		if (GetIEditor()->Get3DEngine()->IsTerainHightMapModifiedByGame())
			GetIEditor()->GetHeightmap()->UpdateEngineTerrain();

		// reset movie-system
		GetIEditor()->GetAnimation()->ResetAnimations(false);

		m_ISystem->GetIPhysicalWorld()->ResetDynamicEntities();
		if (m_IGame)
			m_IGame->ResetState();

		// [Anton] - order changed, see comments for CGameEngine::SetSimulationMode
		//! Send event to switch out of game.
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_OUTOFGAME );
		
		// Switch to third person mode.
	   //GetIEditor()->GetGame()->SetViewMode(true);

		// Hide all drawing of character.
#ifndef _ISNOTFARCRY
			GetIXGame( m_IGame )->HideLocalPlayer( true, true );
#endif
	}

	if (AfxGetMainWnd())
	{
		HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
		m_ISystem->GetIInput()->SetMouseExclusive( false,hWnd );
		m_ISystem->GetIInput()->SetKeyboardExclusive( false,hWnd );

		AfxGetMainWnd()->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetSimulationMode( bool enabled )
{
	if (m_simulationMode == enabled)
		return;
	m_simulationMode = enabled;
	
	if (m_simulationMode)
	{
		// When turning physics on, emulate out of game event.
		m_ISystem->GetAISystem()->Reset();
		// [Anton] the order of the next 3 calls changed, since, EVENT_INGAME loads physics state (if any),
		// and Reset should be called before it
		m_ISystem->GetIPhysicalWorld()->ResetDynamicEntities();
		if (m_IGame)
			m_IGame->ResetState();
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_INGAME );
		GetIEditor()->SetConsoleVar( "ai_ignoreplayer",1 );	
		GetIEditor()->SetConsoleVar( "ai_soundperception",0 );
	}
	else
	{
		// When turning physics off, emulate out of game event.
		m_ISystem->GetAISystem()->Reset();
		m_ISystem->GetIPhysicalWorld()->ResetDynamicEntities();
		if (m_IGame)
			m_IGame->ResetState();
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_OUTOFGAME );
		GetIEditor()->SetConsoleVar( "ai_ignoreplayer",0 );
		GetIEditor()->SetConsoleVar( "ai_soundperception",1 );	
		
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::GenerateAiTriangulation()
{
	CWaitProgress wait( "Generating AI Triangulation" );

	CWaitCursor waitcrs;

	GetIEditor()->SetConsoleVar( "ai_triangulate",1 );
	//GetIEditor()->GetObjectManager()->SendEvent( EVENT_UNLOAD_ENTITY );
	GetIEditor()->GetObjectManager()->SendEvent( EVENT_CLEAR_AIGRAPH );
	
	CLogFile::FormatLine( "Generating Triangulation for Level:%s Mission:%s",(const char*)m_levelPath,(const char*)m_missionName );
	m_IAISystem->FlushSystem();
	m_IAISystem->LoadTriangulation( m_levelPath,m_missionName );
	//GetIEditor()->GetObjectManager()->SendEvent( EVENT_RELOAD_ENTITY );
	GetIEditor()->GetObjectManager()->SendEvent( EVENT_MISSION_CHANGE );
	GetIEditor()->SetConsoleVar( "ai_triangulate",0 );
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::ForceRegisterEntitiesInSectors()
{
	// Reset state of all entities.
	IEntityIt *entityIt = m_IEntitySystem->GetEntityIterator();
	if (entityIt)
	{
		entityIt->MoveFirst();
		for (IEntity *entity = entityIt->Next(); entity != 0; entity = entityIt->Next())
		{
			//if (entity->Re
			//entity->SetRegisterInSectors(false);
			//entity->SetRegisterInSectors(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::ResetResources()
{
	//////////////////////////////////////////////////////////////////////////
	// Before clearing game and engine resources,
	// make sure we dont keep some things in editor.
	//////////////////////////////////////////////////////////////////////////
	GetIEditor()->GetMaterialManager()->ClearAll();
	GetIEditor()->GetParticleManager()->ClearAll();

//	/*
	m_I3DEngine = m_ISystem->GetI3DEngine();
	IGame *pGame = GetIEditor()->GetGame();
	if (!pGame)
		return;

	GetIEditor()->GetIconManager()->Reset();

	GetIEditor()->GetVegetationMap()->UnloadObjectsGeometry();
	CEdMesh::ReleaseAll();

	// Clear everything.
	//m_I3DEngine->ClearRenderResources(); //this function is called by the game already

	m_ISystem->IgnoreUpdates( true );
	pGame->LoadLevelForEditor( m_levelPath,m_missionName );
	/*
	//@HACK: Update game several times to insure client connection.
	for (int i = 0; i < 10; i++)
	{
		pGame->Update();
	}
	m_ISystem->IgnoreUpdates( false );
	m_IGame->HideLocalPlayer(true,true);
	*/
	m_ISystem->IgnoreUpdates( false );
	m_bGameInitialized = true;

	CEntityScriptRegistry::Instance()->LoadScripts();
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::PutSurfaceTypesToGame( bool bExportToEngine )
{
	CCryEditDoc *doc = GetIEditor()->GetDocument();
	IGame *pGame = GetIEditor()->GetGame();
	if (!doc || !pGame)
		return;
	for (int i = 0; i < doc->GetSurfaceTypeCount(); i++)
	{
		CString physMaterial = doc->GetSurfaceType(i)->GetMaterial();
		if (!physMaterial.IsEmpty())
		{
#ifndef _ISNOTFARCRY
				GetIXGame( pGame )->SetTerrainSurface( physMaterial, i );
#endif
		}
	}

	if (bExportToEngine)
	{
		if (!IsLevelLoaded())
			return;

		char szLevelPath[_MAX_PATH];
		strcpy( szLevelPath,GetLevelPath() );
		PathAddBackslash( szLevelPath );

		CGameExporter gameExporter( GetIEditor()->GetSystem() );
		gameExporter.ExportLevelData( szLevelPath,false );

		// Reload level data in engine.
		m_I3DEngine->LoadTerrainSurfacesFromXML();
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetPlayerEquipPack( const char *sEqipPackName )
{
	if (m_IGame && m_bGameInitialized)
	{
#ifndef _ISNOTFARCRY
			GetIXGame( m_IGame )->SetPlayerEquipPackName( sEqipPackName );
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::ReloadResourceFile( const CString &filename )
{
	GetIEditor()->GetRenderer()->EF_ReloadFile( filename );
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetPlayerPos( const Vec3 &pos,bool bEyePos )
{
	m_playerPos = pos;
	if (!m_IGame)
		return;

	IEntity *myPlayer = NULL;
#ifndef _ISNOTFARCRY
		myPlayer = GetIXGame( m_IGame )->GetMyPlayer();
#endif

	if (!myPlayer)
		return;

	//if (m_inGameMode || m_simulationMode)
	if (m_syncPlayerPosition)
	{
		Vec3 p = pos;

		if (bEyePos)
		{
			pe_player_dimensions dim;
			dim.heightEye = 0;
			if (myPlayer->GetPhysics())
			{
				myPlayer->GetPhysics()->GetParams( &dim );
				p.z = p.z - dim.heightEye;
			}
		}
		myPlayer->SetPos( p );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetPlayerAngles( const Vec3 &angles )
{
	m_playerAngles = angles;
	if (m_syncPlayerPosition)
	{
		//if (m_inGameMode || m_simulationMode)
		{
			if (m_IGame)
			{
#ifndef _ISNOTFARCRY
					GetIXGame( m_IGame )->SetViewAngles( angles );
#endif
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SyncPlayerPosition( bool bEnable )
{
	m_syncPlayerPosition = bEnable;
	if (m_syncPlayerPosition)
	{
		SetPlayerPos( m_playerPos );
		SetPlayerAngles( m_playerAngles );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::SetCurrentMOD( const char *sMod )
{
	m_MOD = sMod;
}

//////////////////////////////////////////////////////////////////////////
const char* CGameEngine::GetCurrentMOD() const
{
	return m_MOD;
}

//////////////////////////////////////////////////////////////////////////
void CGameEngine::Update()
{
	if (m_inGameMode)
		return;

	if (m_IGame)
	{
#ifndef _ISNOTFARCRY
			IXGame* pXGame = GetIXGame( m_IGame );
			if (!pXGame->GetMyPlayer())
			{
				m_ISystem->IgnoreUpdates( true );
				//@HACK: Update game several times to insure client connection.
				for (int i = 0; i < 20; i++)
				{
					pXGame->Update();
					if (pXGame->GetMyPlayer())
						break;
				}
				m_ISystem->IgnoreUpdates( false );
			}
#endif
	}

	// Only in Editor mode.
	// Update system.
	int updateFlags = ESYSUPDATE_EDITOR;
	if (!GetSimulationMode())
	{
		updateFlags += ESYSUPDATE_IGNORE_PHYSICS|ESYSUPDATE_IGNORE_AI;
	}

	// [marco] check current sound and vis areas for music etc.	
	// but if in game mode, 'cos is already done in the above call to game->update()
#ifndef _ISNOTFARCRY
	if (m_IGame)	GetIXGame( m_IGame )->CheckSoundVisAreas();
#endif

	GetIEditor()->GetSystem()->Update( updateFlags );

	// [marco] after system update, retrigger areas if necessary			
	if (m_IGame)
	{
#ifndef _ISNOTFARCRY
			if (m_IGame)	GetIXGame( m_IGame )->RetriggerAreas();
			if (m_IGame)	GetIXGame( m_IGame )->HideLocalPlayer( true, true );
#endif
	}
}