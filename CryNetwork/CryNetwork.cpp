//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: crynetwork.cpp
//  Description: dll entry point
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNP.h"
#include "Client.h"
#include "Server.h"
#include "Network.h"

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}
//////////////////////////////////////////////////////////////////////////

#if !defined(XBOX)
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

#ifndef _XBOX
CRYNETWORK_API INetwork *CreateNetwork(ISystem *pSystem)
#else
INetwork *CreateNetwork(ISystem *pSystem)
#endif
{
	gISystem = pSystem;
	CNetwork *pNetwork=new CNetwork;

	if(!pNetwork->Init(gISystem->GetIScriptSystem()))
	{
		delete pNetwork;
		return NULL;
	}
	return pNetwork;
}

#include <CrtDebugStats.h>

