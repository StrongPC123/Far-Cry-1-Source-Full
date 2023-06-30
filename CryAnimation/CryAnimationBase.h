//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryAnimationBase.h
//  Declaration of CryAnimationBase class
//  Access to external stuff used by 3d engine
//  Most classes are derived from this base class to access the external interfaces
//
//	History:
//	-:Created by Vladimir Kajalin
//	-:Taken over by Sergiy Migdalskiy
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_ANIMATION_BASE_HEADER_
#define _CRY_ANIMATION_BASE_HEADER_

#include "FrameProfiler.h"
#include "CVars.h"




//////////////////////////////////////////////////////////////////////////
// There's only one ISystem in the process, just like there is one CryCharManager.
// So this ISystem is kept in the global pointer and is initialized upon creation
// of the CryCharManager and is valid until its destruction. 
// Upon destruction, it's NULLed. In any case, there must be no object needing it 
// after that since the animation system is only active when the Manager object is alive
//////////////////////////////////////////////////////////////////////////

extern ISystem*					g_pISystem;
extern IConsole*				g_pIConsole;
extern ITimer*					g_pITimer;
extern ILog*						g_pILog;
extern ICryPak*					g_pIPak;
extern IStreamEngine*		g_pIStreamEngine;

extern IRenderer*				g_pIRenderer;
extern IPhysicalWorld*	g_pIPhysicalWorld;
extern I3DEngine*				g_pI3DEngine;

extern bool							g_bProfilerOn;
extern CryAnimVars*			g_pCVariables;




// initializes the global values - just remembers the pointer to the system that will
// be kept valid until deinitialization of the class (that happens upon destruction of the
// CryCharManager instance). Also initializes the console variables
__forceinline void g_InitInterfaces(ISystem* pISystem) 
{
	assert (pISystem);
	g_pISystem				= pISystem;
	g_pIConsole				= pISystem->GetIConsole();
	g_pITimer					= pISystem->GetITimer();
	g_pILog						= pISystem->GetILog();
	g_pIPak						=	pISystem->GetIPak();
	g_pIStreamEngine	=	pISystem->GetStreamEngine();

	//we initialize this pointers in CryCharManager::Update() 
	g_pIRenderer			= NULL;	//pISystem->GetIRenderer();
	g_pIPhysicalWorld	= NULL;	//pISystem->GetIPhysicalWorld();
	g_pI3DEngine			=	NULL;	//pISystem->GetI3DEngine();

	//---------------------------------------------------

#ifdef _DEBUG
	enum {numTests = 2};
	for (int i = 0; i < numTests; ++i)
	{
		CryAnimVars* p = new CryAnimVars();
		delete p;
	}
#endif
	
	g_pCVariables = new CryAnimVars();
}


// deinitializes the Class - actually just NULLs the system pointer and deletes the variables
__forceinline void g_DeleteInterfaces()
{
	delete g_pCVariables;
	g_pCVariables						= NULL;

	g_pISystem				= NULL;
	g_pITimer					= NULL;
	g_pILog						= NULL;
	g_pIConsole				= NULL;
	g_pIPak						= NULL;
	g_pIStreamEngine	= NULL;;

	g_pIRenderer			= NULL;
	g_pIPhysicalWorld	= NULL;
	g_pI3DEngine			=	NULL;
}



__forceinline	CCamera& GetViewCamera() { return g_pISystem->GetViewCamera(); }
__forceinline ISystem* GetISystem() { return g_pISystem; }  //we need this one just for the profiler

__forceinline ISystem* g_GetISystem() { return g_pISystem; }
__forceinline ITimer* g_GetTimer() {return g_pITimer;}
__forceinline ILog* g_GetLog() {return g_pILog;}
__forceinline IConsole* g_GetConsole() { return g_pIConsole; }
__forceinline ICryPak*	g_GetPak() { return g_pIPak; }
__forceinline IStreamEngine* g_GetStreamEngine() { return g_pIStreamEngine;}

__forceinline	IPhysicalWorld* GetPhysicalWorld() { return g_pIPhysicalWorld; }
__forceinline	I3DEngine* Get3DEngine() { return g_pI3DEngine; }
__forceinline	IRenderer* g_GetIRenderer() { return g_pIRenderer; }

__forceinline bool IsProfilerOn() { return g_bProfilerOn; }
__forceinline CryAnimVars*	g_GetCVars() { return g_pCVariables; }





inline void g_LogToFile (const char* szFormat, ...)
{
	char szBuffer[0x800];
	va_list args;
	va_start(args,szFormat);
	_vsnprintf (szBuffer, sizeof(szBuffer), szFormat, args);
	va_end(args);
	g_GetLog()->LogToFile ("%s", szBuffer);
}





#ifdef _DEBUG
// this is an alternate log, that will do nothing in non-debug builds
inline void g_Info (const char* szFormat, ...)
{
	if (!g_GetCVars()->ca_EnableAnimationLog())
		return;

	FILE* f = fopen ("Animation.log", "at");
	if (f)
	{
		va_list arg;
		va_start (arg, szFormat);

		fprintf (f, "%5d ", g_GetIRenderer()->GetFrameID());
		vfprintf (f, szFormat, arg);
		fprintf (f, "\n");

		va_end(arg);
		fclose (f);
	}
}
#else
#define g_Info while(0)
#endif




// collector profilers: collect the total time spent on something
extern double g_dTimeAnimLoadBind;
extern double g_dTimeAnimLoadBindNoCal;
extern double g_dTimeAnimLoadBindWithCal;
extern double g_dTimeAnimLoadBindPreallocate;
extern double g_dTimeGeomLoad;
extern double g_dTimeGeomPostInit;
extern double g_dTimeShaderLoad;
extern double g_dTimeGeomChunkLoad;
extern double g_dTimeGeomChunkLoadFileIO;
extern double g_dTimeGenRenderArrays;
extern double g_dTimeAnimLoadFile;
extern double g_dTimeAnimBindControllers;
extern double g_dTimeTest1;
extern double g_dTimeTest2;
extern double g_dTimeTest3;
extern double g_dTimeTest4;

// the number of animations that were loaded asynchronously
// (one animation can be counted several times if it has been loaded/unloaded)
extern unsigned g_nAsyncAnimCounter;
// this is the sum of all delays between animation load and animation load finish, in frames
extern unsigned g_nAsyncAnimFrameDelays;

extern int g_CpuFlags;
extern double g_SecondsPerCycle;

#define ENABLE_GET_MEMORY_USAGE 1

const float g_fDefaultAnimationScale = 0.01f;


// this is current frame id PLUS OR MINUS a few frames.
// can be used in places where it's really not significant for functionality but speed is a must.
extern int g_nFrameID;

// this is true when the game runs in such a mode that requires all bones be updated every frame
extern bool g_bUpdateBonesAlways;

#ifndef AUTO_PROFILE_SECTION
#pragma message ("Warning: ITimer not included")
#else
#undef AUTO_PROFILE_SECTION
#endif

#define AUTO_PROFILE_SECTION(g_fTimer) CITimerAutoProfiler<double> __section_auto_profiler(g_GetTimer(), g_fTimer)

#define DEFINE_PROFILER_FUNCTION() FUNCTION_PROFILER_FAST(g_GetISystem(), PROFILE_ANIMATION, IsProfilerOn())
#define DEFINE_PROFILER_SECTION(NAME) FRAME_PROFILER_FAST(NAME, g_GetISystem(), PROFILE_ANIMATION, IsProfilerOn())
















/*
inline void g_UpdateLoadingScreen(const char *command,...)
{
	if(command)
	{
		va_list		arglist;
		char		buf[512];
		va_start(arglist, command);
		vsprintf(buf, command, arglist);
		va_end(arglist);	
		g_GetLog()->UpdateLoadingScreen(buf);
	}
	else
		g_GetLog()->UpdateLoadingScreen(0);
}

inline void UpdateLoadingScreenPlus(const char *command,...)
{
	va_list		arglist;
	char		buf[512];
	va_start(arglist, command);
	vsprintf(buf, command, arglist);
	va_end(arglist);	
	g_GetLog()->UpdateLoadingScreenPlus(buf);
}


template <typename T>
inline T g_GetConsoleVariable (const char * szVarName, const char * szFileName, const T tDefaultValue)
{
	return GetConsole()->GetVariable(szVarName, szFileName, tDefaultValue);
}

template <typename T>
inline T g_GetConsoleVariable (const char * szVarName, const string& strFileName, const T szDefaultValue)
{
	return GetConsoleVariable (szVarName, strFileName.c_str(), szDefaultValue);
}*/





#endif // _CryAnimationBase_h_
