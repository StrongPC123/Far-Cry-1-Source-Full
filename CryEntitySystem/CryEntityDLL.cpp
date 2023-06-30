// CryEntityDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#if !defined(_XBOX) && !defined(LINUX)
#include <windows.h>
#else
#if !defined(LINUX)
#include <xtl.h>
#endif
#endif
#include <IEntitySystem.h>
#include "EntitySystem.h"

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}

// Local var to turn on/off profiler.
bool g_bProfilerEnabled = false;
//////////////////////////////////////////////////////////////////////////

#if !defined(_XBOX)
 _ACCESS_POOL;
#if !defined(LINUX)
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif
#endif

#if !defined(_XBOX) && !defined(LINUX) && !defined(PS2)
CRYENTITYDLL_API struct IEntitySystem * CreateEntitySystem(ISystem *pISystem)
#else
struct IEntitySystem * CreateEntitySystem(ISystem *pISystem)
#endif
{
	gISystem = pISystem;
	CEntitySystem *pEntitySystem= new CEntitySystem(pISystem);
	if(!pEntitySystem->Init(pISystem))
	{
		pEntitySystem->Release();
		return NULL;
	}
	return pEntitySystem;
}

//////////////////////////////////////////////////////////////////////////

#ifdef GERMAN_GORE_CHECK

#if !defined(_XBOX) && !defined(LINUX) && !defined(PS2)
CRYENTITYDLL_API struct IEntitySystem * CreateMainEntitySystem(ISystem *pISystem)
#else
struct IEntitySystem * CreateMainEntitySystem(ISystem *pISystem)
#endif
{
	gISystem = pISystem;
	return (IEntitySystem *) gISystem;
}

#endif
//////////////////////////////////////////////////////////////////////////


#include <CrtDebugStats.h>
