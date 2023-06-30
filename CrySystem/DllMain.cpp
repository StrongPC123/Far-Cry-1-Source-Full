////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   dllmain.cpp
//  Version:     v1.00
//  Created:     1/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "System.h"

#include "DebugCallStack.h"


#ifdef WIN32
#include <windows.h>

// For lua debugger
HMODULE gDLLHandle = NULL;

#ifdef USING_CRY_MEMORY_MANAGER
//#if !defined(LINUX)
	_ACCESS_POOL
//#endif
#endif


BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
											)
{
	gDLLHandle = (HMODULE)hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	
		
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH: 
		break;
	}
	return TRUE;
}
#endif //WIN32

extern "C"
{
	CRYSYSTEM_API ISystem* CreateSystemInterface( SSystemInitParams &initParams )
	{
		CSystem *pSystem = NULL;
		if (!initParams.pSystem)
		{
			pSystem = new CSystem;
		}
		else
		{
			pSystem = (CSystem*)initParams.pSystem;
		}
		
#ifndef _DEBUG
#ifdef WIN32
		// Install exception handler in Release modes.
		DebugCallStack::instance()->installErrorHandler( pSystem );
#endif
#endif

		if (!pSystem->Init( initParams ))
		{
			delete pSystem;
			return 0;
		}
		return pSystem;
	}
};