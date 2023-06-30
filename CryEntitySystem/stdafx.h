// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2E966576_9327_4B66_9CFD_329F604BE709__INCLUDED_)
#define AFX_STDAFX_H__2E966576_9327_4B66_9CFD_329F604BE709__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////

#include "Cry_Math.h"
#include "Cry_XOptimise.h"


#ifndef _XBOX 
#ifdef WIN32
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif

//#define USE_MEM_POOL 

#define USE_NEWPOOL
#include <CryMemoryManager.h>

#include <CrySizer.h>
 
#include <platform.h>

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
extern bool g_bProfilerEnabled;
 

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#ifdef PS2
#ifndef DISABLE_VERIFY
	#define _VERIFY(a)  \
	{						\
		if(!(a))			\
		{					\
		FORCE_EXIT();		\
		DEBUG_BREAK; \		
		}					\
	}							
#else
	#define VERIFY(a) a;
#endif

#else
#ifndef DISABLE_VERIFY
	#define _VERIFY(a)  \
	{						\
		if(!(a))			\
		{					\
		assert (0);		\
		}					\
	}						
#else
	#define VERIFY(a) a;
#endif

#endif //PS2

// Windows defines
#if !defined(LINUX)
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef int                 INT;
typedef unsigned int        UINT;

#ifndef uchar
typedef unsigned char		uchar;
typedef unsigned int		uint;
typedef unsigned short		ushort;
#endif
#endif

#ifndef PS2
#include <memory.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#if !defined(LINUX)
#include <assert.h>
#endif


/*#ifdef USE_MEM_POOL
#undef malloc
#define malloc(_size) _CryMalloc(g_PoolCtx,_size)
#undef free
#define free(_size) _CryFree(g_PoolCtx,_size)
#undef realloc
#define realloc(_ptr,_size) _CryRealloc(g_PoolCtx,_ptr,_size)
#endif*/

#pragma warning (disable : 4768)
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <map>



#include "EntityDesc.h"
#include <IEntitySystem.h>

#ifndef ____TRACE
#define ____TRACE


#ifdef PS2
_inline void ___TRACE(const char *sFormat, ... )
{

	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	cout << sTraceString << "\n";
	
}

#else
_inline void __cdecl ___TRACE(const char *sFormat, ... )
{

	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	::OutputDebugString(sTraceString);
	
}

#endif

#define _TRACE ___TRACE

#endif //____TRACE


#ifdef _DEBUG

//@FIXME this function should not be inline.
_inline void __cdecl __CRYTEKDLL_TRACE(const char *sFormat, ... )
{
  va_list vl;
  static char sTraceString[1024];

  va_start(vl, sFormat);
  vsprintf(sTraceString, sFormat, vl);
  va_end(vl);

  strcat(sTraceString, "\n");

#ifdef WIN32
  ::OutputDebugString(sTraceString);	
#endif

#ifdef GAMECUBE
  OSReport(sTraceString);
#endif

}

#define TRACE __CRYTEKDLL_TRACE

#else
#define TRACE(str) ;
#endif

#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2E966576_9327_4B66_9CFD_329F604BE709__INCLUDED_)