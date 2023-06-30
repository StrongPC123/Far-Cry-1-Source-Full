//////////////////////////////////////////////////////////////////////
//
//	Game source code
//	
//	File: Game.cpp
// 
//	History:
//	-August 02,2001: Create by Alberto Demichelis
//	-Sep 24,2001 : Modified by Petar Kotevski
//
//////////////////////////////////////////////////////////////////////
 
#include "stdafx.h"
#include <IStreamEngine.h>
#include <ICryPak.h>
#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"

#include "UIHud.h"

#include "WeaponSystemEx.h"
#include "WeaponClass.h"

#include "XVehicleSystem.h"
#include "PlayerSystem.h"
#include "XServer.h"

#include "XSystemBase.h"

#include "UISystem.h"
#include "Flock.h" 

#include "EntityClassRegistry.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"

#include "ScriptObjectLanguage.h"
#include "ScriptObjectPlayer.h"
#include "ScriptObjectSpectator.h"					// CScriptObjectSpectator
#include "ScriptObjectAdvCamSystem.h"				// CScriptObjectAdvCamSystem
#include "ScriptObjectSynched2DTable.h"			// CScriptObjectSynched2DTable
#include "ScriptObjectVehicle.h"
#include "ScriptObjectRenderer.h"
#include "ScriptObjectStream.h"
#include "ScriptObjectWeaponClass.h"
#include "ScriptObjectAI.h"

#include "ScriptObjectUI.h"

#include "ScriptObjectBoids.h"
#include "ScriptTimerMgr.h"

#include "IngameDialog.h"
#include <IEntitySystem.h>
#include <ISound.h>

#include "TagPoint.h"
//#include "XPath.h"

#include "XPlayer.h"
#include "XVehicle.h"

#include <IMovieSystem.h>
#include "CMovieUser.h"

#include "TimeDemoRecorder.h"
#include "GameMods.h"

#ifdef WIN32
#include <winioctl.h>
#include <tchar.h>

typedef std::basic_string< TCHAR > tstring;
typedef std::vector< TCHAR > tvector;
#endif
 
//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// DLL Interface

//////////////////////////////////////////////////////////////////////
// interface of the DLL
IGame* CreateGameInstance()
{
	CXGame *pGame = new CXGame();
	return pGame;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CTagPointManager

class CTagPointManager : public ITagPointManager
{
public:
	CTagPointManager( IGame *pGame )  { m_pGame = (CXGame*) pGame; };
	virtual ~CTagPointManager() {};

	// This function creates a tag point in the game world
	virtual ITagPoint *CreateTagPoint(const string &name, const Vec3 &pos, const Vec3 &angles) 
	{
		return m_pGame->CreateTagPoint( name, pos, angles );
	};

	// Retrieves a tag point by name
	virtual ITagPoint *GetTagPoint(const string &name)
	{
		return m_pGame->GetTagPoint( name );
	}

	// Deletes a tag point from the game
	virtual void RemoveTagPoint(ITagPoint *pPoint)
	{
		m_pGame->RemoveTagPoint( pPoint );
	}

	virtual void AddRespawnPoint(ITagPoint *pPoint)
	{
		m_pGame->AddRespawnPoint( pPoint );
	}

	virtual void RemoveRespawnPoint(ITagPoint *pPoint)
	{
		m_pGame->RemoveRespawnPoint( pPoint );
	}

private:
	CXGame *m_pGame;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// CXGame
//!constructor
CXGame::CXGame()
{
	m_pTimeDemoRecorder = NULL;
	m_pScriptObjectGame = NULL;
	m_pScriptTimerMgr = NULL;
	m_pScriptSystem = NULL;
	m_pServer = NULL;
	m_pClient = NULL;
	m_pLog = NULL;
	m_pServerSnooper = NULL;
	m_pNETServerSnooper = 0;
	m_pRConSystem = 0;
	m_pWeaponSystemEx = NULL;
	m_mapTagPoints.clear();
	m_bMenuInitialized = false;
	m_pCurrentUI = 0;	
	m_pIActionMapManager=NULL;
	m_pIngameDialogMgr = new CIngameDialogMgr();
	m_pUISystem = 0;
	mp_model = 0;
#if !defined(LINUX)
	// to avoid all references to movie user in this file
	m_pMovieUser = new CMovieUser(this);
#endif
	m_nPlayerIconTexId = -1;
	m_nVehicleIconTexId = -1;
	m_nBuildingIconTexId = -1;
	m_nUnknownIconTexId = -1;
	m_bMenuOverlay = false;
	m_bUIOverlay = false;
	m_bUIExclusiveInput = false;
	m_bHideLocalPlayer = false;
	m_pCVarCheatMode=NULL;

	m_fTimeGran=m_fFixedStep=m_fTimeGran2FixedStep=m_frFixedStep = 0;
	m_iFixedStep2TimeGran = 0;
	g_language=0;
	g_playerprofile=0;
	g_serverprofile=0;

	m_tPlayerPersistentData.m_bDataSaved=false;
	m_fFadingStartTime=-1.0f;
	cv_game_physics_quality=NULL;
	m_bMapLoadedFromCheckpoint=false;
	m_bSynchronizing = false;

	//m_fTimeToSaveThumbnail = 0;
	m_pGameMods = NULL;
	m_bLastDoLateSwitch = 0;
	m_bLastCDAuthentication = 0;
	m_bAllowQuicksave = true;

	m_sGameName = "FarCry";
	m_pTagPointManager = new CTagPointManager( this );
	m_nDEBUG_TIMING = 0;
	m_fDEBUG_STARTTIMER = 0;
}

//////////////////////////////////////////////////////////////////////
//!destructor
CXGame::~CXGame()
{
	m_pScriptSystem->BeginCall("Shutdown");
	m_pScriptSystem->PushFuncParam(0);
	m_pScriptSystem->EndCall();

	if (m_pIngameDialogMgr)
		delete m_pIngameDialogMgr;
	m_pIngameDialogMgr=NULL;

#if !defined(LINUX)
	if (m_pMovieUser)
	{
		if (m_pSystem)
		{
			IMovieSystem *pMovieSystem=m_pSystem->GetIMovieSystem();
			if (pMovieSystem)
			{
				if (pMovieSystem->GetUser()==m_pMovieUser)
					pMovieSystem->SetUser(NULL);
			}
		}
		delete m_pMovieUser;
	}
#endif
	m_pMovieUser=NULL;

	// shutdown the client if there is one
	ShutdownClient();

	// shutdown the server if there is one
	ShutdownServer();

	if (m_pUISystem)
	{
		m_pUISystem->Release();
	}
	SAFE_DELETE(m_pUISystem);


	CScriptObjectUI::ReleaseTemplate();
	CScriptObjectPlayer::ReleaseTemplate();
	CScriptObjectFireParam::ReleaseTemplate();
	CScriptObjectWeaponClass::ReleaseTemplate();
	CScriptObjectVehicle::ReleaseTemplate();
	CScriptObjectSpectator::ReleaseTemplate();
	CScriptObjectAdvCamSystem::ReleaseTemplate();
	CScriptObjectSynched2DTable::ReleaseTemplate();
	CScriptObjectBoids::ReleaseTemplate();
	CScriptObjectRenderer::ReleaseTemplate();
	CScriptObjectGame::ReleaseTemplate();
	CScriptObjectInput::ReleaseTemplate();
	CScriptObjectLanguage::ReleaseTemplate();

	CScriptObjectAI::ReleaseTemplate();
	CScriptObjectServer::ReleaseTemplate();
	CScriptObjectServerSlot::ReleaseTemplate();
	CScriptObjectClient::ReleaseTemplate();
	CScriptObjectStream::ReleaseTemplate();

	cl_scope_flare->Release();
	cl_ThirdPersonRange->Release();

	cl_ThirdPersonOffs->Release();
	cl_ThirdPersonOffsAngHor->Release();
	cl_ThirdPersonOffsAngVert->Release();

	cl_display_hud->Release();
  cl_motiontracker->Release();
  cl_hud_pickup_icons->Release();
  cl_msg_notification->Release();
  cl_hud_name->Release();
	ai_num_of_bots->Release();
	p_name->Release();
	p_model->Release();
	mp_model->Release();
	p_color->Release();
	p_always_run->Release();
	g_language->Release();
	g_playerprofile->Release();
	g_serverprofile->Release();
	g_GC_Frequence->Release();
	p_speed_run->Release();
	p_sprint_scale->Release();

	p_sprint_decoy->Release();
	p_sprint_restore_run->Release();
	p_sprint_restore_idle->Release();

	p_speed_walk->Release();
	p_speed_crouch->Release();
	p_speed_prone->Release();
	p_jump_force->Release();
	p_jump_run_time->Release();
	p_jump_walk_time->Release();
//	p_lean->Release();
	p_lean_offset->Release();
	p_bob_pitch->Release();
	p_bob_roll->Release();
	p_bob_length->Release();
	p_bob_weapon->Release();
	p_bob_fcoeff->Release();
	p_weapon_switch->Release();

	cv_game_physics_quality->Release();

	m_jump_vel->Release();
	m_jump_arc->Release();

	b_camera->Release();

//	f_update->Release();
//	f_draw->Release();
//	f_drawDbg->Release();

	sv_timeout->Release();
	cl_timeout->Release();
	cl_loadtimeout->Release();

	g_LevelName->Release();
	g_GameType->Release();
	g_LeftHanded->Release();

	p_DeadBody->Release();
	p_HitImpulse->Release();
	p_RotateHead->Release();
	a_DrawArea->Release();
	a_LogArea->Release();

	m_pCVarCheatMode->Release();

	pl_JumpNegativeImpulse->Release();

	if (m_pRenderer && (m_nPlayerIconTexId>=0))
		m_pRenderer->RemoveTexture(m_nPlayerIconTexId);
	if (m_pRenderer && (m_nVehicleIconTexId>=0))
		m_pRenderer->RemoveTexture(m_nVehicleIconTexId);
	if (m_pRenderer && (m_nBuildingIconTexId>=0))
		m_pRenderer->RemoveTexture(m_nBuildingIconTexId);
	if (m_pRenderer && (m_nUnknownIconTexId>=0))
		m_pRenderer->RemoveTexture(m_nUnknownIconTexId);
	
	SAFE_DELETE(m_pUIHud);
	SAFE_DELETE(m_pWeaponSystemEx);
	SAFE_DELETE(m_pVehicleSystem);
	SAFE_DELETE(m_pPlayerSystem);

	SAFE_DELETE(m_pFlockManager);

	CScriptObjectPlayer::ReleaseTemplate();
	
	//shutdown script stuff
	SAFE_DELETE(m_pScriptObjectGame);	
	SAFE_DELETE(m_pScriptObjectInput);

	SAFE_DELETE(m_pScriptObjectBoids);
	SAFE_DELETE(m_pScriptObjectLanguage);
	SAFE_DELETE(m_pScriptObjectAI);


	// Release the action map
	SAFE_RELEASE(m_pIActionMapManager);
	SAFE_DELETE(m_pScriptTimerMgr);
	// release the tags
	if (!m_mapTagPoints.empty())
	{
		TagPointMap::iterator ti;
		for (ti=m_mapTagPoints.begin();ti!=m_mapTagPoints.end();ti++)
			delete ti->second;
	}

	SAFE_RELEASE(m_pServerSnooper);
	SAFE_RELEASE(m_pNETServerSnooper);
	SAFE_RELEASE(m_pRConSystem);
	SAFE_DELETE(m_pTimeDemoRecorder);
	SAFE_DELETE(m_pGameMods);

	delete m_pTagPointManager;
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::InitClassRegistry()
{
	m_EntityClassRegistry.Init( m_pSystem );
	CPlayerSystem *pPlayerSystem = GetPlayerSystem();
	CVehicleSystem *pVehicleSystem = GetVehicleSystem();
	CWeaponSystemEx *pWeaponSystemEx = GetWeaponSystemEx();	// m10

	assert( pPlayerSystem );
	assert( pVehicleSystem );
	assert( pWeaponSystemEx );

	// Enumerate entity classes.
	EntityClass *entCls = NULL;
	m_EntityClassRegistry.MoveFirst();
	do {
		entCls = m_EntityClassRegistry.Next();
		if (entCls)
		{
			const char* entity_type = entCls->strGameType.c_str();
			EntityClassId ClassId = entCls->ClassId;
			if(strcmp("Player",entity_type)==0)
				pPlayerSystem->AddPlayerClass(ClassId);

			if(strcmp("Vehicle",entity_type)==0)
				pVehicleSystem->AddVehicleClass(ClassId);

			if(strcmp("Projectile",entity_type)==0)
			{
				// cannot be loaded at that point - other scripts must be loaded before
				pWeaponSystemEx->AddProjectileClass(ClassId);
			}
		}
	} while (entCls);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::SoftReset()
{
	m_pLog->Log("Soft Reset Begin");
	//allow to reload scripts with(LoadScript)
	m_pScriptSystem->SetGlobalToNull("_localplayer");
	if(m_pScriptSystem)
		m_pScriptSystem->UnloadScripts();

	std::vector<string> vLoadedWeapons;

	for (int i = 0; i != m_pWeaponSystemEx->GetNumWeaponClasses(); ++i)
		vLoadedWeapons.push_back(m_pWeaponSystemEx->GetWeaponClass(i)->GetName());

	m_pWeaponSystemEx->Reset();
#if !defined(LINUX)
	if (m_pSystem->GetIMovieSystem())
		m_pSystem->GetIMovieSystem()->StopAllSequences();
#endif
	m_pScriptObjectGame->Reset();

	m_pScriptSystem->ForceGarbageCollection();
	//m_p3DEngine->ClearRenderResources();

	m_iLastCmdIdx = 0;

	m_pWeaponSystemEx->Init(this);

	for (std::vector<string>::iterator i = vLoadedWeapons.begin(); i != vLoadedWeapons.end(); ++i)
		AddWeapon((*i).c_str());

	//if (GetMyPlayer())
	//	GetMyPlayer()->SetNeedUpdate(true);

	if (m_pCurrentUI) 
	{
		m_pCurrentUI->Reset();
		m_pCurrentUI->Init(m_pScriptSystem);
	}

	m_pLog->Log("Soft Reset End");
}

//////////////////////////////////////////////////////////////////////
//!reset the game before a level reloading
//!this function allow the reloading of all scripts
//!and wipe out all textures from the 3dengine
void CXGame::Reset()
{
	m_pEntitySystem->Reset();

	// Unload all music.
	if (m_pSystem->GetIMusicSystem())
		m_pSystem->GetIMusicSystem()->Unload();

	//allow to reload scripts with(LoadScript)
	m_pScriptSystem->SetGlobalToNull("_localplayer");
	if(m_pScriptSystem)
		m_pScriptSystem->UnloadScripts();

	m_pWeaponSystemEx->Reset();
	m_XSurfaceMgr.Reset();
	m_XAreaMgr.Clear();
	ClearTagPoints();
#if !defined(LINUX)
	if (m_pSystem->GetIMovieSystem())
		m_pSystem->GetIMovieSystem()->Reset(false);
#endif
	m_pScriptObjectGame->Reset();

	m_pScriptSystem->ForceGarbageCollection();
	m_p3DEngine->ClearRenderResources();

	// Must reset all timers.
	m_pScriptTimerMgr->Reset();

	//clen up the input buffer
	if (m_pSystem->GetIInput())
	{
		m_pSystem->GetIInput()->Update(true);
		m_pSystem->GetIInput()->Update(true);
	}
	if (m_pIActionMapManager)
		m_pIActionMapManager->Reset();

	m_iLastCmdIdx = 0;

	//////////////////////////////////////////////////////////////////////////
	// Reset UI.
	//////////////////////////////////////////////////////////////////////////
	if (m_pUISystem)
	{
		m_pUISystem->UnloadAllModels();
		m_pUISystem->StopAllVideo();
		m_p3DEngine->Enable(1);
		//m_pSystem->GetILog()->Log("UISystem: Enabled 3D Engine!");
	}
	if (m_pCurrentUI) 
		m_pCurrentUI->Reset();
	if (m_pUIHud)
		m_pUIHud->Reset();

	if (GetMyPlayer())
		GetMyPlayer()->SetNeedUpdate(true);
}

IXSystem *CXGame::GetXSystem(){return m_pServer?m_pServer->m_pISystem:m_pClient?m_pClient->m_pISystem:NULL;}
//////////////////////////////////////////////////////////////////////
//! Initialize the game. This must be called before calling other functions of this class.
bool CXGame::Init(struct ISystem *pSystem,bool bDedicatedSrv,bool bInEditor,const char *szGameMod)
{	
	// Setup the system and 3D Engine pointers
	m_pSystem	= pSystem;

	m_pGameMods = new CGameMods(this);

	gISystem = pSystem;
	m_bDedicatedServer=bDedicatedSrv;
	m_XAreaMgr.Init( pSystem );
	m_bEditor=bInEditor;


	m_bRelaunch=false;
	m_bMovieSystemPaused = false;
	m_bIsLoadingLevelFromFile=false;
	
	m_bOK = false;
	m_bUpdateRet = true;
	m_pClient	= NULL;
	m_pServer	= NULL;

  m_pSystem->GetILog()->Log("Game Initialization");
#if !defined(LINUX)	
	IMovieSystem *pMovieSystem=m_pSystem->GetIMovieSystem();
	if (pMovieSystem)
		pMovieSystem->SetUser(m_pMovieUser);
#endif
	if (!m_pTimeDemoRecorder)
		m_pTimeDemoRecorder = new CTimeDemoRecorder(pSystem);
  
	m_pUIHud = NULL;
	m_pNetwork= m_pSystem->GetINetwork();
	m_pLog= m_pSystem->GetILog();
	m_p3DEngine	= m_pSystem->GetI3DEngine();
	m_pRenderer = m_pSystem->GetIRenderer();
	m_pScriptSystem=pSystem->GetIScriptSystem();
	m_pEntitySystem=m_pSystem->GetIEntitySystem();

	// Register game rendering callback.
	//[Timur] m_p3DEngine->SetRenderCallback( OnRenderCallback,this );
	
	// init subsystems
#ifndef _XBOX
	m_pServerSnooper=m_pNetwork->CreateServerSnooper(this);
	m_pNETServerSnooper=m_pNetwork->CreateNETServerSnooper(this);
	m_pRConSystem=m_pNetwork->CreateRConSystem();
#endif
	m_pWeaponSystemEx = new CWeaponSystemEx();
	m_pVehicleSystem = new CVehicleSystem();
	m_pPlayerSystem = new CPlayerSystem();
	m_pFlockManager = new CFlockManager(m_pSystem);

	CScriptObjectUI::InitializeTemplate(m_pScriptSystem);

	// init is not necessary for now, but add here if it later is
	m_pScriptObjectGame=new CScriptObjectGame;
	CScriptObjectGame::InitializeTemplate(m_pScriptSystem);

	m_pScriptObjectInput=new CScriptObjectInput;
	CScriptObjectInput::InitializeTemplate(m_pScriptSystem);
	m_pScriptObjectLanguage=new CScriptObjectLanguage;
	CScriptObjectLanguage::InitializeTemplate(m_pScriptSystem);
	m_pScriptObjectBoids = new CScriptObjectBoids;
	CScriptObjectBoids::InitializeTemplate(m_pScriptSystem);
	m_pScriptObjectAI = new CScriptObjectAI;
	CScriptObjectAI::InitializeTemplate(m_pScriptSystem);
	CScriptObjectServer::InitializeTemplate(m_pScriptSystem);

	CScriptObjectPlayer::InitializeTemplate(m_pScriptSystem);
	CScriptObjectFireParam::InitializeTemplate(m_pScriptSystem);
	CScriptObjectWeaponClass::InitializeTemplate(m_pScriptSystem);
	CScriptObjectVehicle::InitializeTemplate(m_pScriptSystem);
	CScriptObjectSpectator::InitializeTemplate(m_pScriptSystem);
	CScriptObjectAdvCamSystem::InitializeTemplate(m_pScriptSystem);
	CScriptObjectSynched2DTable::InitializeTemplate(m_pScriptSystem);
	CScriptObjectRenderer::InitializeTemplate(m_pScriptSystem);


	m_pScriptObjectGame->Init(m_pScriptSystem, this);
	m_pScriptObjectInput->Init(m_pScriptSystem,this,m_pSystem);


	m_pScriptObjectBoids->Init(m_pScriptSystem,m_pSystem,m_pFlockManager);
	m_pScriptObjectLanguage->Init(m_pScriptSystem,&m_StringTableMgr);
	m_pScriptObjectAI->Init(m_pScriptSystem,m_pSystem,this);

	CScriptObjectServerSlot::InitializeTemplate(m_pScriptSystem);
	CScriptObjectClient::InitializeTemplate(m_pScriptSystem);
	CScriptObjectStream::InitializeTemplate(m_pScriptSystem);
	
	m_pScriptTimerMgr=new CScriptTimerMgr(m_pScriptSystem,m_pSystem->GetIEntitySystem(),this);

// making some constants accessable to the script
	m_pScriptSystem->SetGlobalValue("FireActivation_OnPress",ePressing);
	m_pScriptSystem->SetGlobalValue("FireActivation_OnRelease",eReleasing);
	m_pScriptSystem->SetGlobalValue("FireActivation_OnHold",eHolding);
	
	m_pScriptSystem->SetGlobalValue("ENTITYTYPE_PLAYER", ENTITYTYPE_PLAYER);
	m_pScriptSystem->SetGlobalValue("ENTITYTYPE_WAYPOINT", ENTITYTYPE_WAYPOINT);
	m_pScriptSystem->SetGlobalValue("ENTITYTYPE_OWNTEAM", ENTITYTYPE_OWNTEAM);

/*	m_pScriptSystem->SetGlobalValue("CMD_GO", CMD_GO);
	m_pScriptSystem->SetGlobalValue("CMD_ATTACK", CMD_ATTACK);
	m_pScriptSystem->SetGlobalValue("CMD_DEFEND", CMD_DEFEND);
	m_pScriptSystem->SetGlobalValue("CMD_COVER", CMD_COVER);
	m_pScriptSystem->SetGlobalValue("CMD_BARRAGEFIRE", CMD_BARRAGEFIRE);*/

	InitConsoleVars();

	if (szGameMod && szGameMod[0])
	{
		// apply the mod without restarting as the game just started!
		GetModsInterface()->SetCurrentMod(szGameMod,false);
	}
	
	InitClassRegistry();
		
	// execute the "main"-script (to pre-load other scripts, etc.)
	m_pScriptSystem->ExecuteFile("scripts/main.lua");
	m_pScriptSystem->BeginCall("Init");
	m_pScriptSystem->PushFuncParam(0);
	m_pScriptSystem->EndCall();

	// initialize the surface-manager
	m_XSurfaceMgr.Init(m_pScriptSystem,m_p3DEngine,GetSystem()->GetIPhysicalWorld());
	
	// init key-bindings
	if(!m_bDedicatedServer)
		InitInputMap();

	// create various console-commands/variables
	InitConsoleCommands();
	
	// loading the main language-string-table
	if (!m_StringTableMgr.Load(GetSystem(),*m_pScriptObjectLanguage,g_language->GetString()))
		m_pLog->Log("cannot load language file [%s]",g_language->GetString());
		
	// creating HUD interface
	m_pLog->Log("Initializing UI");
	m_pUIHud = new CUIHud(this,m_pSystem);

	LoadConfiguration("","game.cfg");

	//////////////////////////////////////////////////////////////////////////
	// Materials
	// load materials (once before all, this info stays till we quit the game - no need to load material later)
	// first load normal materials
	m_XSurfaceMgr.LoadMaterials("scripts/materials");
 
	if(!m_bDedicatedServer)
	{
		m_pSystem->GetIConsole()->ShowConsole(0);
		if (!bInEditor)
		{
			//------------------------------------------------------------------------------------------------- 
			m_pUISystem = new CUISystem;

			if (m_pUISystem)
			{
				m_pUISystem->Create(this, m_pSystem, m_pScriptSystem, "Scripts/MenuScreens/UISystem.lua", 1);
			}
			else
			{
				m_pLog->Log("Failed to create UI System!");
			}
			//------------------------------------------------------------------------------------------------- 
		}

		//------------------------------------------------------------------------------------------------- 
		if (m_pUISystem)
		{
			m_bMenuOverlay = 1;
		}
		//------------------------------------------------------------------------------------------------- 
	}
	else
		m_pSystem->GetIConsole()->ShowConsole(true);

	// load textures used as icons by the mini-map
	if (m_pRenderer)
	{
		ITexPic *pPic;
		pPic=m_pRenderer->EF_LoadTexture("gui/map_player", FT_NOREMOVE, 0, eTT_Base);
		if (pPic && pPic->IsTextureLoaded())
			m_nPlayerIconTexId=pPic->GetTextureID();
		pPic=m_pRenderer->EF_LoadTexture("gui/map_vehicle", FT_NOREMOVE, 0, eTT_Base);
		if (pPic && pPic->IsTextureLoaded())
			m_nVehicleIconTexId=pPic->GetTextureID();
		pPic=m_pRenderer->EF_LoadTexture("gui/map_building", FT_NOREMOVE, 0, eTT_Base);
		if (pPic && pPic->IsTextureLoaded())
			m_nBuildingIconTexId=pPic->GetTextureID();
		pPic=m_pRenderer->EF_LoadTexture("gui/map_unknown", FT_NOREMOVE, 0, eTT_Base);
		if (pPic && pPic->IsTextureLoaded())
			m_nUnknownIconTexId=pPic->GetTextureID();
	}

	if(!bInEditor)
		m_pEntitySystem->SetDynamicEntityIdMode(true);	

#if !defined(_XBOX) && !defined(PS2) && !defined(LINUX)
	SetCursor(NULL);
#endif

	//////////////////////////////////////////////////////////////////////////
	
	DevModeInit();
	m_bOK = true;	
	e_deformable_terrain = NULL;

	return (true);
}

//////////////////////////////////////////////////////////////////////
//! game-mainloop
bool CXGame::Run(bool &bRelaunch)
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
	
	if(m_bDedicatedServer)
	{
		return Update();
	}
	else 
	{
		//static float fLastFrame=0;
		m_bRelaunch=false;
		while(1) 
		{		
			if (!Update()) 
				break;
		}

		bRelaunch=m_bRelaunch;
	}
	return true;
}

#if !defined(_XBOX) && !defined(PS2) && !defined(LINUX)
#include <Mmsystem.h>
#include ".\game.h"
#pragma comment (lib , "Winmm.lib")
#else
#define GetCurrentTime() ((unsigned int)(GetSystem()->GetITimer()->GetCurrTime() * 1000.f))
#endif

//////////////////////////////////////////////////////////////////////////
bool CXGame::IsInPause(IProcess *pProcess)
{
	bool bPause = (m_bMenuOverlay && (!IsMultiplayer()));
	//check if the game is in pause or in menu mode
	if ((pProcess->GetFlags() & PROC_MENU) && !IsMultiplayer())
		bPause=true;

	return (bPause);
}

//////////////////////////////////////////////////////////////////////
//! update all game and children
bool CXGame::Update()
{
	if (!m_nDEBUG_TIMING)
	{
		m_fDEBUG_STARTTIMER = m_pSystem->GetITimer()->GetAsyncCurTime();
		m_nDEBUG_TIMING = 1;
	}

	if (!m_bEditor)
	{
		if (!m_bMenuOverlay || !m_pUISystem || m_pUISystem->GetScriptObjectUI()->CanRenderGame())
		{
			m_p3DEngine->Enable(1);
		}
		else
		{
			m_p3DEngine->Enable(0);
		}
	}

	bool bRenderFrame = (!m_pSystem->GetViewCamera().GetPos().IsZero() || m_bMenuOverlay || m_bUIOverlay) 
											&& g_Render->GetIVal() != 0;

	//////////////////////////////////////////////////////////////////////////
	// Start Profiling frame
	m_pSystem->GetIProfileSystem()->StartFrame();
	//////////////////////////////////////////////////////////////////////////
	FUNCTION_PROFILER( m_pSystem,PROFILE_GAME );

	ITimer *pTimer=m_pSystem->GetITimer();
	pTimer->MeasureTime("EnterGameUp");
	//Timur[10/2/2002]
	// Cannot Run Without System.
	assert( m_pSystem );

	float	maxFPS=g_maxfps->GetFVal();
	if(maxFPS>0)
	{
//		float	extraTime = pTimer->GetAsyncCurTime() + 1.0f/maxFPS - pTimer->GetFrameTime();
//		while(extraTime - pTimer->GetAsyncCurTime() > 0)
//			;
		DWORD	extraTime = (DWORD)((1.0f/maxFPS - pTimer->GetFrameTime())*1000.0f);
#if !defined(LINUX)
		if(extraTime>0&&extraTime<300)//thread sleep not process sleep
			Sleep(extraTime);
#endif
	}

	PhysicsVars *pVars = m_pSystem->GetIPhysicalWorld()->GetPhysVars();
	float fTimeGran=pVars->timeGranularity,	fFixedStep=g_MP_fixed_timestep->GetFVal();
	if (fTimeGran!=m_fTimeGran || fFixedStep!=m_fFixedStep)
	{
		m_fTimeGran = fTimeGran; m_fFixedStep = fFixedStep;
		if (fFixedStep>0)
		{
			m_fTimeGran2FixedStep = fTimeGran/fFixedStep;
			m_iFixedStep2TimeGran = (int)(fFixedStep/fTimeGran+0.5f);
			m_frFixedStep = 1.0f/fFixedStep;
		}
		else
			m_iFixedStep2TimeGran = 0;
		m_frTimeGran = 1.0f/fTimeGran;
	}
	pVars->bMultiplayer = IsMultiplayer() ? 1:0;

	//check what is the current process 
	//bool bPause=false;
	IProcess *pProcess=m_pSystem->GetIProcess();
	if (!pProcess)
		return false;

	bool bPause=IsInPause(pProcess);
	if (m_bIsLoadingLevelFromFile)
		bPause=false;
#if !defined(LINUX)	
	// Pauses or unpauses movie system.
	if (bPause != m_bMovieSystemPaused)
	{
		m_bMovieSystemPaused = bPause;
		if (bPause)
			m_pSystem->GetIMovieSystem()->Pause();
		else
			m_pSystem->GetIMovieSystem()->Resume();
	}
#endif
	// [marco] check current sound and vis areas
	// for music etc.	
	CheckSoundVisAreas();

	//int nStartGC=m_pScriptSystem->GetCGCount();
	pTimer->MeasureTime("SomeStuff");
	// update system
	int nPauseMode=0;
	if (bPause)
		nPauseMode=1;
	// TODO: Add pause mode for a cutscene currently playing (pausemode 2)

	if (IsMultiplayer()) {
		pe_params_flags pf; pf.flagsOR = pef_update;
		for(ListOfPlayers::iterator	pl=m_DeadPlayers.begin(); pl!=m_DeadPlayers.end(); pl++)
			if ((*pl)->GetEntity() && (*pl)->GetEntity()->GetPhysics())
				(*pl)->GetEntity()->GetPhysics()->SetParams(&pf);
	}
	
	if (!m_pSystem->Update(IsMultiplayer() ? ESYSUPDATE_MULTIPLAYER:0, nPauseMode)) //Update returns false when quitting
		return (false);

	if (IsMultiplayer()) {
		pe_params_flags pf; pf.flagsAND = ~pef_update;
		for(ListOfPlayers::iterator	pl=m_DeadPlayers.begin(); pl!=m_DeadPlayers.end(); pl++)
			if ((*pl)->GetEntity() && (*pl)->GetEntity()->GetPhysics())
				(*pl)->GetEntity()->GetPhysics()->SetParams(&pf);
	}

	pTimer->MeasureTime("SysUpdate");
	
	// [marco] after system update, retrigger areas if necessary
	if (!bPause)
		RetriggerAreas();

	if (!bPause || (m_pClient && !m_pClient->IsConnected()))
	{	// network start
		FRAME_PROFILER( "GameUpdate:Client",m_pSystem,PROFILE_GAME );

		// update client
		if (m_pClient)
		{
			m_pClient->UpdateClientNetwork();
			pTimer->MeasureTime("Net");

			assert(m_pClient);
			m_pClient->Update();
			
			if(m_pClient->DestructIfMarked())			//  to make sure the client is only released in one place - here
				m_pClient=0;
		}

		pTimer->MeasureTime("ClServ Up");

		////////UPDATE THE NETWORK 
		// [Anton] moved from after the rendering, so that the server has access to the most recent physics data
		// update server
		if (m_pServer)
		{
			FRAME_PROFILER( "GameUpdate:Server",m_pSystem,PROFILE_GAME );

			m_pServer->Update();
		}

		pTimer->MeasureTime("EndServUp");
	}

	m_pNetwork->UpdateNetwork();	// used to update things like the UBI.com services

	DWORD dwCurrentTimeInMS=GetCurrentTime();

	if (!m_pSystem->IsDedicated())
	{
		if(m_pServerSnooper)
			m_pServerSnooper->Update(dwCurrentTimeInMS);

		if(m_pNETServerSnooper)
			m_pNETServerSnooper->Update(dwCurrentTimeInMS);
	}

	if(m_pRConSystem)
	{
		if (m_pClient)
			m_pRConSystem->Update( dwCurrentTimeInMS,m_pClient->GetNetworkClient() );
		else
			m_pRConSystem->Update(dwCurrentTimeInMS);
	}

	// network end

	// system rendering
	if (bRenderFrame)
	{	
		// render begin must be always called anyway to clear buffer, draw buttons etc.
		// even in menu mode
		m_pSystem->RenderBegin();
		
		m_pSystem->Render();
		pTimer->MeasureTime("3SysRend");
	}
		
	// update the HUD	
	if (m_pCurrentUI && !bPause && m_pClient && m_pClient->m_bDisplayHud)		
	{
		FRAME_PROFILER( "GameUpdate:HUD",m_pSystem,PROFILE_GAME );

		// update hud itself
		if(!m_pCurrentUI->Update())
			m_bUpdateRet = false;

    // update ingame-dialog-manager
		if (m_pIngameDialogMgr)
			m_pIngameDialogMgr->Update();
	
    pTimer->MeasureTime("HUD Up");
	}

	if (m_pUISystem && m_pUISystem->IsEnabled())
	{
		FRAME_PROFILER("GameUpdate:UI", m_pSystem, PROFILE_GAME);

		if (m_bMenuOverlay || m_bUIOverlay)
		{
			m_pUISystem->Update();			
			m_pUISystem->Draw();
		}
	}

	if(a_DrawArea->GetIVal())
  {
		m_XAreaMgr.DrawAreas( m_pSystem );
    pTimer->MeasureTime("XAreaDraw");
  }

	 // print time profiling results
#ifndef PS2
	pTimer->MeasureTime((const char*)-1);
#else
	pTimer->MeasureTime("Time");
#endif

	pTimer->MeasureTime("3TimeProf");

	//NETWORK DEBUGGING
	if(m_pClient && m_pClient->cl_netstats->GetIVal()!=0)
	{
		m_pClient->DrawNetStats();
	}
	if(m_pServer && m_pServer->sv_netstats->GetIVal()!=0)
	{
		m_pServer->DrawNetStats(m_pRenderer);
	}

	pTimer->MeasureTime("NetStats");

	// end of frame
	if (bRenderFrame)
  {	
		// same thing as for render begin		
		if (m_pTimeDemoRecorder)
			m_pTimeDemoRecorder->RenderInfo();

		m_pSystem->RenderEnd();
	}
  pTimer->MeasureTime("3Rend Up");
	
	// get messages from process
	//char *szProcessMessage = m_pSystem->GetIProcess()->GetPMessage();
	// process messages from process
	{
		if (m_fFadingStartTime>0)
		{
			if (((m_pSystem->GetITimer()->GetCurrTime())-m_fFadingStartTime)>1.5f)
			{
				m_fFadingStartTime=-1;
				SendMessage(m_szLoadMsg);
			}
		}

		while(!m_qMessages.empty())
		{
			string smsg=m_qMessages.front();
			m_qMessages.pop();
			ProcessPMessages(smsg.c_str());		
		}

		// the messages can switch the game to menu or viceversa
		bPause=IsInPause(pProcess);
	}
	
	//update script timers
	if(m_pScriptTimerMgr)
		m_pScriptTimerMgr->Update( (unsigned long)(pTimer->GetCurrTime()*1000) );
	
	pTimer->MeasureTime("ScrTimerUp");

	if(g_GC_Frequence->GetFVal()>0)
	{
		// Change Script Garbage collection frequency.
		m_pSystem->SetGCFrequency(g_GC_Frequence->GetFVal());
	}

	//////////////////////////////////////////////////////////////////////////
	// Special update function for developers mode.
	//////////////////////////////////////////////////////////////////////////
	if (IsDevModeEnable())		
		DevModeUpdate();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	pTimer->MeasureTime("EndGameUp");

	//////////////////////////////////////////////////////////////////////////
	// End Profiling Frame
	m_pSystem->GetIProfileSystem()->EndFrame();
	//////////////////////////////////////////////////////////////////////////

	return (m_bUpdateRet);
}

void CXGame::UpdateDuringLoading()
{
/*	// to find areas where the function should be called more often
	{
		static DWORD dwTime=GetTickCount();

		DWORD dwNewTime=GetTickCount();

		DWORD dwRelative = dwNewTime-dwTime;

		if(dwRelative>1000)
			m_pSystem->GetILog()->Log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> UpdateDuringLoading %d",dwRelative);
		else 
			m_pSystem->GetILog()->Log("UpdateDuringLoading %d",dwRelative);

		dwTime=dwNewTime;
	}
*/

	if(m_pServer)
		m_pServer->UpdateXServerNetwork();
}



//////////////////////////////////////////////////////////////////////////
bool CXGame::ParseLevelName(const char *szMsg,char *szLevelName,char *szMissionName)
{	
	const char *szPtr=szMsg;
	char *szDest;

	// get level & mission-name
	szPtr=szMsg+11; //skip "StartLevel "
	//find the level name
	memset(szLevelName,0,32);
	szDest=szLevelName;
	while ((*szPtr) && (*szPtr!=' '))		
		*szDest++=*szPtr++;		

	//find the mission name
	memset(szMissionName,0,32);

	if (*szPtr) //if not eos
	{
		szPtr++; //skip space
		szDest=szMissionName;			
		while (*szPtr)		
			*szDest++=*szPtr++;					
	}

	return (true);
}

//////////////////////////////////////////////////////////////////////
//Process the process messages
//it is needed for example to perform actions that must be done
//the next frame
void CXGame::ProcessPMessages(const char *szMsg)
{
	if (!szMsg) 
		return;

	if ((stricmp(szMsg,"EndDemo") == 0) || (stricmp(szMsg,"EndDemoQuit") == 0))	// used for demos (e3, magazine demos)
	{ 
		ITexPic * pPic = m_pRenderer->EF_LoadTexture("Textures/end_screen.dds", FT_NORESIZE, 0, eTT_Base);
		int img = -1;

		if (pPic)
		{
			img = pPic->GetTextureID();
		}

		GetSystem()->GetIRenderer()->BeginFrame();
		//GetSystem()->GetIRenderer()->Draw2dImage(0,0,(float)(GetSystem()->GetIRenderer()->GetWidth()),(float)(GetSystem()->GetIRenderer()->GetHeight()),img,0,1.0f,1.0f,0);
		// hack! hardcoded to fix the incorrect d3d9 renderer matrix
		GetSystem()->GetIRenderer()->SetState(GS_NODEPTHTEST);
		GetSystem()->GetIRenderer()->Draw2dImage(0,0,(float)(800),(float)(600),img,0,1.0f,1.0f,0);
		GetSystem()->GetIRenderer()->Update();
		float	time = GetSystem()->GetITimer()->GetCurrTime();
		// wait a bit and then quit the game
		while((GetSystem()->GetITimer()->GetCurrTime()-time) < 7)
		{
			GetSystem()->GetITimer()->Update();
		}

		if (stricmp(szMsg,"EndDemoQuit") == 0)
		{
			m_bUpdateRet = false;
		}
		else
		{
			this->SendMessage("Switch");
		}
		return;
	}
	else
	if (stricmp(szMsg,"Quit-Yes") == 0)	// quit message
	{
		m_bUpdateRet = false;
		return;
	}
	else
	if (stricmp(szMsg,"Relaunch") == 0)	// relaunch message
	{
		m_bRelaunch = true;
		m_bUpdateRet = false;
		return;
	}
	else
	if (stricmp(szMsg,"Switch")==0)	// switch process (menu <> game)
	{
		//clear the current message
//		::OutputDebugString("###############switch#############\n");
		//m_pSystem->GetIProcess()->SetPMessage("");				

    if(m_bEditor) return; // we are probably in the editor?

		if (m_bMenuOverlay)				// we're in menu-mode; switch to game
		{
			//check if a game is currently running before switching to
			//the 3d-engine

			CPlayer *pPlayer = 0;
			if (GetMyPlayer())
			{
				GetMyPlayer()->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void **)&pPlayer);
			}
			// there must be a client for us to be able to go out of the menu
			// if this is a singleplayer game, and the player is alive, we can go back to game
			// otherwise, if this is not multiplayer game and the player is dead, it is locked in the menu
			// also, the client must not be loading, or waiting to connect
			if (m_pClient && m_pClient->IsConnected() && m_pUISystem->GetScriptObjectUI()->CanSwitch(0))
			{
				if (IsMultiplayer() || (!pPlayer || pPlayer->m_stats.health > 0))
				{
					MenuOff();
				}
			}
		}	
		else if (m_pUISystem->GetScriptObjectUI()->CanSwitch(1))
		{
//			::OutputDebugString("->menu\n");
      // quit if menu disabled (usefull during development)
			ICVar * pCvarNoMenu = m_pSystem->GetIConsole()->CreateVariable("g_NoMenu","0",0);

      if(pCvarNoMenu && pCvarNoMenu->GetIVal())
			{
				m_pSystem->Quit();

				return;
			}

			MenuOn();
		}			
	}
	else if (stricmp(szMsg, "EndCutScene")==0)
	{
		if (m_bMenuOverlay)				// we're in menu-mode; switch to game
		{
			if (m_pClient)
			{
				MenuOff();
			}
			else if(!IsMultiplayer())
			{
				// there's no game, so lets quit...
				// exit from game-must be prompted with "Are you Sure?"
				m_pSystem->Quit();
			}
		}	
	}
	else if (stricmp(szMsg,"LoadLastCheckPoint")==0)
	{		
		// the loading funciotn calls game->update 20 times(??)
		// and the script menu system allows to click the reload
		// button again during game->update, so be sure that the game isn't
		// loading already
		if (!m_bIsLoadingLevelFromFile) 
		{	
			LoadLatest();
		}
	}
	else if (strnicmp(szMsg,"StartLevelFade",14)==0)		 // start a level with fade out
	{
		if (!m_bEditor)
		{
			ICVar *g_LevelStated = GetISystem()->GetIConsole()->GetCVar("g_LevelStated");

			if (g_LevelStated && g_LevelStated->GetIVal() && m_pClient && m_pClient->IsConnected())
			{
				HSCRIPTFUNCTION pfnDisplayLevelStats = m_pScriptSystem->GetFunctionPtr("Game", "DisplayTimeZone");

				if (pfnDisplayLevelStats)
				{
					m_pScriptSystem->BeginCall(pfnDisplayLevelStats);
					m_pScriptSystem->PushFuncParam(GetScriptObject());
					m_pScriptSystem->EndCall();

					m_pScriptSystem->ReleaseFunc(pfnDisplayLevelStats);
				}
			}
			else
			{
				m_p3DEngine->ResetScreenFx();
				m_p3DEngine->SetScreenFx("ScreenFade",1);
				float fFadeTime=1.5f;
				m_p3DEngine->SetScreenFxParam("ScreenFade","ScreenFadeTime", &fFadeTime);
				m_fFadingStartTime=m_pSystem->GetITimer()->GetCurrTime();
				AllowQuicksave(false);
				sprintf(m_szLoadMsg,"StartLevel%s",szMsg+14); // skip levelfade
				// disable input handling during fading to load
				m_pSystem->GetIInput()->EnableEventPosting(0);
			}
		}
		else
			m_pSystem->GetILog()->Log("Skipping Load Level in editor (%s)",szMsg);		
	}
	else if (strnicmp(szMsg,"StartLevel",10)==0)		 // start a level
	{
		if (!m_bEditor)
		{
			ICVar *g_LevelStated = GetISystem()->GetIConsole()->GetCVar("g_LevelStated");

			if (g_LevelStated && g_LevelStated->GetIVal() && m_pClient && m_pClient->IsConnected())
			{
				HSCRIPTFUNCTION pfnDisplayLevelStats = m_pScriptSystem->GetFunctionPtr("Game", "DisplayTimeZone");

				if (pfnDisplayLevelStats)
				{
					m_pScriptSystem->BeginCall(pfnDisplayLevelStats);
					m_pScriptSystem->PushFuncParam(GetScriptObject());
					m_pScriptSystem->EndCall();

					m_pScriptSystem->ReleaseFunc(pfnDisplayLevelStats);
				}
			}
			else
			{
				char szLevelName[32];
				char szMissionName[32];

				m_p3DEngine->ResetScreenFx();
				// always disable cryvision at start
				m_p3DEngine->SetScreenFx("NightVision",0);

				// reset fading in case someone will load a level
				// while the current one is fading out
				m_fFadingStartTime=-1; 
				m_p3DEngine->SetScreenFx("ScreenFade",0);

				ParseLevelName(szMsg,szLevelName,szMissionName);

				bool listen = false;
				if (strcmp(szMissionName, "listen")==0)
				{
					szMissionName[0] = 0;

					//strcpy( szMissionName,"" );
					listen = true;
				};

				// disable input handling during load
				m_pSystem->GetIInput()->EnableEventPosting(0);
				m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
				LoadLevelCS(false, szLevelName, szMissionName, listen);
				// finished loading, reenable input handling			
				m_pSystem->GetIInput()->EnableEventPosting(1);
				m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			}
		}
		else		
			m_pSystem->GetILog()->Log("Skipping Load Level in editor (%s)",szMsg);		
	}			
	else
	if (strnicmp(szMsg,"SaveGame", 8)==0)		// save current game
	{
		if(!m_bEditor)
		{
			//clear the current message
//			m_pSystem->GetIProcess()->SetPMessage("");				
			const char *sname="quicksave";
			if(strlen(szMsg)>8) { 
				sname=szMsg+9;
			}
			Save(sname, NULL, NULL);			
		}
	}
	else
	if (strnicmp(szMsg,"LoadGame", 8)==0)		// load game
	{
		if(!m_bEditor)
		{
      m_p3DEngine->ResetScreenFx();
			//clear the current message
//			m_pSystem->GetIProcess()->SetPMessage("");				
			const char *sname="quicksave";
			if(strlen(szMsg)>8)
			{
				sname=szMsg+9;
			}

			// disable input handling during load
			m_pSystem->GetIInput()->EnableEventPosting(0);
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
			// get out of all the areas localplayer is in - on load it will be retrigererd
			IEntity *pIMyPlayer = GetMyPlayer();
			if( pIMyPlayer )
			{
				IEntityContainer *pIContainer = pIMyPlayer->GetContainer();
				if (pIContainer)
				{
					CPlayer *pPlayer = NULL;
					if (pIContainer->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
						m_XAreaMgr.ExitAllAreas( pPlayer->m_AreaUser );
				}
			}

			if (Load(sname))
			{
				//if loaded successfull, switch to 3d engine 			
				if(m_pClient)
				{
					m_pSystem->SetIProcess(m_p3DEngine);
					m_pSystem->GetIProcess()->SetFlags(PROC_3DENGINE);								
				}
			}

			// finished loading, reenable input
			m_pSystem->GetIInput()->EnableEventPosting(1);
			//m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
		}
	}
// 	else
//	if (stricmp(szMsg,"ConnectTo")==0)
//	{
//	}
}

//////////////////////////////////////////////////////////////////////////
/*! Load a level on the local machine (connecting with a local client)
	@param dedicated if true the local client will not be created
	@param keepclient if true the current client server connection will be kept
	@param szMapName name of the level that has to be loaded
	@param szMissionName name of the mission that has to be loaded
	@param listen allow external clients to connect
*/
//////////////////////////////////////////////////////////////////////////
void CXGame::LoadLevelCS(bool keepclient, const char *szMapName, const char *szMissionName, bool listen)
{
	// need to reset timers as well
	m_pScriptTimerMgr->Reset();

	if (m_pUISystem)
	{
		m_pUISystem->GetScriptObjectUI()->OnSwitch(0);
		m_pUISystem->StopAllVideo();
		m_p3DEngine->Enable(1);

		m_pSystem->GetILog()->Log("UISystem: Enabled 3D Engine!");
	}
#if !defined(LINUX)	
	if (m_pSystem->GetIMovieSystem())
		m_pSystem->GetIMovieSystem()->StopAllCutScenes();
	//m_lstPlayedCutScenes.clear();
#endif		
	bool bDedicated=GetSystem()->IsDedicated();

	string strGameType = g_GameType->GetString();

	AutoSuspendTimeQuota AutoSuspender(GetSystem()->GetStreamEngine());

	assert( szMissionName != 0 );
	
	string sLevelFolder = szMapName;
	if (sLevelFolder.find('\\') == string::npos && sLevelFolder.find('/') == string::npos)
	{
		// This is just a map name, not a folder.
		sLevelFolder = GetLevelsFolder() + "/" + sLevelFolder;
	}

	IConsole *pConsole=GetSystem()->GetIConsole();
	IInput *pInput=GetSystem()->GetIInput();					// might be 0 (e.g. dedicated server)

	if(pInput)
		pInput->SetMouseExclusive(false);
	
	//if(!*szMissionName) szMissionName = "Multiplayer";

	if (!IsMultiplayer())
	{
		m_pSystem->GetIConsole()->Clear();
		m_pSystem->GetIConsole()->SetScrollMax(600);
		m_pSystem->GetIConsole()->ShowConsole(true);

		string sLoadingScreenTexture = string("levels/") + string(szMapName) + string("/loadscreen_") + string(szMapName) + ".dds";

		m_pSystem->GetIConsole()->SetLoadingImage(sLoadingScreenTexture.c_str());
		m_pSystem->GetIConsole()->ResetProgressBar(0x7fffffff);
		m_pSystem->GetILog()->UpdateLoadingScreen("");	// just to draw the console
	}

	if (m_pClient && !keepclient)
	{
		ShutdownClient();
	}

	// start server
	if((!m_pServer || !keepclient) && !StartupServer(listen))
	{
		m_pLog->LogToConsole("Unable to load the level %s,%s [startup server failed]", sLevelFolder.c_str(),szMissionName);
		if(pInput)
			pInput->SetMouseExclusive(true);
		LoadingError("@LoadLevelError");
		return;
	}

	bool bNeedClient = !bDedicated && ((keepclient && !m_pClient) || !keepclient);

	//create local client(must be before the level is loaded)
	if(bNeedClient)
	{
		if(!StartupLocalClient())
		{
			m_pLog->LogToConsole("Unable to load the level %s,mission %s [startup client failed]", sLevelFolder.c_str(),szMissionName);
			if(pInput)
				pInput->SetMouseExclusive(true);
			LoadingError("@LoadLevelError");
			return;
		}
	}

	const char *szMission = szMissionName;
	if (!*szMissionName)
		szMission=strGameType.c_str();

	// KIRILL lets reset - they will be spawned anyway
	m_pSystem->GetAISystem()->Reset();
	
	// refresh the current server info for incoming queries during loading
	m_pServer->GetServerInfo();

	// load the level
	if(!m_pServer->m_pISystem->LoadLevel( sLevelFolder.c_str(),szMission,false))
	{
		m_pLog->LogToConsole("Unable to load the level %s,mission %s \n", sLevelFolder.c_str(),szMissionName);
		if (pInput)
			pInput->SetMouseExclusive(true);
		LoadingError("@LoadLevelError");
		return;
	}

// start and connect a local client
	if(bNeedClient)
	{
		if(m_pClient)
		{
			if (IsMultiplayer() && m_pServer->m_pIServer->GetServerType()!=eMPST_LAN)
     		m_pClient->XConnect("127.0.0.1",false,true);
			else
				m_pClient->XConnect("127.0.0.1");
		}
	}
	
	if(m_pClient)
    m_pClient->OnMapChanged();
	if(m_pServer)
    m_pServer->OnMapChanged(); 
	if(pInput)
		pInput->SetMouseExclusive(true);
	AllowQuicksave(true);
};

//////////////////////////////////////////////////////////////////////////
bool CXGame::GetLevelMissions( const char *szLevelDir,std::vector<string> &missions )
{
	string sLevelPath = szLevelDir;
	
	if (!szLevelDir || sLevelPath.empty())	
		return false;

	string sEPath = sLevelPath+string("/levelinfo.xml");
	string sPaks = sLevelPath + "/*.pak";
	//GetSystem()->GetIPak()->OpenPacks(sPaks.c_str());	
	OpenPacks(sPaks.c_str());

	XmlNodeRef root = GetSystem()->LoadXmlFile( sEPath.c_str() );
	bool bResult = false;
	if (root)
	{
		XmlNodeRef missionsNode = root->findChild( "Missions" );
		if (missionsNode)
		{
			// we found a mission node - the level is valid
			bResult = true;
			for (int i = 0; i < missionsNode->getChildCount(); i++)
			{
				XmlNodeRef missionNode = missionsNode->getChild(i);
				if (missionNode->isTag( "Mission" ))
				{
					const char *sMissionName = missionNode->getAttr( "Name" );
					if (sMissionName)
						missions.push_back( sMissionName );
				}
			}
		}
	}
	//GetSystem()->GetIPak()->
	ClosePacks(sPaks.c_str());
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::IsMultiplayer()
{
	// cannot be in multiplayer when in editor
	if (m_bEditor)
		return false;

	bool bServer=IsServer();
	bool bClient=IsClient();

	if(!bServer && !bClient)
		return false;

	return !bServer || !bClient || m_pServer->m_bListen;
};

//////////////////////////////////////////////////////////////////////////
void CXGame::ResetState(void)
{
//	m_PlayersWithLighs.clear();
	m_pSystem->GetIEntitySystem()->ResetEntities();
	if (GetMyPlayer())
	{
		m_XAreaMgr.ReTriggerArea(GetMyPlayer(), GetMyPlayer()->GetPos(),false);
		GetMyPlayer()->SetNeedUpdate(true);
	}

	m_pSystem->GetIInput()->Update(1);	// flush the keyboard buffers
	m_pSystem->GetIInput()->Update(1);	// flush the keyboard buffers
	m_pSystem->GetIInput()->GetIKeyboard()->ClearKeyState();
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::ConstrainToSandbox(IEntity *pEntity)
{
	bool bPosChanged = false;
	if (IsMultiplayer())
	{
		Vec3 vPos, vBounds[2]={Vec3(0,0,0),Vec3(0,0,0)};
		IPhysicalEntity *pPhysEnt = pEntity->GetPhysics();
		if (pPhysEnt)
		{
			pe_status_pos sp;
			pPhysEnt->GetStatus(&sp);
			vPos = sp.pos;
		}
		else
			vPos=pEntity->GetPos();
		
		vBounds[1].x = vBounds[1].y = (float)m_p3DEngine->GetTerrainSize();
		vBounds[0].z = -100; vBounds[1].z = 500;
		for(int i=0;i<3;i++) 
			if (!inrange(vPos[i], vBounds[0][i],vBounds[1][i]))
			{
				vPos[i] = vBounds[isneg((vBounds[0][i]+vBounds[1][i])*0.5f-vPos[i])][i];
				bPosChanged = true;
			}

		if (bPosChanged)
		{
			if (pPhysEnt)
			{
				pe_params_pos pp;
				pp.pos = vPos;
				pPhysEnt->SetParams(&pp);
			}
			else
				pEntity->SetPos(vPos);
		}
	}
	return bPosChanged;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::GotoMenu(bool bTriggerOnSwitch)
{
	if(m_bEditor)
		return;

	if(!m_pUISystem)			// e.g. dedicated server
		return;

	if (!IsInMenu())
	{
		DeleteMessage("Switch");
		SendMessage("Switch");
	}
	else if (bTriggerOnSwitch)
	{
		m_pUISystem->GetScriptObjectUI()->OnSwitch(1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::GotoGame(bool bTriggerOnSwitch)
{
	if (m_bEditor)
	{
		return;
	}

	if (IsInMenu())
	{
		DeleteMessage("Switch");
		SendMessage("Switch");
	}
	else if (bTriggerOnSwitch)
	{
		m_pUISystem->GetScriptObjectUI()->OnSwitch(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::MenuOn()
{
	// stop sounds and timers affected by game pause
	m_pSystem->GetISoundSystem()->Pause(true,true); 
	m_pScriptTimerMgr->Pause(true);

	if (m_pSystem->GetIMusicSystem())
	{
		m_pSystem->GetIMusicSystem()->Pause(true);
		//m_pSystem->GetIMusicSystem()->Silence();
	}

	if (m_pUISystem && m_pUISystem->IsEnabled())
	{
		m_pSystem->GetIInput()->AddEventListener(m_pUISystem);
		m_pSystem->GetIInput()->ClearKeyState();
		m_pUISystem->GetScriptObjectUI()->OnSwitch(1);
	}

	m_bMenuOverlay = 1;

	if (!m_bEditor)
	{
		if (m_pUISystem->GetScriptObjectUI()->CanRenderGame())
		{
			m_pSystem->GetILog()->Log("UISystem: Enabled 3D Engine!");
			m_p3DEngine->Enable(1);
		}
		else
		{
			m_pSystem->GetILog()->Log("UISystem: Disabled 3D Engine!");
			m_p3DEngine->Enable(0);
		}
	}
	
	_SmartScriptObject pClientStuff(m_pScriptSystem, true);

	if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))
	{
		m_pScriptSystem->BeginCall("ClientStuff","OnPauseGame");
		m_pScriptSystem->PushFuncParam(pClientStuff);
		m_pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::MenuOff()
{
	// resume sounds and timers affected by game pause
	m_pSystem->GetISoundSystem()->Pause(false);
	m_pSystem->GetIMusicSystem()->Pause(false);
	m_pScriptTimerMgr->Pause(false);


	if (m_pUISystem && m_pUISystem->IsEnabled())
	{
		m_pSystem->GetIInput()->RemoveEventListener(m_pUISystem);
		m_pSystem->GetIInput()->ClearKeyState();
		m_pUISystem->GetScriptObjectUI()->OnSwitch(0);

		if (GetMyPlayer())
			m_XAreaMgr.ReTriggerArea(GetMyPlayer(), GetMyPlayer()->GetPos(),false);
		//					m_XAreaMgr.ReTriggerArea(GetMyPlayer(), m_pSystem->GetISoundSystem()->GetListenerPos(),false);
	}

	m_bMenuOverlay = 0;

	m_pSystem->SetIProcess(m_p3DEngine);
	m_pSystem->GetIProcess()->SetFlags(PROC_3DENGINE);

	if (!m_bEditor)
	{
		m_pSystem->GetILog()->Log("UISystem: Enabled 3D Engine!");
		m_p3DEngine->Enable(1);
	}

	_SmartScriptObject pClientStuff(m_pScriptSystem, true);

	if(m_pScriptSystem->GetGlobalValue("ClientStuff",pClientStuff))
	{
		m_pScriptSystem->BeginCall("ClientStuff","OnResumeGame");
		m_pScriptSystem->PushFuncParam(pClientStuff);
		m_pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::DeleteMessage(const char *szMessage)
{
	StringQueue NewQueue;

	while(!m_qMessages.empty())
	{
		string qMessage = m_qMessages.front();

		m_qMessages.pop();

		if (qMessage != szMessage)
		{
			NewQueue.push(qMessage);
		}
	}

	m_qMessages = NewQueue;
}

//////////////////////////////////////////////////////////////////////////
IScriptObject *CXGame::GetScriptObject()
{
	if (!m_pScriptObjectGame)
	{
		return 0;
	}
	
	return m_pScriptObjectGame->GetScriptObject();
}


void CXGame::PlaySubtitle(ISound * pSound)
{
#if !defined(LINUX)	
	if (m_pMovieUser)
		m_pMovieUser->PlaySubtitles(pSound);
#endif
}

vector2f CXGame::GetSubtitleSize(const string &szMessage, float sizex, float sizey, const string &szFontName, const string &szFontEffect)
{
	IFFont *pFont = m_pSystem->GetICryFont()->GetFont(szFontName.c_str());

	pFont->Reset();
	pFont->SetEffect(szFontEffect.c_str());
	pFont->SetSize(vector2f(sizex, sizey));

	wstring szwString;

	m_StringTableMgr.Localize(szMessage, szwString);

	return pFont->GetTextSizeW(szwString.c_str());
}

void CXGame::WriteSubtitle(const string &szMessage, float x, float y, float sizex, float sizey, const color4f &cColor, const string &szFontName, const string &szFontEffect)
{
	IFFont *pFont = m_pSystem->GetICryFont()->GetFont(szFontName.c_str());

	pFont->Reset();
	pFont->SetEffect(szFontEffect.c_str());
	pFont->SetSize(vector2f(sizex, sizey));
	pFont->SetColor(cColor);

	wstring szwString;

	m_StringTableMgr.Localize(szMessage, szwString);

	pFont->DrawStringW(x, y, szwString.c_str());
}

//////////////////////////////////////////////////////////////////////////
IGameMods* CXGame::GetModsInterface()
{
	return m_pGameMods;	
};

//////////////////////////////////////////////////////////////////////////
void CXGame::LoadingError(const char *szError)
{
	m_pRenderer->ClearColorBuffer(Vec3(0,0,0));
	GetSystem()->GetIConsole()->ResetProgressBar(0);
	m_pSystem->GetIConsole()->ShowConsole(false);
	m_pSystem->GetIConsole()->SetScrollMax(600/2);

	m_pScriptSystem->BeginCall("Game", "OnLoadingError");
	m_pScriptSystem->PushFuncParam(GetScriptObject());
	m_pScriptSystem->PushFuncParam(szError);
	m_pScriptSystem->EndCall();
}
//------------------------------------------------------------------------------------------------- 
bool CXGame::GetCDPath(string &szCDPath)
{
	bool bRet( false );
#ifdef WIN32
	DWORD nBufferSize( GetLogicalDriveStrings( 0, 0 ) );
	if( 0 < nBufferSize )
	{
		// get list of all available logical drives
		tvector rawDriveLetters( nBufferSize + 1 );
		GetLogicalDriveStrings( nBufferSize, &rawDriveLetters[ 0 ] );

		// quickly scan all drives
		tvector::size_type i( 0 );
		while( true )
		{
			// check if current drive is cd/dvd drive
			if( DRIVE_CDROM == GetDriveType( &rawDriveLetters[ i ] ) )
			{
				// get volume name
				tvector cdVolumeName( MAX_VOLUME_ID_SIZE + 1 );
				if( FALSE != GetVolumeInformation( &rawDriveLetters[ i ],
					&cdVolumeName[ 0 ], (DWORD) cdVolumeName.size(), 0, 0, 0, 0, 0 ) )
				{
					// check volume name to verify it's Far Cry's game cd/dvd
					tstring cdVolumeLabel( &cdVolumeName[ 0 ] );
					tstring farCryDisk1Label( _T( "FARCRY_1" ) );
					if( cdVolumeLabel == farCryDisk1Label  )
					{
						// found Far Cry's game cd/dvd, copy information and bail out
						szCDPath = &rawDriveLetters[ i ];
						bRet = true;
						break;
					}
				}
			}

			// proceed to next drive
			while( 0 != rawDriveLetters[ i ] )
			{
				++i;
			}
			++i; // skip null termination of current drive

			// check if we're out of drive letters
			if( 0 == rawDriveLetters[ i ] )
			{
				// double null termination found, bail out
				break;
			}
		}
	}
#endif
	return( bRet );
}

ITagPointManager* CXGame::GetTagPointManager()
{
	return m_pTagPointManager;
}
