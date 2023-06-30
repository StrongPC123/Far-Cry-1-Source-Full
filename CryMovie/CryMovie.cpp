// CryMovie.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CryMovie.h"
#include "movie.h"
#include <CrtDebugStats.h>

#ifdef USING_CRY_MEMORY_MANAGER
_ACCESS_POOL
#endif

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH: 
			break;
  }
  return TRUE;
}
#endif //WIN32

extern "C"
{

CRYMOVIE_API IMovieSystem *CreateMovieSystem(ISystem *pSystem)
{
	return new CMovieSystem(pSystem);
};
}