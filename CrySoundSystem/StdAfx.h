// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_)
#define AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Sound must use MultiThread safe STL.
//////////////////////////////////////////////////////////////////////////
//#define _NOTHREADS
//#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////


#pragma warning( disable:4786 ) //Disable "identifier was truncated to '255' characters" warning.

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
#define NOT_USE_CRY_MEMORY_MANAGER
#include <platform.h>

#include <map>
#include <string>
#include <vector>
#include <ILog.h>
#include <IConsole.h>
#include <ISound.h>
#include <ISystem.h>
#ifdef WIN64
#include <CrySound64.h>
#else
#include <CrySound.h>
#endif
#include <Cry_Math.h>
//#include <vector.h>
#include <set>
#include <algorithm>

#ifdef PS2
inline void __CRYTEKDLL_TRACE(const char *sFormat, ... )
#else
_inline void __cdecl __CRYTEKDLL_TRACE(const char *sFormat, ... )
#endif
{
	va_list vl;
	static char sTraceString[1024];
	
	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);

	strcat(sTraceString, "\n");

	::OutputDebugString(sTraceString);	
}

#define TRACE __CRYTEKDLL_TRACE

#ifdef _DEBUG
#ifdef WIN32
#include <crtdbg.h>
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line)
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__)
#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif //WIN32

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef WIN64
#define ASSERT(x) {assert(x);}
#else

#define ASSERT(x)	{ if (!(x)) { TRACE("Assertion Failed (%s) File: \"%s\" Line: %d\n", #x, __FILE__, __LINE__); _asm { int 3 } } }
#endif // WIN64

#else
#define ASSERT(x) {assert(x);}

#endif //_DEBUG

class CHeapGuardian
{
public: CHeapGuardian() {assert (IsHeapValid());} ~CHeapGuardian() {assert (IsHeapValid());}
};

#ifdef _DEBUG
#define GUARD_HEAP //CHeapGuardian __heap_guardian
#else
#define GUARD_HEAP
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_)
