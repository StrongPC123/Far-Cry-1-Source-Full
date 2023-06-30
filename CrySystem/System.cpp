//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code
// 
//	File: System.cpp
//  Description: CryENGINE system core-handle all subsystems
// 
//	History:
//	-Jan 31,2001: Originally Created by Marco Corbetta
//	-: modified by all
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "System.h"
#include <time.h>
//#include "ini_vars.h"
#include "CryLibrary.h"

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#else
#include <xtl.h>
#endif

#include <INetwork.h>
#include <I3DEngine.h>
#include <IAISystem.h>
#include <IRenderer.h>
#include <CryMemoryManager.h>
#include <ICryPak.h>
#include <IMovieSystem.h>
#include <IEntitySystem.h>
#include <IInput.h>
#include <ILog.h>
#include <ISound.h>
#include <IGame.h>

#include "CryPak.h"
#include "XConsole.h"
#include "ScriptSink.h"
#include "Font.h" 
#include "Log.h"
#include "CrySizerStats.h"
#include "CrySizerImpl.h"
#include "DownloadManager.h"

#include "XML\Xml.h"
#include "DataProbe.h"
#include "ApplicationHelper.h"			// CApplicationHelper

#include "CryWaterMark.h"
WATERMARKDATA(_m);

//#ifdef WIN32
#define  PROFILE_WITH_VTUNE
//#endif

// profilers api.
VTuneFunction VTResume = NULL;
VTuneFunction VTPause = NULL;

//////////////////////////////////////////////////////////////////////////
// Single current instance of ISystem
//////////////////////////////////////////////////////////////////////////
ISystem* g_System = NULL;
// these heaps are used by underlying System structures
// to allocate, accordingly, small (like elements of std::set<..*>) and big (like memory for reading files) objects
// hopefully someday we'll have standard MT-safe heap
CMTSafeHeap* g_pBigHeap;
CMTSafeHeap* g_pSmallHeap;

#ifdef WIN32
#pragma comment(lib, "WINMM.lib")
#pragma comment(lib, "dxguid.lib")
#endif

#define DLL_GAME_ENTRANCE_FUNCTION	"CreateGameInstance"

#ifdef WIN32
#include "luadebugger/luadbginterface.h"
#include "luadebugger/LuaDbg.h"
extern HMODULE gDLLHandle;
#endif

#ifndef WIN32

extern "C"
{
	IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp);
	IAISystem* CreateAISystem( ISystem *pSystem );
	IMovieSystem* CreateMovieSystem( ISystem *pSystem );
  //struct IXMLDOMDocument *CreateDOMDocument();
}

#endif

// extern.
extern XDOM::IXMLDOMDocument *CreateDOMDocument();
extern int g_nPrecaution;


//////////////////////////////////////////////////////////////////////////
#include "Validator.h"

//////////////////////////////////////////////////////////////////////////
enum {
	nSmallHeapSize =
#ifndef _TEST_
	0x10000 // 64 Kb
#else
	0 // Unlimited
#endif
};

/////////////////////////////////////////////////////////////////////////////////
// System Implementation.
//////////////////////////////////////////////////////////////////////////
CSystem::CSystem():
	m_SmallHeap(0x1000, nSmallHeapSize), // allocate 1 page, grow max to 64 kbytes
	m_BigHeap(0, 0) // perhaps we won't use this, so don't commit initially; grow up to 16 meg - we shouldn't need more at a time just for allocating memory to read pieces of files
{
	g_System = this;
	g_pBigHeap = &m_BigHeap;
	g_pSmallHeap = &m_SmallHeap;

	m_iHeight = 0;
	m_iWidth = 0;
	m_iColorBits = 0;

	//#ifndef _XBOX
#ifdef WIN32	
	m_hInst = NULL;
	m_hWnd = NULL;
	
#endif

	//////////////////////////////////////////////////////////////////////////
	// Reset handles.
	memset( &m_dll,0,sizeof(m_dll) );
	//////////////////////////////////////////////////////////////////////////

	m_pLuaDebugger = NULL;
	m_pLog = NULL;
	m_pStreamEngine = NULL;
  m_pIPak = NULL;
	m_pIInput = NULL;
	m_pRenderer = NULL;	
	m_pISound = NULL;
	m_pIMusic = NULL;
	m_pICryFont = NULL;
	m_pIFont = NULL;
	m_pScriptSystem=NULL;
	m_pIMovieSystem = NULL;
	m_pI3DEngine=NULL;
	m_pICryCharManager = NULL;
	m_pAISystem=NULL;
	m_pEntitySystem = NULL;
	m_pConsole = NULL;
	m_pNetwork = NULL;
	m_rWidth=NULL;
	m_rHeight=NULL;
	m_rColorBits=NULL;
	m_rDepthBits=NULL;
	m_cvSSInfo=NULL;
	m_rStencilBits=NULL;
	m_rFullscreen=NULL;
	m_rDriver=NULL;
	m_sysNoUpdate=NULL;
	m_pMemoryManager = NULL;
	m_pProcess = NULL;
	m_pGame = NULL;
	m_pIPhysicalWorld=NULL;
	m_pValidator = NULL;
	m_pDefaultValidator = NULL;
	m_sys_StreamCallbackTimeBudget=0;
	m_sys_StreamCompressionMask=0;

	m_pScriptBindings=NULL;
	//[Timur] m_CreateDOMDocument = NULL;

	m_cvAIUpdate = NULL;
	i_direct_input = NULL;

	m_pScriptSink = NULL;
	m_pUserCallback = NULL;
	sys_script_debugger = NULL;
	m_sysWarnings = NULL;
	m_sys_profile = NULL;
	m_sys_profile_graphScale = NULL;
	m_sys_profile_pagefaultsgraph = NULL;
	m_sys_profile_graph = NULL;
	m_sys_profile_filter = NULL;
	m_sys_profile_network = NULL;
	m_sys_profile_peak = NULL;
	m_sys_profile_memory = NULL;
	m_sys_spec = NULL;
	m_sys_firstlaunch = NULL;
	m_pCpu = NULL;

	m_profile_old = 0;

	m_bQuit = false;
	m_bRelaunch=false;
	m_bRelaunched=false;
	m_bTestMode = false;
	m_bEditor = false;
	m_bIgnoreUpdates = false;

	m_nStrangeRatio = 1000;
	// no mem stats at the moment
	m_pMemStats = NULL;
	m_pSizer = NULL;

	m_pCVarQuit=NULL;

	m_pDownloadManager = 0;

	// default game MOD is root
	memset(m_szGameMOD,0,256);
#if defined( _DATAPROBE )
	m_pDataProbe = new CDataProbe;
#endif
	m_bForceNonDevMode=false;
	m_bWasInDevMode = false;
	m_bInDevMode = false;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
CSystem::~CSystem()
{
	ShutDown(m_bRelaunch);

	FreeLib(m_dll.hNetwork);
	FreeLib(m_dll.hAI);
	FreeLib(m_dll.hInput);
	FreeLib(m_dll.hScript);
	FreeLib(m_dll.hPhysics);
	FreeLib(m_dll.hEntitySystem);
	FreeLib(m_dll.hRenderer);
	FreeLib(m_dll.hSound);
	FreeLib(m_dll.hFlash);
	FreeLib(m_dll.hFont);
	FreeLib(m_dll.hMovie);
	FreeLib(m_dll.hIndoor);
	FreeLib(m_dll.h3DEngine);
	FreeLib(m_dll.hAnimation);
	FreeLib(m_dll.hGame);
	SAFE_DELETE(m_pDataProbe);
	g_System = NULL;
	g_pBigHeap = NULL;
	g_pSmallHeap = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::FreeLib(IN OUT HMODULE hLibModule)
{
	if (hLibModule) 
	{ 
		CryFreeLibrary(hLibModule);
		(hLibModule)=NULL; 
	} 
}

//////////////////////////////////////////////////////////////////////////
void CSystem::SetGCFrequency( const float fRate )
{
	if(m_pScriptSink)
		m_pScriptSink->SetGCFreq( fRate );
	else
	{
		assert(0);  
	}
}

//////////////////////////////////////////////////////////////////////////
WIN_HMODULE CSystem::LoadDLL( const char *dllName,bool bQuitIfNotFound)
{ 
	WIN_HMODULE handle = CryLoadLibrary( dllName ); 

	if (!handle)      
	{
#if defined(LINUX)
		printf ("Error loading DLL: %s, error :  %s\n", dllName, dlerror());
		if (bQuitIfNotFound)
			Quit();
		else
			return 0;
#else
		if (bQuitIfNotFound)
		{		
			Error( "Error loading DLL: %s, error code %d",dllName, GetLastError());
			Quit();
		}
		return 0;
	#endif //LINUX
	}
#if defined(_DATAPROBE) && !defined(LINUX)
	IDataProbe::SModuleInfo module;
	module.filename = dllName;
	module.handle = gDLLHandle;
	m_pDataProbe->AddModule(module);
#endif
	return handle;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::ShowDebugger(const char *pszSourceFile, int iLine, const char *pszReason)
{
#ifdef WIN32
	if (m_bInDevMode)
	{	
		if (GetLuaDebugger() != NULL)
			::InvokeDebugger(GetLuaDebugger(), pszSourceFile, iLine, pszReason);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
IConsole *CSystem::GetIConsole()
{
	return m_pConsole;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::SetForceNonDevMode( const bool bValue )
{
	m_bForceNonDevMode=bValue;
	if (bValue)
		SetDevMode(false);
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::GetForceNonDevMode() const
{
	return m_bForceNonDevMode;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::SetDevMode( bool bEnable )
{
	if (!bEnable)
	{
		// set pak priority to files inside the pak to avoid
		// trying to open unnecessary files and to avoid cheating
		if (sys_script_debugger)
			sys_script_debugger->ForceSet("0");
		if (m_cvPakPriority)
			m_cvPakPriority->ForceSet("1");
	}
	else
	{
		if (m_cvPakPriority)
			m_cvPakPriority->ForceSet("0");
	}
	if (bEnable)
		m_bWasInDevMode = true;
	m_bInDevMode = bEnable;
}

///////////////////////////////////////////////////
void CSystem::ShutDown(bool bRelaunch)
{		
	CryLogAlways("System Shutdown");
	
	if (m_pISound)
	{
		// turn EAX off otherwise it affects all Windows sounds!
		m_pISound->SetEaxListenerEnvironment(EAX_PRESET_OFF);
	}

	m_FrameProfileSystem.Done();

	if (m_sys_firstlaunch)
		m_sys_firstlaunch->Set( "0" );

	if (m_bEditor)
	{
		// restore the old saved cvars
		m_pConsole->GetCVar("r_Width")->Set(m_iWidth);
		m_pConsole->GetCVar("r_Height")->Set(m_iHeight);
		m_pConsole->GetCVar("r_ColorBits")->Set(m_iColorBits);
	}

	if (m_bEditor && !m_bRelaunch)
	{
		SaveConfiguration();
	}

	if (bRelaunch)
	{
		SaveConfiguration();
		/*
		// Release threads.
		SAFE_RELEASE(m_pGame); 
		SAFE_RELEASE(m_pIMusic);
		SAFE_RELEASE(m_pISound);
		SAFE_RELEASE(m_pNetwork);
		SAFE_DELETE(m_pStreamEngine);
		SAFE_RELEASE(m_pRenderer); 
		*/
		return;
	}

	//if (!m_bEditor && !bRelaunch)
	if (!m_bEditor)
	{	 		
		if (m_pCVarQuit && m_pCVarQuit->GetIVal())		
		{
			SaveConfiguration();

			SAFE_RELEASE(m_pGame);
			FreeLib(m_dll.hGame);

			SAFE_RELEASE(m_pRenderer);
      FreeLib(m_dll.hRenderer);

			if (!bRelaunch)
			{
#if defined(LINUX)
				return;//safe clean return
#else
				exit(EXIT_SUCCESS);	
#endif
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Release Game.
	//////////////////////////////////////////////////////////////////////////	
	if (m_pEntitySystem)
	  m_pEntitySystem->Reset();
	SAFE_RELEASE(m_pGame); 
	if (m_pIPhysicalWorld)
	{
		m_pIPhysicalWorld->SetPhysicsStreamer(0);
		m_pIPhysicalWorld->SetPhysicsEventClient(0);
	}

	ShutDownScriptBindings();

	//prevent the script system to try to delete non script data
	if (m_pScriptSystem)
	{
		m_pScriptSystem->ForceGarbageCollection();
		m_pScriptSystem->UnbindUserdata();
	}

	//////////////////////////////////////////////////////////////////////////
	// Clear 3D Engine resources.
	if (m_pI3DEngine)
		m_pI3DEngine->ClearRenderResources(m_bEditor);
	//////////////////////////////////////////////////////////////////////////

	// Release console variables.
	
	SAFE_RELEASE(m_pCVarQuit);
	SAFE_RELEASE(m_rWidth);
	SAFE_RELEASE(m_rHeight);
	SAFE_RELEASE(m_rColorBits);
	SAFE_RELEASE(m_rDepthBits);
	SAFE_RELEASE(m_cvSSInfo);
	SAFE_RELEASE(m_rStencilBits);
	SAFE_RELEASE(m_rFullscreen);
	SAFE_RELEASE(m_rDriver);

	SAFE_RELEASE(m_sysWarnings);
	SAFE_RELEASE(m_sys_profile);
	SAFE_RELEASE(m_sys_profile_graph);
	SAFE_RELEASE(m_sys_profile_pagefaultsgraph);
	SAFE_RELEASE(m_sys_profile_graphScale);
	SAFE_RELEASE(m_sys_profile_filter);
	SAFE_RELEASE(m_sys_profile_network);
	SAFE_RELEASE(m_sys_profile_peak);
	SAFE_RELEASE(m_sys_profile_memory);
	SAFE_RELEASE(m_sys_spec);
	SAFE_RELEASE(m_sys_firstlaunch);
	SAFE_RELEASE(m_sys_StreamCallbackTimeBudget);
	SAFE_RELEASE(m_sys_StreamCompressionMask);

#ifdef WIN32
	if (m_pLuaDebugger)
	{
		delete m_pLuaDebugger;
		m_pLuaDebugger = NULL;
	}
#endif

	if (m_pIInput)
	{
		m_pIInput->ShutDown();
		m_pIInput = NULL;
	}

	SAFE_RELEASE(m_pIMovieSystem);
	SAFE_RELEASE(m_pEntitySystem);
	SAFE_RELEASE(m_pAISystem);
	SAFE_RELEASE(m_pICryFont);
	SAFE_RELEASE(m_pIMusic);
	SAFE_RELEASE(m_pISound);
	SAFE_RELEASE(m_pNetwork);
	SAFE_RELEASE(m_pICryCharManager);
	SAFE_RELEASE(m_pI3DEngine);
	SAFE_RELEASE(m_pIPhysicalWorld);
  if (m_pConsole)
    m_pConsole->FreeRenderResources();
	SAFE_RELEASE(m_pRenderer);

	SAFE_DELETE(m_pIPak);

	SAFE_RELEASE(m_pConsole);
	SAFE_RELEASE(m_pScriptSystem);
	SAFE_DELETE( m_pScriptSink );

	SAFE_DELETE(m_pMemStats);
	SAFE_DELETE(m_pSizer);
	SAFE_DELETE(m_pStreamEngine);
	SAFE_DELETE(m_pDefaultValidator);

	SAFE_RELEASE(m_pDownloadManager);

	if (m_pLog)
		m_pLog->EnableVerbosity(false);	// in order for the logs after this line to work
	
	DebugStats(false, false);//true);
	CryLogAlways("");
	CryLogAlways("release mode memory manager stats:");
	DumpMMStats(true);

	// Log must be last thing released.
	SAFE_RELEASE(m_pLog);
	SAFE_DELETE(m_pCpu);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void CSystem::Quit()
{
	m_bQuit=true;
#ifdef WIN32
	if (m_bEditor)
	{
		PostQuitMessage(0);
	}
#endif
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::IsQuitting()
{
	return (m_bQuit);
}
 




// to parse the command-line in a consistent way
class CCommandLineSink_ConsoleCommands :public CApplicationHelper::ICmdlineArgumentSink
{
public:
	//! constructor
	CCommandLineSink_ConsoleCommands( CSystem &rSystem ) :m_rSystem(rSystem)
	{
	}

	virtual void ReturnArgument( const char *inszArgument )
	{
		m_rSystem.GetIConsole()->ExecuteString(inszArgument);
	}

	// ---------------------------------------------------------------

	CSystem &						m_rSystem;				//!< reference to the system
};




//////////////////////////////////////////////////////////////////////////
bool CSystem::CreateGame( const SGameInitParams &params )
{
#if defined(WIN32) || defined(LINUX)
	if (m_bEditor)
	{
		//////////////////////////////////////////////////////////////////////////
		// SCRIPT BINDINGS
		//////////////////////////////////////////////////////////////////////////
		CryLogAlways("Initializing Script Bindings");
		if(!InitScriptBindings())
		{
			return false;
		}
	}

	if (params.pGame)
	{
		m_pGame = params.pGame;
		return true;
	}

	if (!params.sGameDLL)
	{
		Error( "Error in CSystem::CreateGame, Game DLL filename not specified" );
		return false;
	}

	char szDLLname[256];
	strcpy(szDLLname,params.sGameDLL);
	
	// try to load a game.dll from the MOD folder
	if (m_szGameMOD[0]) 
	{
		char szFolderName[256];
#if defined(WIN64)
		sprintf(szFolderName,"mods\\%s\\bin64\\%s",m_szGameMOD,szDLLname);
#elif defined(LINUX32)
		sprintf(szFolderName,"../mods/%s/bin32linux/%s",m_szGameMOD,szDLLname);
#elif defined(LINUX64)
		sprintf(szFolderName,"../mods/%s/bin64linux/%s",m_szGameMOD,szDLLname);
#else
		sprintf(szFolderName,"mods\\%s\\bin32\\%s",m_szGameMOD,szDLLname);
#endif
		strcpy(szDLLname,szFolderName); 
		m_dll.hGame = LoadDLL(szDLLname,false);
		if (!m_dll.hGame)
		{
			strcpy(szDLLname,params.sGameDLL);
			m_dll.hGame = LoadDLL(szDLLname);
		}
	}
	else
	{	
		m_dll.hGame = LoadDLL(szDLLname);
	}

	if (!m_dll.hGame)
		return false;
	

	PFNCREATEGAMEINSTANCE pfCreateGameInstance;
	pfCreateGameInstance = (PFNCREATEGAMEINSTANCE) CryGetProcAddress(m_dll.hGame,DLL_GAME_ENTRANCE_FUNCTION);
	if ( pfCreateGameInstance == NULL )
		return false;

	m_pGame = pfCreateGameInstance();

	m_pGame->Init(this, params.bDedicatedServer,m_bEditor,m_szGameMOD);
	/*
	if (m_szGameMOD[0])
	{
		// apply the mod without restarting as the game just started!
		m_pGame->GetModsInterface()->SetCurrentMod(m_szGameMOD,false);
	}
	*/

	if (m_pIPhysicalWorld)
	{
		m_pIPhysicalWorld->SetPhysicsStreamer(m_pGame->GetPhysicsStreamer());
		m_pIPhysicalWorld->SetPhysicsEventClient(m_pGame->GetPhysicsEventClient());
	}

	if (m_bInDevMode)
	{
		m_bWasInDevMode = true;
		CryLogAlways("DEVMODE is Enabled");

		ICVar	*pCVar=m_pConsole->GetCVar("zz0x067MD4");
		if (pCVar)
		{ 
			pCVar->Set("DEVMODE");
			if(m_pScriptSystem->ExecuteFile("DevMode.Lua",false))
				CryLogAlways("   Loading DevMode.lua: Ok!");
		}
	}
			
#else
	if (params.pGame)
	{
		m_pGame = params.pGame;
	}
	else
	{
		m_pGame = CreateGameInstance();
		m_pGame->Init(this, m_bEditor);
	}

	if (m_pIPhysicalWorld)
	{
		m_pIPhysicalWorld->SetPhysicsStreamer(m_pGame->GetPhysicsStreamer());
		m_pIPhysicalWorld->SetPhysicsEventClient(m_pGame->GetPhysicsEventClient());
	}
#endif

	if (!m_pGame)
	{
		Error( "Error Creating Game Interface" );
		return false;
	}

	/*
	// set log verbosity to 8 if dev mode enabled
	if (m_pGame->IsDevModeEnable())
	{	
		ICVar *pCVar=m_pConsole->GetCVar("log_Verbosity");
		if (pCVar)
			pCVar->Set("8");
		pCVar=m_pConsole->GetCVar("log_FileVerbosity");
		if (pCVar)
			pCVar->Set("8");
	}
	*/

	if (!m_bEditor && m_szGameMOD[0] && (stricmp(m_szGameMOD,"FarCry")!=0))
	{
		// execute modexe.lua, to for instance launch a map immediately
		// for an sp game or a Total Conversion
		// modexe.lua must be inside the pak, for mp security check		
		m_pScriptSystem->ExecuteFile("ModExe.lua");
	}


	// parse command line arguments without minus e.g. "g_gametype ASSAULT" "start_server testy"
	{
		CCommandLineSink_ConsoleCommands CmdlineSink(*this);

		CApplicationHelper::ParseArguments(params.szGameCmdLine,0,&CmdlineSink);
	}



	return true;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::SetIProcess(IProcess *process)
{
	m_pProcess = process; 
	//if (m_pProcess)
		//m_pProcess->SetPMessage("");
}

// update _time, _frametime
//////////////////////////////////////////////////////////////////////////
void CSystem::UpdateScriptSink()
{
	assert(m_pScriptSink);

	if(m_pScriptSink)
		m_pScriptSink->Update(false);		// LUA Garbage collection might be called in here
}

// nPauseMode: 0=normal(no pause)
// nPauseMode: 1=menu/pause
// nPauseMode: 2=cutscene
//////////////////////////////////////////////////////////////////////
bool CSystem::Update( int updateFlags, int nPauseMode )
{
	FUNCTION_PROFILER( this,PROFILE_SYSTEM );

	if (m_pGame)
	{
		bool bDevMode = m_pGame->GetModuleState( EGameDevMode );
		if (bDevMode != m_bInDevMode)
			SetDevMode(bDevMode);
	}
#ifdef PROFILE_WITH_VTUNE
	if (m_bInDevMode)
	{	
		if (VTPause != NULL && VTResume != NULL)
		{
			static bool bVtunePaused = true;
			
			bool bPaused = false;
			
			if (GetISystem()->GetIInput())
			{
				bPaused = !(GetISystem()->GetIInput()->GetKeyState(XKEY_SCROLLLOCK) & 1);
			}

			{
				if (bVtunePaused && !bPaused)
				{
					GetILog()->LogToConsole("VTune Resume");
					VTResume();
				}
				if (!bVtunePaused && bPaused)
				{
					VTPause();
					GetILog()->LogToConsole("VTune Pause");
				}
				bVtunePaused = bPaused;
			}
		}
	}
#endif //PROFILE_WITH_VTUNE

	m_Time.MeasureTime("Enter SysUp");	
	if (m_pStreamEngine)
		m_pStreamEngine->Update(0);
	
	m_pStreamEngine->SetCallbackTimeQuota( m_sys_StreamCallbackTimeBudget->GetIVal() );
	m_pStreamEngine->SetStreamCompressionMask( m_sys_StreamCompressionMask->GetIVal() );

	if (m_pICryCharManager)
		m_pICryCharManager->Update();

	if (m_bIgnoreUpdates)
		return true;

  //static bool sbPause = false; 
	//bool bPause = false;

	//check what is the current process 
	IProcess *pProcess=GetIProcess();
	if (!pProcess)
		return (true); //should never happen

	bool bNoUpdate = false;
	if (m_sysNoUpdate && m_sysNoUpdate->GetIVal())
	{
		bNoUpdate = true;
		updateFlags = ESYSUPDATE_IGNORE_AI | ESYSUPDATE_IGNORE_PHYSICS;
	}

	//if ((pProcess->GetFlags() & PROC_MENU) || (m_sysNoUpdate && m_sysNoUpdate->GetIVal()))
	//		bPause = true;

	//check if we are quitting from the game
	if (IsQuitting())
		return (false);
	
	

#ifndef _XBOX
#ifdef WIN32
  // process window messages
	{
		FRAME_PROFILER( "SysUpdate:PeekMessage",this,PROFILE_SYSTEM );

		if (m_hWnd && ::IsWindow((HWND)m_hWnd))
		{
			MSG msg;
			while (PeekMessage(&msg, (HWND)m_hWnd, 0, 0, PM_REMOVE))       
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
  }
#endif
#endif

	m_Time.MeasureTime("WndMess");

	//////////////////////////////////////////////////////////////////////
	//update time subsystem	
	m_Time.Update();

	float fFrameTime = m_Time.GetFrameTime();

	//////////////////////////////////////////////////////////////////////////
	// Update script system.
	if (m_pScriptSink)
	{
		FRAME_PROFILER( "SysUpdate:ScriptSink",this,PROFILE_SYSTEM );

		UpdateScriptSink();
	}
	
	if (m_pIInput)
	{
		if (!(updateFlags&ESYSUPDATE_EDITOR))
		{
			//////////////////////////////////////////////////////////////////////
			//update input system
#if defined(_XBOX) || defined(LINUX)
			m_pIInput->Update(true);
#else
			bool bFocus = (GetFocus()==m_hWnd) || m_bEditor;
			m_pIInput->Update(bFocus);
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////
	//update console system
	if (m_pConsole)
	{
		FRAME_PROFILER( "SysUpdate:Console",this,PROFILE_SYSTEM );

		if (!(updateFlags&ESYSUPDATE_EDITOR))
		{
			m_pConsole->Update();
			m_Time.MeasureTime("TmInConUp");
		}
	}

	//////////////////////////////////////////////////////////////////////	
	// update physic system	
	//static float time_zero = 0;
	if ((nPauseMode!=1) && !(updateFlags&ESYSUPDATE_IGNORE_PHYSICS))
	{
		FRAME_PROFILER( "SysUpdate:physics",this,PROFILE_SYSTEM );

		float fCurTime = m_Time.GetCurrTime();
		float fPrevTime = m_pIPhysicalWorld->GetPhysicsTime();

		int iPrevTime=m_pIPhysicalWorld->GetiPhysicsTime(), iCurTime;
		if (!(updateFlags&ESYSUPDATE_MULTIPLAYER))
			m_pIPhysicalWorld->TimeStep(fFrameTime);
		else
		{
			if (m_pGame->UseFixedStep())
			{
				m_pIPhysicalWorld->TimeStep(fCurTime-fPrevTime, 0);
				iCurTime = m_pIPhysicalWorld->GetiPhysicsTime();

				m_pIPhysicalWorld->SetiPhysicsTime(m_pGame->SnapTime(iPrevTime));
				int i,iInterval, iStep=m_pGame->GetiFixedStep();
				float fFixedStep = m_pGame->GetFixedStep();
				iInterval = min(20*iStep,m_pGame->SnapTime(iCurTime)-m_pGame->SnapTime(iPrevTime));
				for(i=iInterval; i>0; i-=iStep)
				{
					m_pGame->ExecuteScheduledEvents();
					m_pIPhysicalWorld->TimeStep(fFixedStep, ent_rigid|ent_skip_flagged);
				}

				m_pIPhysicalWorld->SetiPhysicsTime(iPrevTime);
				m_pIPhysicalWorld->TimeStep(fCurTime-fPrevTime, ent_rigid|ent_flagged_only);

				m_pIPhysicalWorld->SetiPhysicsTime(iPrevTime);
				m_pIPhysicalWorld->TimeStep(fCurTime-fPrevTime, ent_living|ent_independent|(iInterval>0 ? ent_deleted:0));
			}
			else
				m_pIPhysicalWorld->TimeStep(fCurTime-fPrevTime);

			if (fabsf(m_pIPhysicalWorld->GetPhysicsTime()-fCurTime)>0.01f)
			{
				GetILog()->LogToConsole("Adjusting physical world clock by %.5f", fCurTime-m_pIPhysicalWorld->GetPhysicsTime());
				m_pIPhysicalWorld->SetPhysicsTime(fCurTime);
			}
		}
		m_Time.MeasureTime("PhysicsUp");
	}

	if ((nPauseMode==0) && !(updateFlags&ESYSUPDATE_IGNORE_AI))
	{
		FRAME_PROFILER( "SysUpdate:AI",this,PROFILE_SYSTEM );
		//////////////////////////////////////////////////////////////////////
		//update AI system
		if (m_pAISystem && !m_cvAIUpdate->GetIVal())
			m_pAISystem->Update();
		m_Time.MeasureTime("AISys Up");
	}

	if ((nPauseMode!=1))
	{
		//////////////////////////////////////////////////////////////////////
		//update entity system	
		if (m_pEntitySystem && !bNoUpdate)
			m_pEntitySystem->Update();
		m_Time.MeasureTime("EntSys Up");
	}

	//////////////////////////////////////////////////////////////////////////
	// Update movie system (Must be after updating EntitySystem and AI.
	//////////////////////////////////////////////////////////////////////////	
	// the movie system already disables AI physics etc.
	{
		if (m_pIMovieSystem && !(updateFlags&ESYSUPDATE_EDITOR) && !bNoUpdate)
		{
			float fMovieFrameTime = fFrameTime;
			if (fMovieFrameTime > 0.1f) // Slow frame rate fix.
				fMovieFrameTime = 0.1f;
			m_pIMovieSystem->Update(fMovieFrameTime);
			m_Time.MeasureTime("MovieSys");
		}
	}

	//////////////////////////////////////////////////////////////////////
	//update process (3D engine)
	if (!(updateFlags&ESYSUPDATE_EDITOR) && !bNoUpdate)
	{
		if (m_pProcess && (m_pProcess->GetFlags() & PROC_3DENGINE))
		{
			if ((nPauseMode!=1))
			if (!IsEquivalent(m_ViewCamera.GetPos(),Vec3(0,0,0),VEC_EPSILON))
			{			
				if (m_pI3DEngine)
				{
					m_pI3DEngine->SetCamera(m_ViewCamera);			
					m_pProcess->Update();

					//////////////////////////////////////////////////////////////////////////
					// Strange, !do not remove... ask Timur for the meaning of this.
					//////////////////////////////////////////////////////////////////////////
					if (m_nStrangeRatio > 32767)
					{
						g_nPrecaution = 1 + (rand()%3); // lets get nasty.
					}
					//////////////////////////////////////////////////////////////////////////
				}
			}
		}
		else
		{
			if (m_pProcess)
				m_pProcess->Update();	    
		}
	}


	m_Time.MeasureTime("3DEng Up"); // I3DEngine::Update() is empty

	//////////////////////////////////////////////////////////////////////
	//update sound system
  if ((nPauseMode!=1) && m_pISound && !bNoUpdate)
	{
		FRAME_PROFILER( "SysUpdate:Sound",this,PROFILE_SYSTEM );

		if (updateFlags&ESYSUPDATE_EDITOR)
		{
			// Only In Editor mode also update listener position.
			m_pISound->SetListener(m_ViewCamera,Vec3(0,0,0));
		}

    m_pISound->Update();
		m_Time.MeasureTime("SoundSysUp");
	}

	if (m_pIMusic && !bNoUpdate)
	{
		FRAME_PROFILER( "SysUpdate:Music",this,PROFILE_SYSTEM );

		m_pIMusic->Update();
		m_Time.MeasureTime("MusicSysUp");
	}

	if (m_pDownloadManager && !bNoUpdate)
	{
		m_pDownloadManager->Update();
	}

	//////////////////////////////////////////////////////////////////////////
	// Strange, !do not remove... ask Timur for the meaning of this.
	//////////////////////////////////////////////////////////////////////////
	if (!(updateFlags&ESYSUPDATE_EDITOR) && !bNoUpdate && m_nStrangeRatio > 1000)
	{
		if (m_pProcess && (m_pProcess->GetFlags() & PROC_3DENGINE))
			m_nStrangeRatio += (1 + (10*rand())/RAND_MAX);
	}

	return !m_bQuit;	
}

//////////////////////////////////////////////////////////////////////////
// XML stuff
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
XDOM::IXMLDOMDocument *CSystem::CreateXMLDocument()
{
	return CreateDOMDocument();
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CSystem::CreateXmlNode( const char *sNodeName )
{
	return new CXmlNode( sNodeName );
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CSystem::LoadXmlFile( const char *sFilename )
{
	XmlParser parser;
	XmlNodeRef node = parser.parse( sFilename );
	return node;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CSystem::LoadXmlFromString( const char *sXmlString )
{
	XmlParser parser;
	XmlNodeRef node = parser.parseBuffer( sXmlString );
	return node;
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::CheckLogVerbosity( int verbosity )
{
	if (verbosity <= m_pLog->GetVerbosityLevel())
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Warning( EValidatorModule module,EValidatorSeverity severity,int flags,const char *file,const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[MAX_WARNING_LENGTH];
	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	SValidatorRecord record;
	record.file = file;
	record.text = szBuffer;
	record.module = module;
	record.severity = severity;
	record.flags = flags;
	m_pValidator->Report( record );
}

//////////////////////////////////////////////////////////////////////////
ISystem *GetISystem()
{
	return (g_System);
}

#ifdef USE_FRAME_PROFILER
//////////////////////////////////////////////////////////////////////////
void CSystem::StartProfilerSection( CFrameProfilerSection *pProfileSection )
{
	m_FrameProfileSystem.StartProfilerSection( pProfileSection );
}

//////////////////////////////////////////////////////////////////////////
void CSystem::EndProfilerSection( CFrameProfilerSection *pProfileSection )
{
	m_FrameProfileSystem.EndProfilerSection( pProfileSection );
}
#endif //USE_FRAME_PROFILER

//////////////////////////////////////////////////////////////////////////
void CSystem::VTuneResume()
{
#ifdef PROFILE_WITH_VTUNE
	if (VTResume)
		VTResume();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::VTunePause()
{
#ifdef PROFILE_WITH_VTUNE
	if (VTPause)
		VTPause();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Deltree(const char *szFolder, bool bRecurse)
{
	__finddata64_t fd;
	string filespec = szFolder;
	filespec += "*.*";

	intptr_t hfil = 0;
	if ((hfil = _findfirst64(filespec.c_str(), &fd)) == -1)
	{
		return;
	}

	do
	{
		if (fd.attrib & _A_SUBDIR)
		{
			string name = fd.name;

			if ((name != ".") && (name != ".."))
			{
				if (bRecurse)
				{
					name = szFolder;
					name += fd.name;
					name += "/";

					Deltree(name.c_str(), bRecurse);
				}
			}
		}
		else
		{
			string name = szFolder;
			
			name += fd.name;

			DeleteFile(name.c_str());
		}

	} while(!_findnext64(hfil, &fd));

	_findclose(hfil);

	RemoveDirectory(szFolder);
}


//////////////////////////////////////////////////////////////////////////
void CSystem::OpenBasicPaks()
{

	const char *szLanguage = NULL;
	m_pScriptSystem->GetGlobalValue("g_language", szLanguage);
	//////////////////////////////////////////////////////////////////////////
	// load language pak
	if (!szLanguage)
	{
		// if the language value cannot be found, let's default to the english pak
		OpenLanguagePak("english"); 
	}
	else
	{ 
		OpenLanguagePak(szLanguage);
	}
	
	string paksFolder = string(DATA_FOLDER)+"/*.pak";
	// Open all *.pak files in root folder.
	m_pIPak->OpenPacks( "*.pak" );
	m_pIPak->OpenPacks( "",paksFolder.c_str() );
}

//////////////////////////////////////////////////////////////////////////
void CSystem::OpenLanguagePak( const char *sLanguage )
{	
	// load language pak
	char szPakName[_MAX_PATH];
	sprintf(szPakName,"%s/Localized/%s.pak",DATA_FOLDER,sLanguage );
	if (!m_pIPak->OpenPack( "",szPakName ))
	{
		// make sure the localized language is found - not really necessary, for TC		
		CryLogAlways("Localized language content(%s - %s) not available or modified from the original installation.",sLanguage,szPakName);
	}

	// load patch language data
	memset(szPakName,0,_MAX_PATH);
	sprintf(szPakName,"%s/Localized/%s1.pak",DATA_FOLDER,sLanguage );
	m_pIPak->OpenPack("",szPakName);

	// load patch language data
	memset(szPakName,0,_MAX_PATH);
	sprintf(szPakName,"%s/Localized/%s2.pak",DATA_FOLDER,sLanguage );
	m_pIPak->OpenPack("",szPakName);
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Strange()
{
	m_nStrangeRatio += (1 + (100*rand())/RAND_MAX);
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Relaunch(bool bRelaunch)
{
	if (m_sys_firstlaunch)
		m_sys_firstlaunch->Set( "0" );

	m_bRelaunch = bRelaunch;
	SaveConfiguration();
}
