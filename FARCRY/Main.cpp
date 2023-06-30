//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: Main.cpp
//  Description: Game Entry point
//
//  History:
//  - August 27, 2001: Created by Alberto Demichelis
//  - October 2, 2002: Modified by Timur Davidenko.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <windows.h>
#include <process.h>
#endif

//#define FARCRY_CD_CHECK_RUSSIAN
#define FARCRY_CD_LABEL _T("FARCRY_1")

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
	int key1[4] = {1873613783,235688123,812763783,1745863682};
	TEA_DECODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key1 );
	int key2[4] = {1897178562,734896899,156436554,902793442};
	TEA_ENCODE( (unsigned int*)data,(unsigned int*)data,32,(unsigned int*)key2 );
}


#define NOT_USE_CRY_MEMORY_MANAGER

#include <platform.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include "Cry_Math.h"
#include <Cry_Camera.h>


#include <IRenderer.h>
#include <ILog.h>
#include <ISystem.h>
#include <IGame.h>
#include <IConsole.h>
#include <IInput.h>
#include <IStreamEngine.h>
#include "resource.h"								// IDI_ICON

// _WAT_comments
//#ifdef USE_MEM_POOL
//_DECLARE_POOL("FarCry",1000*1024);
//#endif
//

static ISystem *g_pISystem=NULL;
static bool g_bSystemRelaunch = false;
static char szMasterCDFolder[_MAX_PATH];

#ifdef WIN32
static HMODULE g_hSystemHandle=NULL;
#define DLL_SYSTEM "CrySystem.dll"
#define DLL_GAME	 "CryGame.dll"
#endif

#ifndef PS2
#if !defined(PS2)
bool RunGame(HINSTANCE hInstance,const char *sCmdLine);
#else
bool RunGame(HINSTANCE hInstance);
#endif


#ifdef _XBOX
void main()
{
	RunGame(NULL,"");
}


#ifndef _DEBUG



int _strcmpi(   const char *string1,   const char *string2 )
{
   return _stricmp( string1, string2 );
}


int system( const char *command )
{
  return 0;
}


char * getenv( const char *varname )
{
  return 0;
}


#endif //_DEBUG
#endif // _XBOX



//FNC_CryFree _CryFree = NULL;

#ifndef _XBOX

void SetMasterCDFolder()
{
	char szExeFileName[_MAX_PATH];
	// Get the path of the executable
	GetModuleFileName( GetModuleHandle(NULL), szExeFileName, sizeof(szExeFileName));

	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( szExeFileName, drive, dir, fname, ext );
	_makepath( path_buffer, drive,dir,NULL,NULL );
	strcat( path_buffer,".." );
	SetCurrentDirectory( path_buffer );
	GetCurrentDirectory( sizeof(szMasterCDFolder),szMasterCDFolder );
}

#ifdef FARCRY_CD_CHECK_RUSSIAN
#include <winioctl.h>
#include <tchar.h>
typedef std::basic_string< TCHAR > tstring;
typedef std::vector< TCHAR > tvector;
void CheckFarCryCD( HINSTANCE hInstance )
{
	bool bRet( false );

	DWORD nBufferSize( GetLogicalDriveStrings( 0, 0 ) );
	if( 0 < nBufferSize )
	{
		// get list of all available logical drives
		tvector rawDriveLetters( nBufferSize + 1 );
		GetLogicalDriveStrings( nBufferSize, &rawDriveLetters[ 0 ] );

		// quickly scan all drives
		tvector::size_type i( 0 );
		while( true )
		{
			// check if current drive is cd/dvd drive
			if( DRIVE_CDROM == GetDriveType( &rawDriveLetters[ i ] ) )
			{
				// get volume name
				tvector cdVolumeName( MAX_VOLUME_ID_SIZE + 1 );
				if( FALSE != GetVolumeInformation( &rawDriveLetters[ i ],
					&cdVolumeName[ 0 ], (DWORD) cdVolumeName.size(), 0, 0, 0, 0, 0 ) )
				{
					// check volume name to verify it's Far Cry's game cd/dvd
					tstring cdVolumeLabel( &cdVolumeName[ 0 ] );
					if( cdVolumeLabel == FARCRY_CD_LABEL)
					{
						// found Far Cry's game cd/dvd, copy information and bail out
						//szCDPath = &rawDriveLetters[ i ];
						return;
					}
				}
			}

			// proceed to next drive
			while( 0 != rawDriveLetters[ i ] )
			{
				++i;
			}
			++i; // skip null termination of current drive

			// check if we're out of drive letters
			if( 0 == rawDriveLetters[ i ] )
			{
				// double null termination found, bail out
				break;
			}
		}
	}

	// Not CD/DVD with FARCRY_1 label found. Give to user warning message and bail out.
	char str[1024];
	LoadString( hInstance,IDS_NOCD,str,sizeof(str) );
	MessageBox( NULL,str,_T("CD Check Error"),MB_OK|MB_ICONERROR );
	exit(1);
}
#else
void CheckFarCryCD( HINSTANCE hInstance ) {};
#endif // FARCRY_CD_CHECK_RUSSIAN

///////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
#ifdef _DEBUG
	int tmpDbgFlag;
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);

	// Check heap every
	//_CrtSetBreakAlloc(119065);
#endif


  // [marco] If a previous instance is running, activate
  // the old one and terminate the new one, depending
	// on command line devmode status
  HWND hwndPrev;
	static char szWndClass[] = "CryENGINE";
	bool bDevMode=false;
	bool bRelaunching=false;
	if (lpCmdLine)
	{
		if (strstr(lpCmdLine,"-DEVMODE"))
			bDevMode=true;
		if (strstr(lpCmdLine,"-RELAUNCHING"))
			bRelaunching=true;
	}
	// in devmode we don't care, we allow to run multiple instances
	// for mp debugging
  if (!bDevMode)
  {
		hwndPrev = FindWindow (szWndClass, NULL);
		// not in devmode and we found another window - see if the
		// system is relaunching, in this case is fine 'cos the application
		// will be closed immediately after
		if (hwndPrev && !bRelaunching)
		{
			SetForegroundWindow (hwndPrev);
			return (-1);
		}
	}

	CheckFarCryCD(hInstance);
	SetMasterCDFolder();

#if !defined(PS2)
	RunGame(hInstance,lpCmdLine);
#else
	RunGame(hInstance);
#endif
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Window procedure

	RECT rect;

	switch (msg)
	{
	case WM_MOVE:
					{
						// TODO
						break;
					}

		case WM_DISPLAYCHANGE:
					{
						// TODO
						break;
					}

		case WM_SIZE:
					{
						// Check to see if we are losing our window
						if ((wParam == SIZE_MAXHIDE) || (wParam == SIZE_MINIMIZED))
						{
							// TODO
							break;
						}

						GetClientRect(hWnd, &rect);
						if (g_pISystem && !g_bSystemRelaunch)
							g_pISystem->GetIRenderer()->ChangeViewport(0, 0, LOWORD(lParam), HIWORD(lParam));

						break;
					}

		case WM_ACTIVATEAPP:
			{
				if (!wParam)
				{
					if (g_pISystem && g_pISystem->GetIInput())
					{
						g_pISystem->GetIInput()->ClearKeyState();
						g_pISystem->GetIInput()->SetMouseExclusive(false);
						g_pISystem->GetIInput()->SetKeyboardExclusive(false);
					}
				}
				else
				{
					if (g_pISystem && g_pISystem->GetIInput())
					{
						g_pISystem->GetIInput()->ClearKeyState();
						g_pISystem->GetIInput()->SetMouseExclusive(true);
						g_pISystem->GetIInput()->SetKeyboardExclusive(true);
					}
				}
				break;
			}

		case WM_MOUSEACTIVATE:
					{
						return MA_ACTIVATEANDEAT;
					}

		case WM_ACTIVATE:
					{
						if (wParam == WA_INACTIVE)
						{
							if (g_pISystem && g_pISystem->GetIInput())
							{
								g_pISystem->GetIInput()->ClearKeyState();
								g_pISystem->GetIInput()->SetMouseExclusive(false);
								g_pISystem->GetIInput()->SetKeyboardExclusive(false);

							}
						}
						else if ((wParam == WA_ACTIVE) ||(wParam == WA_CLICKACTIVE))
						{
							if (g_pISystem && g_pISystem->GetIInput())
							{
								g_pISystem->GetIInput()->SetMouseExclusive(true);
								g_pISystem->GetIInput()->SetKeyboardExclusive(true);
							}
						}
						break;
					}

		case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
					{
						return 0;
					}

		case WM_SETFOCUS:
					{
						if (g_pISystem && g_pISystem->GetIInput())
						{
							g_pISystem->GetIInput()->ClearKeyState();
							g_pISystem->GetIInput()->SetMouseExclusive(true);
							g_pISystem->GetIInput()->SetKeyboardExclusive(true);
						}

						break;
					}

		case WM_KILLFOCUS:
					{
						if (g_pISystem && g_pISystem->GetIInput())
						{
							g_pISystem->GetIInput()->ClearKeyState();
							g_pISystem->GetIInput()->SetMouseExclusive(false);
							g_pISystem->GetIInput()->SetKeyboardExclusive(false);
						}
						break;
					}

		case WM_DESTROY: 
					{
						// TODO
						break;
					}
		case WM_HOTKEY:
			return 0;
			break;

		case WM_SYSKEYDOWN:
			{
				if (g_pISystem && g_pISystem->GetIInput())
					g_pISystem->GetIInput()->FeedVirtualKey(wParam,lParam,true);
				break;
			}
		case WM_SYSKEYUP:
			{
				if (g_pISystem && g_pISystem->GetIInput())
					g_pISystem->GetIInput()->FeedVirtualKey(wParam,lParam,false);
				break;
			}
		case WM_KEYDOWN:
			if (g_pISystem && g_pISystem->GetIInput())
					g_pISystem->GetIInput()->FeedVirtualKey(wParam,lParam,true);
			break;
		case WM_KEYUP:
			if (g_pISystem && g_pISystem->GetIInput())
				g_pISystem->GetIInput()->FeedVirtualKey(wParam,lParam,false);
			break;

		case WM_CHAR:
					{
						break;
					}

		case 0x020A: // WM_MOUSEWHEEL
					g_pISystem->GetIInput()->GetIMouse()->SetMouseWheelRotation((short) HIWORD(wParam));
					break;

		case WM_QUIT:
			{
				/*m_pGame->Release();
				m_pGame = NULL;
				*/
				/*
				if (g_pISystem)
				{
					g_pISystem->Quit();
				}
				*/
				break;
			}
		case WM_CLOSE:
			{
				if (g_pISystem)
				{
					g_pISystem->Quit();
				}
				break;
			}
	}

	return (DefWindowProc(hWnd, msg, wParam, lParam));
}

bool RegisterWindow(HINSTANCE hInst)
{
	// Register a window class
 
	WNDCLASS wc;

	wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 4;
	wc.cbWndExtra    = 4;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor       = NULL;
	wc.hbrBackground =(HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "CryENGINE";

	if (!RegisterClass(&wc))
		return false;
	else
		return true;
}
#endif

#else	//PS2

bool RunGamePS2(IGame *hInstance)
{
	SSystemInitParams sip;
	sip.sLogFileName = "log.txt";

	g_pISystem = CreateSystemInterface( sip );
	if (!g_pISystem)
	{
		//Error( "CreateSystemInterface Failed" );
		return false;
	}

	// Enable Log verbosity.
	g_pISystem->GetILog()->EnableVerbosity(true);

	/////////////////////////////////////////////////////////////////////
	// INITIAL CONSOLE STATUS IS ACTIVE
	/////////////////////////////////////////////////////////////////////
	g_pISystem->GetIConsole()->ShowConsole(true);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	SGameInitParams gip;
	if (!g_pISystem->CreateGame( gip ))
	{
		//Error( "CreateGame Failed" );
		return false;
	}

	IGame *pGame = g_pISystem->GetIGame();



	pGame->Run(bRelaunch);

	// Release System and Game.
	g_pISystem->Release();
	g_pISystem = NULL;

	return true;
}

#endif	//PS2


// returns the decimal string representation of the given int
string IntToString (int nNumber)
{
	char szNumber[16];
	//	itoa (nNumber, szNumber, 10);
	sprintf (szNumber, "%d", nNumber);
	return szNumber;
}

// returns hexadecimal string representation of the given dword
string UIntToHexString(DWORD dwNumber)
{
	char szNumber[24];
	sprintf (szNumber, "0x%X", dwNumber);
	return szNumber;
}

string TryFormatWinError(DWORD dwError)
{
#ifdef WIN32
	LPVOID lpMsgBuf;   // pointer to the buffer that will accept the formatted message
	//DWORD dwLastError = OsGetLastError(); // the last user error for which the description should be formatted
	DWORD dwFormattedMsgLen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

	if (!dwFormattedMsgLen)
		// error. return both the user error and the error received during formatting the user error
		return string();
	else
	{// the lpMsgBuf contains allocated by the system call message that is to be returned.
		// we'll copy it into sResult and free it and return sResult
		string sResult = (LPCTSTR) lpMsgBuf;
		LocalFree (lpMsgBuf);
		while (!sResult.empty() && ((unsigned char)sResult[sResult.length()-1]) < 0x20)
			sResult.resize(sResult.length()-1);
		return sResult;
	}
#else
	return "Unknown error";
#endif
}

// returns the string representation (in natural language) of the last error retrieved by GetLastError()
string FormatWinError(DWORD dwError)
{
	string sResult = TryFormatWinError(dwError);

	if (sResult.empty())
		// error. return both the user error and the error received during formatting the user error
		sResult = "Error " + IntToString (GetLastError()) + " while formatting error message";

	return sResult + "\n(" + (dwError & 0x80000000 ? UIntToHexString(dwError):IntToString(dwError)) + ")";
}

#define MAX_CMDLINE_LEN 256
#include <crtdbg.h>
///////////////////////////////////////////////
// Load the game DLL and run it

static void
InvokeExternalConfigTool()
{
#if defined(WIN32) || defined(WIN64)
	try
	{
		// build tmp working directory
		char tmpWorkingDir[ MAX_PATH ];
		GetModuleFileName( 0, tmpWorkingDir, MAX_PATH );
		strlwr( tmpWorkingDir );

		// look for \bin as it should work for either \bin32 and \bin64
		char* pBin( strstr( tmpWorkingDir, "\\bin" ) );
		if( 0 != pBin )
		{
			// trunc tmp working directory path to X:\...\MasterCD
			*pBin = 0;

			// save current working directory
			char curWorkingDir[ MAX_PATH ];
			GetCurrentDirectory( MAX_PATH, curWorkingDir );

			// set temporary working directory and launch external config tool
			SetCurrentDirectory( tmpWorkingDir );
			_spawnl( _P_WAIT, "Bin32\\FarCryConfigurator.exe", "Bin32\\FarCryConfigurator.exe", "/Caller=FarCry", 0 );

			// restore current working directory
			SetCurrentDirectory( curWorkingDir );
		}
	}
	catch( ... )
	{
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
//#define GERMAN_GORE_CHECK

#ifdef GERMAN_GORE_CHECK
#include "IEntitySystem.h"
#endif
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
bool RunGame(HINSTANCE hInstance,const char *sCmdLine)
{
	InvokeExternalConfigTool();

	HWND hWnd=NULL;
	// initialize the system
	bool bRelaunch=false;

	char szLocalCmdLine[MAX_CMDLINE_LEN];
	memset(szLocalCmdLine,0,MAX_CMDLINE_LEN);
	if (sCmdLine)
		strncpy(szLocalCmdLine,sCmdLine,MAX_CMDLINE_LEN);

	do {
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

		SSystemInitParams sip;
		sip.sLogFileName = "log.txt";

		if (szLocalCmdLine[0])
		{
			int nLen=(int)strlen(szLocalCmdLine);
			if (nLen>MAX_CMDLINE_LEN)
				nLen=MAX_CMDLINE_LEN;
			strncpy(sip.szSystemCmdLine,szLocalCmdLine,nLen);
		}

#ifndef _XBOX
		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////
		if (!hWnd && !RegisterWindow(hInstance))
		{
			if (!hWnd && RegisterWindow(hInstance))
			{
				MessageBox(0, "Cannot Register Window\n", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
				return false;
			}
		}

		g_hSystemHandle = LoadLibrary(DLL_SYSTEM);
		if (!g_hSystemHandle)
		{
			DWORD dwLastError = GetLastError();

			MessageBox( NULL,("CrySystem.dll Loading Failed:\n" + TryFormatWinError(dwLastError)).c_str(),"FarCry Error",MB_OK|MB_ICONERROR );
			return false;
		}

		PFNCREATESYSTEMINTERFACE pfnCreateSystemInterface =
			(PFNCREATESYSTEMINTERFACE)::GetProcAddress( g_hSystemHandle,"CreateSystemInterface" );

		// Initialize with instance and window handles.
		sip.hInstance = hInstance;
		sip.hWnd = hWnd;
		sip.pSystem = g_pISystem;
		sip.pCheckFunc = AuthCheckFunction;

		// initialize the system
		g_pISystem = pfnCreateSystemInterface( sip );
		if (!g_pISystem)
		{
			MessageBox( NULL,"CreateSystemInterface Failed","FarCry Error",MB_OK|MB_ICONERROR );
			return false;
		}
#else
		// initialize the system
		g_pISystem = CreateSystemInterface( sip );
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef GERMAN_GORE_CHECK
			string sVar=string("g")+"_"+"g"+"o"+"r"+"e";
			ICVar *pGore=g_pISystem->GetIConsole()->CreateVariable(sVar.c_str(),"1",VF_DUMPTODISK|VF_READONLY);
			pGore->ForceSet("1");
#endif
//////////////////////////////////////////////////////////////////////////

		// Enable Log verbosity.
		g_pISystem->GetILog()->EnableVerbosity(true);

		{
			AutoSuspendTimeQuota suspender (g_pISystem->GetStreamEngine());

			/////////////////////////////////////////////////////////////////////
			// INITIAL CONSOLE STATUS IS INACTIVE
			/////////////////////////////////////////////////////////////////////
			g_pISystem->GetIConsole()->ShowConsole(false);
			g_pISystem->GetIConsole()->SetScrollMax(600/2);


	#ifdef WIN32
			SGameInitParams ip;
			ip.sGameDLL = DLL_GAME;
			if (szLocalCmdLine[0])
				strncpy(ip.szGameCmdLine,szLocalCmdLine,sizeof(ip.szGameCmdLine));

#ifdef GORE_CHECK
			ICVar *pGore=g_pISystem->GetIConsole()->CreateVariable("g_gore","1",VF_DUMPTODISK|VF_READONLY);
			pGore->ForceSet("1");
#endif

			if (!g_pISystem->CreateGame( ip ))
			{
				MessageBox( NULL,"CreateGame Failed: CryGame.dll","FarCry Error",MB_OK|MB_ICONERROR );
				return false;
			}

	#else
			SGameInitParams ip;
			if (!g_pISystem->CreateGame( ip ))
			{
				//Error( "CreateGame Failed" );
				return false;
			}
	#endif

//			g_pISystem->GetIConsole()->ExecuteString(sCmdLine);

		}

		g_bSystemRelaunch = false;

		// set the controls to exclusive mode
		g_pISystem->GetIInput()->ClearKeyState();
		g_pISystem->GetIInput()->SetMouseExclusive(true);
		g_pISystem->GetIInput()->SetKeyboardExclusive(true);

		IGame *pGame = g_pISystem->GetIGame();

//////////////////////////////////////////////////////////////////////////
#ifdef GERMAN_GORE_CHECK
		
		string sDLL=string("C")+"r"+"y"+"E"+"n"+"t"+"i"+"t"+"y"+"S"+"y"+"s"+"t"+"e"+"m"+"."+"d"+"l"+"l";

		HMODULE tDLL= ::LoadLibrary(sDLL.c_str());
		if (!tDLL)
		{
			return false;
			g_pISystem->Release();
			g_pISystem->Release();
			g_pISystem++;
		}
		
		PFNCREATEMAINENTITYSYSTEM pfnCreateEntitySystem;

		string sFunc=string("C")+"r"+"e"+"a"+"t"+"e"+"M"+"a"+"i"+"n"+"E"+"n"+"t"+"i"+"t"+"y"+"S"+"y"+"s"+"t"+"e"+"m";

		pfnCreateEntitySystem = (PFNCREATEMAINENTITYSYSTEM) ::GetProcAddress( tDLL, sFunc.c_str());

		if (!pfnCreateEntitySystem)
		{			
			return false;
			g_pISystem->Release();			
			g_pISystem->Release();
			g_pISystem++;
		}

		::FreeLibrary(tDLL);
		
#endif
//////////////////////////////////////////////////////////////////////////

		pGame->Run(bRelaunch);

		// remove the previous cmdline in case we relaunch
		memset(szLocalCmdLine,0,MAX_CMDLINE_LEN);

		if (g_pISystem)
		{
			const char *szMod=NULL;
			IGameMods *pMods=pGame->GetModsInterface();
			if (pMods)
				szMod=pMods->GetCurrentMod();

			if (szMod!=NULL && (strlen(szMod)>0))
			{
				// the game is relaunching because the MOD changed -
				// add it as system paramter for the next relaunch
				//strncpy(szLocalCmdLine,szMod,MAX_CMDLINE_LEN);
				sprintf(szLocalCmdLine,"-MOD:%s",szMod);
			}

			hWnd = (HWND)g_pISystem->GetIRenderer()->GetHWND();

			g_pISystem->Relaunch(bRelaunch);
			g_bSystemRelaunch = true;

			if (!bRelaunch)
				SAFE_RELEASE(g_pISystem);
		}

#ifdef WIN32

		/*
		// Call to free all allocated memory.
		if (g_bSystemRelaunch)
		{
			typedef void* (*PFN_CRYFREEMEMORYPOOLS)();
			PFN_CRYFREEMEMORYPOOLS pfnCryFreeMemoryPools = (PFN_CRYFREEMEMORYPOOLS)::GetProcAddress( g_hSystemHandle,"CryFreeMemoryPools" );
			if (pfnCryFreeMemoryPools)
				pfnCryFreeMemoryPools();
		}
		*/

		if (!bRelaunch)
			::FreeLibrary(g_hSystemHandle);
		g_hSystemHandle= NULL;

		if (hWnd)
		{
			::DestroyWindow((HWND)hWnd);
			hWnd = NULL;
		}
#endif;

	} while(false);

	if (bRelaunch)
	{
#ifdef WIN32
		// Start a new FarCry process, before exiting this one.


		// use the new command line when restarting

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset( &pi,0,sizeof(pi) );
		memset( &si,0,sizeof(si) );
		si.cb = sizeof(si);
		char szExe[_MAX_PATH];
		GetModuleFileName( NULL,szExe,sizeof(szExe) );

		// [marco] must alloc a new one 'cos could be modified
		// by CreateProcess
		char *szBuf=NULL;
		if (szLocalCmdLine[0])
		{
			szBuf = new char[strlen(szLocalCmdLine) + strlen(szExe) + strlen("-RELAUNCHING") + 4];
			sprintf(szBuf,"%s %s -RELAUNCHING",szExe,szLocalCmdLine);
		}
		else
		{
			szBuf = new char[strlen(szExe) + strlen("-RELAUNCHING") + 4];
			sprintf(szBuf,"%s -RELAUNCHING",szExe);
		}

		CreateProcess(0,szBuf,NULL,NULL,FALSE,NULL,NULL,szMasterCDFolder,&si,&pi );

		// Now terminate this process as fast as possible.
		ExitProcess( 0 );

#endif WIN32
	}

	return true;
}