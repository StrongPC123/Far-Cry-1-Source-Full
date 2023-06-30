////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cry3dengine.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Defines the DLL entry point, implements access to other modules
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// init memory pool usage
#ifndef GAMECUBE
#ifndef _XBOX
//#if !defined(LINUX)
_ACCESS_POOL;
//#endif//LINUX 
#endif //_XBOX
#endif


#include "3dengine.h"

#define MAX_ERROR_STRING 4096

//////////////////////////////////////////////////////////////////////

#include "StencilShadowConnectivity.h"
#include "StencilShadowConnectivityBuilder.h"				// CStencilShadowConnectivityBuilder
#include "StencilShadowEdgeDetector.h"							// CStencilShadowEdgeDetector

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
ISystem* GetISystem()
{
	return Cry3DEngineBase::m_pSys;
}
//////////////////////////////////////////////////////////////////////////

#if !defined(GAMECUBE) && !defined(PS2) && !defined(_XBOX) && !defined(LINUX)
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved	) 
{
  return TRUE;
}
#endif

//#include <time.h>

I3DEngine * CreateCry3DEngine(ISystem	* pSystem, const char * szInterfaceVersion)
{/*
	__time64_t ltime1, ltime2;
	_time64( &ltime1 );
	
	int nElemNum = 100000*64/sizeof(float);
	unsigned short * pShorts = new unsigned short[nElemNum];
	float * pFloats = new float[nElemNum];

	for(int t=0; t<1000; t++)
	{
		for(int i=0; i<nElemNum; i++)
		{
			pFloats[i] = 0.01f*pShorts[i];
		}
	}

	_time64( &ltime2 );
	float fTimeDif = float(ltime2 - ltime1);

	delete [] pShorts;
	delete [] pFloats;

	char buff[32];
	snprintf(buff,"%.2f",fTimeDif/1000);
	MessageBox(0, buff, "aa", MB_OK);

	return 0;
*/
#if !defined(LINUX)
  if(strcmp(szInterfaceVersion,g3deInterfaceVersion))
    pSystem->GetIConsole()->Exit("Error: CreateCry3DEngine(): 3dengine interface version error");
#endif
  C3DEngine * p3DEngine = new C3DEngine(pSystem);
	return p3DEngine;
}

void Cry3DEngineBase::UpdateLoadingScreen(const char *command,...)
{
  if(command)
  {
    va_list		arglist;
    char		buf[512];
    va_start(arglist, command);
    vsnprintf(buf, sizeof(buf), command, arglist);
    va_end(arglist);	
    GetLog()->UpdateLoadingScreen(buf);
  }
  else
    GetLog()->UpdateLoadingScreen(0);
}

void Cry3DEngineBase::UpdateLoadingScreenPlus(const char *command,...)
{
  va_list		arglist;
  char		buf[512];
  va_start(arglist, command);
  vsnprintf(buf, sizeof(buf), command, arglist);
  va_end(arglist);	
  GetLog()->UpdateLoadingScreenPlus(buf);
}

//IRenderer	*Cry3DEngineBase::GetRenderer()             
//{ return GetSystem()->GetIRenderer(); }

//ITimer	  *Cry3DEngineBase::GetTimer()                
//{ return GetSystem()->GetITimer(); }

//ILog	    *Cry3DEngineBase::GetLog()                  
//{ return GetSystem()->GetILog(); }

//IPhysicalWorld  *Cry3DEngineBase::GetPhysicalWorld()  
//{ return GetSystem()->GetIPhysicalWorld(); }

CCamera & Cry3DEngineBase::GetViewCamera() 
{ return m_pSys->GetViewCamera(); }

float Cry3DEngineBase::GetCurTimeSec() 
{ return (m_pSys->GetITimer()->GetCurrTime()); }

float Cry3DEngineBase::GetCurAsyncTimeSec() 
{ return (m_pSys->GetITimer()->GetAsyncCurTime()); }

//IConsole * Cry3DEngineBase::GetConsole()
//{ return GetSystem()->GetIConsole(); }

//I3DEngine * Cry3DEngineBase::Get3DEngine()
//{ return GetSystem()->GetI3DEngine(); }

//CVars * Cry3DEngineBase::GetCVars()
//{ return ((C3DEngine*)GetSystem()->GetI3DEngine())->GetCVars(); }

CVisAreaManager * Cry3DEngineBase::GetVisAreaManager()
{ return ((C3DEngine*)m_pSys->GetI3DEngine())->GetVisAreaManager(); }

//////////////////////////////////////////////////////////////////////////
void Cry3DEngineBase::Warning( int flags,const char *file,const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[MAX_ERROR_STRING];
	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	// Call to validating warning of system.
	m_pSys->Warning( VALIDATOR_MODULE_3DENGINE,VALIDATOR_WARNING,flags,file,"%s",szBuffer );
}

#include <CrtDebugStats.h>

