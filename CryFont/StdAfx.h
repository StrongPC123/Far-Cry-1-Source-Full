// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__6E7C4805_F57C_4D50_81A7_7745B3776538__INCLUDED_)
#define AFX_STDAFX_H__6E7C4805_F57C_4D50_81A7_7745B3776538__INCLUDED_

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

#pragma warning(disable:4786) // identifier was truncated to '255' characters in the debug information

#include <platform.h>

#ifndef _XBOX
#ifdef WIN32
// Insert your headers here
#undef GetCharWidth
#undef GetCharHeight
#endif
#else
#include <xtl.h>
#endif

#include <stdlib.h>

#define USE_NEWPOOL
#include <CryMemoryManager.h>

// TODO: reference additional headers your program requires here
// Safe memory freeing
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

//STL headers
#include <locale>
#include <vector>
#include <list>
#include <map>	
//#include <xdebug>

#ifdef _DEBUG

#include <crtdbg.h>
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line)
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__)

#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif

#if !defined(LINUX)
#include <Windows.h>
#endif

#ifdef WIN64
#undef GetCharWidth
#undef GetCharHeight
#endif

#include <IFont.h>
#if !defined(LINUX)
#include <assert.h>
#endif


//! Include main interfaces.
#include <IEntitySystem.h>
#include <IProcess.h>
//#include <ITimer.h>
#include <ISystem.h>
#include <ILog.h>
//#include <CryPhysics.h>
#include <IConsole.h>
#include <IRenderer.h>
#include <ICryPak.h>



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__6E7C4805_F57C_4D50_81A7_7745B3776538__INCLUDED_)
