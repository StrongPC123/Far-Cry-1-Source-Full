// CryPhysics.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
//#include <float.h>

#ifndef GAMECUBE
#ifndef _XBOX
//#if !defined(LINUX)
_ACCESS_POOL;
//#endif//LINUX
#endif //_XBOX
#include <CrtDebugStats.h>
#endif

#include "IPhysics.h"
#include "geoman.h"
#include "bvtree.h"
#include "geometry.h"
#include "rigidbody.h"
#include "physicalplaceholder.h"
#include "physicalentity.h"
#include "physicalworld.h"

float g_costab[SINCOSTABSZ],g_sintab[SINCOSTABSZ];

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
#endif

#ifndef _XBOX
CRYPHYSICS_API IPhysicalWorld *CreatePhysicalWorld(ISystem *pSystem)
#else
IPhysicalWorld *CreatePhysicalWorld(ISystem *pSystem)
#endif
{
	gISystem = pSystem;
	g_bHasSSE = (pSystem->GetCPUFlags() & CPUF_SSE)!=0;
	for(int i=0; i<SINCOSTABSZ; i++) {
		g_costab[i] = cosf(i*(pi*0.5f/SINCOSTABSZ));
		g_sintab[i] = sinf(i*(pi*0.5f/SINCOSTABSZ));
	}
	//_controlfp(_EM_ZERODIVIDE,_MCW_EM);
	return new CPhysicalWorld(pSystem->GetILog());
}

