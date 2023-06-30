////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   stdafx.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE)
#define AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE

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

#include <platform.h>

#ifdef GAMECUBE
#include "GCDefines.h"
#endif

// Insert your headers here
#ifndef GAMECUBE
#ifndef _XBOX

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif

#else
#include <xtl.h>
#endif
#endif

#include <stdlib.h>
#include <stdio.h>

// enable memory pool usage
#ifndef GAMECUBE
#define USE_NEWPOOL
#include <CryMemoryManager.h>
#endif

#ifndef uchar
typedef unsigned char		uchar;
typedef unsigned int		uint;
typedef unsigned short	ushort;
#endif

#if !defined(LINUX)
	#include <assert.h>
#endif
#include <vector>
#include <list>
#include <map>	
#include <set>
#include <algorithm>


#if defined(PS2) || defined(GAMECUBE)
  using std::min;
  using std::max;
#endif

#ifndef __forceinline
#define __forceinline inline
#endif

#if !defined(min) && !defined(LINUX)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define ZeroStruct( t ) { memset( &t,0,sizeof(t) ); }

#define MAX_PATH_LENGTH 512

#ifndef stricmp
inline int __cdecl stricmp(const char *dst, const char *src)
{
  int f,l;
  do
  {
    if ( ((f=(unsigned char)(*(dst++))) >= 'A') && (f<='Z'))
      f -= ('A' - 'a');
    
    if ( ((l=(unsigned char)(*(src++))) >= 'A') && (l<='Z'))
      l -= ('A' - 'a');
  } while ( f && (f == l) );

  return(f - l);
}
#endif

#ifndef strnicmp
inline int __cdecl strnicmp (const char * first, const char * last, size_t count)
{
  int f,l;
  if ( count )
  {
    do
    {
      if ( ((f=(unsigned char)(*(first++))) >= 'A') && (f<='Z') )
        f -= 'A' - 'a';

      if ( ((l=(unsigned char)(*(last++))) >= 'A') && (l<='Z'))
        l -= 'A' - 'a';
    } while ( --count && f && (f == l) );

    return( f - l );
  }

  return 0;
}
#endif


#include <ITimer.h>
#include <IProcess.h>
#include "Cry_Math.h"
#include "Cry_XOptimise.h"
#include <Cry_Camera.h>
#include <ILog.h>
#include <ISystem.h>
#include <IConsole.h>
#include <IPhysics.h>
#include <IRenderer.h>
#include <IEntityRenderState.h>
#include <I3DEngine.h>
#include <IGame.h>
#include <icryanimation.h>
#include <icrypak.h>
#include <CryFile.h>

class  IPhysicalWorld;
struct IEntityRender;

#if defined(WIN32) && defined(_DEBUG)

#include <crtdbg.h>
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line)
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__)

// memman
#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif	// defined(WIN32) && defined(_DEBUG)


//#include <StlDbgAlloc.h>

#include "TArrays.h"

#include "Cry_Math.h"
#include "CryHeaders.h"

#include "Cry3DEngineBase.h"
#include "file.h"
#include <float.h>

#include "TArrays.h"
#include "list2.h"
#include "terrain.h"
#include "cvars.h"
#include "crysizer.h"
#include "StlUtils.h"

inline float L1Distance2D(const Vec3 &v0, const Vec3 &v1)	{	return max(Ffabs(v0.x-v1.x),Ffabs(v0.y-v1.y));	}	

inline float GetDist2D(float x1, float y1, float x2, float y2)
{
  float xm = (x1-x2);
  float ym = (y1-y2);
  return cry_sqrtf(xm*xm + ym*ym);
}

#if !defined(LINUX)	//than it does already exist
inline int vsnprintf(char * buf, int size, const char * format, va_list & args)
{
	int res = _vsnprintf(buf, size, format, args);
	assert(res>=0 && res<size); // just to know if there was problems in past
	buf[size-1]=0;
	return res;
}
#endif

inline int snprintf(char * buf, int size, const char * format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	int res = vsnprintf(buf, size, format, arglist);
	va_end(arglist);	
	return res;
}

#endif // !defined(AFX_STDAFX_H__8B93AD4E_EE86_4127_9BED_37AC6D0F978B__INCLUDED_3DENGINE)
