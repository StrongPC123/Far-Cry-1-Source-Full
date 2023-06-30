//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code (c) Crytek 2001-2004
//
//	File: SystemInit.cpp
//  Description: CryENGINE system core-handle all subsystems
//
//	History:
//	-Feb 09,2004: split from system.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "System.h"
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
#include "XML\Xml.h"
#include "DataProbe.h"
#include "ApplicationHelper.h"				// CApplicationHelper

#define  PROFILE_WITH_VTUNE

//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "luadebugger/luadbginterface.h"
#include "luadebugger/LuaDbg.h"
extern HMODULE gDLLHandle;
#endif

//////////////////////////////////////////////////////////////////////////

#if defined(LINUX)
#		define DLL_SOUND				"crysoundsystem.so"
#		define DLL_NETWORK			"crynetwork.so"
#		define DLL_ENTITYSYSTEM	"cryentitysystem.so"
#		define DLL_INPUT				"cryinput.so"
#		define DLL_PHYSICS			"cryphysics.so"
#		define DLL_MOVIE				"crymovie.so"
#		define DLL_AI						"cryaisystem.so"
#		define DLL_FONT					"cryfont.so"
#		define DLL_3DENGINE			"cry3dengine.so"
#		define DLL_NULLRENDERER	"xrendernull.so"
#else
#	define DLL_SOUND				"CrySoundSystem.dll"
#	define DLL_NETWORK			"CryNetwork.dll"
#	define DLL_ENTITYSYSTEM	"CryEntitySystem.dll"
#	define DLL_INPUT				"CryInput.dll"
#	define DLL_PHYSICS			"CryPhysics.dll"
#	define DLL_MOVIE				"CryMovie.dll"
#	define DLL_AI						"CryAISystem.dll"
#	define DLL_FONT					"CryFont.dll"
#	define DLL_3DENGINE			"Cry3DEngine.dll"
#	define DLL_NULLRENDERER	"XRenderNULL.dll"
#endif

#define DEFAULT_LOG_FILENAME "Log.txt"

//////////////////////////////////////////////////////////////////////////
#include "Validator.h"
//////////////////////////////////////////////////////////////////////////
bool CSystem::OpenRenderLibrary(const char *t_rend)
{
	#ifdef _XBOX
		return OpenRenderLibrary(R_DX8_RENDERER);
	#endif

  int nRenderer = R_DX9_RENDERER;
#if defined(LINUX)
	return OpenRenderLibrary(R_NULL_RENDERER);
#else
  if (stricmp(t_rend, "NULL") != 0)
  {
    char szVendor[256];
    char szDevice[512];
    szVendor[0] = 0;
    szDevice[0] = 0;
    nRenderer = AutoDetectRenderer(szVendor, szDevice);
    if (nRenderer < 0)
    {      
			CryError(  "System: Error: VideoCard %s (%s) is not supported by FarCry engine", szVendor, szDevice);
      return false;
    }
    GetILog()->LogToFile( "System: VideoCard Detected: %s (%s)", szVendor, szDevice);
  }

	if (stricmp(t_rend, "Auto") == 0)
  {
    switch(nRenderer)
    {
      case R_DX9_RENDERER:
        GetILog()->LogToFile("System: Using Direct3D9 renderer...");
    	  break;
      case R_DX8_RENDERER:
        GetILog()->LogToFile("System: Using Direct3D8 renderer...");
    	  break;
      case R_GL_RENDERER:
        GetILog()->LogToFile("System: Using OpenGL renderer...");
    	  break;
      case R_NULL_RENDERER:
        GetILog()->LogToFile("System: Using NULL renderer...");
    	  break;
      default:
        GetILog()->LogToFile( "System: Error: Unknown renderer type");
        return false;
    }
    return OpenRenderLibrary(nRenderer);
  }

	if (stricmp(t_rend, "OpenGL") == 0)
    return OpenRenderLibrary(R_GL_RENDERER);
  else
	if (stricmp(t_rend, "Direct3D8") == 0)
		return OpenRenderLibrary(R_DX8_RENDERER);
  else
  if (stricmp(t_rend, "Direct3D9") == 0)
    return OpenRenderLibrary(R_DX9_RENDERER);
  else
  if (stricmp(t_rend, "NULL") == 0)
    return OpenRenderLibrary(R_NULL_RENDERER);

	Error( "Unknown renderer type: %s", t_rend );
#endif
	return (false);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::OpenRenderLibrary(int type)
{
  SCryRenderInterface sp;

#ifdef _XBOX
  type = R_DX8_RENDERER;
#endif
#if defined(LINUX)
	type = R_NULL_RENDERER;
#endif

	static int test_int = 0;

  sp.ipConsole = GetIConsole();
  sp.ipLog = GetILog();
  sp.ipSystem = this;
  sp.ipTest_int = &test_int;
  sp.ipTimer = GetITimer();
	sp.pIPhysicalWorld = m_pIPhysicalWorld;

#ifndef _XBOX
	char libname[128];
	if (type == R_GL_RENDERER)
    strcpy(libname, "XRenderOGL.dll");
	else
	if (type == R_DX8_RENDERER)
		strcpy(libname, "XRenderD3D8.dll");
  else
  if (type == R_DX9_RENDERER)
    strcpy(libname, "XRenderD3D9.dll");
  else
  if (type == R_NULL_RENDERER)
    strcpy(libname, DLL_NULLRENDERER);
	else
	{
		Error("No renderer specified");
		return (false);
	}
	m_dll.hRenderer = LoadDLL(libname);
	if (!m_dll.hRenderer)
		return false;

	typedef IRenderer *(PROCREND)(int argc, char* argv[], SCryRenderInterface *sp);
  PROCREND *Proc = (PROCREND *) CryGetProcAddress(m_dll.hRenderer, "PackageRenderConstructor");
	if (!Proc)
	{
		Error( "Error: Library '%s' isn't Crytek render library", libname);
		FreeLib(m_dll.hRenderer);
		return false;
	}
 
	m_pRenderer = Proc(0, NULL, &sp);
	if (!m_pRenderer)
	{
		Error( "Error: Couldn't construct render driver '%s'", libname);
		FreeLib(m_dll.hRenderer);
		return false;
	}
	m_pRenderer->SetType(type);
#else
  m_pRenderer = (IRenderer*)PackageRenderConstructor(0, NULL, &sp);
  m_pRenderer->SetType(type);
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
IRenderer* CSystem::CreateRenderer(bool fullscreen, void* hinst, void* hWndAttach )
{
	IRenderer *curr = m_pRenderer;
	if (!m_pRenderer)
		CreateRendererVars();
#ifdef WIN32
	int currwidth, currheight, currbpp, currzbpp, currsbpp;
	char currtype;

	if (m_bEditor)
	{
		// save current screen width/height/bpp
		m_iWidth = m_pConsole->GetCVar("r_Width")->GetIVal();
		m_iHeight = m_pConsole->GetCVar("r_Height")->GetIVal();
		m_iColorBits = m_pConsole->GetCVar("r_ColorBits")->GetIVal();
	}

	if (!curr)
	{
		currwidth = 64;
		currheight = 64;
		currbpp = 32;
		currzbpp = 24;
		currsbpp = 8;
		currtype = R_GL_RENDERER;
	}
	else
	{
		currwidth = curr->GetWidth();
		currheight = curr->GetHeight();
		currbpp = curr->GetColorBpp();
		currzbpp = curr->GetDepthBpp();
		currsbpp = curr->GetStencilBpp();
		currtype = curr->GetType();
	}

	if (OpenRenderLibrary(m_rDriver->GetString()))
	{
		m_pRenderer->Init(0, 0, currwidth, currheight, currbpp, currzbpp, currsbpp, fullscreen, hinst, hWndAttach);
	}

	// In editor Console should be initialized now.
	// And 3D Engine created.
	if (m_bEditor)
	{
		// In Editor mode Font must be initialized here.
		InitFont();
		CryLogAlways("Console initialization");
		InitConsole();
		CryLogAlways("Animation System initialization");
		InitAnimationSystem();
		CryLogAlways("Initializing 3D Engine");
		Init3DEngine();
		// Assign 3D engine to process.
		m_pProcess = m_pI3DEngine;
		m_pProcess->SetFlags(PROC_3DENGINE);
	}

	return m_pRenderer;

#else
//#pragma message("CreateNewRenderer NOT IMPLEMENTED (Only WIN32)")
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitNetwork()
{

#ifndef _XBOX
	PFNCREATENETWORK pfnCreateNetwork;
	m_dll.hNetwork = LoadDLL( DLL_NETWORK );
	if (!m_dll.hNetwork)
		return false;

	pfnCreateNetwork=(PFNCREATENETWORK) CryGetProcAddress(m_dll.hNetwork,"CreateNetwork");
	m_pNetwork=pfnCreateNetwork(this);
	if(m_pNetwork==NULL)
	{
		Error( "Error creating Network System (CreateNetwork) !" );
		return false;
	}
#else
	m_pNetwork=CreateNetwork();
	if(m_pNetwork==NULL)
	{
		Error("Error creating Network System (CreateNetwork) !");
		return false;
	}
#endif
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitEntitySystem(WIN_HINSTANCE hInstance, WIN_HWND hWnd)
{
	/////////////////////////////////////////////////////////////////////////////////
	// Load and initialize the entity system
	/////////////////////////////////////////////////////////////////////////////////
#ifndef _XBOX
	PFNCREATEENTITYSYSTEM pfnCreateEntitySystem;

	// Load the DLL
	m_dll.hEntitySystem = LoadDLL( DLL_ENTITYSYSTEM );
	if (!m_dll.hEntitySystem)
		return false;

	// Obtain factory pointer
	pfnCreateEntitySystem = (PFNCREATEENTITYSYSTEM) CryGetProcAddress( m_dll.hEntitySystem, "CreateEntitySystem");

	if (!pfnCreateEntitySystem)
	{
		Error( "Error querying entry point of Entity System Module (CryEntitySystem.dll) !");
		return false;
	}
	// Create the object
	m_pEntitySystem = pfnCreateEntitySystem(this);
	if (!m_pEntitySystem)
	{
		Error( "Error creating Entity System");
		return false;
	}
#else
	// Create the object
	m_pEntitySystem = CreateEntitySystem(this);
	if (!m_pEntitySystem)
	{
		Error( "Error creating Entity System");
		return false;
	}
#endif

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitInput(WIN_HINSTANCE hinst, WIN_HWND hwnd)
{
	m_dll.hInput = LoadDLL(DLL_INPUT);
	if (!m_dll.hInput)
		return false;

	bool bUseDirectInput = i_direct_input->GetIVal()?true:false;
	if (m_bEditor)
		bUseDirectInput = false;

	CRY_PTRCREATEINPUTFNC *pfnCreateInput;
	pfnCreateInput = (CRY_PTRCREATEINPUTFNC *) CryGetProcAddress(m_dll.hInput, "CreateInput");
	if (pfnCreateInput)
		m_pIInput = pfnCreateInput( this, hinst, m_hWnd, bUseDirectInput);
	if (!m_pIInput)
	{
		Error( "Error creating Input system" );
		return (false);
	}

	return (true);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitConsole()
{
//	m_Console->Init(this);

	// Ignore when run in Editor.
	if (m_bEditor && !m_pRenderer)
		return true;

	char *filename = "Textures\\Console\\DefaultConsole.tga";
	if (filename)
	{
		ITexPic * conimage = m_pRenderer->EF_LoadTexture(filename,FT_NORESIZE,0,eTT_Base);
		if (conimage)
  		m_pConsole->SetImage(conimage,false);
	}
	else
	{
		Error("Error: Cannot open %s", filename);
	}

	return (true);
}

//////////////////////////////////////////////////////////////////////////
// attaches the given variable to the given container;
// recreates the variable if necessary
ICVar* CSystem::attachVariable (const char* szVarName, int* pContainer, const char*szComment,int dwFlags)
{
	IConsole* pConsole = GetIConsole();

	ICVar* pOldVar = pConsole->GetCVar (szVarName);
	int nDefault;
	if (pOldVar)
	{
		nDefault = pOldVar->GetIVal();
		pConsole->UnregisterVariable(szVarName, true);
	}

	// NOTE: maybe we should preserve the actual value of the variable across the registration,
	// because of the strange architecture of IConsole that converts int->float->int

	pConsole->Register(szVarName, pContainer, float(*pContainer), dwFlags, szComment);

	ICVar* pVar = pConsole->GetCVar(szVarName);

#ifdef _DEBUG
	// test if the variable really has this container
	assert (*pContainer == pVar->GetIVal());
	++*pContainer;
	assert (*pContainer == pVar->GetIVal());
	--*pContainer;
#endif

	if (pOldVar)
	{
		// carry on the default value from the old variable anyway
		pVar->Set(nDefault);
	}
	return pVar;
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitRenderer(WIN_HINSTANCE hinst, WIN_HWND hwnd,const char *szCmdLine)
{
  CreateRendererVars();

	if(m_bDedicatedServer)
	{
		m_sSavedRDriver=m_rDriver->GetString();
		m_rDriver->Set("NULL");
	}

	if (!OpenRenderLibrary(m_rDriver->GetString()))
		return false;

#ifdef WIN32

	if(!m_bDedicatedServer)
	{
		// [marco] If a previous instance is running, activate
		// the old one and terminate the new one, depending
		// on command line devmode status - is done here for
		// smart people that double-click 20 times on the farcry
		// icon even before the previous detection on the main
		// executable has time to be executed - so now the game will quit
		HWND hwndPrev;
		static char szWndClass[] = "CryENGINE";
		
		// in devmode we don't care, we allow to run multiple instances
		// for mp debugging
		if (!m_bInDevMode)
		{
			hwndPrev = FindWindow (szWndClass, NULL);
			// not in devmode and we found another window - see if the
			// system is relaunching, in this case is fine 'cos the application
			// will be closed immediately after
			if (hwndPrev && (hwndPrev!=m_hWnd) && !m_bRelaunched)
			{
				SetForegroundWindow (hwndPrev);
				//MessageBox( NULL,"You cannot start multiple instances of FarCry !\n If you are starting it from a desktop icon, do not double click on it more than once.\n The program will now quit.\n" ,"ERROR: STARTING MULTIPLE INSTANCES OF FARCRY ",MB_OK|MB_ICONERROR );
				Quit();
			}
		}
	}
#endif

#ifdef WIN32
	if (m_pRenderer)
	{
		m_hWnd = m_pRenderer->Init(0, 0, m_rWidth->GetIVal(), m_rHeight->GetIVal(), m_rColorBits->GetIVal(), m_rDepthBits->GetIVal(), m_rStencilBits->GetIVal(), m_rFullscreen->GetIVal() ? true : false, hinst, hwnd);
		if (m_hWnd)
			return true;
		return (false);
	}
#else
	if (m_pRenderer)
	{
		WIN_HWND h = m_pRenderer->Init(0, 0, m_rWidth->GetIVal(), m_rHeight->GetIVal(), m_rColorBits->GetIVal(), m_rDepthBits->GetIVal(), m_rStencilBits->GetIVal(), m_rFullscreen->GetIVal() ? true : false, hinst, hwnd);
		if (h)
			return true;
		return (false);
	}
#endif
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitSound(WIN_HWND hwnd)
{
#if !defined(LINUX)
#ifndef _XBOX
	m_dll.hSound = LoadDLL(DLL_SOUND);
	if(!m_dll.hSound)
		return false;

	PFNCREATESOUNDSYSTEM pfnCreateSoundSystem = (PFNCREATESOUNDSYSTEM) CryGetProcAddress(m_dll.hSound,"CreateSoundSystem");
	if (!pfnCreateSoundSystem)
	{
		Error( "Error loading function CreateSoundSystem");
		return false;
	}

	m_pISound = pfnCreateSoundSystem(this,hwnd);
#else
  m_pISound = CreateSoundSystem(this,hwnd);
#endif
	if (!m_pISound)
	{
		Error( "Error creating the sound system interface");
		return false;
	}
	m_pIMusic = m_pISound->CreateMusicSystem();
	if (!m_pIMusic)
	{
		Error( "Error creating the music system interface");
		return false;
	}
	
#endif
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitPhysics()
{
#ifndef _XBOX
	m_dll.hPhysics = LoadDLL(DLL_PHYSICS);
	if(!m_dll.hPhysics)
		return false;

	IPhysicalWorld *(*pfnCreatePhysicalWorld)(ISystem *pSystem) = (IPhysicalWorld*(*)(ISystem*)) CryGetProcAddress(m_dll.hPhysics,"CreatePhysicalWorld");
	if(!pfnCreatePhysicalWorld)
	{
		Error( "Error loading function CreatePhysicalWorld" );
		return false;
	}

	m_pIPhysicalWorld = pfnCreatePhysicalWorld(this);
#else
	m_pIPhysicalWorld = CreatePhysicalWorld(this);
#endif

	if(!m_pIPhysicalWorld)
	{
		Error( "Error creating the physics system interface" );
		return false;
	}
	m_pIPhysicalWorld->Init();

	// Register physics console variables.
	IConsole *pConsole = GetIConsole();

  PhysicsVars *pVars = m_pIPhysicalWorld->GetPhysVars();

  pConsole->Register("p_fly_mode", &pVars->bFlyMode, (float)pVars->bFlyMode,VF_CHEAT,
		"Toggles fly mode.\n"
		"Usage: p_fly_mode [0/1]");
  pConsole->Register("p_collision_mode", &pVars->iCollisionMode, (float)pVars->iCollisionMode,VF_CHEAT,
		"This variable is obsolete.\n");
  pConsole->Register("p_single_step_mode", &pVars->bSingleStepMode, (float)pVars->bSingleStepMode,VF_CHEAT,
		"Toggles physics system 'single step' mode."
		"Usage: p_single_step_mode [0/1]\n"
		"Default is 0 (off). Set to 1 to switch physics system (except\n"
		"players) to single step mode. Each step must be explicitly\n"
		"requested with a 'p_do_step' instruction.");
  pConsole->Register("p_do_step", &pVars->bDoStep, (float)pVars->bDoStep,VF_CHEAT,
		"Steps physics system forward when in single step mode.\n"
		"Usage: p_do_step 1\n"
		"Default is 0 (off). Each 'p_do_step 1' instruction allows\n"
		"the physics system to advance a single step.");
  pConsole->Register("p_fixed_timestep", &pVars->fixedTimestep, pVars->fixedTimestep,VF_CHEAT,
		"Toggles fixed time step mode."
		"Usage: p_fixed_timestep [0/1]\n"
		"Forces fixed time step when set to 1. When set to 0, the\n"
		"time step is variable, based on the frame rate.");
  pConsole->Register("p_draw_helpers", &pVars->iDrawHelpers, (float)pVars->iDrawHelpers,VF_CHEAT,
		"Toggles display of various physical helpers. The value is a bitmask:\n"
		"bit 0  - show contact points\n"
		"bit 1  - show physical geometry\n"
		"bit 8  - show helpers for static objects\n"
		"bit 9  - show helpers for sleeping physicalized objects (rigid bodies, ragdolls)\n"
		"bit 10 - show helpers for active physicalized objects\n"
		"bit 11 - show helpers for players\n"
		"bit 12 - show helpers for independent entities (alive physical skeletons,particles,ropes)\n"
		"bits 16-31 - level of bounding volume trees to display (if 0, it just shows geometry)\n"
		"Examples: show static objects - 258, show active rigid bodies - 1026, show players - 2050");
  pConsole->Register("p_max_contact_gap", &pVars->maxContactGap, pVars->maxContactGap, VF_REQUIRE_NET_SYNC,
		"Sets the gap, enforced whenever possible, between\n"
		"contacting physical objects."
		"Usage: p_max_contact_gap 0.01\n"
		"This variable is used for internal tweaking only.");
  pConsole->Register("p_max_contact_gap_player", &pVars->maxContactGapPlayer, pVars->maxContactGapPlayer, 0,
		"Sets the safe contact gap for player collisions with\n"
		"the physical environment."
		"Usage: p_max_contact_gap_player 0.01\n"
		"This variable is used for internal tweaking only.");
  pConsole->Register("p_gravity_z", &pVars->gravity.z, pVars->gravity.z, CVAR_FLOAT);
  pConsole->Register("p_max_substeps", &pVars->nMaxSubsteps, (float)pVars->nMaxSubsteps, VF_REQUIRE_NET_SYNC,
		"Limits the number of substeps allowed in variable time step mode.\n"
		"Usage: p_max_substeps 5\n"
		"Objects that are not allowed to perform time steps\n"
		"beyond some value make several substeps.");
	pConsole->Register("p_prohibit_unprojection", &pVars->bProhibitUnprojection, (float)pVars->bProhibitUnprojection, 0,
		"This variable is obsolete.");
	pConsole->Register("p_enforce_contacts", &pVars->bEnforceContacts, (float)pVars->bEnforceContacts, 0,
		"This variable is obsolete.");
	pConsole->Register("p_damping_group_size", &pVars->nGroupDamping, (float)pVars->nGroupDamping, 0,
		"Sets contacting objects group size\n"
		"before group damping is used."
		"Usage: p_damping_group_size 3\n"
		"Used for internal tweaking only.");
	pConsole->Register("p_group_damping", &pVars->groupDamping, pVars->groupDamping, 0,
		"Toggles damping for object groups.\n"
		"Usage: p_group_damping [0/1]\n"
		"Default is 1 (on). Used for internal tweaking only.");
	pConsole->Register("p_break_on_validation", &pVars->bBreakOnValidation, (float)pVars->bBreakOnValidation, 0,
		"Toggles break on validation error.\n"
		"Usage: p_break_on_validation [0/1]\n"
		"Default is 0 (off). Issues DebugBreak() call in case of\n"
		"a physics parameter validation error.");
	pConsole->Register("p_time_granularity", &pVars->timeGranularity, pVars->timeGranularity, 0,
		"Sets physical time step granularity.\n"
		"Usage: p_time_granularity [0..0.1]\n"
		"Used for internal tweaking only.");
	pConsole->Register("p_list_active_objects", &pVars->bLogActiveObjects, (float)pVars->bLogActiveObjects);
	pConsole->Register("p_profile_entities", &pVars->bProfileEntities, (float)pVars->bProfileEntities, 0,
		"Enables per-entity time step profiling");
	pConsole->Register("p_GEB_max_cells", &pVars->nGEBMaxCells, (float)pVars->nGEBMaxCells, 0,
		"Specifies the cell number threshold after which GetEntitiesInBox issues a warning");
	pConsole->Register("p_max_velocity", &pVars->maxVel, pVars->maxVel, 0,
		"Clamps physicalized objects' velocities to this value");
	pConsole->Register("p_max_player_velocity", &pVars->maxVelPlayers, pVars->maxVelPlayers, 0,
		"Clamps players' velocities to this value");

	pConsole->Register("p_max_MC_iters", &pVars->nMaxMCiters, (float)pVars->nMaxMCiters, VF_REQUIRE_NET_SYNC,
		"Specifies the maximum number of microcontact solver iterations");
	pConsole->Register("p_accuracy_MC", &pVars->accuracyMC, pVars->accuracyMC, 0,
		"Desired accuracy of microcontact solver (velocity-related, m/s)");
	pConsole->Register("p_accuracy_LCPCG", &pVars->accuracyLCPCG, pVars->accuracyLCPCG, 0,
		"Desired accuracy of LCP CG solver (velocity-related, m/s)");
	pConsole->Register("p_max_contacts", &pVars->nMaxContacts, (float)pVars->nMaxContacts, VF_REQUIRE_NET_SYNC,
		"Maximum contact number, after which contact reduction mode is activated");
	pConsole->Register("p_max_plane_contacts", &pVars->nMaxPlaneContacts, (float)pVars->nMaxPlaneContacts, 0,
		"Maximum number of contacts lying in one plane between two rigid bodies\n"
		"(the system tries to remove the least important contacts to get to this value)");
	pConsole->Register("p_max_plane_contacts_distress", &pVars->nMaxPlaneContactsDistress, (float)pVars->nMaxPlaneContactsDistress, 0,
		"Same as p_max_plane_contacts, but is effective if total number of contacts is above p_max_contacts");
	pConsole->Register("p_max_LCPCG_subiters", &pVars->nMaxLCPCGsubiters, (float)pVars->nMaxLCPCGsubiters, VF_REQUIRE_NET_SYNC,
		"Limits the number of LCP CG solver inner iterations (should be of the order of the number of contacts)");
	pConsole->Register("p_max_LCPCG_subiters_final", &pVars->nMaxLCPCGsubitersFinal, (float)pVars->nMaxLCPCGsubitersFinal, VF_REQUIRE_NET_SYNC,
		"Limits the number of LCP CG solver inner iterations during the final iteration (should be of the order of the number of contacts)");
	pConsole->Register("p_max_LCPCG_microiters", &pVars->nMaxLCPCGmicroiters, (float)pVars->nMaxLCPCGmicroiters, VF_REQUIRE_NET_SYNC,
		"Limits the total number of per-contact iterations during one LCP CG iteration\n"
		"(number of microiters = number of subiters * number of contacts)");
	pConsole->Register("p_max_LCPCG_microiters_final", &pVars->nMaxLCPCGmicroitersFinal, (float)pVars->nMaxLCPCGmicroitersFinal, VF_REQUIRE_NET_SYNC,
		"Same as p_max_LCPCG_microiters, but for the final LCP CG iteration");
	pConsole->Register("p_max_LCPCG_iters", &pVars->nMaxLCPCGiters, (float)pVars->nMaxLCPCGiters, VF_REQUIRE_NET_SYNC,
		"Maximum number of LCP CG iterations");
	pConsole->Register("p_min_LCPCG_improvement", &pVars->minLCPCGimprovement, pVars->minLCPCGimprovement, 0,
		"Defines a required residual squared length improvement, in fractions of 1");
	pConsole->Register("p_max_LCPCG_fruitless_iters", &pVars->nMaxLCPCGFruitlessIters, (float)pVars->nMaxLCPCGFruitlessIters, 0,
		"Maximum number of LCP CG iterations w/o improvement (defined by p_min_LCPCGimprovement)");
	pConsole->Register("p_accuracy_LCPCG_no_improvement", &pVars->accuracyLCPCGnoimprovement, pVars->accuracyLCPCGnoimprovement, 0,
		"Required LCP CG accuracy that allows to stop if there was no improvement after p_max_LCPCG_fruitless_iters");
	pConsole->Register("p_min_separation_speed", &pVars->minSeparationSpeed, pVars->minSeparationSpeed, 0,
		"Used a threshold in some places (namely, to determine when a particle\n"
		"goes to rest, and a sliding condition in microcontact solver)");
	pConsole->Register("p_use_distance_contacts", &pVars->bUseDistanceContacts, (float)pVars->bUseDistanceContacts, 0,
		"Allows to use distance-based contacts (is forced off in multiplayer)");
	pConsole->Register("p_unproj_vel_scale", &pVars->unprojVelScale, pVars->unprojVelScale, 0,
		"Requested unprojection velocity is set equal to penetration depth multiplied by this number");
	pConsole->Register("p_max_unproj_vel", &pVars->maxUnprojVel, pVars->maxUnprojVel, 0,
		"Limits the maximum unprojection velocity request");
	pConsole->Register("p_penalty_scale", &pVars->penaltyScale, pVars->penaltyScale, 0, 
		"Scales the penalty impulse for objects that use the simple solver");
	pConsole->Register("p_max_contact_gap_simple", &pVars->maxContactGapSimple, pVars->maxContactGapSimple, 0, 
		"Specifies the maximum contact gap for objects that use the simple solver");
	pConsole->Register("p_skip_redundant_colldet", &pVars->bSkipRedundantColldet, (float)pVars->bSkipRedundantColldet, 0, 
		"Specifies whether to skip furher collision checks between two convex objects using the simple solver\n"
		"when they have enough contacts between them");
	pConsole->Register("p_limit_simple_solver_energy", &pVars->bLimitSimpleSolverEnergy, (float)pVars->bLimitSimpleSolverEnergy, 0, 
		"Specifies whether the energy added by the simple solver is limited (0 or 1)");
	pConsole->Register("p_max_world_step", &pVars->maxWorldStep, pVars->maxWorldStep, 0, 
		"Specifies the maximum step physical world can make (larger steps will be truncated)");

	if (m_bEditor)
	{
		// Setup physical grid for Editor.
		int nCellSize = 16;
		m_pIPhysicalWorld->SetupEntityGrid(2,vectorf(0,0,0), (2048)/nCellSize,(2048)/nCellSize, (float)nCellSize,(float)nCellSize);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitMovieSystem()
{
#if !defined(LINUX)
#ifdef WIN32
	m_dll.hMovie = LoadDLL(DLL_MOVIE);
	if(!m_dll.hMovie)
		return false;

	PFNCREATEMOVIESYSTEM pfnCreateMovieSystem = (PFNCREATEMOVIESYSTEM) CryGetProcAddress(m_dll.hMovie,"CreateMovieSystem");
	if (!pfnCreateMovieSystem)
	{
		Error( "Error loading function CreateMovieSystem" );
		return false;
	}

	m_pIMovieSystem = pfnCreateMovieSystem(this);
#else
	m_pIMovieSystem = CreateMovieSystem( this );
#endif

	if (!m_pIMovieSystem)
	{
		Error("Error creating the movie system interface");
		return false;
	}
#endif
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitAISystem()
{
#ifndef _XBOX
	m_dll.hAI = LoadDLL(DLL_AI);
	if (!m_dll.hAI)
		return true;

	IAISystem *(*pFnCreateAISystem)(ISystem*) = (IAISystem *(*)(ISystem*)) CryGetProcAddress(m_dll.hAI,"CreateAISystem");
	if (!pFnCreateAISystem)
	{
		Error( "Cannot fins entry proc in AI system");
		return true;
	}
	m_pAISystem = pFnCreateAISystem(this);
	if (!m_pAISystem)
		Error( "Cannot instantiate AISystem class" );
#else
	m_pAISystem = CreateAISystem(this);
	if (!m_pAISystem)
		Error("Cannot instantiate AISystem class");
#endif

	return true;
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitScriptSystem()
{
#ifndef _XBOX
#if defined(LINUX)
	m_dll.hScript = LoadDLL("cryscriptsystem.so");
#else
	m_dll.hScript = LoadDLL("CryScriptSystem.dll");
#endif
	if(m_dll.hScript==NULL)
		return (false);

	CREATESCRIPTSYSTEM_FNCPTR fncCreateScriptSystem;
	fncCreateScriptSystem = (CREATESCRIPTSYSTEM_FNCPTR) CryGetProcAddress(m_dll.hScript,"CreateScriptSystem");
	if(fncCreateScriptSystem==NULL)
	{
		Error( "Error initializeing ScriptSystem" );
		return (false);
	}

	m_pScriptSink = new CScriptSink(this,m_pConsole);
	m_pScriptSystem=fncCreateScriptSystem(this,m_pScriptSink,NULL,true);
	if(m_pScriptSystem==NULL)
	{
		Error( "Error initializeing ScriptSystem" );
		delete m_pScriptSink;
		m_pScriptSink = NULL;
		return (false);
	}
#else
	m_pScriptSink = new CScriptSink(this,m_pConsole);
	m_pScriptSystem=CreateScriptSystem(m_pScriptSink,NULL,true);
	if (m_pScriptSystem==NULL)
	{
		Error( "Error initializeing ScriptSystem" );
		delete m_pScriptSink;
		m_pScriptSink = NULL;
    return (false);
	}
#endif

	if (m_pScriptSink)
		m_pScriptSink->Init();

	assert( m_pConsole );
	//@HACK!
	((CXConsole*)m_pConsole)->SetScriptSystem(m_pScriptSystem);

	m_pScriptSystem->PostInit();

	return (true);
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitFileSystem()
{
	m_pIPak = new CCryPak(m_pLog,&m_PakVar);

	if (m_bEditor)
		m_pIPak->RecordFileOpen( true );

	return(m_pIPak->Init(""));
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::InitStreamEngine()
{
	// to temporarily switch the whole streaming off
	// streaming engine will be forced to single-threaded synchronous mode
	//m_pStreamEngine = new CStreamEngine(m_pIPak, m_pLog, 0, false);

	m_pStreamEngine = new CStreamEngine(m_pIPak, m_pLog);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
bool CSystem::InitFont()
{
	// In Editor mode Renderer is not initialized yet, so skip InitFont.
	if (m_bEditor && !m_pRenderer)
		return true;

#ifndef _XBOX
	m_dll.hFont = LoadDLL(DLL_FONT);
	if(!m_dll.hFont)
		return (false);

	PFNCREATECRYFONTINTERFACE pfnCreateCryFontInstance = (PFNCREATECRYFONTINTERFACE) CryGetProcAddress(m_dll.hFont,"CreateCryFontInterface");
	if(!pfnCreateCryFontInstance)
	{
		Error( "Error loading CreateCryFontInstance" );
		return (false);
	}

	m_pICryFont = pfnCreateCryFontInstance(this);
	if(!pfnCreateCryFontInstance)
	{
		Error( "Error loading CreateCryFontInstance" );
		return false;
	}
#else
	m_pICryFont = CreateCryFontInterface(this);
	if(!m_pICryFont)
	{
		Error( "Error loading CreateCryFontInstance" );
		return false;
	}
#endif

	// Load the default font
	IFFont *pConsoleFont = m_pICryFont->NewFont("Console");
	m_pIFont = m_pICryFont->NewFont("Default");
	if(!m_pIFont || !pConsoleFont)
	{
		Error( "Error creating the default fonts" );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	string szFontPath = "languages/fonts/default.xml";

	if(!m_pIFont->Load(szFontPath.c_str()))
	{
		string szError = "Error loading the default font from ";
		szError += szFontPath;
		szError += ". You're probably running the executable from the wrong working folder.";
		Error(szError.c_str());

		return false;
	}

	int n = szFontPath.find("default.xml");
	assert(n != string::npos);

	szFontPath.replace(n, strlen("default.xml"), "console.xml");

	if(!pConsoleFont->Load(szFontPath.c_str()))
	{
		string szError = "Error loading the console font from ";
		szError += szFontPath;
		szError += ". You're probably running the executable from the wrong working folder.";
		Error(szError.c_str());

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::Init3DEngine()
{
  ::SetLastError(0);
  m_dll.h3DEngine = LoadDLL(DLL_3DENGINE);
	if (!m_dll.h3DEngine)
		return false;

	PFNCREATECRY3DENGINE pfnCreateCry3DEngine;
	pfnCreateCry3DEngine = (PFNCREATECRY3DENGINE) CryGetProcAddress( m_dll.h3DEngine, "CreateCry3DEngine");
	if (!pfnCreateCry3DEngine)
	{
		Error("CreateCry3DEngine is not exported api function in Cry3DEngine.dll");
		return false;
	} 

	m_pI3DEngine = (*pfnCreateCry3DEngine)(this,g3deInterfaceVersion);

  if (!m_pI3DEngine )
	{
    Error( "Error Creating 3D Engine interface" );
		return false;
	}

	if (!m_pI3DEngine->Init())
	{
		Error( "Error Initializing 3D Engine" );
		return false;
	}
	m_pProcess = m_pI3DEngine;
	m_pProcess->SetFlags(PROC_3DENGINE);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::InitAnimationSystem()
{
#if defined(LINUX)
	m_dll.hAnimation = LoadDLL("cryanimation.so");
#else
	m_dll.hAnimation = LoadDLL("CryAnimation.dll");
#endif
	if (!m_dll.hAnimation)
		return false;

	PFNCREATECRYANIMATION pfnCreateCharManager;
	pfnCreateCharManager = (PFNCREATECRYANIMATION) CryGetProcAddress( m_dll.hAnimation, "CreateCharManager");
	if (!pfnCreateCharManager)
		return false;

	m_pICryCharManager = (*pfnCreateCharManager)(this,gAnimInterfaceVersion);

	if (m_pICryCharManager)
		GetILog()->LogPlus(" ok"); 
	else
		GetILog()->LogPlus (" FAILED");

	return m_pICryCharManager != NULL;
} 

//////////////////////////////////////////////////////////////////////////
void CSystem::InitVTuneProfiler()
{
#ifdef PROFILE_WITH_VTUNE
	HMODULE hModule = CryLoadLibrary( "VTuneApi.dll" );
	if (hModule)
	{
		VTPause = (VTuneFunction) CryGetProcAddress( hModule, "VTPause");
		VTResume = (VTuneFunction) CryGetProcAddress( hModule, "VTResume");
	}
#endif //PROFILE_WITH_VTUNE
}




// to parse the command-line in a consistent way
class CCommandLineSink_EarlyCommands :public CApplicationHelper::ICmdlineArgumentSink
{
public:
	//! constructor
	CCommandLineSink_EarlyCommands( CSystem &rSystem ) 
		:m_rSystem(rSystem), m_bRelaunching(false), m_bDevMode(false)
	{
	}

	virtual void ReturnArgument( const char *inszArgument )
	{
		// ----------------------------
		// e.g. -IP:2.13.35.55
		if(strnicmp(inszArgument,"IP:",3)==0)
		{
			m_sLocalIP=string(&(inszArgument[3]));			// local IPAddress (needed if we have several servers on one machine)
			return;
		}

#ifdef PROFILE_WITH_VTUNE
		// ----------------------------
		// Init VTune Profiler DLL.
		if(stricmp(inszArgument,"VTUNE")==0)
		{
			m_rSystem.InitVTuneProfiler();
			return;
		}
#endif //PROFILE_WITH_VTUNE

		// ----------------------------
		// e.g. -MOD:CS
		if(strnicmp(inszArgument,"MOD:",4)==0)
		{
			m_sMod = string(&(inszArgument[4]));			// mod folder
			return;
		}

		// ----------------------------
		// e.g. -LOGFILE:CS (useful for running several servers from the same directory)
		if(strnicmp(inszArgument,"LOGFILE:",8)==0)
		{
			m_sLogFile = string(&(inszArgument[8]));	// log file
			return;
		}

		// ----------------------------
		// Developer mode on
		if(stricmp(inszArgument,"DEVMODE")==0)
		{
			m_bDevMode=true;
			return;
		}

		// ----------------------------
		// Relaunching e.g. MOD has changed
		if(stricmp(inszArgument,"RELAUNCHING")==0)
		{
			m_bRelaunching=true;
			return;
		}
	}

	// ---------------------------------------------------------------

	CSystem &						m_rSystem;				//!< reference to the system
	string							m_sMod;						//!< -MOD
	string							m_sLogFile;				//!< -LOGFILE
	bool								m_bDevMode;				//!< -DEVMODE
	string							m_sLocalIP;				//!<
	bool								m_bRelaunching;		//!<
};




// System initialization
/////////////////////////////////////////////////////////////////////////////////
// INIT
/////////////////////////////////////////////////////////////////////////////////
bool CSystem::Init( const SSystemInitParams &params )
{
	// parse command line arguments minus e.g. "-IP:23.34.2.2" "-DEVMODE"
	CCommandLineSink_EarlyCommands CmdlineSink(*this);

	CApplicationHelper::ParseArguments(params.szSystemCmdLine,&CmdlineSink,0);

	// Get file version information.
	QueryVersionInfo();

	m_FrameProfileSystem.Init( this );

	m_hInst = (WIN_HINSTANCE)params.hInstance;
	m_hWnd = (WIN_HWND)params.hWnd;

	m_bEditor = params.bEditor;
	m_bTestMode = params.bTestMode;
	m_pUserCallback = params.pUserCallback;
	m_bDedicatedServer = params.bDedicatedServer;
	m_bRelaunched = CmdlineSink.m_bRelaunching;

	if (!params.pValidator)
	{
		m_pDefaultValidator = new SDefaultValidator(this);
		m_pValidator = m_pDefaultValidator;
	}
	else
	{
		m_pValidator = params.pValidator;
	}

	if (!params.pLog)
	{
		m_pLog = new CLog(this);
		if (CmdlineSink.m_sLogFile.size())
			m_pLog->SetFileName(CmdlineSink.m_sLogFile.c_str());
		else if (params.sLogFileName)
			m_pLog->SetFileName(params.sLogFileName);
		else
			m_pLog->SetFileName(DEFAULT_LOG_FILENAME);

		LogVersion();
	}
	else
  {
		m_pLog = params.pLog;
	}

#ifdef USE_MEM_POOL
	//Timur[9/30/2002]
	/*
	m_pMemoryManager = GetMemoryManager();
	if(g_hMemManager)
	{

		FNC_GetMemoryManager fncGetMemoryManager=(FNC_GetMemoryManager) CryGetProcAddress(g_hMemManager,"GetMemoryManager");
		if(fncGetMemoryManager)
		{
			m_pMemoryManager=fncGetMemoryManager();
		}
	}
	*/
#endif

	//////////////////////////////////////////////////////////////////////////
	// CREATE CONSOLE
	//////////////////////////////////////////////////////////////////////////
	m_pConsole = new CXConsole;

	//////////////////////////////////////////////////////////////////////////
	// FILE SYSTEM
	//////////////////////////////////////////////////////////////////////////

#if !defined(PS2) && !defined (GC)
  m_pCpu = new CCpuFeatures;
  m_pCpu->Detect();
#endif

	CryLogAlways("OS User name: '%s'",GetUserName());

	CryLogAlways("File System Initialization");

	InitFileSystem();

	if (CmdlineSink.m_sMod!="")
	{
		// [marco] prevent an hack from happening
		if (stricmp(CmdlineSink.m_sMod.c_str(),"FarCry")!=0)
		{		
			// check for a command line MOD, before
			// initializing the system
			strcpy(m_szGameMOD,CmdlineSink.m_sMod.c_str());

			// set this as game path for IPak, BEFORE
			// starting initializing subsystems and
			// AFTER the pak has been initialized
			string sMOD=string("Mods/")+string(m_szGameMOD);		
			m_pIPak->AddMod(sMOD.c_str());
		}
		else
			memset(m_szGameMOD,0,MAX_PATH);
	}
	else
		memset(m_szGameMOD,0,MAX_PATH);

	CryLogAlways("Stream Engine Initialization");
	InitStreamEngine();


	//////////////////////////////////////////////////////////////////////////
	// SCRIPT SYSTEM
	//////////////////////////////////////////////////////////////////////////
	CryLogAlways("Script System Initialization");
	if(!InitScriptSystem())
		return false;

	//////////////////////////////////////////////////////////////////////////
	// After creation of script system we can create system vars.
	CreateSystemVars();
	//////////////////////////////////////////////////////////////////////////

	if(m_bEditor || CmdlineSink.m_bDevMode)
		SetDevMode(true);											// In Dev mode.
	 else
		SetDevMode(false);										// Not Dev mode.

	//////////////////////////////////////////////////////////////////////////
	//Load config files
	//////////////////////////////////////////////////////////////////////////

	LoadConfiguration("System.Cfg");
	LoadConfiguration("SystemCfgOverride.Cfg");

	//////////////////////////////////////////////////////////////////////////
	// After loading configuration.
	//////////////////////////////////////////////////////////////////////////
	InitScriptDebugger();

	//////////////////////////////////////////////////////////////////////////
	// Open basic pak files.
	//////////////////////////////////////////////////////////////////////////
	OpenBasicPaks();

	//////////////////////////////////////////////////////////////////////////
	// NETWORK
	//////////////////////////////////////////////////////////////////////////
	if (!params.bPreview)
	{
		CryLogAlways("Network initialization");
		InitNetwork();

		m_pNetwork->SetLocalIP((char *)(CmdlineSink.m_sLocalIP.c_str()));
	}
	//////////////////////////////////////////////////////////////////////////
	// PHYSICS
	//////////////////////////////////////////////////////////////////////////
	//if (!params.bPreview)
	{
		CryLogAlways("Physics initialization");
		if (!InitPhysics())
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// MOVIE
	//////////////////////////////////////////////////////////////////////////
	//if (!params.bPreview)
	{
#if defined(LINUX)
		CryLogAlways("MovieSystem initialization skipped for Linux dedicated server");
#else
		CryLogAlways("MovieSystem initialization");
		if (!InitMovieSystem())
			return false;
#endif
	}

	if (!params.bEditor)
	{
		//////////////////////////////////////////////////////////////////////////
		// RENDERER
		//////////////////////////////////////////////////////////////////////////
		CryLogAlways("Renderer initialization");
		if (!InitRenderer(m_hInst, m_hWnd,params.szSystemCmdLine))
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// CONSOLE
	//////////////////////////////////////////////////////////////////////////
	CryLogAlways("Console initialization");
	if (!InitConsole())
		return false;

	//////////////////////////////////////////////////////////////////////////
	// TIME
	//////////////////////////////////////////////////////////////////////////
	CryLogAlways("Time initialization");
	if (!m_Time.Init(this))
		return (false);
	m_Time.Reset();

	//////////////////////////////////////////////////////////////////////////
	// INPUT
	//////////////////////////////////////////////////////////////////////////
	if (!params.bPreview && !params.bDedicatedServer)
	{
		CryLogAlways("Input initialization");
		if (!InitInput(m_hInst, m_hWnd))
			return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// SOUND
	//////////////////////////////////////////////////////////////////////////
	if (!params.bPreview && !params.bDedicatedServer)
	{
		CryLogAlways("Sound initialization");
		if (!InitSound(m_hWnd))
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// FONT
	//////////////////////////////////////////////////////////////////////////
	if(!params.bDedicatedServer)
	{
		CryLogAlways("Font initialization");
		if (!InitFont())
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// AI
	//////////////////////////////////////////////////////////////////////////
	if (!params.bPreview)
	{
		CryLogAlways("AI initialization");
		if (!InitAISystem())
			return false;
	}

	m_pConsole->Init(this);

//#ifndef MEM_STD
//  CConsole::AddCommand("MemStats",::DumpAllocs);
//#endif
	//////////////////////////////////////////////////////////////////////////
	// ENTITY SYSTEM
	//////////////////////////////////////////////////////////////////////////
	if (!params.bPreview)
	{
		CryLogAlways("Entity system initialization");
		if (!InitEntitySystem(m_hInst, m_hWnd))
			return false;
	}

	if (!params.bEditor)
	{
		//////////////////////////////////////////////////////////////////////////
		// Init Animation system
		//////////////////////////////////////////////////////////////////////////
		CryLogAlways("Initializing Animation System");
		if (!InitAnimationSystem())
			return false;
		//////////////////////////////////////////////////////////////////////////
		// Init 3d engine
		//////////////////////////////////////////////////////////////////////////
		CryLogAlways("Initializing 3D Engine");
		if (!Init3DEngine())
			return false;

		//////////////////////////////////////////////////////////////////////////
		// SCRIPT BINDINGS
		//////////////////////////////////////////////////////////////////////////
		CryLogAlways("Initializing Script Bindings");
		if(!InitScriptBindings())
		{
			return false;
		}
	}

	m_pDownloadManager = new CDownloadManager;
	m_pDownloadManager->Create(this);


	//////////////////////////////////////////////////////////////////////////
	// Check loader.
	//////////////////////////////////////////////////////////////////////////
#if defined(_DATAPROBE) && !defined(LINUX)
	CDataProbe probe;
	if (!params.pCheckFunc || !probe.CheckLoader( params.pCheckFunc ))
	{
		int *p = 0;
		*p = 1;
		Strange();
	}
#endif

	SetAffinity();

	return (true);
}

//////////////////////////////////////////////////////////////////////////
void CSystem::CreateSystemVars()
{
	m_pCVarQuit = GetIConsole()->CreateVariable("ExitOnQuit","1",VF_DUMPTODISK);

	i_direct_input = GetIConsole()->CreateVariable("i_direct_input", "1", VF_DUMPTODISK,
		"Toggles direct input capability.\n"
		"Usage: i_direct_input [0/1]\n"
		"Default is 1 (on).");

	//////////////////////////////////////////////////////////////////////////
	// SCRIPT DEBUGGER
	//////////////////////////////////////////////////////////////////////////
	sys_script_debugger=m_pConsole->CreateVariable("sys_script_debugger","0",VF_DUMPTODISK|VF_CHEAT,
		"Enables the script debugger.\n"
		"Usage: sys_script_debugger [0/1]");


	m_cvAIUpdate = GetIConsole()->CreateVariable("ai_noupdate","0",VF_CHEAT);

	m_cvMemStats = GetIConsole()->CreateVariable("MemStats", "0", 0);
	m_cvMemStatsThreshold = GetIConsole()->CreateVariable ("MemStatsThreshold", "32000", 0);
	m_cvMemStatsMaxDepth = GetIConsole()->CreateVariable("MemStatsMaxDepth", "4", 0);

	m_sys_StreamCallbackTimeBudget = GetIConsole()->CreateVariable("sys_StreamCallbackTimeBudget", "50000", 0,
		"Time budget, in microseconds, to be spent every frame in StreamEngine callbacks.\n"
		"Additive with cap: if more time is spent, the next frame gets less budget, and\n"
		"there's never more than this value per frame.");

	m_sys_StreamCompressionMask = GetIConsole()->CreateVariable("sys_StreamCompressionMask", "8", VF_CHEAT|VF_REQUIRE_NET_SYNC,
		"Stream compression bit mask, used for network compression(lower bandwidth)\n");
	// hidden information:
	// bit 3 (8): cookies removed from network stream 1=on 0=off

	m_PakVar.nPriority  = 1;
	m_PakVar.nReadSlice = 0;
	m_PakVar.nLogMissingFiles = 0;
	m_cvPakPriority = attachVariable("sys_PakPriority", &m_PakVar.nPriority,"If set to 1, tells CryPak to try to open the file in pak first, then go to file system",VF_READONLY|VF_CHEAT);
	m_cvPakReadSlice = attachVariable("sys_PakReadSlice", &m_PakVar.nReadSlice,"If non-0, means number of kilobytes to use to read files in portions. Should only be used on Win9x kernels");
	m_cvPakLogMissingFiles = attachVariable("sys_PakLogMissingFiles",&m_PakVar.nLogMissingFiles, "If non-0, missing file names go to mastercd/MissingFilesX.log. 1) only resulting report  2) run-time report is ON, one entry per file  3) full run-time report");

	m_sysNoUpdate = GetIConsole()->CreateVariable("sys_noupdate","0",VF_CHEAT,
		"Toggles updating of system with sys_script_debugger.\n"
		"Usage: sys_noupdate [0/1]\n"
		"Default is 0 (system updates during debug).");

	m_sysWarnings = GetIConsole()->CreateVariable("sys_warnings","0",VF_DUMPTODISK,
		"Toggles printing system warnings.\n"
		"Usage: sys_warnings [0/1]\n"
		"Default is 0 (off).");

	m_cvSSInfo =  GetIConsole()->CreateVariable("sys_SSInfo","0",VF_DUMPTODISK,
		"Show SourceSafe information (Name,Comment,Date) for file errors."
		"Usage: sys_SSInfo [0/1]\n"
		"Default is 0 (off).");

	m_cvEntitySuppressionLevel = GetIConsole()->CreateVariable("e_EntitySuppressionLevel","0",VF_DUMPTODISK,
		"Defines the level at which entities are spawned. Entities marked with lower level will not be spawned - 0 means no level."
		"Usage: e_EntitySuppressionLevel [0-infinity]\n"
		"Default is 0 (off).");

	m_sys_profile = GetIConsole()->CreateVariable("profile","0",0,"Enable profiling.\n"
		"Usage: profile #\n"
		"Where # sets the profiling to:\n"
		"	0: Profiling off\n"
		"	1: Self Time\n"
		"	2: Hierarchical Time\n"
		"	3: Extended Self Time\n"
		"	4: Extended Hierarchical Time\n"
		"	5: Peaks Time\n"
		"	6: Subsystem Info\n"
		"	7: Calls Numbers\n"
		"	8: Standart Deviation\n"
		"	-1: Profiling enabled, but not displayed\n"
		"Default is 0 (off).");
	m_sys_profile_filter = GetIConsole()->CreateVariable("profile_filter","",0,
		"Profiles a specified subsystem.\n"
		"Usage: profile_filter subsystem\n"
		"Where 'subsystem' may be:\n"
		"Renderer\n"
		"3DEngine\n"
		"Animation\n"
		"AI\n"
		"Entity\n"
		"Physics\n"
		"Sound\n"
		"System\n"
		"Game\n"
		"Editor\n"
		"Script\n"
		"Network\n"
		);
	m_sys_profile_graph = GetIConsole()->CreateVariable("profile_graph","0",0,
		"Enable drawing of profiling graph.\n");
	m_sys_profile_graphScale = GetIConsole()->CreateVariable("profile_graphScale","100",0,
		"Sets the scale of profiling histograms.\n"
		"Usage: profileGraphScale 100\n");
	m_sys_profile_pagefaultsgraph = GetIConsole()->CreateVariable("profile_pagefaults","0",0,
		"Enable drawing of page faults graph.\n");
	m_sys_profile_network = GetIConsole()->CreateVariable("profile_network","0",0,
		"Enables network profiling.\n" );
	m_sys_profile_peak = GetIConsole()->CreateVariable("profile_peak","10",0,
		"Profiler Peaks Tollerance in Milliseconds.\n" );
	m_sys_profile_memory = GetIConsole()->CreateVariable("MemInfo","0",0,"Display memory information by modules" );

	m_sys_skiponlowspec = GetIConsole()->CreateVariable( "sys_skiponlowspec", "0", VF_DUMPTODISK | VF_SAVEGAME,
		"avoids loading of expendable entites.\n" );

	m_sys_spec=GetIConsole()->CreateVariable("sys_spec","1",VF_DUMPTODISK | VF_SAVEGAME,
		"Tells the system cfg spec.\n" );

	m_sys_firstlaunch = GetIConsole()->CreateVariable( "sys_firstlaunch", "0", VF_DUMPTODISK,
		"Indicates that the game was run for the first time.\n" );
}

//////////////////////////////////////////////////////////////////////////
void CSystem::InitScriptDebugger()
{
#ifdef WIN32
	if(sys_script_debugger->GetIVal()!=0)
	{
		m_pLuaDebugger = new CLUADbg;
		m_pScriptSystem->EnableDebugger(m_pScriptSink);

		_Tiny_InitApp((HINSTANCE)::GetModuleHandle(NULL), (HINSTANCE) gDLLHandle, NULL, NULL, IDI_SMALL);

		_TinyVerify(m_pLuaDebugger->Create(NULL, _T("Lua Debugger"), WS_OVERLAPPEDWINDOW, 0, NULL,
			NULL, (ULONG_PTR) LoadMenu(_Tiny_GetResourceInstance(), MAKEINTRESOURCE(IDC_LUADBG))));

		m_pLuaDebugger->SetSystem((ISystem *) this);
	}
#endif
}

