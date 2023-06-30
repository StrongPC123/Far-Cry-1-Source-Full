
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

// CryGame.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CryGame.h"

#ifndef	_XBOX
//#if !defined(LINUX)
_ACCESS_POOL;
//#endif//LINUX
#endif //_XBOX

#if !defined(PS2) && !defined(_XBOX) && !defined(LINUX)
// DLL-EntryPoint
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
#ifdef USE_MEM_POOL
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if(!CHECK_POOL())
			return FALSE;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
#endif
    return TRUE;
}
#endif
 
#include <CrtDebugStats.h>
