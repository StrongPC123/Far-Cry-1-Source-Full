// CrySoundSystem.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#ifdef USING_CRY_MEMORY_MANAGER
#ifndef _XBOX
_ACCESS_POOL;
#endif //_XBOX
#else //USING_CRY_MEMORY_MANAGER
//! Structre filled by call to CryModuleGetMemoryInfo()
struct CryModuleMemoryInfo
{
	//! Total Ammount of memory allocated.
	int allocated;
	//! Total Ammount of memory freed.
	int freed;
	//! Total number of memory allocations.
	int num_allocations;
};
extern "C" __declspec(dllexport) void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo )
{
#if (defined CS_VERSION_372) || (defined CS_VERSION_361)
  unsigned int nCurrentAlloced;
	unsigned int nMaxAlloced;
#else
  size_t nCurrentAlloced;
	size_t nMaxAlloced;
#endif
  CS_GetMemoryStats(&nCurrentAlloced, &nMaxAlloced);
	pMemInfo->allocated = nMaxAlloced;
	pMemInfo->freed = 0;
	pMemInfo->num_allocations = 0;
};
#endif //USING_CRY_MEMORY_MANAGER

#include "DummySound.h"	
#include <Cry_Camera.h>
#include "SoundSystem.h"
//////////////////////////////////////////////////////////////////////////////////////////////
// dll interface	

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
extern "C" ISoundSystem* CreateSoundSystem(struct ISystem* pISystem, void* pInitData)
{
	gISystem = pISystem;
	//create a brand new sound system
#ifndef _XBOX
	CSoundSystem* pSoundSystem = new CSoundSystem(pISystem, (HWND)pInitData);

	if (!pSoundSystem || !pSoundSystem->IsOK())
	{
		//if the sound system cannot be created or initialized,
		//create the dummy sound system (NULL sound system, same as for
		//dedicated server)

		if (pSoundSystem)
			pSoundSystem->Release();
#endif		

		CDummySoundSystem *pDummySound=new CDummySoundSystem(pISystem, (HWND)pInitData);
		return pDummySound;

#ifndef _XBOX		
	}

	return pSoundSystem;
#endif
}

#ifndef __MWERKS__
#ifndef _XBOX
///////////////////////////////////////////////
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
    return TRUE;
}
#endif //_XBOX
#endif

#include <CrtDebugStats.h>
