//////////////////////////////////////////////////////////////////////
//
//	Crytek Animation DLL source code
//	
//	File: CryAnimation.cpp
//	Description :
//     Defines the entry point for the DLL application.
//     Implements the base class for major CryAnimation classes
//
//	History:
//	- September 10, 2001: Created by Vladimir Kajalin
//  - August    15, 2002: Taken over by Sergiy Migdalskiy
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdarg.h>
#include "CVars.h"
#include "CryCharManager.h"
#include "CryAnimationBase.h"
//#include "CryAnimation.h"

//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// cached interfaces - valid during the whole session, when the character manager is alive; then get erased
ISystem*				g_pISystem				= NULL;
ITimer*					g_pITimer					= NULL;
ILog*						g_pILog						= NULL;
IConsole*				g_pIConsole				= NULL;
ICryPak*				g_pIPak						= NULL;
IStreamEngine*	g_pIStreamEngine	= NULL;;

IRenderer*			g_pIRenderer			= NULL;
IPhysicalWorld*	g_pIPhysicalWorld	= NULL;
I3DEngine*			g_pI3DEngine			= NULL;



// this is current frame id PLUS OR MINUS a few frames.
// can be used in places where it's really not significant for functionality but speed is a must.
int g_nFrameID = 0;

// this is true when the game runs in such a mode that requires all bones be updated every frame
bool g_bUpdateBonesAlways = false;

bool g_bProfilerOn = false;

// the cached console variable interfaces that are valid when the CryCharManager singleton is alive
CryAnimVars*	g_pCVariables		= NULL;


double g_dTimeAnimLoadBind;
double g_dTimeAnimLoadBindPreallocate;
double g_dTimeAnimLoadBindNoCal;
double g_dTimeAnimLoadBindWithCal;
double g_dTimeGeomLoad;
double g_dTimeGeomPostInit;
double g_dTimeShaderLoad;
double g_dTimeGeomChunkLoad;
double g_dTimeGeomChunkLoadFileIO;
double g_dTimeGenRenderArrays;
double g_dTimeAnimBindControllers;
double g_dTimeAnimLoadFile;
double g_dTimeTest1;
double g_dTimeTest2;
double g_dTimeTest3;
double g_dTimeTest4;

int g_CpuFlags;
double g_SecondsPerCycle;

// the number of animations that were loaded asynchronously
// (one animation can be counted several times if it has been loaded/unloaded)
unsigned g_nAsyncAnimCounter = 0;
// this is the sum of all delays between animation load and animation load finish, in frames
unsigned g_nAsyncAnimFrameDelays = 0;

