// CryScriptSystem.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

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


#include <string>
#include <map>
#include <CrtDebugStats.h>
