// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


#define _WIN32_WINNT 0x0500		// min Win2000 for GetConsoleWindow()     Baustelle

// Windows Header Files:
#include <windows.h>



#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

#include <vector>


extern HMODULE g_hInst;



#ifndef ReleasePpo
#define ReleasePpo(ppo) \
	if (*(ppo) != NULL) \
		{ \
		(*(ppo))->Release(); \
		*(ppo) = NULL; \
		} \
		else (VOID)0
#endif



// TODO: reference additional headers your program requires here
