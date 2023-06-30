#include "stdafx.h"
#include "DedicatedServer.h"
#include "CryLibrary.h"
#include <IConsole.h>										// ICVar
#include <ITimer.h>											// CTimeValue

static HMODULE g_hSystemHandle=NULL;
#if !defined(LINUX)
	#define DLL_SYSTEM "CrySystem.dll"
	#define DLL_GAME	 "CryGame.dll"
#else 
	#define DLL_SYSTEM "crysystem.so"
	#define DLL_GAME	 "crygame.so"
#endif   


SSystemInitParams			g_SystemInitParams;											//!< inital statup parameters system
static ISystem *			g_pISystem=NULL;												//!<
ICVar *								g_psvDedicatedMaxRate=NULL;							//!<

//
ISystem *GetISystem()
{
	return g_pISystem;
}


//////////////////////////////////////////////////////////////////////////
// Timur.
// This is FarCry.exe authentication function, this code is not for public release!!
//////////////////////////////////////////////////////////////////////////
void AuthCheckFunction( void *data )
{
	// src and trg can be the same pointer (in place encryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit:  int key[4] = {n1,n2,n3,n4};
	// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) {\
	register unsigned int y=v[0],z=v[1],n=32,sum=0; \
	while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

	// src and trg can be the same pointer (in place decryption)
	// len must be in bytes and must be multiple of 8 byts (64bits).
	// key is 128bit: int key[4] = {n1,n2,n3,n4};
	// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) { \
	register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32; \
	while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

	// Data assumed to be 32 bytes.
	int key1[4] = {389623487,373673863,657846392,378467832};
	TEA_DECODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key1 );
	int key2[4] = {1982697467,3278962783,278963782,287678311};
	TEA_ENCODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key2 );
}



void print( const char *insTxt, ... )
{
	if (!g_pISystem)
	{
		return;
	}

	va_list	ArgList;
	assert(g_pISystem);

	ILog *pLog = g_pISystem->GetILog();		assert(pLog);
	va_start(ArgList,insTxt);

	pLog->LogV(IMiniLog::eAlways, insTxt, ArgList);
	va_end(ArgList);
}

// returns the decimal string representation of the given int
std::string IntToString (int nNumber)
{
	char szNumber[16];
	
	sprintf (szNumber, "%d", nNumber);

	return szNumber;
}

// returns hexadecimal string representation of the given dword
std::string UIntToHexString(unsigned long dwNumber)
{
	char szNumber[24];
	
	sprintf (szNumber, "0x%X", dwNumber);

	return szNumber;
}

 
bool InitDedicatedServer_System( const char *sInCmdLine )
{
#if defined(LINUX)
	g_SystemInitParams.sLogFileName = "log_LinuxSV.txt";
#else
	g_SystemInitParams.sLogFileName = "log_WinSV.txt";
#endif
#ifndef _XBOX
  
	g_hSystemHandle = CryLoadLibrary(DLL_SYSTEM);
 
	if (!g_hSystemHandle)  
	{
#if defined(LINUX)
		printf ("CrySystem.so Loading Failed: %s\n", dlerror());
#else
		MessageBox( NULL,"CrySystem.dll Loading Failed (wrong working directory?):\n","FarCry Error",MB_OK|MB_ICONERROR );
#endif
		return false;
	}

	assert(g_hSystemHandle);
	PFNCREATESYSTEMINTERFACE pfnCreateSystemInterface = 
		(PFNCREATESYSTEMINTERFACE) CryGetProcAddress( g_hSystemHandle,"CreateSystemInterface" );

	// Initialize with instance and window handles.
	g_SystemInitParams.hInstance = NULL;
	g_SystemInitParams.hWnd = NULL;
	g_SystemInitParams.bDedicatedServer=true;
	strcpy( g_SystemInitParams.szSystemCmdLine,sInCmdLine );
	g_SystemInitParams.pCheckFunc = AuthCheckFunction;

	// initialize the system
	g_pISystem = pfnCreateSystemInterface( g_SystemInitParams );

	if (!g_pISystem)
	{
		MessageBox( NULL,"CreateSystemInterface Failed","FarCry Error",MB_OK|MB_ICONERROR );
		return false;
	}
	IConsole *pConsole = g_pISystem->GetIConsole();

	g_psvDedicatedMaxRate = pConsole->CreateVariable("sv_DedicatedMaxRate","80",0,
		"Set the maximum update frequency (per second) for the dedicated server. Higher values may improve the network quality.\n"
		"Lower values free processor resources (better multitasking).\n"
		"Usage: sv_DedicatedMaxRate 20\n"
		"Default value is 80.");

	// don't load lightmaps
	{
		ICVar *pLightmapVar=pConsole->GetCVar("e_light_maps");		assert(pLightmapVar);

		assert((pLightmapVar->GetFlags()&VF_DUMPTODISK)==0);
		assert((pLightmapVar->GetFlags()&VF_SAVEGAME)==0);

		pLightmapVar->Set(0);
	}

#else
		// initialize the system
		g_pISystem = CreateSystemInterface( g_SystemInitParams );
#endif

	// Enable Log verbosity.
	g_pISystem->GetILog()->EnableVerbosity(true);
	return true;
}


bool InitDedicatedServer_Game( const char *sInCmdLine )
{
	/////////////////////////////////////////////////////////////////////
	// INITIAL CONSOLE STATUS IS ACTIVE
	/////////////////////////////////////////////////////////////////////
	g_pISystem->GetIConsole()->ShowConsole(true);

	SGameInitParams ip;
	
	ip.bDedicatedServer=true;
	ip.sGameDLL = DLL_GAME;

#if defined(WIN32) || defined(LINUX)
	strncpy(ip.szGameCmdLine,sInCmdLine,sizeof(ip.szGameCmdLine));
	if (!g_pISystem->CreateGame( ip ))
	{
		MessageBox( NULL,"CreateGame Failed: CryGame.dll","FarCry Error",MB_OK|MB_ICONERROR );
		return false;
	}
#else
	if (!g_pISystem->CreateGame( ip ))
	{
		//Error( "CreateGame Failed" );
		return false;
	}
#endif

	return true;
}




//!
void PrintDedicatedServerStatus()
{
	assert(g_psvDedicatedMaxRate);

	print("FAR CRY dedicated server\n\n");
	print("sv_DedicatedMaxRate=%.2f\n",g_psvDedicatedMaxRate->GetFVal());
}


void DeInitDedicatedServer()
{
	if (g_psvDedicatedMaxRate)
		g_psvDedicatedMaxRate->Release();
	g_psvDedicatedMaxRate = 0;

	SAFE_RELEASE(g_pISystem);

	g_pISystem = NULL;

#ifdef WIN32
		::FreeLibrary(g_hSystemHandle);
#endif;
}

void PrintGoodbyeMessage()
{
	print("------------------------------------------------------ \n\n");
	print("                                       <RETURN>\n");
}


void PrintWelcomeMessage() 
{
	print("--------------------------------------------------------------------------------\n");

	print("To run a dedicated server you should save a server-profile in the game.\n");
	print("With the following commands you can run this profile.\n");
	print("\n");
	print("SProfile_run <profilename>    .. to start the game with the settings/map in the profile\n");
	print("start_server <map>            .. to start a different map (set g_gametype before e.g. ASSAULT)\n");
	print("\n\n");
}
  
#if defined(LINUX) 
void SleepIfNeeded(bool &bFirstTime)
#else 
void SleepIfNeeded()   
#endif
{ 
	ITimer *pTimer=GetISystem()->GetITimer();

	static CTimeValue timPrevTime=pTimer->GetCurrTimePrecise();

	CTimeValue timNewTime=pTimer->GetCurrTimePrecise();
	CTimeValue timPassed=timNewTime-timPrevTime;

	timPrevTime=timNewTime;

	float fStepsPerSecond = g_psvDedicatedMaxRate->GetFVal();

	float used_ms=timPassed.GetMilliSeconds();
	float min_ms=1000.0f/fStepsPerSecond;
	
	int sleep=(int)(min_ms-used_ms);

#if defined(LINUX) 
	if(!bFirstTime)
	{
		if(sleep > 0 && sleep<100)														// sleep might be very big when loading a level (because of resetting the timer)
			Sleep(sleep); // server should not run at 100%
	}
	bFirstTime = false;
#else
	if(sleep > 0 && sleep<100)														// sleep might be very big when loading a level (because of resetting the timer)
		Sleep(sleep); // server should not run at 100%
#endif
}

//-------------------------------------------------------------------------------------------------
const char *Strip(const char *inszText)
{
	static char buf[2048];

	char *in = (char *)inszText;
	char *out = (char *)buf;
	char ch;

	while (*in)
	{
		ch = *in;

		if (ch == '$')
		{
			if (*(in+1))
			{
				in++;

				if ((*in) == '$')
				{
					*out++ = ch;
				}
			}
			else
			{
				*out++ = ch;
			}
		}
		else
		{
			*out++ = ch;
		}

		++in;
	}

	*out = 0;

	return buf;
};
