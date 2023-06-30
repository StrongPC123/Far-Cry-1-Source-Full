// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>

#if !defined(LINUX)
	#include <tchar.h>
#else
	#include <curses.h>									// getch() 
#endif

#if !defined(LINUX)
	#define NOT_USE_CRY_MEMORY_MANAGER
#endif
#include <platform.h>

// TODO: reference additional headers your program requires here
